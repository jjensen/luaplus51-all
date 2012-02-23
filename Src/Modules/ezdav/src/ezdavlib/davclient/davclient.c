#include "webdav.h"
#include "high_level_webdav_functions.h"
#include "tlsf.h"

static void *http_tlsf_allocator(void *ud, void *ptr, size_t nsize) {
	tlsf_pool pool = (tlsf_pool)ud;
	return tlsf_realloc(pool, ptr, nsize);
}

int usedByteCount;
static void http_tlsf_walker(void* ptr, size_t size, int used, void* user)
{
	if (used)
		usedByteCount += size;
}

int main() {
	// Initialize Winsock.
	WSADATA wsaData;
	int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );

	HTTP_CONNECTION* connection;
	DAV_OPENDIR_DATA oddata;

	int ret;
	unsigned char* mem = malloc(50000);
	tlsf_pool pool = tlsf_create(mem, 50000);
	http_set_allocator(http_tlsf_allocator, pool);

//	int ret = http_connect(&connection, "demo.sabredav.org", 80, "testuser", "test");
	ret = dav_connect(&connection, "demo.sabredav.org", 80, "testuser", "test");
//	int ret = dav_connect(&connection, "localhost", 8080, NULL, NULL);

//	ret = dav_copy_to_server(connection, "d:/inffas32.obj", "/inffas32.obj");
//	ret = dav_delete(connection, "/inffas32.obj");
	usedByteCount = 0;
	tlsf_walk_heap(pool, http_tlsf_walker, NULL);


	ret = dav_opendir(connection, "/", &oddata);
	while (ret) {
		int hi = 5;
		ret = dav_readdir(&oddata);
		if (!ret)
			break;
		hi = 10;
	}
	usedByteCount = 0;
	tlsf_walk_heap(pool, http_tlsf_walker, NULL);

	dav_closedir(&oddata);

	usedByteCount = 0;
	tlsf_walk_heap(pool, http_tlsf_walker, NULL);

	ret = dav_copy_from_server(connection, "/public/Modern Photo Letter.pdf", "a.pdf");
	usedByteCount = 0;
	tlsf_walk_heap(pool, http_tlsf_walker, NULL);
	ret = dav_copy_from_server(connection, "/document.pdf", "d.pdf");
//	ret = dav_copy_to_server(connection, "u:/screen2.jpg", "/screen2.jpg");
//	ret = dav_copy_to_server(connection, "u:/Quicken_Deluxe_2007.exe", "/qd.exe");

	dav_disconnect(&connection);

	return 0;
}
