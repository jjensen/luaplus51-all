#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <assert.h>
#include <ctype.h>
#if !defined(_WIN32)
#include <sys/select.h>
#endif /* _WIN32 */
#ifdef  LINUX
#include <sys/types.h>
#include <sys/socket.h>
#include <tcpd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
typedef int SOCKET;
#define INVALID_SOCKET				-1
#endif

#if defined(_WIN32)
#include <winsock.h>
#include <limits.h>
#define close	closesocket
#else
#include <unistd.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <unistd.h>

#define SOCKET					int
#define INVALID_SOCKET			-1
#endif
#include "http.h"
#include "strutl.h"
#include "global.h"
#include "digcalc.h"
#include "date_decode.h"

#define HTTP_READ_BUFFER_SIZE (16*1024)

struct http_connection {
	SOCKET socketd;
	int status;
	char *host;
	struct sockaddr_in address;
	char *actualHost;
	int resolved_address;
	int persistent;
	int lazy;
	HTTP_AUTH_INFO *auth_info;
	char read_buffer[HTTP_READ_BUFFER_SIZE];
	int read_count;
	int read_index;
	int __http_exec_error;
	char __http_exec_error_msg[256];
	int (*connect_callback)(void *);
	void *connect_userData;
};

#if defined(_WIN32)
#define SOCKET_EWOULDBLOCK			WSAEWOULDBLOCK
#define socket_errno				WSAGetLastError()
#else
#define SOCKET_EWOULDBLOCK			EWOULDBLOCK
#define socket_errno				errno
#endif

/* socket_waitfd, socket_send, socket_recv, and socket_setnonblocking all borrowed from LuaSocket. */

/* IO error codes */
enum {
    IO_DONE = 0,        /* operation completed successfully */
    IO_TIMEOUT = -1,    /* operation timed out */
    IO_CLOSED = -2,     /* the connection has been closed */
	IO_UNKNOWN = -3
};

/*-------------------------------------------------------------------------*\
* Wait for readable/writable/connected socket with timeout
\*-------------------------------------------------------------------------*/
#if defined(_WIN32)

#define WAITFD_R        1
#define WAITFD_W        2
#define WAITFD_E        4
#define WAITFD_C        (WAITFD_E|WAITFD_W)

int socket_waitfd(SOCKET socket, int sw) {
    int ret;
    fd_set rfds, wfds, efds, *rp = NULL, *wp = NULL, *ep = NULL;
    if (sw & WAITFD_R) {
        FD_ZERO(&rfds);
		FD_SET(socket, &rfds);
        rp = &rfds;
    }
    if (sw & WAITFD_W) { FD_ZERO(&wfds); FD_SET(socket, &wfds); wp = &wfds; }
    if (sw & WAITFD_C) { FD_ZERO(&efds); FD_SET(socket, &efds); ep = &efds; }
    ret = select(0, rp, wp, ep, NULL);
    if (ret == -1) return socket_errno;
    if (ret == 0) return IO_TIMEOUT;
    if (sw == WAITFD_C && FD_ISSET(socket, &efds)) return IO_CLOSED;
    return IO_DONE;
}


int socket_send(SOCKET socket, const char *data, size_t count, int flags)
{
    int err;
    /* loop until we send something or we give up on error */
    for ( ;; ) {
        /* try to send something */
		int put = send(socket, data, (int) count, 0);
        /* if we sent something, we are done */
        if (put > 0) {
			return put;
        }
        /* deal with failure */
        err = socket_errno;
        /* we can only proceed if there was no serious error */
        if (err != SOCKET_EWOULDBLOCK) return -1;
        /* avoid busy wait */
        if ((err = socket_waitfd(socket, WAITFD_W)) != IO_DONE) return -1;
    }
    /* can't reach here */
    return IO_UNKNOWN;
}


int socket_recv(SOCKET socket, char *data, size_t count, int flags) {
    int err;
    for ( ;; ) {
        int taken = recv(socket, data, (int) count, 0);
        if (taken > 0) {
			return taken;
        }
        if (taken == 0) return IO_CLOSED;
        err = socket_errno;
        if (err != SOCKET_EWOULDBLOCK) return -1;
        if ((err = socket_waitfd(socket, WAITFD_R)) != IO_DONE) return -1;
    }
    return IO_UNKNOWN;
}


/*-------------------------------------------------------------------------*\
 * Put socket into non-blocking mode
 \*-------------------------------------------------------------------------*/
void socket_setnonblocking(SOCKET socket) {
	u_long nonBlocking = 1;
	ioctlsocket(socket, FIONBIO, &nonBlocking);
}

#else

#define WAITFD_R        1
#define WAITFD_W        2
#define WAITFD_C        (WAITFD_R|WAITFD_W)

int socket_waitfd(SOCKET socket, int sw) {
    int ret;
    fd_set rfds, wfds, *rp, *wp;
    struct timeval tv, *tp;
    double t;
    do {
        /* must set bits within loop, because select may have modifed them */
        rp = wp = NULL;
        if (sw & WAITFD_R) { FD_ZERO(&rfds); FD_SET(socket, &rfds); rp = &rfds; }
        if (sw & WAITFD_W) { FD_ZERO(&wfds); FD_SET(socket, &wfds); wp = &wfds; }
        ret = select(socket+1, rp, wp, NULL, NULL);
    } while (ret == -1 && errno == EINTR);
    if (ret == -1) return errno;
    if (ret == 0) return IO_TIMEOUT;
    if (sw == WAITFD_C && FD_ISSET(socket, &rfds)) return IO_CLOSED;
    return IO_DONE;
}


int socket_send(SOCKET socket, const char *data, size_t count, int flags)
{
    int err;
    /* loop until we send something or we give up on error */
    for ( ;; ) {
        long put = (long) send(socket, data, count, 0);
        /* if we sent anything, we are done */
        if (put > 0) {
            return put;
        }
        err = errno;
        /* send can't really return 0, but EPIPE means the connection was
         closed */
        if (put == 0 || err == EPIPE) return -1;
        /* we call was interrupted, just try again */
        if (err == EINTR) continue;
        /* if failed fatal reason, report error */
        if (err != EAGAIN) return -1;
        /* wait until we can send something or we timeout */
        if ((err = socket_waitfd(socket, WAITFD_W)) != IO_DONE) return -1;
    }
    /* can't reach here */
    return IO_UNKNOWN;
}


int socket_recv(SOCKET socket, char *data, size_t count, int flags) {
    int err;
    for ( ;; ) {
        long taken = (long) recv(socket, data, count, 0);
        if (taken > 0) {
            return taken;
        }
        err = errno;
        if (taken == 0) return -1;
        if (err == EINTR) continue;
        if (err != EAGAIN) return -1;
        if ((err = socket_waitfd(socket, WAITFD_R)) != IO_DONE) return -1;
    }
    return IO_UNKNOWN;
}


void socket_setnonblocking(SOCKET socket) {
	fcntl(socket, F_SETFL, O_NONBLOCK);
}

#endif

static void *http_default_allocator(void *ud, void *ptr, size_t nsize) {
	(void)ud;
	if (nsize == 0) {
		free(ptr);
		return NULL;
	}
	else
		return realloc(ptr, nsize);
}

http_allocator _http_allocator = http_default_allocator;
void* _http_allocator_user_data;

void http_set_allocator(http_allocator allocator, void* userdata)
{
	_http_allocator = allocator;
	if (!_http_allocator)
		_http_allocator = http_default_allocator;
	_http_allocator_user_data = userdata;
}

const char *http_method[9] = { "GET", "PUT", "POST", "LOCK", "UNLOCK", "PROPFIND", "PROPPATCH", "MKCOL", "DELETE" };

void
http_append_auth_request_parameter(HTTP_AUTH_INFO *info, HTTP_AUTH_PARAMETER *parameter)
{
	if(info->first_parameter == NULL)
	{
		info->first_parameter = parameter;
	}
	else
	{
		info->last_parameter->next_parameter = parameter;
	}
	parameter->prev_parameter = info->last_parameter;
	info->last_parameter = parameter;
}

int
http_add_auth_parameter(HTTP_AUTH_INFO *info, const char *name, const char *value)
{
	HTTP_AUTH_PARAMETER *new_parameter = NULL;
	if(info == NULL || name == NULL || value == NULL)
	{
		return HT_INVALID_ARGUMENT;
	}
	new_parameter = (HTTP_AUTH_PARAMETER *) _http_allocator(_http_allocator_user_data, 0, sizeof(HTTP_AUTH_PARAMETER));
	if(new_parameter == NULL)
	{
		return HT_MEMORY_ERROR;
	}
	memset(new_parameter, 0, sizeof(HTTP_AUTH_PARAMETER));
	new_parameter->name = wd_strdup(name);
	new_parameter->value = wd_strdup(value);
	if(new_parameter->name == NULL || new_parameter->value == NULL)
	{
		return HT_MEMORY_ERROR;
	}
	http_append_auth_request_parameter(info, new_parameter);
	return HT_OK;
}

int http_reconnect(HTTP_CONNECTION *connection);

static int
	http_connect_helper(HTTP_CONNECTION **connection, const char *host, short port, const char *username, const char *password, int lazy)
{
	HTTP_CONNECTION *new_connection = NULL;
	if(connection == NULL)
	{
		return HT_INVALID_ARGUMENT;
	}
	*connection = NULL;
	if(connection == NULL || host == NULL)
	{
		return HT_INVALID_ARGUMENT;
	}
	new_connection = (HTTP_CONNECTION *) _http_allocator(_http_allocator_user_data, 0, sizeof(HTTP_CONNECTION));
	memset(new_connection, 0, sizeof(HTTP_CONNECTION));
	new_connection->read_count = new_connection->read_index = 0;
	new_connection->address.sin_family = AF_INET;
	new_connection->address.sin_port = htons(port);
	if(username != NULL && password != NULL)
	{
		new_connection->auth_info = (HTTP_AUTH_INFO *) _http_allocator(_http_allocator_user_data, 0, sizeof(HTTP_AUTH_INFO));
		if(new_connection->auth_info == NULL)
		{
			http_disconnect(&new_connection);
			return HT_MEMORY_ERROR;
		}
		memset(new_connection->auth_info, 0, sizeof(HTTP_AUTH_INFO));
		http_add_auth_parameter(new_connection->auth_info, "username", username);
		http_add_auth_parameter(new_connection->auth_info, "password", password);
	}
	new_connection->host = wd_strdup(host);
	if(new_connection->host == NULL)
	{
		http_disconnect(&new_connection);
		return HT_MEMORY_ERROR;
	}
	new_connection->socketd = INVALID_SOCKET;
	if (!lazy)
	{
		new_connection->persistent = 1;
		if (http_reconnect(new_connection) != HT_OK)
		{
			http_disconnect(&new_connection);
			return HT_NETWORK_ERROR;
		}
		new_connection->persistent = 0;
	}
	new_connection->lazy = lazy;
	new_connection->persistent = HT_TRUE;
	new_connection->status = HT_OK;
	*connection = new_connection;
	return HT_OK;
}

int
	http_connect(HTTP_CONNECTION **connection, const char *host, short port, const char *username, const char *password)
{
	return http_connect_helper(connection, host, port, username, password, 0);
}

int
	http_connect_lazy(HTTP_CONNECTION **connection, const char *host, short port, const char *username, const char *password)
{
	return http_connect_helper(connection, host, port, username, password, 1);
}

int
http_check_socket(HTTP_CONNECTION *connection)
{
	fd_set socket_set;
	struct timeval tv = { 0, 0 };
	FD_ZERO(&socket_set);
	FD_SET(connection->socketd, &socket_set);
	if(select(0, &socket_set, NULL, NULL, &tv) == 1)
	{
		return HT_NETWORK_ERROR;
	}
	return HT_OK;
}

int
http_reconnect(HTTP_CONNECTION *connection)
{
	unsigned int ipaddr = 0;
	if(connection == NULL)
	{
		return HT_INVALID_ARGUMENT;
	}
	if(connection->socketd != INVALID_SOCKET && connection->status == HT_OK)
	{
		if(http_check_socket(connection) == HT_OK)
		{
			return HT_OK;
		}
	}
	if(!connection->persistent)
	{
		return HT_ILLEGAL_OPERATION;
	}
	if(connection->socketd != INVALID_SOCKET)
	{
		close(connection->socketd);
	}
	connection->socketd = socket(AF_INET, SOCK_STREAM, 0);
	if(connection->socketd == INVALID_SOCKET)
	{
		return HT_RESOURCE_UNAVAILABLE;
	}
	{
		int rcvsize = 128 * 1024;
		setsockopt(connection->socketd, SOL_SOCKET, SO_RCVBUF, (char *)&rcvsize, (int)sizeof(rcvsize));
	}
	{
		int flag = 1;
		setsockopt(connection->socketd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
	}
	while (!connection->resolved_address)
	{
		if((ipaddr = inet_addr(connection->host)) != INADDR_NONE)
		{
			memcpy(&connection->address.sin_addr, &ipaddr, sizeof(struct in_addr));
		}
		else
		{
			struct hostent *hostinfo = (struct hostent *) gethostbyname(connection->host);
			if(hostinfo == NULL)
			{
				if (connection->lazy)
				{
					if (connection->connect_callback)
						if (connection->connect_callback(connection->connect_userData) == 0)
							return HT_HOST_UNAVAILABLE;
					continue;
				}
				return HT_HOST_UNAVAILABLE;
			}
			memcpy(&connection->address.sin_addr, hostinfo->h_addr, 4);
		}
		connection->resolved_address = 1;
	}
	if (connection->lazy)
	{
		while (connect(connection->socketd, (struct sockaddr *) &connection->address, sizeof(struct sockaddr_in)) != 0)
		{
			if (connection->connect_callback)
				if (connection->connect_callback(connection->connect_userData) == 0)
					return HT_HOST_UNAVAILABLE;
#if defined(_WIN32)
			Sleep(100);
#endif // _WIN32
		}
	}
	else
	{
		if(connect(connection->socketd, (struct sockaddr *) &connection->address, sizeof(struct sockaddr_in)) != 0)
		{
			close(connection->socketd);
			return HT_NETWORK_ERROR;
		}
	}
	socket_setnonblocking(connection->socketd);
	connection->status = HT_OK;
	return HT_OK;
}

int
http_request_reconnection(HTTP_CONNECTION *connection)
{
	if(!connection->persistent)
	{
		return HT_ILLEGAL_OPERATION;
	}
	close(connection->socketd);
	connection->socketd = INVALID_SOCKET;
	return HT_OK;
}

void http_destroy_auth_parameter(HTTP_AUTH_PARAMETER **parameter)
{
	if(parameter != NULL && *parameter != NULL)
	{
		_http_allocator(_http_allocator_user_data, (*parameter)->name, 0);
		_http_allocator(_http_allocator_user_data, (*parameter)->value, 0);
		_http_allocator(_http_allocator_user_data, *parameter, 0);
	}
}

void http_destroy_auth_info(HTTP_AUTH_INFO **auth_info)
{
	HTTP_AUTH_PARAMETER *parameter_cursor = NULL, *next_parameter = NULL;
	if(auth_info != NULL && *auth_info != NULL)
	{
		for(parameter_cursor = (*auth_info)->first_parameter; parameter_cursor != NULL; parameter_cursor = next_parameter)
		{
			next_parameter = parameter_cursor->next_parameter;
			http_destroy_auth_parameter(&parameter_cursor);
		}
		_http_allocator(_http_allocator_user_data, (*auth_info)->method, 0);
		_http_allocator(_http_allocator_user_data, *auth_info, 0);
		auth_info = NULL;
	}
}

int
http_disconnect(HTTP_CONNECTION **connection)
{
	if(connection == NULL || *connection == NULL)
	{
		return HT_INVALID_ARGUMENT;
	}
	close((*connection)->socketd);
	_http_allocator(_http_allocator_user_data, (*connection)->host, 0);
	http_destroy_auth_info(&(*connection)->auth_info);
	_http_allocator(_http_allocator_user_data, *connection, 0);
	*connection = NULL;
	return HT_OK;
}

const char *http_hoststring(HTTP_CONNECTION *connection)
{
	return connection->host;
}

void
http_collect_strings(HTTP_MEMORY_STORAGE *storage, const char *first_string, ...)
{
	va_list marker;
	const char *string = NULL;
	int length, error = HT_OK;
	va_start(marker, first_string);
	string = first_string;
	while(string != NULL)
	{
		if(string != NULL)
		{
			length = strlen(string);
			http_storage_write(storage, string, length);
		}
		string = va_arg(marker, const char *);
	}
	va_end(marker);
}

int
http_send_strings(HTTP_CONNECTION *connection, const char *first_string, ...)
{
	va_list marker;
	const char *string = NULL;
	int length, error = HT_OK;
	if(connection->status == HT_OK)
	{
		va_start(marker, first_string);
		string = first_string;
		while(string != NULL)
		{
			if(string != NULL)
			{
				length = strlen(string);
				if(socket_send(connection->socketd, string, length, 0) != length)
				{
					error = HT_NETWORK_ERROR;
					break;
				}
			}
			string = va_arg(marker, const char *);
		}
		va_end(marker);
		connection->status = error;
	}
	return connection->status;
}

int
http_send_storage(HTTP_CONNECTION *connection, HTTP_STORAGE *storage)
{
	char read_buffer[HTTP_READ_BUFFER_SIZE];
	int read_count = 0, network_error = HT_OK, io_error;
	if(connection->status == HT_OK)
	{
		http_storage_seek(storage, 0);
		while((io_error = http_storage_read(storage, read_buffer, HTTP_READ_BUFFER_SIZE, &read_count)) == HT_OK && read_count != 0 && network_error == HT_OK)
		{
			if(socket_send(connection->socketd, read_buffer, read_count, 0) != read_count)
			{
				network_error = HT_NETWORK_ERROR;
			}
		}
		connection->status = network_error | io_error;
	}
	return connection->status;
};

void http_destroy_header_field(HTTP_HEADER_FIELD **field)
{
	if(field != NULL && *field != NULL)
	{
		_http_allocator(_http_allocator_user_data, (*field)->name, 0);
		_http_allocator(_http_allocator_user_data, (*field)->value, 0);
		_http_allocator(_http_allocator_user_data, *field, 0);
	}
}

void http_destroy_request(HTTP_REQUEST **request)
{
	HTTP_HEADER_FIELD *field_cursor = NULL, *next_field = NULL;
	if(request != NULL && *request != NULL)
	{
		_http_allocator(_http_allocator_user_data, (*request)->resource, 0);
		for(field_cursor = (*request)->first_header_field; field_cursor != NULL; field_cursor = next_field)
		{
			next_field = field_cursor->next_field;
			http_destroy_header_field(&field_cursor);
		}
		http_storage_destroy(&(*request)->content);
		_http_allocator(_http_allocator_user_data, *request, 0);
		*request = NULL;
	}
}

int
http_create_request(HTTP_REQUEST **request, int method, const char *resource)
{
	HTTP_REQUEST *new_request = NULL;
	if(request == NULL || resource == NULL || method < HTTP_FIRST_METHOD || method > HTTP_LAST_METHOD)
	{
		return HT_INVALID_ARGUMENT;
	}
	new_request = (HTTP_REQUEST *) _http_allocator(_http_allocator_user_data, 0, sizeof(HTTP_REQUEST));
	if(new_request == NULL)
	{
		return HT_MEMORY_ERROR;
	}
	memset(new_request, 0, sizeof(HTTP_REQUEST));
	new_request->method = method;
	new_request->resource = wd_strdup_url_encoded(resource);
	if(new_request->resource == NULL)
	{
		http_destroy_request(&new_request);
		return HT_MEMORY_ERROR;
	}
	*request = new_request;
	return HT_OK;
}

int
http_add_header_field(HTTP_REQUEST *request, const char *field_name, const char *field_value)
{
	HTTP_HEADER_FIELD *new_header_field = NULL;
	if(request == NULL || field_name == NULL || field_value == NULL)
	{
		return HT_INVALID_ARGUMENT;
	}
	new_header_field = (HTTP_HEADER_FIELD *) _http_allocator(_http_allocator_user_data, 0, sizeof(HTTP_HEADER_FIELD));
	if(new_header_field == NULL)
	{
		return HT_MEMORY_ERROR;
	}
	memset(new_header_field, 0, sizeof(HTTP_HEADER_FIELD));
	new_header_field->name = wd_strdup(field_name);
	new_header_field->value = wd_strdup(field_value);
	if(new_header_field->name == NULL || new_header_field->value == NULL)
	{
		http_destroy_header_field(&new_header_field);
		return HT_MEMORY_ERROR;
	}
	if(request->first_header_field == NULL)
	{
		request->first_header_field = new_header_field;
	}
	else
	{
		request->last_header_field->next_field = new_header_field;
	}
	new_header_field->prev_field = request->last_header_field;
	request->last_header_field = new_header_field;
	return HT_OK;
}

int
http_add_header_field_number(HTTP_REQUEST *request, const char *field_name, int field_value)
{
	char number_buffer[32];
	if(field_value == HT_INFINITY)
	{
		return http_add_header_field(request, field_name, "infinity");
	}
	else
	{
		sprintf(number_buffer, "%d", field_value);
		return http_add_header_field(request, field_name, number_buffer);
	}
}

const char *
http_find_auth_parameter(HTTP_AUTH_INFO *info, const char *parameter_name, const char *default_value)
{
	HTTP_AUTH_PARAMETER *parameter_cursor = NULL;
	if(info == NULL || parameter_name == NULL)
	{
		return default_value;
	}
	for(parameter_cursor = info->first_parameter; parameter_cursor != NULL; parameter_cursor = parameter_cursor->next_parameter)
	{
		if(strcasecmp(parameter_cursor->name, parameter_name) == 0)
		{
			return parameter_cursor->value;
		}
	}
	return default_value;
}

int
http_collect_authorization_header_field(HTTP_MEMORY_STORAGE *storage, HTTP_CONNECTION *connection, HTTP_REQUEST *request)
{
	char *credentials = NULL, *user_pass = NULL, nonce_count[9], cnonce[9];
	const char *username = NULL, *password = NULL, *realm = NULL;
	const char *nonce = NULL, *opaque = NULL, *algorithm = NULL;
	const char *qop = NULL, *message_qop = "";
	HASHHEX HA1;
	HASHHEX HEntity = "";
	HASHHEX response_digest;
	if(connection == NULL || request == NULL)
	{
		return HT_INVALID_ARGUMENT;
	}
	if(connection->auth_info == NULL || connection->auth_info->method == NULL)
	{
		return HT_SERVICE_UNAVAILABLE;
	}
	if(strcasecmp(connection->auth_info->method, "basic") == 0)
	{
		connection->auth_info->count++;
		username = http_find_auth_parameter(connection->auth_info, "username", NULL);
		password = http_find_auth_parameter(connection->auth_info, "password", NULL);
		if(username == NULL || password == NULL)
		{
			return HT_RESOURCE_UNAVAILABLE;
		}
		user_pass = (char *) _http_allocator(_http_allocator_user_data, 0, (strlen(username) + 1 + strlen(password) + 1) * sizeof(char));
		strcpy(user_pass, username);
		strcat(user_pass, ":");
		strcat(user_pass, password);
		credentials = wd_strdup_base64(user_pass);
		http_collect_strings(storage, "Authorization: Basic ", credentials, "\r\n", NULL);
		_http_allocator(_http_allocator_user_data, credentials, 0);
		_http_allocator(_http_allocator_user_data, user_pass, 0);
		return HT_OK;
	}
	else if(strcasecmp(connection->auth_info->method, "digest") == 0)
	{
		username = http_find_auth_parameter(connection->auth_info, "username", NULL);
		password = http_find_auth_parameter(connection->auth_info, "password", NULL);
		realm = http_find_auth_parameter(connection->auth_info, "realm", "");
		nonce = http_find_auth_parameter(connection->auth_info, "nonce", "");
		opaque = http_find_auth_parameter(connection->auth_info, "opaque", "");
		qop = http_find_auth_parameter(connection->auth_info, "qop", NULL);
		algorithm  = http_find_auth_parameter(connection->auth_info, "algorithm", "MD5");
		if(username == NULL || password == NULL)
		{
			return HT_RESOURCE_UNAVAILABLE;
		}
		if(strcasecmp(algorithm, "MD5") != 0)
		{
			return HT_SERVICE_UNAVAILABLE;
		}
		if(qop != NULL)
		{
			message_qop = "auth";
			sprintf(nonce_count, "%08X", connection->auth_info->count);
			sprintf(cnonce, "%08x", (unsigned int)time(NULL));
		}
		DigestCalcHA1(algorithm, username, realm, password, nonce, cnonce, HA1);
		DigestCalcResponse(HA1, nonce, nonce_count, cnonce, message_qop, http_method[request->method], request->resource, HEntity, response_digest);
		if(message_qop != NULL)
		{
			http_collect_strings(storage, "Authorization: Digest username=\"", username, "\", realm=\"", realm, "\", nonce=\"", nonce, "\", uri=\"", request->resource, "\", qop=\"", message_qop, "\", nc=", nonce_count, ", cnonce=\"", cnonce, "\", response=\"", response_digest, "\", opaque=\"", opaque, "\r\n", NULL);
		}
		else
		{
			http_collect_strings(storage, "Authorization: Digest username=\"", username, "\", realm=\"", realm, "\", nonce=\"", nonce, "\", uri=\"", request->resource, "\", response=\"", response_digest, "\", opaque=\"", opaque, "\r\n", NULL);
		}
		return HT_OK;
	}
	return HT_SERVICE_UNAVAILABLE;
}

int
http_send_request(HTTP_CONNECTION *connection, HTTP_REQUEST *request)
{
	const char *version = "HTTP/1.1";
	char size_buffer[32] = "";
	int read_count = 0, size = 0, error = HT_OK;
	HTTP_HEADER_FIELD *field_cursor = NULL;
	HTTP_MEMORY_STORAGE* storage;
	http_reconnect(connection);
	if(connection->status != HT_OK)
	{
		return connection->status;
	}

	http_create_memory_storage((HTTP_MEMORY_STORAGE**)&storage, NULL, 0);

	http_collect_strings(storage, http_method[request->method], " ", NULL);
	if (connection->actualHost)
	{
		http_collect_strings(storage, "http://", connection->actualHost, NULL);
	}
	http_collect_strings(storage, request->resource, " ", version, "\r\n", NULL);
	for(field_cursor = request->first_header_field; field_cursor != NULL; field_cursor = field_cursor->next_field)
	{
		http_collect_strings(storage, field_cursor->name, ": ", field_cursor->value, "\r\n", NULL);
	}
	if(connection->host != NULL)
	{
		http_collect_strings(storage, "Host: ", connection->host, "\r\n", NULL);
	}
	if(connection->auth_info != NULL)
	{
		http_collect_authorization_header_field(storage, connection, request);
	}
	if(connection->persistent)
	{
		http_collect_strings(storage, "Connection: Keep-Alive\r\n", NULL);
	}
	else
	{
		http_collect_strings(storage, "Connection: Close\r\n", NULL);
	}
	if(request->content != NULL)
	{
		http_storage_getsize(request->content, &size);
		sprintf(size_buffer, "%d", size);
		http_collect_strings(storage, "Content-Length: ", size_buffer, "\r\n", NULL);
	}
	http_collect_strings(storage, "\r\n", NULL);
	if(request->content != NULL && request->method == HTTP_PROPFIND)
	{
		char read_buffer[HTTP_READ_BUFFER_SIZE];
		int read_count = 0;
		http_storage_seek(request->content, 0);
		while(http_storage_read(request->content, read_buffer, HTTP_READ_BUFFER_SIZE, &read_count) == HT_OK && read_count != 0)
		{
			http_storage_write(storage, read_buffer, read_count);
		}
	}
	if (http_send_storage(connection, (HTTP_STORAGE*)storage) == HT_NETWORK_ERROR)
	{
		http_reconnect(connection);
		http_send_storage(connection, (HTTP_STORAGE*)storage);
	}
	http_storage_destroy(&storage);
	if(request->content != NULL && request->method != HTTP_PROPFIND)
	{
		http_send_storage(connection, request->content);
	}
	return connection->status;
}

int
http_add_response_header_field(HTTP_RESPONSE *response, const char *field_name, const char *field_value)
{
	HTTP_HEADER_FIELD *new_header_field = NULL;
	if(response == NULL || field_name == NULL || field_value == NULL)
	{
		return HT_INVALID_ARGUMENT;
	}
	new_header_field = (HTTP_HEADER_FIELD *) _http_allocator(_http_allocator_user_data, 0, sizeof(HTTP_HEADER_FIELD));
	if(new_header_field == NULL)
	{
		return HT_MEMORY_ERROR;
	}
	memset(new_header_field, 0, sizeof(HTTP_HEADER_FIELD));
	new_header_field->name = wd_strdup(field_name);
	new_header_field->value = wd_strdup(field_value);
	if(new_header_field->name == NULL || new_header_field->value == NULL)
	{
		http_destroy_header_field(&new_header_field);
		return HT_MEMORY_ERROR;
	}
	if(response->first_header_field == NULL)
	{
		response->first_header_field = new_header_field;
	}
	else
	{
		response->last_header_field->next_field = new_header_field;
	}
	new_header_field->prev_field = response->last_header_field;
	response->last_header_field = new_header_field;
	return HT_OK;
}

int
http_append_last_response_header_field_value(HTTP_RESPONSE *response, const char *field_value)
{
	char *new_field_value = NULL;
	int new_field_value_length = 0;
	if(response == NULL || field_value == NULL)
	{
		return HT_INVALID_ARGUMENT;
	}
	if(response->last_header_field == NULL)
	{
		return HT_ILLEGAL_OPERATION;
	}
	new_field_value_length = strlen(response->last_header_field->value) + strlen(field_value);
	new_field_value = (char *) _http_allocator(_http_allocator_user_data, response->last_header_field->value, new_field_value_length);
	if(new_field_value == NULL)
	{
		return HT_MEMORY_ERROR;
	}
	response->last_header_field->value = new_field_value;
	strcat(response->last_header_field->value, field_value);
	return HT_OK;
}

int
http_set_response_status(HTTP_RESPONSE *response, const char *status_code, const char *status_msg, const char *version)
{
	char *new_status_msg = NULL, *new_version = NULL;
	if(response == NULL || status_code == NULL || status_msg == NULL || version == NULL)
	{
		return HT_INVALID_ARGUMENT;
	}
	if(response->status_code != 0 || response->status_msg != NULL || response->version != NULL)
	{
		return HT_ILLEGAL_OPERATION;
	}
	new_status_msg = wd_strdup(status_msg);
	new_version = wd_strdup(version);
	if(new_status_msg == NULL || new_version == NULL)
	{
		_http_allocator(_http_allocator_user_data, new_status_msg, 0);
		_http_allocator(_http_allocator_user_data, new_version, 0);
		return HT_MEMORY_ERROR;
	}
	response->status_code = (status_code[0] - '0') * 100 + (status_code[1] - '0') * 10 + (status_code[2] - '0');
	response->status_msg = new_status_msg;
	response->version = new_version;
	return HT_OK;
}

void
http_destroy_response(HTTP_RESPONSE **response)
{
	HTTP_HEADER_FIELD *field_cursor = NULL, *next_field = NULL;
	if(response != NULL && *response != NULL)
	{
		_http_allocator(_http_allocator_user_data, (*response)->status_msg, 0);
		_http_allocator(_http_allocator_user_data, (*response)->version, 0);
		for(field_cursor = (*response)->first_header_field; field_cursor != NULL; field_cursor = next_field)
		{
			next_field = field_cursor->next_field;
			http_destroy_header_field(&field_cursor);
		}
		http_storage_destroy(&(*response)->content);
		_http_allocator(_http_allocator_user_data, *response, 0);
		*response = NULL;
	}
}

int
http_create_response(HTTP_RESPONSE **response)
{
	HTTP_RESPONSE *new_response = NULL;
	if(response == NULL)
	{
		return HT_INVALID_ARGUMENT;
	}
	new_response = (HTTP_RESPONSE *) _http_allocator(_http_allocator_user_data, 0, sizeof(HTTP_RESPONSE));
	if(new_response == NULL)
	{
		return HT_MEMORY_ERROR;
	}
	memset(new_response, 0, sizeof(HTTP_RESPONSE));
	*response = new_response;
	return HT_OK;
}

#define HTTP_RECEIVING_STATUS_LINE		1
#define HTTP_RECEIVING_HEADER_FIELDS	2
#define HTTP_RECEIVING_BOUNDARY			3
#define HTTP_RECEIVING_CHUNKED_HEADER	4
#define HTTP_RECEIVING_CONTENT			5
#define HTTP_THE_DEVIL_TAKES_IT			6

int
http_receive_response_header(HTTP_CONNECTION *connection, HTTP_RESPONSE *response)
{
	char *line_buffer = NULL, *new_line_buffer = NULL;
	const char *status_code = NULL, *status_msg = NULL, *version = NULL;
	char *field_name = NULL, *field_value = NULL, *colon = NULL, *space = NULL;
	int line_index = 0, line_buffer_size = 0;
	int stage = HTTP_RECEIVING_STATUS_LINE;
	if(connection == NULL || response == NULL)
	{
		return HT_INVALID_ARGUMENT;
	}
	if(connection->status != HT_OK)
	{
		return connection->status;
	}
	while(stage <= HTTP_RECEIVING_HEADER_FIELDS && (connection->read_count = socket_recv(connection->socketd, connection->read_buffer, HTTP_READ_BUFFER_SIZE, 0)) > 0)
	{
		connection->read_index = 0;
		while((stage == HTTP_RECEIVING_STATUS_LINE || stage == HTTP_RECEIVING_HEADER_FIELDS) && connection->read_index < connection->read_count)
		{
			if(line_index >= line_buffer_size)
			{
				line_buffer_size += 128;
				new_line_buffer = (char *) _http_allocator(_http_allocator_user_data, line_buffer, line_buffer_size);
				if(new_line_buffer == NULL)
				{
					_http_allocator(_http_allocator_user_data, line_buffer, 0);
					return HT_MEMORY_ERROR;
				}
				line_buffer = new_line_buffer;
			}
			if(connection->read_buffer[connection->read_index] == '\n')
			{
				line_buffer[line_index] = '\0';
				if(line_index > 0)
				{
					if(stage == HTTP_RECEIVING_STATUS_LINE)
					{
						space = strchr(line_buffer, ' ');
						if(space != NULL)
						{
							space[0] = '\0';
							version = line_buffer;
							status_code = space + 1;
							space = strchr(space + 1, ' ');
							if(space != NULL)
							{
								status_msg = space + 1;
								http_set_response_status(response, status_code, status_msg, version);
								stage = HTTP_RECEIVING_HEADER_FIELDS;
							}
						}
					}
					else if(stage == HTTP_RECEIVING_HEADER_FIELDS)
					{
						if(line_buffer[0] == ' ' || line_buffer[0] == '\t')
						{
							field_value = line_buffer + 1;
							http_append_last_response_header_field_value(response, field_value);
						}
						else
						{
							colon = strchr(line_buffer, ':');
							if(colon != NULL)
							{
								colon[0] = '\0';
								field_name = line_buffer;
								field_value = colon + 2;
								http_add_response_header_field(response, field_name, field_value);
							}
						}
					}
				}
				else
				{
					stage = HTTP_RECEIVING_CONTENT;
				}
				line_index = 0;
			}
			else if(connection->read_buffer[connection->read_index] == '\r')
			{
			}
			else
			{
				line_buffer[line_index++] = connection->read_buffer[connection->read_index];
			}
			connection->read_index++;
		}
	}
	_http_allocator(_http_allocator_user_data, line_buffer, 0);
	if (connection->read_count < 0)
		return HT_NETWORK_ERROR;
	return HT_OK;
}

int
http_receive_response_entity(HTTP_CONNECTION *connection, HTTP_RESPONSE *response)
{
	char *line_buffer = NULL, *new_line_buffer = NULL;
	int line_index = 0, line_buffer_size = 0;
	int content_read_count = 0, content_size = 0;
	int is_chunked = 0;
	int is_multipart = 0;
	int stage = HTTP_RECEIVING_CONTENT;
	int nextStage = HTTP_RECEIVING_CONTENT;
	char* content_type = NULL;
	char* boundary = NULL;
	int multipart_content_read_count = 0, multipart_content_size = 0;
	HTTP_RESPONSE *multipart_response = NULL;
	int connection_close_requested;
	if(connection == NULL || response == NULL)
	{
		return HT_INVALID_ARGUMENT;
	}
	if(connection->status != HT_OK)
	{
		return connection->status;
	}
	connection_close_requested = http_has_header_field(response, "Connection", "Close");
	if(response->status_code != 204 && response->status_code != 205 && response->status_code != 304 && response->status_code > 199)
	{
		content_size = http_find_header_field_number(response, "Content-Length", LONG_MIN);
		content_read_count = 0;
		is_chunked = strcmp(http_find_header_field(response, "Transfer-Encoding", ""), "chunked") == 0;
		stage = is_chunked ? HTTP_RECEIVING_CHUNKED_HEADER : HTTP_RECEIVING_CONTENT; /* start reading the body */
		{
			char* ptr;
			const char* in_content_type = http_find_header_field(response, "Content-Type", NULL);
			if (in_content_type)
			{
				content_type = wd_strdup(in_content_type);

				ptr = content_type;
				while (*ptr  &&  *ptr != ';'  &&  *ptr != ' ')
					++ptr;

				if (*ptr == ';')
				{
					char* separator;
					*ptr++ = 0;

					separator = content_type;
					while (*separator  &&  *separator != '/')
						++separator;

					if (*separator == '/')
					{
						*separator = 0;
						is_multipart = strcmp(content_type, "multipart") == 0;
					}

					if (is_multipart)
					{
						while (*ptr  &&  *ptr == ' ')
							++ptr;

					separator = ptr;
					while (*separator  &&  *separator != '=')
						++separator;

						if (*separator == '=')
						{
							*separator = 0;
							if (strcmp(ptr, "boundary") == 0)
								boundary = separator + 1;
							nextStage = HTTP_RECEIVING_BOUNDARY;
							if (!is_chunked)
								stage = HTTP_RECEIVING_BOUNDARY;
						}
					}
				}
			}
		}
	}
	if (!is_multipart  &&  content_size <= 0  &&  !is_chunked  &&  !connection_close_requested)
	{
		stage = HTTP_THE_DEVIL_TAKES_IT; /* no content */
	}
	while(stage <= HTTP_RECEIVING_CONTENT)
	{
		if(content_size != LONG_MIN && content_read_count == content_size)
		{
			if (!is_chunked || content_size == 0)
			{
				stage = HTTP_THE_DEVIL_TAKES_IT;
				break;
			}
			if (is_chunked)
			{
				nextStage = stage;
				stage = HTTP_RECEIVING_CHUNKED_HEADER;
			}
		}
		if (connection->read_index == connection->read_count)
		{
			if ((connection->read_count = socket_recv(connection->socketd, connection->read_buffer, HTTP_READ_BUFFER_SIZE, 0)) <= 0)
				break;
			connection->read_index = 0;
		}

		if (stage == HTTP_RECEIVING_CHUNKED_HEADER  ||  stage == HTTP_RECEIVING_BOUNDARY  ||  stage == HTTP_RECEIVING_HEADER_FIELDS)
		{
			while(connection->read_index < connection->read_count)
			{
				if(line_index >= line_buffer_size)
				{
					line_buffer_size += 128;
					new_line_buffer = (char *) _http_allocator(_http_allocator_user_data, line_buffer, line_buffer_size);
					if(new_line_buffer == NULL)
					{
						_http_allocator(_http_allocator_user_data, line_buffer, 0);
						return HT_MEMORY_ERROR;
					}
					line_buffer = new_line_buffer;
				}
				if(connection->read_buffer[connection->read_index] == '\n')
				{
					line_buffer[line_index] = '\0';
					if(line_index > 0)
					{
						if(stage == HTTP_RECEIVING_CHUNKED_HEADER)
						{
							int ch = 0;
							int c = 0;
							int i = 0;
							int numDigits = 8;

							char* semicolon = strchr(line_buffer, ';');
							if (semicolon)
								*semicolon = 0;

							ch = tolower(line_buffer[i]);
							do {
								if (isdigit(ch))
									c = 16*c + (ch-'0');
								else if (ch >= 'a' && ch <= 'f')
									c = 16*c + (ch-'a') + 10;
								++i;
								ch = tolower(line_buffer[i]);
							} while (i<numDigits && (isdigit(ch) || (ch >= 'a' && ch <= 'f')));

							content_size = c;

							content_read_count = 0;
							stage = nextStage; /* start reading the body */
							line_index = 0;

							connection->read_index++;
							break;
/*							else if (content_size == 0)
							{
								stage = HTTP_THE_DEVIL_TAKES_IT;
							}*/
						}
						else if(stage == HTTP_RECEIVING_BOUNDARY)
						{
							if (line_index > 3  &&  line_buffer[0] == '-'  &&  line_buffer[1] == '-')
							{
								int is_end = line_index > 5  &&  line_buffer[line_index - 1] == '-'  &&  line_buffer[line_index - 2] == '-';
								if (is_end)
									line_buffer[line_index - 2] = 0;
								if (strcmp(line_buffer + 2, boundary) == 0)
								{
									if (multipart_response)
										http_destroy_response(&multipart_response);
									if (is_end)
									{
										connection->read_index++;
										stage = HTTP_THE_DEVIL_TAKES_IT;
										break;
									}
									else
									{
										stage = HTTP_RECEIVING_HEADER_FIELDS;
										http_create_response(&multipart_response);
									}
								}
							}
						}
						else if(stage == HTTP_RECEIVING_HEADER_FIELDS)
						{
							if(line_buffer[0] == ' ' || line_buffer[0] == '\t')
							{
								char* field_value = line_buffer + 1;
								http_append_last_response_header_field_value(multipart_response, field_value);
							}
							else
							{
								char* colon = strchr(line_buffer, ':');
								if(colon != NULL)
								{
									char* field_name;
									char* field_value;
									colon[0] = '\0';
									field_name = line_buffer;
									field_value = colon + 2;
									http_add_response_header_field(multipart_response, field_name, field_value);
								}
							}
						}
					}
					else
					{
						if (stage == HTTP_RECEIVING_HEADER_FIELDS)
						{
							const char* content_range = http_find_header_field(multipart_response, "Content-Range", "");
							const char* ptr = content_range;
							int start;
							int end;
							while (*ptr  &&  *ptr != ' ')
								++ptr;
							if (strncmp(content_range, "bytes", 5) == 0)
							{
								++ptr;
								start = atoi(ptr);
								while (*ptr  &&  *ptr != '-')
									++ptr;
								++ptr;
								end = atoi(ptr);

								multipart_content_size = end - start + 1;
								multipart_content_read_count = 0;
							}

							connection->read_index++;
							++content_read_count;
							stage = HTTP_RECEIVING_CONTENT;
							break;
						}
					}
					line_index = 0;
				}
				else if(connection->read_buffer[connection->read_index] == '\r')
				{
				}
				else
				{
					line_buffer[line_index++] = connection->read_buffer[connection->read_index];
				}
				connection->read_index++;
				++content_read_count;
			}
		}
		else
		{
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
			size_t content_size_to_read;
			if (is_multipart)
			{
				content_size_to_read = min(multipart_content_size - multipart_content_read_count, connection->read_count - connection->read_index);
				if (is_chunked)
					content_size_to_read = min(content_size - content_read_count, content_size_to_read);
			}
			else if (content_size == LONG_MIN)
			{
				content_size_to_read = connection->read_count - connection->read_index;
			}
			else
			{
				content_size_to_read = min(content_size - content_read_count, connection->read_count - connection->read_index);
			}
			if(response->content != NULL)
			{
				http_storage_write(response->content, &connection->read_buffer[connection->read_index], content_size_to_read);
			}
			content_read_count += content_size_to_read;
			if (is_multipart)
			{
				multipart_content_size -= content_size_to_read;
				if (multipart_content_size == 0)
					stage = HTTP_RECEIVING_BOUNDARY;
			}
			connection->read_index += content_size_to_read;
		}
	}
	if(response->content != NULL)
	{
		http_storage_close(response->content);
	}
	if(connection->persistent  &&  !connection->lazy)
	{
		if(connection_close_requested)
		{
			http_request_reconnection(connection);
		}
	}
	_http_allocator(_http_allocator_user_data, line_buffer, 0);
	if (multipart_response)
		http_destroy_response(&multipart_response);
	if (content_type)
		_http_allocator(_http_allocator_user_data, content_type, 0);
	if (connection_close_requested  &&  connection->read_count < 0  &&  connection->read_count != IO_CLOSED)
		return HT_NETWORK_ERROR;
	if (!connection_close_requested  &&  connection->read_count < 0)
		return HT_NETWORK_ERROR;
	return HT_OK;
}

int
http_receive_response(HTTP_CONNECTION *connection, HTTP_RESPONSE *response)
{
	int error;
	if((error = http_receive_response_header(connection, response)) != HT_OK)
	{
		return error;
	}
	if((error = http_receive_response_entity(connection, response)) != HT_OK)
	{
		return error;
	}
	return HT_OK;
}

int
http_scan_auth_request_parameters(HTTP_CONNECTION *connection, HTTP_RESPONSE *response)
{
	HTTP_AUTH_PARAMETER *new_parameter = NULL;
	int len = 0, end = 0;
	const char *ptr = NULL;
	if(connection == NULL || connection->auth_info == NULL || response == NULL)
	{
		return HT_INVALID_ARGUMENT;
	}
	ptr = http_find_header_field(response, "WWW-Authenticate", NULL);
	if(ptr == NULL)
	{
		return HT_RESOURCE_UNAVAILABLE;
	}
	wd_strclrws(&ptr);
	len = wd_strchrpos(ptr, ' ');
	if(len == -1)
	{
		return HT_RESOURCE_UNAVAILABLE;
	}
	connection->auth_info->method = wd_strndup(ptr, len);
	ptr += len + 1;
	while(!end)
	{
		wd_strclrws(&ptr);
		len = wd_strchrpos(ptr, '=');
		if(len != -1)
		{
			new_parameter = (HTTP_AUTH_PARAMETER *) _http_allocator(_http_allocator_user_data, 0, sizeof(HTTP_AUTH_PARAMETER));
			memset(new_parameter, 0, sizeof(HTTP_AUTH_PARAMETER));
			new_parameter->name = wd_strndup(ptr, len);
			ptr += len + 1;
			wd_strclrws(&ptr);
			len = wd_strchrqpos(ptr, ',');
			if(len != -1)
			{
				new_parameter->value = wd_strnunqdup(ptr, len);
				ptr += len + 1;
			}
			else
			{
				new_parameter->value = wd_strnunqdup(ptr, strlen(ptr));
				end = 1;
			}
			http_append_auth_request_parameter(connection->auth_info, new_parameter);
		}
		else
		{
			end = 1;
		}
	}
	return HT_OK;
}

const char *
http_find_header_field(HTTP_RESPONSE *response, const char *field_name, const char *default_value)
{
	HTTP_HEADER_FIELD *field_cursor = NULL;
	if(response == NULL || field_name == NULL)
	{
		return NULL;
	}
	for(field_cursor = response->first_header_field; field_cursor != NULL; field_cursor = field_cursor->next_field)
	{
		if(strcasecmp(field_cursor->name, field_name) == 0)
		{
			return field_cursor->value;
		}
	}
	return default_value;
}

long int
http_find_header_field_number(HTTP_RESPONSE *response, const char *field_name, int default_value)
{
	const char *value = NULL;
	value = http_find_header_field(response, field_name, NULL);
	if(value == NULL)
	{
		return default_value;
	}
	return strtol(value, NULL, 10);
}

int
http_has_header_field(HTTP_RESPONSE *response, const char *field_name, const char *field_value)
{
	const char *value = NULL;
	if(response == NULL || field_name == NULL || field_value == NULL)
	{
		return HT_FALSE;
	}
	value = http_find_header_field(response, field_name, NULL);
	if(value == NULL)
	{
		return HT_FALSE;
	}
	if(strcasecmp(value, field_value) == 0)
	{
		return HT_TRUE;
	}
	return HT_FALSE;
}

int
http_exec_error(HTTP_CONNECTION *connection)
{
	return connection->__http_exec_error;
}

const char *
http_exec_error_msg(HTTP_CONNECTION *connection)
{
	return connection->__http_exec_error_msg;
}

void
http_exec_set_response_error(HTTP_CONNECTION *connection, HTTP_RESPONSE *response)
{
	if(response != NULL)
	{
		connection->__http_exec_error = response->status_code;
		if (response->status_msg)
			strncpy(connection->__http_exec_error_msg, response->status_msg, 255);
		else
			connection->__http_exec_error_msg[0] = 0;
		connection->__http_exec_error_msg[255] = '\0';
	}
}

void
http_exec_set_sys_error(HTTP_CONNECTION *connection, int error)
{
	connection->__http_exec_error = error;
	switch(error)
	{
		case HT_OK: strcpy(connection->__http_exec_error_msg, "Sys: OK"); break;
		case HT_FATAL_ERROR: strcpy(connection->__http_exec_error_msg, "Sys: Fatal errror"); break;
		case HT_INVALID_ARGUMENT: strcpy(connection->__http_exec_error_msg, "Sys: Invalid function argument"); break;
		case HT_SERVICE_UNAVAILABLE: strcpy(connection->__http_exec_error_msg, "Sys: Service unavailable"); break;
		case HT_RESOURCE_UNAVAILABLE: strcpy(connection->__http_exec_error_msg, "Sys: Resource unavailable"); break;
		case HT_MEMORY_ERROR: strcpy(connection->__http_exec_error_msg, "Sys: Memory error"); break;
		case HT_NETWORK_ERROR: strcpy(connection->__http_exec_error_msg, "Sys: Network error"); break;
		case HT_ILLEGAL_OPERATION: strcpy(connection->__http_exec_error_msg, "Sys: Illegal operation"); break;
		case HT_HOST_UNAVAILABLE: strcpy(connection->__http_exec_error_msg, "Sys: Host not found"); break;
		case HT_IO_ERROR: strcpy(connection->__http_exec_error_msg, "Sys: I/O Error"); break;
	}
}

int
http_exec(HTTP_CONNECTION *connection, int method, const char *resource,
		HTTP_EVENT_HANDLER on_request_header, HTTP_EVENT_HANDLER on_request_entity,
		HTTP_EVENT_HANDLER on_response_header, HTTP_EVENT_HANDLER on_response_entity, void *data)
{
	HTTP_REQUEST *request = NULL;
	HTTP_RESPONSE *response = NULL;
	int error = HT_OK;
	if((error = http_create_request(&request, method, resource)) != HT_OK
			|| (error = http_create_response(&response)) != HT_OK)
	{
		http_destroy_request(&request);
		http_destroy_response(&response);
		http_exec_set_sys_error(connection, error);
		return error;
	}
	if(on_request_header != NULL)
	{
		if((error = on_request_header(connection, request, response, data)) != HT_OK)
		{
			http_destroy_request(&request);
			http_destroy_response(&response);
			http_exec_set_sys_error(connection, error);
			return error;
		}
	}
	if(on_request_entity != NULL)
	{
		if((error = on_request_entity(connection, request, response, data)) != HT_OK)
		{
			http_destroy_request(&request);
			http_destroy_response(&response);
			http_exec_set_sys_error(connection, error);
			return error;
		}
	}
	if((error = http_send_request(connection, request)) != HT_OK
			|| (error = http_receive_response_header(connection, response)) != HT_OK)
	{
		http_destroy_request(&request);
		http_destroy_response(&response);
		http_exec_set_sys_error(connection, error);
		return error;
	}
	if(response->status_code == 401)
	{
		if(connection->auth_info != NULL && connection->auth_info->method == NULL)
		{
			if(http_scan_auth_request_parameters(connection, response) == HT_OK)
			{
				http_receive_response_entity(connection, response);
				http_destroy_response(&response);
				if((error = http_create_response(&response)) != HT_OK
						|| (error = http_send_request(connection, request)) != HT_OK
						|| (error = http_receive_response_header(connection, response)) != HT_OK)
				{
					http_destroy_request(&request);
					http_destroy_response(&response);
					http_exec_set_sys_error(connection, error);
					return error;
				}
			}
		}
	}
	if(on_response_header != NULL)
	{
		if((error = on_response_header(connection, request, response, data)) != HT_OK)
		{
			if (error == HT_IO_ERROR)
				error = 404;
			http_receive_response_entity(connection, response);
			http_exec_set_sys_error(connection, error);
			return error;
		}
	}
	http_receive_response_entity(connection, response);
	if(on_response_entity != NULL)
	{
		if((error = on_response_entity(connection, request, response, data)) != HT_OK)
		{
			if (error == HT_IO_ERROR)
				error = 404;
			http_exec_set_sys_error(connection, error);
			return error;
		}
	}
	http_exec_set_response_error(connection, response);
	http_destroy_request(&request);
	http_destroy_response(&response);
	return error;
}

void http_set_actual_host(HTTP_CONNECTION *connection, const char* actualHost)
{
	if (connection->actualHost)
		_http_allocator(_http_allocator_user_data, connection->actualHost, 0);
	connection->actualHost = wd_strdup(actualHost);
}

void http_set_connect_callback(HTTP_CONNECTION *connection, int (*connect_callback)(void *), void *userData)
{
	connection->connect_callback = connect_callback;
	connection->connect_userData = userData;
}


typedef struct http_range_copy_from_server_instance {
	int start;
	int end;
	const char *destination;
} HTTP_RANGE_COPY_FROM_SERVER_INSTANCE;


static int http_range_copy_from_server_on_request_header(HTTP_CONNECTION *connection, HTTP_REQUEST *request, HTTP_RESPONSE *response, void *data)
{
	HTTP_RANGE_COPY_FROM_SERVER_INSTANCE *instance = (HTTP_RANGE_COPY_FROM_SERVER_INSTANCE *) data;
	char buffer[100];
	int error = HT_OK;

	if (instance->end != 0)
		sprintf(buffer, "bytes=%d-%d", instance->start, instance->end);
	else
		sprintf(buffer, "bytes=%d", instance->start);

	if((error = http_add_header_field(request, "Range", buffer)) != HT_OK)
	{
		return error;
	}
	return HT_OK;
}


static int http_range_copy_from_server_to_direct_memory_on_response_header(HTTP_CONNECTION *connection, HTTP_REQUEST *request, HTTP_RESPONSE *response, void *data)
{
	HTTP_RANGE_COPY_FROM_SERVER_INSTANCE *instance = (HTTP_RANGE_COPY_FROM_SERVER_INSTANCE *) data;
	int error = HT_OK;
	if(response->status_code == 200 || response->status_code == 206)
	{
		if((error = http_create_memory_storage((HTTP_MEMORY_STORAGE ** ) &response->content, (char*)instance->destination, instance->end != 0 ? instance->end - instance->start + 1 : -instance->start)) != HT_OK)
		{
			return error;
		}
	}
	return HT_OK;
}


int http_range_copy_from_server_to_direct_memory(HTTP_CONNECTION *connection, const char *src, int start, int end, unsigned char *dest)
{
	HTTP_RANGE_COPY_FROM_SERVER_INSTANCE instance;
	memset(&instance, 0, sizeof(HTTP_RANGE_COPY_FROM_SERVER_INSTANCE));
	instance.start = start;
	instance.end = end;
	instance.destination = (const char*)dest;
	if(http_exec(connection, HTTP_GET, src, http_range_copy_from_server_on_request_header, NULL,
				http_range_copy_from_server_to_direct_memory_on_response_header, NULL, (void *) &instance) != HT_OK)
	{
		//
	}
	return http_exec_error(connection);
}


