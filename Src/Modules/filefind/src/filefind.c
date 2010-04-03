#include "lua.h"
#include "lauxlib.h"
#if defined(WIN32)
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#endif /* WIN32 */
#include <time.h>
#include <ctype.h>
#define FILEGLOB_NEED_FILETIME_TO_TIME_T_CONVERSION
#include "fileglob.h"


#if defined(WIN32)

static int _filefind_push_FILETIME(lua_State* L, FILETIME* filetime) {
	lua_newtable(L);
	lua_pushnumber(L, (lua_Number)filetime->dwLowDateTime);
	lua_rawseti(L, -2, 1);
	lua_pushnumber(L, (lua_Number)filetime->dwHighDateTime);
	lua_rawseti(L, -2, 2);
	return 1;
}

#else

static int _filefind_push_FILETIME(lua_State* L, fileglob_uint64 filetime) {
	lua_newtable(L);
	lua_pushnumber(L, (lua_Number)(unsigned int)(filetime & 0xffffffff));
	lua_rawseti(L, -2, 1);
	lua_pushnumber(L, (lua_Number)(unsigned int)(filetime >> 32));
	lua_rawseti(L, -2, 2);
	return 1;
}

#endif


#define FILEFIND_METATABLE "FileFindMetaTable"

#if defined(WIN32)

struct FileFindInfo {
	int first;			/* default true */
	HANDLE handle;		/* INVALID_HANDLE_VALUE */
	WIN32_FIND_DATA fd;
};

static int FileFindNextMatch(struct FileFindInfo* info) {
	if (info->handle == INVALID_HANDLE_VALUE)
		return 0;

	while (FindNextFile(info->handle, &info->fd)) {
		if (info->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (strcmp(info->fd.cFileName, ".") == 0  ||  strcmp(info->fd.cFileName, "..") == 0)
				continue;
		}
		return 1;
	}

	FindClose(info->handle);
	info->handle = INVALID_HANDLE_VALUE;
	return 0;
}


#else

struct FileFindInfo {
	int first;
	DIR* dirp;
	struct dirent* dp;
	struct stat attr;
	char* path;
	char* pathEnd;
	char* wildcard;
};

static int FileFindNextMatch(struct FileFindInfo* info) {
	while ((info->dp = readdir(info->dirp)) != NULL) {
		if (fileglob_WildMatch(info->wildcard, info->dp->d_name, 0)) {
			strcpy(info->pathEnd, info->dp->d_name);
			stat(info->path, &info->attr);
			if ((info->attr.st_mode & S_IFDIR) != 0) {
				if (strcmp(info->dp->d_name, ".") == 0  ||  strcmp(info->dp->d_name, "..") == 0)
					continue;				
			}
			*info->pathEnd = 0;
			return 1;
		}
	}
	closedir(info->dirp);
	info->dirp = NULL;
	return 0;
}

#endif // WIN32

static struct FileFindInfo* filefind_checkmetatable(lua_State *L, int index) {
  void *ud = luaL_checkudata(L, index, FILEFIND_METATABLE);
  luaL_argcheck(L, ud != NULL, index, "filefind object expected");
  return (struct FileFindInfo*)ud;
}


static int filefind_next(lua_State *L) {
	struct FileFindInfo* info = filefind_checkmetatable(L, 1);
	if (FileFindNextMatch(info)) {
		lua_pushvalue(L, 1);
		return 1;
	}
	return 0;
}


static int filefind_close(lua_State *L) {
	struct FileFindInfo* info = filefind_checkmetatable(L, 1);
#if defined(WIN32)
	if (info->handle) {
		FindClose(info->handle);
		info->handle = NULL;
	}
#else
	if (info->dirp) {
		closedir(info->dirp);
		info->dirp = NULL;
	}
#endif
	return 0;
}


static int filefind_gc(lua_State *L) {
	struct FileFindInfo* info = filefind_checkmetatable(L, 1);
#if defined(WIN32)
	if (info->handle != INVALID_HANDLE_VALUE)
		FindClose(info->handle);
#else
	if (info->dirp)
		closedir(info->dirp);
	free(info->path);
	free(info->wildcard);
#endif
	return 0;
}


static const struct luaL_reg filefind_index_functions[] = {
	{ "next",					filefind_next },
	{ "close",					filefind_close },
	{ NULL, NULL },
};



static int filefind_index_filename_helper(lua_State* L, struct FileFindInfo* info) {
#if defined(WIN32)
	lua_pushstring(L, info->fd.cFileName);
#else
	lua_pushstring(L, info->dp->d_name);
#endif
	return 1;
}


static int filefind_index_filename(lua_State* L) {
	return filefind_index_filename_helper(L, filefind_checkmetatable(L, 1));
}


static int filefind_index_creation_time_helper(lua_State* L, struct FileFindInfo* info) {
#if defined(WIN32)
	lua_pushnumber(L, (lua_Number)fileglob_ConvertToTime_t(&info->fd.ftCreationTime));
#else 
	lua_pushnumber(L, info->attr.st_ctime);
#endif
	return 1;
}


static int filefind_index_creation_time(lua_State* L) {
	return filefind_index_creation_time_helper(L, filefind_checkmetatable(L, 1));
}


static int filefind_index_access_time_helper(lua_State* L, struct FileFindInfo* info) {
#if defined(WIN32)
	lua_pushnumber(L, (lua_Number)fileglob_ConvertToTime_t(&info->fd.ftLastAccessTime));
#else 
	lua_pushnumber(L, info->attr.st_atime);
#endif
	return 1;
}


static int filefind_index_access_time(lua_State* L) {
	return filefind_index_access_time_helper(L, filefind_checkmetatable(L, 1));
}


static int filefind_index_write_time_helper(lua_State* L, struct FileFindInfo* info) {
#if defined(WIN32)
	lua_pushnumber(L, (lua_Number)fileglob_ConvertToTime_t(&info->fd.ftLastWriteTime));
#else 
	lua_pushnumber(L, info->attr.st_mtime);
#endif
	return 1;
}


static int filefind_index_write_time(lua_State* L) {
	return filefind_index_write_time_helper(L, filefind_checkmetatable(L, 1));
}


#if !defined(WIN32)
#define Int32x32To64(a, b) ((long long)((long)(a)) * (long long)((long)(b)))
#endif


static int filefind_index_creation_FILETIME_helper(lua_State* L, struct FileFindInfo* info) {
#if defined(WIN32)
	return _filefind_push_FILETIME(L, &info->fd.ftCreationTime);
#else
	return _filefind_push_FILETIME(L, Int32x32To64(info->attr.st_ctime, 10000000) + 116444736000000000);
#endif
}


static int filefind_index_creation_FILETIME(lua_State* L) {
	return filefind_index_creation_FILETIME_helper(L, filefind_checkmetatable(L, 1));
}


static int filefind_index_access_FILETIME_helper(lua_State* L, struct FileFindInfo* info) {
#if defined(WIN32)
	return _filefind_push_FILETIME(L, &info->fd.ftLastAccessTime);
#else
	return _filefind_push_FILETIME(L, Int32x32To64(info->attr.st_atime, 10000000) + 116444736000000000);
#endif
}


static int filefind_index_access_FILETIME(lua_State* L) {
	return filefind_index_access_FILETIME_helper(L, filefind_checkmetatable(L, 1));
}


static int filefind_index_write_FILETIME_helper(lua_State* L, struct FileFindInfo* info) {
#if defined(WIN32)
	return _filefind_push_FILETIME(L, &info->fd.ftLastWriteTime);
#else
	return _filefind_push_FILETIME(L, Int32x32To64(info->attr.st_mtime, 10000000) + 116444736000000000);
#endif
}


static int filefind_index_write_FILETIME(lua_State* L) {
	return filefind_index_write_FILETIME_helper(L, filefind_checkmetatable(L, 1));
}


static int filefind_index_size_helper(lua_State* L, struct FileFindInfo* info) {
#if defined(WIN32)
	lua_pushnumber(L, (lua_Number)((unsigned __int64)info->fd.nFileSizeLow + ((unsigned __int64)info->fd.nFileSizeHigh << 32)));
#else
	lua_pushnumber(L, info->attr.st_size);
#endif
	return 1;
}


static int filefind_index_size(lua_State* L) {
	return filefind_index_size_helper(L, filefind_checkmetatable(L, 1));
}


static int filefind_index_is_directory_helper(lua_State* L, struct FileFindInfo* info) {
#if defined(WIN32)
	lua_pushboolean(L, (info->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
#else
	lua_pushboolean(L, (info->attr.st_mode & S_IFDIR) != 0);
#endif
	return 1;
}


static int filefind_index_is_directory(lua_State* L) {
	return filefind_index_is_directory_helper(L, filefind_checkmetatable(L, 1));
}


static int filefind_index_is_readonly_helper(lua_State* L, struct FileFindInfo* info) {
#if defined(WIN32)
	lua_pushboolean(L, (info->fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0);
#else
	lua_pushboolean(L, (info->attr.st_mode & S_IWUSR) != 0);
#endif
	return 1;
}


static int filefind_index_is_readonly(lua_State* L) {
	return filefind_index_is_readonly_helper(L, filefind_checkmetatable(L, 1));
}


static int filefind_index_table(lua_State* L) {
	struct FileFindInfo* info = filefind_checkmetatable(L, 1);
	lua_newtable(L);
	filefind_index_filename_helper(L, info);
	lua_setfield(L, -2, "filename");
	filefind_index_creation_time_helper(L, info);
	lua_setfield(L, -2, "creation_time");
	filefind_index_access_time_helper(L, info);
	lua_setfield(L, -2, "access_time");
	filefind_index_write_time_helper(L, info);
	lua_setfield(L, -2, "write_time");
	filefind_index_creation_FILETIME_helper(L, info);
	lua_setfield(L, -2, "creation_FILETIME");
	filefind_index_access_FILETIME_helper(L, info);
	lua_setfield(L, -2, "access_FILETIME");
	filefind_index_write_FILETIME_helper(L, info);
	lua_setfield(L, -2, "write_FILETIME");
	filefind_index_size_helper(L, info);
	lua_setfield(L, -2, "size");
	filefind_index_is_directory_helper(L, info);
	lua_setfield(L, -2, "is_directory");
	filefind_index_is_readonly_helper(L, info);
	lua_setfield(L, -2, "is_readonly");
	return 1;
}


static const struct luaL_reg filefind_index_properties[] = {
	{ "filename",				filefind_index_filename },
	{ "creation_time",			filefind_index_creation_time },
	{ "access_time",			filefind_index_access_time },
	{ "write_time",				filefind_index_write_time },
	{ "creation_FILETIME",		filefind_index_creation_FILETIME },
	{ "access_FILETIME",		filefind_index_access_FILETIME },
	{ "write_FILETIME",			filefind_index_write_FILETIME },
	{ "size",					filefind_index_size },
	{ "is_directory",			filefind_index_is_directory },
	{ "is_readonly",			filefind_index_is_readonly },
	{ "table",					filefind_index_table },
	{ NULL, NULL },
};



static int filefind_index(lua_State *L) {
	lua_CFunction function;
	struct FileFindInfo* info = filefind_checkmetatable(L, 1);
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


static int filefind_tostring(lua_State *L) {
	struct FileFindInfo* info = filefind_checkmetatable(L, 1);
	char buffer[100];
	int top;
#if defined(WIN32)
	if (info->handle == INVALID_HANDLE_VALUE) {
		lua_pushstring(L, "[filefind object]: empty");
		return 1;
	}

	top = lua_gettop(L);
	lua_pushstring(L, "[filefind object]: filename = \"");
	lua_pushstring(L, info->fd.cFileName);
	lua_pushstring(L, "\"");
	sprintf(buffer, ", creation_time = %u", fileglob_ConvertToTime_t(&info->fd.ftCreationTime));
	lua_pushstring(L, buffer);
	sprintf(buffer, ", access_time = %u", fileglob_ConvertToTime_t(&info->fd.ftLastAccessTime));
	lua_pushstring(L, buffer);
	sprintf(buffer, ", write_time = %u", fileglob_ConvertToTime_t(&info->fd.ftLastWriteTime));
	lua_pushstring(L, buffer);
	sprintf(buffer, ", creation_FILETIME = { %u, %u }", info->fd.ftCreationTime.dwLowDateTime, info->fd.ftCreationTime.dwHighDateTime);
	lua_pushstring(L, buffer);
	sprintf(buffer, ", access_FILETIME = { %u, %u }", info->fd.ftLastAccessTime.dwLowDateTime, info->fd.ftLastAccessTime.dwHighDateTime);
	lua_pushstring(L, buffer);
	sprintf(buffer, ", write_FILETIME = { %u, %u }", info->fd.ftLastWriteTime.dwLowDateTime, info->fd.ftLastWriteTime.dwHighDateTime);
	lua_pushstring(L, buffer);
	sprintf(buffer, ", size = %I64d", ((unsigned __int64)info->fd.nFileSizeLow + ((unsigned __int64)info->fd.nFileSizeHigh << 32)));
	lua_pushstring(L, buffer);
	sprintf(buffer, ", is_directory = %s", (info->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ? "true" : "false");
	lua_pushstring(L, buffer);
	sprintf(buffer, ", is_readonly = %s", (info->fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ? "true" : "false");
	lua_pushstring(L, buffer);
	lua_concat(L, lua_gettop(L) - top);
	return 1;
#else
	return 0;
#endif
}


static int filefind_create_metatable(lua_State *L) {
	luaL_newmetatable(L, FILEFIND_METATABLE);				// metatable
	lua_pushliteral(L, "__gc");								// metatable __gc
	lua_pushcfunction(L, filefind_gc);						// metatable __gc function
	lua_settable(L, -3);									// metatable
	lua_pushliteral(L, "__tostring");						// metatable __tostring
	lua_pushcfunction(L, filefind_tostring);				// metatable __tostring function
	lua_settable(L, -3);									// metatable
	lua_pushliteral(L, "__index");							// metatable __index
	lua_newtable(L);										// metatable __index table table
	luaL_register (L, NULL, filefind_index_functions);
	lua_newtable(L);										// metatable __index table table
	luaL_register(L, NULL, filefind_index_properties);
	lua_pushvalue( L, -4 );
	lua_pushcclosure( L, filefind_index, 3 );
	lua_settable(L, -3);

	lua_pop(L, 1);
	return 0;
}


static int l_filefind_first(lua_State *L) {
	const char* wildcard = luaL_checkstring(L, 1);

	struct FileFindInfo* info = (struct FileFindInfo*)lua_newuserdata(L, sizeof(struct FileFindInfo));

	info->first = 1;
#if defined(WIN32)
	info->handle = FindFirstFile(wildcard, &info->fd);
	if (info->handle != INVALID_HANDLE_VALUE) {
		if (info->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (strcmp(info->fd.cFileName, ".") == 0  ||  strcmp(info->fd.cFileName, "..") == 0)
				FileFindNextMatch(info);
		}
	}
#else
	{
		char* ptr;
		char* slashPtr;

		info->path = malloc(strlen(wildcard) + 256);
		strcpy(info->path, wildcard);
		for (ptr = info->path; *ptr; ++ptr)
			if (*ptr == '\\')
				*ptr = '/';
		slashPtr = strrchr(info->path, '/');
		wildcard = slashPtr ? slashPtr + 1 : info->path;
		info->wildcard = malloc(strlen(wildcard) + 1);
		strcpy(info->wildcard, wildcard);
		if (slashPtr)
			*++slashPtr = 0;
		else
			info->path[0] = 0;
		info->pathEnd = info->path + strlen(info->path);
		info->dirp = opendir(slashPtr ? info->path : ".");

		if (!FileFindNextMatch(info))
			return 0;
	}
#endif

	luaL_getmetatable(L, FILEFIND_METATABLE);
	lua_setmetatable(L, -2);

	return 1;
}


static int LS_filefind_matchiter(lua_State* L) {
	struct FileFindInfo* info = (struct FileFindInfo*)(lua_touserdata(L, lua_upvalueindex(1)));

#if defined(WIN32)
	if (info->handle == INVALID_HANDLE_VALUE) {
		info->handle = NULL;
		return 0;
	}
#else
	if (!info->dirp)
		return 0;
#endif

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


static int l_filefind_match(lua_State* L) {
	l_filefind_first(L);
	lua_pushcclosure(L, LS_filefind_matchiter, 1);
	return 1;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define FILEGLOB_METATABLE "FileGlobMetaTable"

static struct _fileglob* glob_checkmetatable(lua_State *L, int index) {
	struct _fileglob* glob;
	void *ud = luaL_checkudata(L, index, FILEGLOB_METATABLE);
	luaL_argcheck(L, ud != NULL, index, "fileglob object expected");
	glob = (struct _fileglob*)*(void**)ud;
	luaL_argcheck(L, glob != NULL, index, "fileglob object is already closed");
	return glob;
}


static int glob_next(lua_State *L) {
	struct _fileglob* glob = glob_checkmetatable(L, 1);
	lua_pushboolean(L, fileglob_Next(glob));
	return 1;
}


static int glob_close(lua_State *L) {
	struct _fileglob* glob = glob_checkmetatable(L, 1);
	if (glob) {
		fileglob_Destroy(glob);
		*(void **)(lua_touserdata(L, 1)) = NULL;
	}
	return 0;
}


static int glob_gc(lua_State *L) {
	glob_close(L);
	return 0;
}


static const struct luaL_reg glob_index_functions[] = {
	{ "next",					glob_next },
	{ "close",					glob_close },
	{ NULL, NULL },
};


static int _glob_push_FILETIME(lua_State* L, fileglob_uint64 filetime) {
	lua_newtable(L);
	lua_pushnumber(L, (lua_Number)(filetime & 0xffffffff));
	lua_rawseti(L, -2, 1);
	lua_pushnumber(L, (lua_Number)(filetime >> 32));
	lua_rawseti(L, -2, 2);
	return 1;
}


static int glob_index_filename_helper(lua_State* L, struct _fileglob* glob) {
	lua_pushstring(L, fileglob_FileName(glob));
	return 1;
}


static int glob_index_filename(lua_State* L) {
	return glob_index_filename_helper(L, glob_checkmetatable(L, 1));
}


static int glob_index_creation_time_helper(lua_State* L, struct _fileglob* glob) {
	lua_pushnumber(L, (lua_Number)fileglob_CreationTime(glob));
	return 1;
}


static int glob_index_creation_time(lua_State* L) {
	return glob_index_creation_time_helper(L, glob_checkmetatable(L, 1));
}


static int glob_index_access_time_helper(lua_State* L, struct _fileglob* glob) {
	lua_pushnumber(L, (lua_Number)fileglob_AccessTime(glob));
	return 1;
}


static int glob_index_access_time(lua_State* L) {
	return glob_index_access_time_helper(L, glob_checkmetatable(L, 1));
}


static int glob_index_write_time_helper(lua_State* L, struct _fileglob* glob) {
	lua_pushnumber(L, (lua_Number)fileglob_WriteTime(glob));
	return 1;
}


static int glob_index_write_time(lua_State* L) {
	return glob_index_write_time_helper(L, glob_checkmetatable(L, 1));
}


static int glob_index_creation_FILETIME(lua_State* L) {
	return _glob_push_FILETIME(L, fileglob_CreationFILETIME(glob_checkmetatable(L, 1)));
}


static int glob_index_access_FILETIME(lua_State* L){
	return _glob_push_FILETIME(L, fileglob_AccessFILETIME(glob_checkmetatable(L, 1)));
}


static int glob_index_write_FILETIME(lua_State* L) {
	return _glob_push_FILETIME(L, fileglob_WriteFILETIME(glob_checkmetatable(L, 1)));
}


static int glob_index_size_helper(lua_State* L, struct _fileglob* glob) {
	lua_pushnumber(L, (lua_Number)fileglob_FileSize(glob));
	return 1;
}


static int glob_index_size(lua_State* L) {
	return glob_index_size_helper(L, glob_checkmetatable(L, 1));
}


static int glob_index_is_directory_helper(lua_State* L, struct _fileglob* glob) {
	lua_pushboolean(L, fileglob_IsDirectory(glob));
	return 1;
}


static int glob_index_is_directory(lua_State* L) {
	return glob_index_is_directory_helper(L, glob_checkmetatable(L, 1));
}


static int glob_index_is_readonly_helper(lua_State* L, struct _fileglob* glob) {
	lua_pushboolean(L, fileglob_IsReadOnly(glob));
	return 1;
}


static int glob_index_is_readonly(lua_State* L) {
	return glob_index_is_readonly_helper(L, glob_checkmetatable(L, 1));
}


static int glob_index_table(lua_State* L) {
	struct _fileglob* glob = glob_checkmetatable(L, 1);
	lua_newtable(L);
	glob_index_filename_helper(L, glob);
	lua_setfield(L, -2, "filename");
	glob_index_creation_time_helper(L, glob);
	lua_setfield(L, -2, "creation_time");
	glob_index_access_time_helper(L, glob);
	lua_setfield(L, -2, "access_time");
	glob_index_write_time_helper(L, glob);
	lua_setfield(L, -2, "write_time");
	_glob_push_FILETIME(L, fileglob_CreationFILETIME(glob));
	lua_setfield(L, -2, "creation_FILETIME");
	_glob_push_FILETIME(L, fileglob_AccessFILETIME(glob));
	lua_setfield(L, -2, "access_FILETIME");
	_glob_push_FILETIME(L, fileglob_WriteFILETIME(glob));
	lua_setfield(L, -2, "write_FILETIME");
	glob_index_size_helper(L, glob);
	lua_setfield(L, -2, "size");
	glob_index_is_directory_helper(L, glob);
	lua_setfield(L, -2, "is_directory");
	glob_index_is_readonly_helper(L, glob);
	lua_setfield(L, -2, "is_readonly");
	return 1;
}


static const struct luaL_reg glob_index_properties[] = {
	{ "filename",				glob_index_filename },
	{ "creation_time",			glob_index_creation_time },
	{ "access_time",			glob_index_access_time },
	{ "write_time",				glob_index_write_time },
	{ "creation_FILETIME",		glob_index_creation_FILETIME },
	{ "access_FILETIME",		glob_index_access_FILETIME },
	{ "write_FILETIME",			glob_index_write_FILETIME },
	{ "size",					glob_index_size },
	{ "is_directory",			glob_index_is_directory },
	{ "is_readonly",			glob_index_is_readonly },
	{ "table",					glob_index_table },
	{ NULL, NULL },
};


static int glob_index(lua_State *L) {
	lua_CFunction function;
	struct _fileglob* glob = glob_checkmetatable(L, 1);
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
	struct _fileglob* glob = *(struct _fileglob**)(lua_touserdata(L, 1));
	fileglob_uint64 filetime;
	char buffer[100];
	int top;
	if (!glob) {
		lua_pushstring(L, "[glob object]: done");
		return 1;
	}

	top = lua_gettop(L);
	lua_pushstring(L, "[glob object]: filename = \"");
	lua_pushstring(L, fileglob_FileName(glob));
	lua_pushstring(L, "\"");
	sprintf(buffer, ", creation_time = %lld", fileglob_CreationTime(glob));
	lua_pushstring(L, buffer);
	sprintf(buffer, ", access_time = %lld", fileglob_AccessTime(glob));
	lua_pushstring(L, buffer);
	sprintf(buffer, ", write_time = %lld", fileglob_WriteTime(glob));
	lua_pushstring(L, buffer);
	filetime = fileglob_CreationFILETIME(glob);
	sprintf(buffer, ", creation_FILETIME = { %u, %u }", (unsigned int)(filetime & 0xffffffff), (unsigned int)(filetime >> 32));
	lua_pushstring(L, buffer);
	filetime = fileglob_AccessFILETIME(glob);
	sprintf(buffer, ", access_FILETIME = { %u, %u }", (unsigned int)(filetime & 0xffffffff), (unsigned int)(filetime >> 32));
	lua_pushstring(L, buffer);
	filetime = fileglob_WriteFILETIME(glob);
	sprintf(buffer, ", write_FILETIME = { %u, %u }", (unsigned int)(filetime & 0xffffffff), (unsigned int)(filetime >> 32));
	lua_pushstring(L, buffer);
	sprintf(buffer, ", size = %lld", fileglob_FileSize(glob));
	lua_pushstring(L, buffer);
	sprintf(buffer, ", is_directory = %s", fileglob_IsDirectory(glob) ? "true" : "false");
	lua_pushstring(L, buffer);
	sprintf(buffer, ", is_readonly = %s", fileglob_IsReadOnly(glob)? "true" : "false");
	lua_pushstring(L, buffer);
	lua_concat(L, lua_gettop(L) - top);
	return 1;
}


static int glob_create_metatable(lua_State *L) {
	luaL_newmetatable(L, FILEGLOB_METATABLE);				// metatable
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


static int l_fileglob_first(lua_State *L) {
	const char* pattern = luaL_checkstring(L, 1);
	*(void **)(lua_newuserdata(L, sizeof(void *))) = fileglob_Create(pattern);

	luaL_getmetatable(L, FILEGLOB_METATABLE);
	lua_setmetatable(L, -2);

	return 1;
}


static int l_fileglob_matchiter(lua_State* L) {
	struct _fileglob* glob = *(struct _fileglob**)(lua_touserdata(L, lua_upvalueindex(1)));

	if (!fileglob_Next(glob))
		return 0;

	lua_pushvalue(L, lua_upvalueindex(1));
	return 1;
}


static int l_fileglob_glob(lua_State* L) {
	l_fileglob_first(L);
	lua_pushcclosure(L, l_fileglob_matchiter, 1);
	return 1;
}


static const struct luaL_reg filefind_lib[] = {
	{ "first", l_filefind_first },
	{ "match", l_filefind_match },
	{ "glob", l_fileglob_glob },
	{NULL, NULL},
};


int luaopen_filefind(lua_State* L) {
	luaL_register(L, "filefind", filefind_lib);
	filefind_create_metatable(L);
	glob_create_metatable(L);
	return 1;
}


