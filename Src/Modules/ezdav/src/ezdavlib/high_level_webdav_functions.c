#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "strutl.h"
#include "http.h"
#include "webdav.h"
#include "xml_tree.h"
#include "http_storage.h"
#include "high_level_webdav_functions.h"

extern http_allocator _http_allocator;
extern void* _http_allocator_user_data;

typedef struct dav_lock_database DAV_LOCK_DATABASE;
typedef struct dav_lockentry_info DAV_LOCKENTRY_INFO;

struct dav_lock_database {
	DAV_LOCKENTRY_INFO *first_lockentry;
	DAV_LOCKENTRY_INFO *last_lockentry;
};

struct dav_lockentry_info {
	char *locktoken;
	char *owner;
	char *host;
	char *resource;
	int depth;
	DAV_LOCKENTRY_INFO *prev_lockentry;
	DAV_LOCKENTRY_INFO *next_lockentry;
};

DAV_LOCK_DATABASE *global_lock_database = NULL;

int
dav_initialize_lock_database(void)
{
	global_lock_database = (DAV_LOCK_DATABASE *) _http_allocator(_http_allocator_user_data, 0, sizeof(DAV_LOCK_DATABASE));
	memset(global_lock_database, 0, sizeof(DAV_LOCK_DATABASE));
	return HT_OK;
}

void 
append_lockentry_info(DAV_LOCK_DATABASE *lock_database, DAV_LOCKENTRY_INFO *lockentry)
{
	if(lock_database->first_lockentry == NULL)
	{
		lock_database->first_lockentry = lockentry;
	}
	else
	{
		lock_database->last_lockentry->next_lockentry = lockentry;
	}
	lockentry->prev_lockentry = lock_database->last_lockentry;
	lock_database->last_lockentry = lockentry;
}

void 
remove_lockentry_info(DAV_LOCK_DATABASE *lock_database, DAV_LOCKENTRY_INFO *lockentry)
{
	if(lockentry->prev_lockentry != NULL) 
	{
		lockentry->prev_lockentry->next_lockentry = lockentry->next_lockentry;
	}
	if(lockentry->next_lockentry != NULL)
	{
		lockentry->next_lockentry->prev_lockentry = lockentry->prev_lockentry;
	}
	if(lock_database->first_lockentry == lockentry)
	{
		lock_database->first_lockentry = lockentry->next_lockentry;
	}
	if(lock_database->last_lockentry == lockentry)
	{
		lock_database->last_lockentry = lockentry->prev_lockentry;
	}
}

void
dav_destroy_lockentry_info(DAV_LOCKENTRY_INFO **lockentry)
{
	if(lockentry != NULL && *lockentry != NULL)
	{
		_http_allocator(_http_allocator_user_data, (*lockentry)->host, 0);
		_http_allocator(_http_allocator_user_data, (*lockentry)->resource, 0);
		_http_allocator(_http_allocator_user_data, (*lockentry)->locktoken, 0);
		_http_allocator(_http_allocator_user_data, *lockentry, 0);
		*lockentry = NULL;
	}
}

int
dav_resource_is_under_lock(const char *host, const char *resource, DAV_LOCKENTRY_INFO *lockentry)
{
	int i, length = 0, depth = 0;
	if(strcasecmp(lockentry->host, host) == 0)
	{
		length = strlen(lockentry->resource);
		if(strncasecmp(lockentry->resource, resource, length) == 0)
		{
			if(resource[length] == '\0')
			{
				return HT_TRUE;
			}
			else if(resource[length - 1] == '/')
			{
				if(lockentry->depth == HT_INFINITY)
				{
					return HT_TRUE;
				}
				else if(lockentry->depth > 0)
				{
					depth = 1;
					for(i = length; resource[i] != '\0'; i++)
					{
						if(resource[i] == '/')
						{
							depth++;
						}
					}
					if(lockentry->depth >= depth)
					{
						return HT_TRUE;
					}
				}
			}
		}
	}
	return HT_FALSE;
}

DAV_LOCKENTRY_INFO *
dav_get_lockentry_from_database(const char *host, const char *resource)
{
	DAV_LOCKENTRY_INFO *lockentry_cursor = NULL;
	if(global_lock_database != NULL)
	{
		for(lockentry_cursor = global_lock_database->first_lockentry; lockentry_cursor != NULL; lockentry_cursor = lockentry_cursor->next_lockentry)
		{
			if(dav_resource_is_under_lock(host, resource, lockentry_cursor))
			{
				return lockentry_cursor;
			}
		}
	}
	return NULL;
}

void
dav_remove_lockentry_from_database(const char *host, const char *resource)
{
	DAV_LOCKENTRY_INFO *lockentry_cursor = NULL;
	if(global_lock_database != NULL)
	{
		for(lockentry_cursor = global_lock_database->first_lockentry; lockentry_cursor != NULL; lockentry_cursor = lockentry_cursor->next_lockentry)
		{
			if(strcasecmp(lockentry_cursor->host, host) == 0 && strcasecmp(lockentry_cursor->resource, resource) == 0)
			{
				remove_lockentry_info(global_lock_database, lockentry_cursor);
				dav_destroy_lockentry_info(&lockentry_cursor);
				return;
			}
		}
	}
}

int
dav_add_lockentry_to_database(const char *host, const char *resource, const char *locktoken, const char *owner, int depth) 
{
	DAV_LOCKENTRY_INFO *new_lockentry_info = NULL;
	if(global_lock_database != NULL)
	{
		if(host == NULL || resource == NULL || locktoken == NULL || owner == NULL)
		{
			return HT_INVALID_ARGUMENT;
		}
		dav_remove_lockentry_from_database(host, resource);
		new_lockentry_info = (DAV_LOCKENTRY_INFO *) _http_allocator(_http_allocator_user_data, 0, sizeof(DAV_LOCKENTRY_INFO));
		if(new_lockentry_info == NULL)
		{
			return HT_MEMORY_ERROR;
		}
		memset(new_lockentry_info, 0, sizeof(DAV_LOCKENTRY_INFO));
		new_lockentry_info->host = wd_strdup(host);
		new_lockentry_info->resource = wd_strdup(resource);
		new_lockentry_info->locktoken = wd_strdup(locktoken);
		new_lockentry_info->owner = wd_strdup(owner);
		new_lockentry_info->depth = depth;
		if(new_lockentry_info->host == NULL || new_lockentry_info->resource == NULL || new_lockentry_info->locktoken == NULL || new_lockentry_info->owner == NULL)
		{
			dav_destroy_lockentry_info(&new_lockentry_info);
			return HT_MEMORY_ERROR;
		}
		append_lockentry_info(global_lock_database, new_lockentry_info);
	}
	return HT_OK;
}


void
dav_finalize_lock_database(void)
{
	DAV_LOCKENTRY_INFO *lockentry_cursor = NULL, *next_lockentry = NULL;
	if(global_lock_database != NULL)
	{
		for(lockentry_cursor = global_lock_database->first_lockentry; lockentry_cursor != NULL; lockentry_cursor = next_lockentry)
		{
			next_lockentry = lockentry_cursor->next_lockentry;
			dav_destroy_lockentry_info(&lockentry_cursor);
		}
		_http_allocator(_http_allocator_user_data, global_lock_database, 0);
		global_lock_database = NULL;
	}
}

void
dav_scan_locktoken_into_database(const char *host, const char *resource, DAV_PROP *prop)
{
	DAV_ACTIVELOCK *activelock = NULL;
	if((activelock = dav_find_activelock(prop, DAV_LOCKSCOPE_EXCLUSIVE, DAV_LOCKTYPE_WRITE)) != NULL)
	{
		dav_add_lockentry_to_database(host, resource, activelock->locktoken, activelock->owner, activelock->depth);
	}
}

const char *
get_depth_str(int depth)
{
	static char buffer[32];
	if(depth == HT_INFINITY)
	{
		return "infinity";
	}
	sprintf(buffer, "%d", depth);
	return buffer;
}

int
dav_load_lock_database(const char *filepath)
{
	XML_TREE *tree = NULL;
	XML_NODE *node_cursor = NULL, *child_node_cursor = NULL;
	DAV_LOCKENTRY_INFO *new_lockentry_info = NULL;
	HTTP_FILE_STORAGE *storage = NULL;
	int error;
	if(global_lock_database != NULL)
	{
		if((error = http_create_file_storage(&storage, filepath, "r")) != HT_OK)
		{
			return error;
		}
		if((error = xml_tree_build_from_storage(&tree, (HTTP_STORAGE *) storage)) != XT_OK)
		{
			http_storage_destroy(&storage);
			return error;
		}
		http_storage_destroy(&storage);
		if(tree->root_node->first_child_node != NULL)
		{
			if(strcmp(tree->root_node->first_child_node->name, "dav_lockentry_database") == 0)
			{
				for(node_cursor = tree->root_node->first_child_node->first_child_node; node_cursor != NULL; node_cursor = node_cursor->next_node)
				{
					if(strcmp(node_cursor->name, "lockentry_info") == 0)
					{
						new_lockentry_info = (DAV_LOCKENTRY_INFO *) _http_allocator(_http_allocator_user_data, 0, sizeof(DAV_LOCKENTRY_INFO));
						memset(new_lockentry_info, 0, sizeof(DAV_LOCKENTRY_INFO));
						for(child_node_cursor = node_cursor->first_child_node; child_node_cursor != NULL; child_node_cursor = child_node_cursor->next_node)
						{
							if(strcmp(child_node_cursor->name, "host") == 0)
							{
								new_lockentry_info->host = wd_strdup(child_node_cursor->data);
							}
							else if(strcmp(child_node_cursor->name, "resource") == 0)
							{
								new_lockentry_info->resource = wd_strdup(child_node_cursor->data);
							}
							else if(strcmp(child_node_cursor->name, "locktoken") == 0)
							{
								new_lockentry_info->locktoken = wd_strdup(child_node_cursor->data);
							}
							else if(strcmp(child_node_cursor->name, "depth") == 0)
							{
								if(strcasecmp(child_node_cursor->data, "infinity") == 0)
								{
									new_lockentry_info->depth = HT_INFINITY;
								}
								else
								{
									new_lockentry_info->depth = strtol(child_node_cursor->data, NULL, 10);
								}
							}
						}
						append_lockentry_info(global_lock_database, new_lockentry_info);
					}
				}
			}
		}
		xml_tree_destroy(&tree);
	}
	return HT_TRUE;
}

int
dav_save_lock_database(const char *filepath)
{
	XML_TREE *tree = NULL;
	DAV_LOCKENTRY_INFO *lockentry_cursor = NULL;
	HTTP_FILE_STORAGE *storage = NULL;
	int error;
	if(global_lock_database != NULL)
	{
		if((error = xml_tree_create(&tree)) != XT_OK)
		{
			return error;
		}
		if((error = xml_tree_start_node(tree, "dav_lockentry_database", "http://davim.sourceforge.net/")) != XT_OK)
		{
			xml_tree_destroy(&tree);
			return error;
		}
		for(lockentry_cursor = global_lock_database->first_lockentry; lockentry_cursor != NULL; lockentry_cursor = lockentry_cursor->next_lockentry)
		{
			if((error = xml_tree_start_node(tree, "lockentry_info", "http://davim.sourceforge.net/")) != XT_OK)
			{
				xml_tree_destroy(&tree);
				return error;
			}
			if((error = xml_tree_add_node(tree, "host", "http://davim.sourceforge.net/", lockentry_cursor->host)) != XT_OK)
			{
				xml_tree_destroy(&tree);
				return error;
			}
			if((error = xml_tree_add_node(tree, "resource", "http://davim.sourceforge.net/", lockentry_cursor->resource)) != XT_OK)
			{
				xml_tree_destroy(&tree);
				return error;
			}
			if((error = xml_tree_add_node(tree, "locktoken", "http://davim.sourceforge.net/", lockentry_cursor->locktoken)) != XT_OK)
			{
				xml_tree_destroy(&tree);
				return error;
			}
			if((error = xml_tree_add_node(tree, "owner", "http://davim.sourceforge.net/", lockentry_cursor->owner)) != XT_OK)
			{
				xml_tree_destroy(&tree);
				return error;
			}
			if((error = xml_tree_add_node(tree, "depth", "http://davim.sourceforge.net/", get_depth_str(lockentry_cursor->depth))) != XT_OK)
			{
				xml_tree_destroy(&tree);
				return error;
			}
			if((error = xml_tree_close_node(tree, "lockentry_info", "http://davim.sourceforge.net/")) != XT_OK)
			{
				xml_tree_destroy(&tree);
				return error;
			}
		}
		if((error = xml_tree_close_node(tree, "dav_lockentry_database", "http://davim.sourceforge.net/")) != XT_OK)
		{
			xml_tree_destroy(&tree);
			return error;
		}
		if((error = http_create_file_storage(&storage, filepath, "w+")) != HT_OK)
		{
			xml_tree_destroy(&tree);
			return error;
		}
		if((error = xml_tree_write_to_storage(tree, (HTTP_STORAGE *) storage)) != XT_OK)
		{
			http_storage_destroy(&storage);
			xml_tree_destroy(&tree);
			return error;
		}
		http_storage_close(storage);
		http_storage_destroy(&storage);
		xml_tree_destroy(&tree);
	}
	return HT_TRUE;
}

int 
	dav_connect(HTTP_CONNECTION **connection, const char *host, short port, const char *username, const char *password)
{
	return http_connect(connection, host, port, username, password) == HT_OK;
}

int 
	dav_connect_lazy(HTTP_CONNECTION **connection, const char *host, short port, const char *username, const char *password)
{
	return http_connect_lazy(connection, host, port, username, password) == HT_OK;
}

int 
dav_disconnect(HTTP_CONNECTION **connection)
{
	return http_disconnect(connection) == HT_OK;
}

int
dav_add_if_header_field(HTTP_CONNECTION *connection, HTTP_REQUEST *request)
{
	char *if_field_value = NULL;
	int if_field_value_length = 0;
	DAV_LOCKENTRY_INFO *lockentry_info = NULL;
	if((lockentry_info = dav_get_lockentry_from_database(http_hoststring(connection), request->resource)) != NULL)
	{
		if(strlen(lockentry_info->resource) == strlen(request->resource))
		{
			if_field_value_length = 4 + strlen(lockentry_info->locktoken);
			if_field_value = (char *) _http_allocator(_http_allocator_user_data, 0, if_field_value_length + 1);
			sprintf(if_field_value, "(<%s>)", lockentry_info->locktoken);
		}
		else
		{
			if_field_value_length = 3 + strlen(lockentry_info->resource) + 4 + strlen(lockentry_info->locktoken);
			if_field_value = (char *) _http_allocator(_http_allocator_user_data, 0, if_field_value_length + 1);
			sprintf(if_field_value, "<%s> (<%s>)", lockentry_info->resource, lockentry_info->locktoken);
		}
		http_add_header_field(request, "If", if_field_value);
		_http_allocator(_http_allocator_user_data, if_field_value, 0);
	}
	return HT_OK;
}

int
dav_opendir_on_request_header(HTTP_CONNECTION *connection, HTTP_REQUEST *request, HTTP_RESPONSE *response, void *data)
{
	DAV_OPENDIR_DATA *oddata = (DAV_OPENDIR_DATA *) data;
	int error = HT_OK;
	if((error = http_add_header_field_number(request, "Depth", oddata->directory_length == 0 ? 0 : 1)) != HT_OK)
	{
		return error;
	}
	return HT_OK;
}

int
dav_opendir_on_request_entity(HTTP_CONNECTION *connection, HTTP_REQUEST *request, HTTP_RESPONSE *response, void *data)
{
	DAV_OPENDIR_DATA *oddata = (DAV_OPENDIR_DATA *) data;
	DAV_PROPFIND *propfind = NULL;
	int error = HT_OK;
	if((error = dav_create_propfind(&propfind)) != HT_OK
			|| (error = dav_add_find_prop(propfind, "getcontentlength")) != HT_OK
			|| (error = dav_add_find_prop(propfind, "creationdate")) != HT_OK
			|| (error = dav_add_find_prop(propfind, "getlastmodified")) != HT_OK
			|| (error = dav_add_find_prop(propfind, "resourcetype")) != HT_OK
			|| (error = dav_add_find_prop_comma_delimited(propfind, oddata->additional_prop)) != HT_OK
			|| (error = http_create_memory_storage((HTTP_MEMORY_STORAGE **) &request->content, NULL, 0)) != HT_OK
			|| (error = dav_write_propfind_to_storage(propfind, request->content)) != HT_OK)
	{
		dav_propfind_destroy(&propfind);
		return error;
	}
	dav_propfind_destroy(&propfind);
	return HT_OK;
}

int
dav_opendir_on_response_header(HTTP_CONNECTION *connection, HTTP_REQUEST *request, HTTP_RESPONSE *response, void *data)
{
	int error = HT_OK;
	if(response->status_code == 207)
	{
		error = http_create_memory_storage((HTTP_MEMORY_STORAGE **) &response->content, NULL, 0);
	}
	return error;
}

int
dav_opendir_on_response_entity(HTTP_CONNECTION *connection, HTTP_REQUEST *request, HTTP_RESPONSE *response, void *data)
{
	DAV_OPENDIR_DATA *oddata = (DAV_OPENDIR_DATA *) data;
	DAV_RESPONSE *response_cursor = NULL;
	DAV_PROP *prop = NULL;
	int error = HT_OK;
	if(response->status_code == 207 && response->content != NULL)
	{
		error = dav_create_multistatus_from_storage(&oddata->multistatus, response->content);
		if(oddata->multistatus != NULL)
		{
			oddata->response_cursor = oddata->multistatus->first_response;
			for(response_cursor = oddata->multistatus->first_response; response_cursor != NULL; response_cursor = response_cursor->next_response)
			{
				if((prop = dav_find_prop(response_cursor, 200, 200)) != NULL)
				{
					dav_scan_locktoken_into_database(http_hoststring(connection), response_cursor->href, prop);
				}
			}
		}
	}
	return error;
}

int
dav_opendir_ex_helper(HTTP_CONNECTION *connection, const char *directory, unsigned int directory_length, const char *additional_prop, DAV_OPENDIR_DATA *oddata)
{
	DAV_PROPFIND *propfind = NULL;
	if(oddata == NULL || directory == NULL)
	{
		return HT_FALSE;
	}
	memset(oddata, 0, sizeof(DAV_OPENDIR_DATA));
	oddata->directory_length = directory_length;
	oddata->additional_prop = additional_prop;
	if(http_exec(connection, HTTP_PROPFIND, directory, 
				dav_opendir_on_request_header, dav_opendir_on_request_entity, 
				dav_opendir_on_response_header, dav_opendir_on_response_entity, (void *) oddata) != HT_OK)
	{
		return HT_FALSE;
	}
	return (http_exec_error(connection) == 207);
}

int
dav_opendir_ex(HTTP_CONNECTION *connection, const char *directory, const char *additional_prop, DAV_OPENDIR_DATA *oddata)
{
	return dav_opendir_ex_helper(connection, directory, strlen(directory), additional_prop, oddata);
}

int
dav_opendir(HTTP_CONNECTION *connection, const char *directory, DAV_OPENDIR_DATA *oddata)
{
	return dav_opendir_ex(connection, directory, NULL, oddata);
}

int
dav_attributes_ex(HTTP_CONNECTION *connection, const char *path, const char *additional_prop, DAV_OPENDIR_DATA *oddata)
{
	return dav_opendir_ex_helper(connection, path, 0, additional_prop, oddata);
}

int
dav_attributes(HTTP_CONNECTION *connection, const char *path, DAV_OPENDIR_DATA *oddata)
{
	return dav_attributes_ex(connection, path, NULL, oddata);
}

int
dav_readdir(DAV_OPENDIR_DATA *oddata)
{
	const char* href;
	DAV_ACTIVELOCK *activelock = NULL;
	if(oddata == NULL) 
	{
		return HT_FALSE;
	}
	if(oddata->response_cursor == NULL)
	{
		return HT_FALSE;
	}
	if(oddata->response_cursor->href == NULL)
	{
		return HT_FALSE;
	}
	oddata->href = oddata->response_cursor->href;
	oddata->prop = dav_find_prop(oddata->response_cursor, 200, 200);
	oddata->response_cursor = oddata->response_cursor->next_response;
	if(strlen(oddata->href) <= oddata->directory_length)
	{
		return dav_readdir(oddata);
	}
	href = oddata->href;
	if (strncmp(href, "http://", 7) == 0) {
		href += 7;
		while (*href && *href != '/')
			++href;
	}

	oddata->filename = href + oddata->directory_length;
	if(oddata->prop != NULL)
	{
		oddata->size = oddata->prop->getcontentlength;
		oddata->cdate = oddata->prop->creationdate;
		oddata->mdate = oddata->prop->getlastmodified;
		oddata->type = oddata->prop->resourcetype;
		if((activelock = dav_find_activelock(oddata->prop, DAV_LOCKSCOPE_EXCLUSIVE, DAV_LOCKTYPE_WRITE)) != NULL)
		{
			oddata->lockowner = activelock->owner;
		}
		else
		{
			oddata->lockowner = NULL;
		}
	}
	return HT_TRUE;
}

void
dav_closedir(DAV_OPENDIR_DATA *oddata)
{
	if(oddata != NULL) 
	{
		dav_multistatus_destroy(&oddata->multistatus);
	}
}

typedef struct dav_copy_from_server_instance {
	const char *dest_filepath;
} DAV_COPY_FROM_SERVER_INSTANCE;

int
dav_copy_from_server_on_response_header(HTTP_CONNECTION *connection, HTTP_REQUEST *request, HTTP_RESPONSE *response, void *data)
{
	DAV_COPY_FROM_SERVER_INSTANCE *instance = (DAV_COPY_FROM_SERVER_INSTANCE *) data;
	int error = HT_OK;
	if(response->status_code == 200 || response->status_code == 206)
	{
		if((error = http_create_file_storage((HTTP_FILE_STORAGE ** ) &response->content, instance->dest_filepath, "wb+")) != HT_OK)
		{
			return error;
		}
	}
	return HT_OK;
}

int
dav_copy_from_server(HTTP_CONNECTION *connection, const char *src, const char *dest)
{
	DAV_COPY_FROM_SERVER_INSTANCE instance;
	memset(&instance, 0, sizeof(DAV_COPY_FROM_SERVER_INSTANCE));
	instance.dest_filepath = dest;
	if(http_exec(connection, HTTP_GET, src, NULL, NULL, 
				dav_copy_from_server_on_response_header, NULL, (void *) &instance) != HT_OK)
	{
		return HT_FALSE;
	}
	return (http_exec_error(connection) == 200);
}

typedef struct dav_copy_to_server_instance {
	const char *src_filepath;
} DAV_COPY_TO_SERVER_INSTANCE;

int
dav_copy_to_server_on_request_header(HTTP_CONNECTION *connection, HTTP_REQUEST *request, HTTP_RESPONSE *response, void *data)
{
	int error = HT_OK;
	if((error = dav_add_if_header_field(connection, request)) != HT_OK)
	{
		return error;
	}
	return HT_OK;
}

int
dav_copy_to_server_on_request_entity(HTTP_CONNECTION *connection, HTTP_REQUEST *request, HTTP_RESPONSE *response, void *data)
{
	DAV_COPY_TO_SERVER_INSTANCE *instance = (DAV_COPY_TO_SERVER_INSTANCE *) data;
	int error = HT_OK;
	if((error = http_create_file_storage((HTTP_FILE_STORAGE **) &request->content, instance->src_filepath, "rb")) != HT_OK)
	{
		return error;
	}
	return HT_OK;
}

int
dav_copy_to_server(HTTP_CONNECTION *connection, const char *src, const char *dest)
{
	DAV_COPY_TO_SERVER_INSTANCE instance;
	memset(&instance, 0, sizeof(DAV_COPY_TO_SERVER_INSTANCE));
	instance.src_filepath = src;
	if(http_exec(connection, HTTP_PUT, dest, dav_copy_to_server_on_request_header, dav_copy_to_server_on_request_entity, 
				NULL, NULL, (void *) &instance) != HT_OK)
	{
		return HT_FALSE;
	}
	return (http_exec_error(connection) == 200 || http_exec_error(connection) == 201);
}

int
dav_mkdir_on_request_header(HTTP_CONNECTION *connection, HTTP_REQUEST *request, HTTP_RESPONSE *response, void *data)
{
	int error = HT_OK;
	if((error = dav_add_if_header_field(connection, request)) != HT_OK)
	{
		return error;
	}
	return HT_OK;
}

int
dav_mkdir(HTTP_CONNECTION *connection, const char *dir)
{
	if(http_exec(connection, HTTP_MKCOL, dir, dav_mkdir_on_request_header, NULL, NULL, NULL, NULL) != HT_OK)
	{
		return HT_FALSE;
	}
	return (http_exec_error(connection) == 200 || http_exec_error(connection) == 201);
};

int
dav_delete_on_request_header(HTTP_CONNECTION *connection, HTTP_REQUEST *request, HTTP_RESPONSE *response, void *data)
{
	int error = HT_OK;
	if((error = dav_add_if_header_field(connection, request)) != HT_OK)
	{
		return error;
	}
	return HT_OK;
}

int
dav_delete(HTTP_CONNECTION *connection, const char *resource)
{
	if(http_exec(connection, HTTP_DELETE, resource, dav_delete_on_request_header, NULL, 
				NULL, NULL, NULL) != HT_OK)
	{
		return HT_FALSE;
	}
	if(http_exec_error(connection) == 204)
	{
		dav_remove_lockentry_from_database(http_hoststring(connection), resource);
	}
	return (http_exec_error(connection) == 204);
}

int
dav_lock_on_request_header(HTTP_CONNECTION *connection, HTTP_REQUEST *request, HTTP_RESPONSE *response, void *data)
{
	int error = HT_OK;
	if((error = http_add_header_field_number(request, "Depth", HT_INFINITY)) != HT_OK
			|| (error = dav_add_if_header_field(connection, request)) != HT_OK)
	{
		return error;
	}
	return HT_OK;
}

int
dav_lock_on_request_entity(HTTP_CONNECTION *connection, HTTP_REQUEST *request, HTTP_RESPONSE *response, void *data)
{
	DAV_LOCKINFO *lockinfo = NULL;
	const char *owner = (const char *) data;
	int error = HT_OK;
	if((error = http_create_memory_storage((HTTP_MEMORY_STORAGE **) &request->content, NULL, 0)) != HT_OK
			|| (error = dav_create_lockinfo(&lockinfo, DAV_LOCKSCOPE_EXCLUSIVE, DAV_LOCKTYPE_WRITE, owner)) != HT_OK
			|| (error = dav_write_lockinfo_to_storage(lockinfo, request->content)) != HT_OK)
	{
		dav_lockinfo_destroy(&lockinfo);
		return error;
	}
	dav_lockinfo_destroy(&lockinfo);
	return HT_OK;
}

int
dav_lock_on_response_header(HTTP_CONNECTION *connection, HTTP_REQUEST *request, HTTP_RESPONSE *response, void *data)
{
	int error = HT_OK;
	if(response->status_code == 200)
	{
		if((error = http_create_memory_storage((HTTP_MEMORY_STORAGE **) &response->content, NULL, 0)) != HT_OK)
		{
			return error;
		}
	}
	return HT_OK;
}


int
dav_lock_on_response_entity(HTTP_CONNECTION *connection, HTTP_REQUEST *request, HTTP_RESPONSE *response, void *data)
{
	int error = HT_OK;
	DAV_PROP *prop = NULL;
	if(response->content != NULL)
	{
		if((error = dav_create_prop_from_storage(&prop, response->content)) != HT_OK)
		{
			return error;
		}
		dav_scan_locktoken_into_database(http_hoststring(connection), request->resource, prop);
		dav_prop_destroy(&prop);
	}
	return HT_OK;
}

int
dav_lock(HTTP_CONNECTION *connection, const char *resource, const char *owner)
{
	if(http_exec(connection, HTTP_LOCK, resource, dav_lock_on_request_header, dav_lock_on_request_entity, 
				dav_lock_on_response_header, dav_lock_on_response_entity, (void *) owner) != HT_OK)
	{
		return HT_FALSE;
	}
	return (http_exec_error(connection) == 200);
}

char *
strdup_locktoken(const char *token)
{
	char *bracketed_token = NULL;
	bracketed_token = (char *) _http_allocator(_http_allocator_user_data, 0, strlen(token) + 4);
	if(bracketed_token == NULL)
	{
		return NULL;
	}
	strcpy(bracketed_token, "<");
	strcat(bracketed_token, token);
	strcat(bracketed_token, ">");
	return bracketed_token;
}

int
dav_unlock_on_request_header(HTTP_CONNECTION *connection, HTTP_REQUEST *request, HTTP_RESPONSE *response, void *data)
{
	int error = HT_OK;
	DAV_LOCKENTRY_INFO *lockentry_info = NULL;
	char *bracketed_token = NULL;
	lockentry_info = dav_get_lockentry_from_database(http_hoststring(connection), request->resource);
	if(lockentry_info != NULL)
	{
		bracketed_token = strdup_locktoken(lockentry_info->locktoken);
		if((error = http_add_header_field(request, "Lock-Token", bracketed_token)) != HT_OK)
		{
			_http_allocator(_http_allocator_user_data, bracketed_token, 0);
			return error;
		}
		_http_allocator(_http_allocator_user_data, bracketed_token, 0);
		dav_remove_lockentry_from_database(http_hoststring(connection), request->resource);
	}
	return HT_OK;
}

int
dav_unlock(HTTP_CONNECTION *connection, const char *resource)
{
	if(http_exec(connection, HTTP_UNLOCK, resource, dav_unlock_on_request_header, NULL, 
				NULL, NULL, NULL) != HT_OK)
	{
		return HT_FALSE;
	}
	return (http_exec_error(connection) == 204);
}

int
dav_abandon_lock(HTTP_CONNECTION *connection, const char *resource)
{
	dav_remove_lockentry_from_database(http_hoststring(connection), resource);
	return HT_TRUE;
}

int 
dav_error(HTTP_CONNECTION *connection)
{
	return http_exec_error(connection);
}

const char * 
dav_error_msg(HTTP_CONNECTION *connection)
{
	return http_exec_error_msg(connection);
}
