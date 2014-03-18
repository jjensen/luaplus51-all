#if defined(_WIN32)
#include <winsock.h>
#endif // _WIN32
#include <string.h>
#include "lua.h"
#include "lauxlib.h"
#include "high_level_webdav_functions.h"
#include "davglob.h"

#if LUA_VERSION_NUM >= 502
#define luaL_register(a, b, c) luaL_setfuncs(a, c, 0)
#endif

#if _MSC_VER  &&  _MSC_VER <= 1300
double ui64ToDouble(unsigned __int64 ui64)
{
  __int64 i64 = (ui64 & 0x7FFFFFFFFFFFFFF);
  double dbl = (double) i64;
  if (ui64 & 0x8000000000000000)
    dbl += (double) 0x8000000000000000;
  return dbl;

}
#else
double ui64ToDouble(unsigned long long ui64)
{
	return (double)ui64;
}
#endif


#define EZDAV_CONNECTION_METATABLE "ezdav.connection"
#define EZDAV_DIR_METATABLE "ezdav.dir"

struct FileFindInfo {
	int first;
	DAV_OPENDIR_DATA oddata;
	char* path;
	char* pathEnd;
	char* wildcard;
};

static int FileFindNextMatch(struct FileFindInfo* info) {
	while (dav_readdir(&info->oddata)) {
		if (info->oddata.filename[0]) {
			if (info->wildcard) {
				if (davglob_WildMatch(info->wildcard, info->oddata.filename, 0)) {
					strcpy(info->pathEnd, info->oddata.filename);
					if (info->oddata.type == OD_DIRECTORY) {
						if (strcmp(info->oddata.filename, ".") == 0  ||  strcmp(info->oddata.filename, "..") == 0)
							continue;
					}
					*info->pathEnd = 0;
					return 1;
				}
			} else {
				return 1;
			}
		}
	}
	dav_closedir(&info->oddata);
	return 0;
}


static struct FileFindInfo* ezdav_dir_checkmetatable(lua_State *L, int index) {
  void *ud = luaL_checkudata(L, index, EZDAV_DIR_METATABLE);
  luaL_argcheck(L, ud != NULL, index, "ezdav.dir object expected");
  return (struct FileFindInfo*)ud;
}


static int ezdav_dir_next(lua_State *L) {
	struct FileFindInfo* info = ezdav_dir_checkmetatable(L, 1);
	if (FileFindNextMatch(info)) {
		lua_pushvalue(L, 1);
		return 1;
	}
	return 0;
}


static int ezdav_dir_close_helper(lua_State *L, struct FileFindInfo* info) {
	dav_closedir(&info->oddata);

	if (info->path) {
		free(info->path);
		info->path = NULL;
	}
	if (info->wildcard) {
		free(info->wildcard);
		info->wildcard = NULL;
	}
	return 0;
}


static int ezdav_dir_close(lua_State *L) {
	struct FileFindInfo* info = ezdav_dir_checkmetatable(L, 1);
	return ezdav_dir_close_helper(L, info);
}


static int ezdav_dir_gc(lua_State *L) {
	struct FileFindInfo* info = ezdav_dir_checkmetatable(L, 1);
	return ezdav_dir_close_helper(L, info);
}


static const struct luaL_Reg ezdav_dir_index_functions[] = {
	{ "next",					ezdav_dir_next },
	{ "close",					ezdav_dir_close },
	{ NULL, NULL },
};



static int ezdav_dir_index_filename_helper(lua_State* L, struct FileFindInfo* info) {
	lua_pushstring(L, info->oddata.filename);
	return 1;
}


static int ezdav_dir_index_filename(lua_State* L) {
	return ezdav_dir_index_filename_helper(L, ezdav_dir_checkmetatable(L, 1));
}


static int ezdav_dir_index_href_helper(lua_State* L, struct FileFindInfo* info) {
	lua_pushstring(L, info->oddata.href);
	return 1;
}


static int ezdav_dir_index_href(lua_State* L) {
	return ezdav_dir_index_href_helper(L, ezdav_dir_checkmetatable(L, 1));
}


static int ezdav_dir_index_creation_time_helper(lua_State* L, struct FileFindInfo* info) {
	lua_pushnumber(L, (lua_Number)info->oddata.cdate);
	return 1;
}


static int ezdav_dir_index_creation_time(lua_State* L) {
	return ezdav_dir_index_creation_time_helper(L, ezdav_dir_checkmetatable(L, 1));
}


static int ezdav_dir_index_write_time_helper(lua_State* L, struct FileFindInfo* info) {
	lua_pushnumber(L, (lua_Number)info->oddata.mdate);
	return 1;
}


static int ezdav_dir_index_write_time(lua_State* L) {
	return ezdav_dir_index_write_time_helper(L, ezdav_dir_checkmetatable(L, 1));
}


static int ezdav_dir_index_size_helper(lua_State* L, struct FileFindInfo* info) {
	lua_pushnumber(L, info->oddata.size);
	return 1;
}


static int ezdav_dir_index_size(lua_State* L) {
	return ezdav_dir_index_size_helper(L, ezdav_dir_checkmetatable(L, 1));
}


static int ezdav_dir_index_is_directory_helper(lua_State* L, struct FileFindInfo* info) {
	lua_pushboolean(L, info->oddata.type == OD_DIRECTORY);
	return 1;
}


static int ezdav_dir_index_is_directory(lua_State* L) {
	return ezdav_dir_index_is_directory_helper(L, ezdav_dir_checkmetatable(L, 1));
}


static int ezdav_dir_index_table_helper(lua_State* L, struct FileFindInfo* info) {
	lua_newtable(L);
	ezdav_dir_index_filename_helper(L, info);
	lua_setfield(L, -2, "filename");
	ezdav_dir_index_href_helper(L, info);
	lua_setfield(L, -2, "href");
	ezdav_dir_index_creation_time_helper(L, info);
	lua_setfield(L, -2, "creation_time");
	ezdav_dir_index_write_time_helper(L, info);
	lua_setfield(L, -2, "write_time");
	ezdav_dir_index_size_helper(L, info);
	lua_setfield(L, -2, "size");
	ezdav_dir_index_is_directory_helper(L, info);
	lua_setfield(L, -2, "is_directory");
	return 1;
}


static int ezdav_dir_index_table(lua_State* L) {
	struct FileFindInfo* info = ezdav_dir_checkmetatable(L, 1);
	return ezdav_dir_index_table_helper(L, info);
}


static const struct luaL_Reg ezdav_dir_index_properties[] = {
	{ "filename",				ezdav_dir_index_filename },
	{ "href",					ezdav_dir_index_href },
	{ "creation_time",			ezdav_dir_index_creation_time },
	{ "write_time",				ezdav_dir_index_write_time },
	{ "size",					ezdav_dir_index_size },
	{ "is_directory",			ezdav_dir_index_is_directory },
	{ "table",					ezdav_dir_index_table },
	{ NULL, NULL },
};



static int ezdav_dir_index(lua_State *L) {
	lua_CFunction function;
	struct FileFindInfo* info = ezdav_dir_checkmetatable(L, 1);
	const char* key = luaL_checklstring( L, 2, NULL );
	lua_getfield( L, lua_upvalueindex( 1 ), key );
	if ( !lua_isnil( L, -1 ) )
		return 1;
	lua_getfield( L, lua_upvalueindex( 2 ), key );
	function = lua_tocfunction( L, -1 );
	if ( function )
		return function( L );
	lua_getfield( L, lua_upvalueindex( 3 ), key );
	if ( !lua_isnil( L, -1 ) )
		return 1;
    luaL_argerror(L, 2, "improper key");
	return 1;
}


static int ezdav_dir_tostring(lua_State *L) {
	struct FileFindInfo* info = ezdav_dir_checkmetatable(L, 1);
	int top;

	top = lua_gettop(L);
	lua_pushstring(L, "[ezdav.dir object]: filename = \"");
	ezdav_dir_index_filename_helper(L, info);
	lua_pushstring(L, "\", href = \"");
	ezdav_dir_index_href_helper(L, info);
	lua_pushstring(L, "\", creation_time = ");
	ezdav_dir_index_creation_time_helper(L, info);
	lua_pushstring(L, ", write_time = ");
	ezdav_dir_index_write_time_helper(L, info);
	lua_pushstring(L, ", size = ");
	ezdav_dir_index_size_helper(L, info);
	lua_pushstring(L, ", is_directory = ");
	ezdav_dir_index_size_helper(L, info);
	lua_pushstring(L, lua_toboolean(L, -1) ? "true" : "false");
	lua_remove(L, -2);
	lua_concat(L, lua_gettop(L) - top);
	return 1;
}


static int ezdav_dir_create_metatable(lua_State *L) {
	luaL_newmetatable(L, EZDAV_DIR_METATABLE);				// metatable
	lua_pushliteral(L, "__gc");								// metatable __gc
	lua_pushcfunction(L, ezdav_dir_gc);						// metatable __gc function
	lua_settable(L, -3);									// metatable
	lua_pushliteral(L, "__tostring");						// metatable __tostring
	lua_pushcfunction(L, ezdav_dir_tostring);				// metatable __tostring function
	lua_settable(L, -3);									// metatable
	lua_pushliteral(L, "__index");							// metatable __index
	lua_newtable(L);										// metatable __index table table
	luaL_register (L, NULL, ezdav_dir_index_functions);
	lua_newtable(L);										// metatable __index table table
	luaL_register(L, NULL, ezdav_dir_index_properties);
	lua_pushvalue( L, -4 );
	lua_pushcclosure( L, ezdav_dir_index, 3 );
	lua_settable(L, -3);

	lua_pop(L, 1);
	return 0;
}


static int l_ezdav_dir_first(lua_State *L, HTTP_CONNECTION* connection) {
	const char* wildcard = luaL_checkstring(L, 2);
	const char* origWildcard = wildcard;

	struct FileFindInfo* info = (struct FileFindInfo*)lua_newuserdata(L, sizeof(struct FileFindInfo));

	char* ptr;
	char* slashPtr;
	int hasWildcard = 0;

	info->path = malloc(strlen(wildcard) + 256);
	strcpy(info->path, wildcard);
	for (ptr = info->path; *ptr; ++ptr) {
		if (*ptr == '\\')
			*ptr = '/';
		else if (*ptr == '*'  ||  *ptr == '?')
			hasWildcard = 1;
	}
	slashPtr = strrchr(info->path, '/');
	if (slashPtr  &&  info->path + strlen(info->path) - 1 == slashPtr) {
		char* oldSlashPtr = slashPtr;
		*slashPtr = 0;
		slashPtr = strrchr(info->path, '/');
		*oldSlashPtr = '/';
	}
	if (hasWildcard) {
		wildcard = slashPtr ? slashPtr + 1 : info->path;
		info->wildcard = malloc(strlen(wildcard) + 1);
		strcpy(info->wildcard, wildcard);
		if (slashPtr)
			*++slashPtr = 0;
		else
			info->path[0] = 0;
	} else {
		info->wildcard = NULL;
	}
	info->pathEnd = info->path + strlen(info->path);

	info->first = 1;

	if (info->wildcard) {
		if (dav_opendir(connection, slashPtr ? info->path : ".", &info->oddata) == HT_FALSE)
			return 0;
	} else {
		if (dav_attributes(connection, info->path, &info->oddata) == HT_FALSE)
			return 0;
	}
	if (!FileFindNextMatch(info))
		return 0;

	luaL_getmetatable(L, EZDAV_DIR_METATABLE);
	lua_setmetatable(L, -2);

	return 1;
}


static int ezdav_dir_matchiter(lua_State* L) {
	struct FileFindInfo* info = (struct FileFindInfo*)(lua_touserdata(L, lua_upvalueindex(1)));

	if (!info->oddata.multistatus)
		return 0;

	if (info->first) {
		info->first = 0;
		lua_pushvalue(L, lua_upvalueindex(1));
		return 1;
	}

	if (FileFindNextMatch(info)) {
		lua_pushvalue(L, lua_upvalueindex(1));
		return 1;
	}

	return 0;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define EZDAV_GLOB_METATABLE "ezdav.glob"

static struct _davglob* glob_checkmetatable(lua_State *L, int index) {
	struct _davglob* glob;
	void *ud = luaL_checkudata(L, index, EZDAV_GLOB_METATABLE);
	luaL_argcheck(L, ud != NULL, index, "ezdav.glob object expected");
	glob = (struct _davglob*)*(void**)ud;
	luaL_argcheck(L, glob != NULL, index, "ezdav.glob object is already closed");
	return glob;
}


static int glob_next(lua_State *L) {
	struct _davglob* glob = glob_checkmetatable(L, 1);
	lua_pushboolean(L, davglob_Next(glob));
	return 1;
}


static int glob_close(lua_State *L) {
	struct _davglob* glob = glob_checkmetatable(L, 1);
	if (glob) {
		davglob_Destroy(glob);
		*(void **)(lua_touserdata(L, 1)) = NULL;
	}
	return 0;
}


static int glob_gc(lua_State *L) {
	glob_close(L);
	return 0;
}


static const struct luaL_Reg glob_index_functions[] = {
	{ "next",					glob_next },
	{ "close",					glob_close },
	{ NULL, NULL },
};


static int glob_index_filename_helper(lua_State* L, struct _davglob* glob) {
	lua_pushstring(L, davglob_FileName(glob));
	return 1;
}


static int glob_index_filename(lua_State* L) {
	return glob_index_filename_helper(L, glob_checkmetatable(L, 1));
}


static int glob_index_creation_time_helper(lua_State* L, struct _davglob* glob) {
	lua_pushnumber(L, (lua_Number)ui64ToDouble(davglob_CreationTime(glob)));
	return 1;
}


static int glob_index_creation_time(lua_State* L) {
	return glob_index_creation_time_helper(L, glob_checkmetatable(L, 1));
}


static int glob_index_write_time_helper(lua_State* L, struct _davglob* glob) {
	lua_pushnumber(L, (lua_Number)ui64ToDouble(davglob_WriteTime(glob)));
	return 1;
}


static int glob_index_write_time(lua_State* L) {
	return glob_index_write_time_helper(L, glob_checkmetatable(L, 1));
}


static int glob_index_size_helper(lua_State* L, struct _davglob* glob) {
	lua_pushnumber(L, (lua_Number)ui64ToDouble(davglob_FileSize(glob)));
	return 1;
}


static int glob_index_size(lua_State* L) {
	return glob_index_size_helper(L, glob_checkmetatable(L, 1));
}


static int glob_index_is_directory_helper(lua_State* L, struct _davglob* glob) {
	lua_pushboolean(L, davglob_IsDirectory(glob));
	return 1;
}


static int glob_index_is_directory(lua_State* L) {
	return glob_index_is_directory_helper(L, glob_checkmetatable(L, 1));
}


static int glob_index_table(lua_State* L) {
	struct _davglob* glob = glob_checkmetatable(L, 1);
	lua_newtable(L);
	glob_index_filename_helper(L, glob);
	lua_setfield(L, -2, "filename");
	glob_index_creation_time_helper(L, glob);
	lua_setfield(L, -2, "creation_time");
	glob_index_write_time_helper(L, glob);
	lua_setfield(L, -2, "write_time");
	glob_index_size_helper(L, glob);
	lua_setfield(L, -2, "size");
	glob_index_is_directory_helper(L, glob);
	lua_setfield(L, -2, "is_directory");
	return 1;
}


static const struct luaL_Reg glob_index_properties[] = {
	{ "filename",				glob_index_filename },
	{ "creation_time",			glob_index_creation_time },
	{ "write_time",				glob_index_write_time },
	{ "size",					glob_index_size },
	{ "is_directory",			glob_index_is_directory },
	{ "table",					glob_index_table },
	{ NULL, NULL },
};


static int glob_index(lua_State *L) {
	lua_CFunction function;
	struct _davglob* glob = glob_checkmetatable(L, 1);
	const char* key = luaL_checklstring( L, 2, NULL );
	lua_getfield( L, lua_upvalueindex( 1 ), key );
	if ( !lua_isnil( L, -1 ) )
		return 1;
	lua_getfield( L, lua_upvalueindex( 2 ), key );
	function = lua_tocfunction( L, -1 );
	if ( function )
		return function( L );
	lua_getfield( L, lua_upvalueindex( 3 ), key );
	if ( !lua_isnil( L, -1 ) )
		return 1;
    luaL_argerror(L, 2, "improper key");
	return 1;
}


static int glob_tostring(lua_State *L) {
	struct _davglob* glob = *(struct _davglob**)(lua_touserdata(L, 1));
	char buffer[100];
	int top;
	if (!glob) {
		lua_pushstring(L, "[glob object]: done");
		return 1;
	}

	top = lua_gettop(L);
	lua_pushstring(L, "[glob object]: filename = \"");
	lua_pushstring(L, davglob_FileName(glob));
	lua_pushstring(L, "\"");
	sprintf(buffer, ", creation_time = %lld", davglob_CreationTime(glob));
	lua_pushstring(L, buffer);
	sprintf(buffer, ", write_time = %lld", davglob_WriteTime(glob));
	lua_pushstring(L, buffer);
	sprintf(buffer, ", size = %lld", davglob_FileSize(glob));
	lua_pushstring(L, buffer);
	sprintf(buffer, ", is_directory = %s", davglob_IsDirectory(glob) ? "true" : "false");
	lua_pushstring(L, buffer);
	lua_concat(L, lua_gettop(L) - top);
	return 1;
}


static int glob_create_metatable(lua_State *L) {
	luaL_newmetatable(L, EZDAV_GLOB_METATABLE);				// metatable
	lua_pushliteral(L, "__gc");								// metatable __gc
	lua_pushcfunction(L, glob_gc);							// metatable __gc function
	lua_settable(L, -3);									// metatable
	lua_pushliteral(L, "__tostring");						// metatable __tostring
	lua_pushcfunction(L, glob_tostring);					// metatable __tostring function
	lua_settable(L, -3);									// metatable
	lua_pushliteral(L, "__index");							// metatable __index
	lua_newtable(L);										// metatable __index table table
	luaL_register (L, NULL, glob_index_functions);
	lua_newtable(L);										// metatable __index table table
	luaL_register(L, NULL, glob_index_properties);
	lua_pushvalue( L, -4 );
	lua_pushcclosure( L, glob_index, 3 );
	lua_settable(L, -3);

	lua_pop(L, 1);
	return 0;
}


static int l_davglob_first(lua_State *L, HTTP_CONNECTION* connection) {
	const char* pattern = luaL_checkstring(L, 2);
	*(void **)(lua_newuserdata(L, sizeof(void *))) = davglob_Create(connection, pattern);

	luaL_getmetatable(L, EZDAV_GLOB_METATABLE);
	lua_setmetatable(L, -2);

	return 1;
}


static int l_davglob_matchiter(lua_State* L) {
	struct _davglob* glob = *(struct _davglob**)(lua_touserdata(L, lua_upvalueindex(1)));

	if (!davglob_Next(glob))
		return 0;

	lua_pushvalue(L, lua_upvalueindex(1));
	return 1;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static HTTP_CONNECTION* ezdav_connection_checkmetatable_internal(lua_State *L, int index) {
	HTTP_CONNECTION* connection;
	void *ud = luaL_checkudata(L, index, EZDAV_CONNECTION_METATABLE);
	luaL_argcheck(L, ud != NULL, index, "ezdav.connection object expected");
	connection = (HTTP_CONNECTION*)*(void**)ud;
	return connection;
}


static HTTP_CONNECTION* ezdav_connection_checkmetatable(lua_State *L, int index) {
	HTTP_CONNECTION* connection = ezdav_connection_checkmetatable_internal(L, index);
	luaL_argcheck(L, connection != NULL, index, "ezdav.connection object is already closed");
	return connection;
}


static int ezdav_connection_attributes(lua_State* L) {
	HTTP_CONNECTION* connection = ezdav_connection_checkmetatable_internal(L, 1);
	if (l_ezdav_dir_first(L, connection) == 0) {
		http_exec_set_sys_error(connection, 404);
		return 0;
	}

	luaL_getmetatable(L, EZDAV_DIR_METATABLE);
	lua_setmetatable(L, -2);
	return 1;
}


static int ezdav_connection_close(lua_State *L) {
	HTTP_CONNECTION* connection = ezdav_connection_checkmetatable_internal(L, 1);
	if (connection) {
		dav_disconnect(&connection);
		*(void **)(lua_touserdata(L, 1)) = NULL;
	}
	return 0;
}


static int ezdav_connection_glob(lua_State* L) {
	HTTP_CONNECTION* connection = ezdav_connection_checkmetatable_internal(L, 1);
	l_davglob_first(L, connection);
	lua_pushcclosure(L, l_davglob_matchiter, 1);
	return 1;
}


static int ezdav_connection_match(lua_State *L) {
	HTTP_CONNECTION* connection = ezdav_connection_checkmetatable_internal(L, 1);
	l_ezdav_dir_first(L, connection);
	lua_pushcclosure(L, ezdav_dir_matchiter, 1);
	return 1;
}


static int ezdav_connection_mkdir(lua_State *L) {
	HTTP_CONNECTION* connection = ezdav_connection_checkmetatable(L, 1);
	lua_pushboolean(L, dav_mkdir(connection, luaL_checkstring(L, 2)));
	return 1;
}


static int ezdav_connection_delete(lua_State *L) {
	HTTP_CONNECTION* connection = ezdav_connection_checkmetatable(L, 1);
	lua_pushboolean(L, dav_delete(connection, luaL_checkstring(L, 2)));
	return 1;
}


static int ezdav_connection_copy_to_server(lua_State *L) {
	HTTP_CONNECTION* connection = ezdav_connection_checkmetatable(L, 1);
	lua_pushboolean(L, dav_copy_to_server(connection, luaL_checkstring(L, 2), luaL_checkstring(L, 3)));
	return 1;
}


static int ezdav_connection_copy_from_server(lua_State *L) {
	HTTP_CONNECTION* connection = ezdav_connection_checkmetatable(L, 1);
	lua_pushboolean(L, dav_copy_from_server(connection, luaL_checkstring(L, 2), luaL_checkstring(L, 3)));
	return 1;
}


typedef struct dav_range_copy_from_server_instance {
	const char *dest_filepath;
	const char *byte_range;
} DAV_RANGE_COPY_FROM_SERVER_INSTANCE;

int dav_range_copy_from_server_on_request_header(HTTP_CONNECTION *connection, HTTP_REQUEST *request, HTTP_RESPONSE *response, void *data)
{
	DAV_RANGE_COPY_FROM_SERVER_INSTANCE *instance = (DAV_RANGE_COPY_FROM_SERVER_INSTANCE *) data;
	char buffer[10000];
	int error = HT_OK;
	sprintf(buffer, "bytes=%s", instance->byte_range);

	if((error = http_add_header_field(request, "Range", buffer)) != HT_OK)
	{
		return error;
	}
	return HT_OK;
}


int dav_range_copy_from_server_on_response_header(HTTP_CONNECTION *connection, HTTP_REQUEST *request, HTTP_RESPONSE *response, void *data)
{
	DAV_RANGE_COPY_FROM_SERVER_INSTANCE *instance = (DAV_RANGE_COPY_FROM_SERVER_INSTANCE *) data;
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

int dav_range_copy_from_server(HTTP_CONNECTION *connection, const char *src, const char *dest, const char *byte_range)
{
	DAV_RANGE_COPY_FROM_SERVER_INSTANCE instance;
	memset(&instance, 0, sizeof(DAV_RANGE_COPY_FROM_SERVER_INSTANCE));
	instance.dest_filepath = dest;
	instance.byte_range = byte_range;
	if(http_exec(connection, HTTP_GET, src, dav_range_copy_from_server_on_request_header, NULL, 
				dav_range_copy_from_server_on_response_header, NULL, (void *) &instance) != HT_OK)
	{
		return HT_FALSE;
	}
	return (http_exec_error(connection) == 200  ||  http_exec_error(connection) == 206);
}

static int ezdav_connection_range_copy_from_server(lua_State *L) {
	HTTP_CONNECTION* connection = ezdav_connection_checkmetatable(L, 1);
	lua_pushboolean(L, dav_range_copy_from_server(connection, luaL_checkstring(L, 2), luaL_checkstring(L, 3), luaL_checkstring(L, 4)));
	return 1;
}


static int ezdav_connection_lock(lua_State *L) {
	HTTP_CONNECTION* connection = ezdav_connection_checkmetatable(L, 1);
	lua_pushboolean(L, dav_lock(connection, luaL_checkstring(L, 2), luaL_checkstring(L, 3)));
	return 1;
}


static int ezdav_connection_unlock(lua_State *L) {
	HTTP_CONNECTION* connection = ezdav_connection_checkmetatable(L, 1);
	lua_pushboolean(L, dav_unlock(connection, luaL_checkstring(L, 2)));
	return 1;
}


static int ezdav_connection_abandon_lock(lua_State *L) {
	HTTP_CONNECTION* connection = ezdav_connection_checkmetatable(L, 1);
	lua_pushboolean(L, dav_unlock(connection, luaL_checkstring(L, 2)));
	return 1;
}


static int ezdav_connection_error(lua_State *L) {
	HTTP_CONNECTION* connection = ezdav_connection_checkmetatable(L, 1);
	lua_pushinteger(L, dav_error(connection));
	return 1;
}


static int ezdav_connection_error_message(lua_State *L) {
	HTTP_CONNECTION* connection = ezdav_connection_checkmetatable(L, 1);
	lua_pushstring(L, dav_error_msg(connection));
	return 1;
}


static int ezdav_connection_set_actual_host(lua_State *L) {
	HTTP_CONNECTION* connection = ezdav_connection_checkmetatable(L, 1);
	const char* host = luaL_checkstring(L, 2);
	http_set_actual_host(connection, host);

	return 0;
}


static int ezdav_connection_gc(lua_State *L) {
	ezdav_connection_close(L);
	return 0;
}


static const struct luaL_Reg ezdav_connection_index_functions[] = {
	{ "attributes",			ezdav_connection_attributes },
	{ "close",				ezdav_connection_close },
	{ "glob",				ezdav_connection_glob },
	{ "match",				ezdav_connection_match },
	{ "mkdir",				ezdav_connection_mkdir },
	{ "delete",				ezdav_connection_delete },
	{ "copy_to_server",		ezdav_connection_copy_to_server },
	{ "copy_from_server",	ezdav_connection_copy_from_server },
	{ "range_copy_from_server",	ezdav_connection_range_copy_from_server },
	{ "lock",				ezdav_connection_lock },
	{ "unlock",				ezdav_connection_unlock },
	{ "abandon_lock",		ezdav_connection_abandon_lock },
	{ "error",				ezdav_connection_error },
	{ "error_message",		ezdav_connection_error_message },
	{ "set_actual_host",	ezdav_connection_set_actual_host },
	{ NULL, NULL },
};


static int ezdav_connection_tostring(lua_State *L) {
	return 0;
}


static int ezdav_connection_create_metatable(lua_State *L) {
	luaL_newmetatable(L, EZDAV_CONNECTION_METATABLE);		// metatable
	lua_pushliteral(L, "__gc");								// metatable __gc
	lua_pushcfunction(L, ezdav_connection_gc);				// metatable __gc function
	lua_settable(L, -3);									// metatable
	lua_pushliteral(L, "__tostring");						// metatable __tostring
	lua_pushcfunction(L, ezdav_connection_tostring);		// metatable __tostring function
	lua_settable(L, -3);									// metatable
	lua_pushliteral(L, "__index");							// metatable __index
	lua_newtable(L);										// metatable __index functions
	luaL_register(L, NULL, ezdav_connection_index_functions);
	lua_settable(L, -3);									// metatable

	lua_pop(L, 1);
	return 0;
}


static int l_ezdav_connection_open(lua_State *L) {
	const char* host = luaL_checkstring(L, 1);
	short port = (short)luaL_checknumber(L, 2);
	const char* username = luaL_optstring(L, 3, NULL);
	const char* password = luaL_optstring(L, 4, NULL);
	HTTP_CONNECTION* connection;
	if (dav_connect(&connection, host, port, username, password)) {
		*(void **)(lua_newuserdata(L, sizeof(void *))) = connection;

		luaL_getmetatable(L, EZDAV_CONNECTION_METATABLE);
		lua_setmetatable(L, -2);

		return 1;
	}

	return 0;
}


static int l_ezdav_connection_lazy_open(lua_State *L) {
	const char* host = luaL_checkstring(L, 1);
	short port = (short)luaL_checknumber(L, 2);
	const char* username = luaL_optstring(L, 3, NULL);
	const char* password = luaL_optstring(L, 4, NULL);
	HTTP_CONNECTION* connection;
	if (dav_connect_lazy(&connection, host, port, username, password)) {
		*(void **)(lua_newuserdata(L, sizeof(void *))) = connection;

		luaL_getmetatable(L, EZDAV_CONNECTION_METATABLE);
		lua_setmetatable(L, -2);
	}

	return 1;
}



static const struct luaL_Reg ezdav_lib[] = {
	{ "open", l_ezdav_connection_open },
	{ "lazy_open", l_ezdav_connection_lazy_open },
	{NULL, NULL},
};


int luaopen_ezdav(lua_State* L) {
#if defined(_WIN32)
	// Initialize Winsock.
	WSADATA wsaData;
	int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
#endif // _WIN32

	ezdav_dir_create_metatable(L);
	ezdav_connection_create_metatable(L);
	glob_create_metatable(L);
	lua_newtable(L);
	luaL_register(L, NULL, ezdav_lib);
	return 1;
}


