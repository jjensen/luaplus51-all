#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#include <windows.h>
#include <io.h>
#include <malloc.h>
#include <direct.h>
#include <sys/utime.h>

#else
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <utime.h>
#endif /* _WIN32 */

#include "pusherror.h"

#if LUA_VERSION_NUM >= 502
#define luaL_register(a, b, c) luaL_setfuncs(a, c, 0)
#endif

/*
 * "os.path" API implementation
 */
int path_create(const char* inPath)
{
#if defined(_WIN32)
  char path[MAX_PATH];
#else
  char path[FILENAME_MAX];
#endif
  char* pathPtr = path;
  char* startOfPathPtr = path;
  char ch;

  if (*inPath == '/'  ||  *inPath == '\\') {
    *pathPtr++ = *inPath;
    inPath++;           // Skip the initial /
#if defined(_WIN32)
    if (*inPath == '/'  ||  *inPath == '\\') {
      // UNC share
      *pathPtr++ = *inPath;
      inPath++;         // Skip the initial /
      // Copy the machine name.
      while ( (ch = *inPath++) ) {
        *pathPtr++ = ch;
        if (ch == '/'  ||  ch == '\\')
          break;
      }
      // Copy the share name.
      while ( (ch = *inPath++) ) {
        *pathPtr++ = ch;
        if (ch == '/'  ||  ch == '\\')
          break;
      }
      startOfPathPtr = pathPtr;
    }
#endif
  }

  /* Copy the rest of the path into a buffer we can modify. */
  while ((ch = *inPath++)) {
    if (ch == '/'  ||  ch == '\\') {
#if defined(_WIN32)
      *pathPtr++ = '\\';
#else
      *pathPtr++ = '/';
#endif /* _WIN32 */
    } else
      *pathPtr++ = ch;
  }
  *pathPtr = 0;

  /* Determine which directories already exist. */
  --pathPtr;
  while (pathPtr > startOfPathPtr) {
    struct stat fileInfo;
    char ch;
    while (pathPtr > startOfPathPtr  &&  *pathPtr != '/'  &&  *pathPtr != '\\')
      --pathPtr;
    ch = *pathPtr;
    if (ch == '/'  ||  ch == '\\') {
      *pathPtr = 0;
#if defined(_WIN32)
      if (pathPtr > path + 1) {
        if (pathPtr[-1] == ':') {
          *pathPtr = ch;
          break;
        }
      }
#endif /* _WIN32 */
      if (stat(path, &fileInfo) == 0) {
#if defined(_WIN32)
        if (fileInfo.st_mode & _S_IFDIR) {
#else
        if (S_ISDIR(fileInfo.st_mode)) {
#endif // defined(_WIN32)
          *pathPtr = ch;
          break;
        } else {
          return 0;
        }
      }
      *pathPtr = ch;
    }
    --pathPtr;
  }

  ++pathPtr;

  /* Create any remaining directories. */
  while ((ch = *pathPtr)) {
    if (ch == '/'  ||  ch == '\\') {
      *pathPtr = 0;
#if defined(_WIN32)
      if (!CreateDirectory(path, NULL)  &&  GetLastError() != ERROR_ALREADY_EXISTS)
        return 0;
      *pathPtr++ = '\\';
#else
      if (mkdir(path, 0777)  &&  errno != EEXIST)
        return 0;
      *pathPtr++ = '/';
#endif
    } else {
      ++pathPtr;
    }
  }

  return 1;
}


int path_destroy(const char* inDirName)
{
  int ret = 1;
#if defined(_WIN32)
  char dirName[MAX_PATH];
  char* dirNamePtr = dirName;
  char ch;
  WIN32_FIND_DATA fd;
  HANDLE handle;

  if (*inDirName == 0)
    return ret;

  while (ch = *inDirName++) {
    if (ch == '/'  ||  ch == '\\')
      *dirNamePtr++ = '\\';
    else
      *dirNamePtr++ = ch;
  }
  if (dirNamePtr[-1] != '\\')
    *dirNamePtr++ = '\\';
  *dirNamePtr = 0;

  strcpy(dirNamePtr, "*.*");

  handle = FindFirstFile(dirName, &fd);
  *dirNamePtr = 0;

  if (handle != INVALID_HANDLE_VALUE) {
    do {
      if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        int skipDir = fd.cFileName[0] == '.'  &&
          (fd.cFileName[1] == 0  ||  (fd.cFileName[1] == '.'  &&  fd.cFileName[2] == 0));
        if (!skipDir) {
          strcpy(dirNamePtr, fd.cFileName);
          strcat(dirNamePtr, "\\");
          if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
            if (!path_destroy(dirName)) {
              ret = 0;
            }
          } else {
            if (!RemoveDirectory(dirNamePtr))
              ret = 0;
          }
        }
      } else {
        strcpy(dirNamePtr, fd.cFileName);
        SetFileAttributes(dirName, FILE_ATTRIBUTE_ARCHIVE);
        if (!DeleteFile(dirName)) {
          ret = 0;
        }
      }
    } while (FindNextFile(handle, &fd));

    FindClose(handle);
  }

  *--dirNamePtr = 0;
  if (!RemoveDirectory(dirName))
    ret = 0;
#else
  char dirName[FILENAME_MAX];
  char* dirNamePtr;
  char ch;

  if (*inDirName == 0)
    return ret;

  dirNamePtr = dirName;

  while ((ch = *inDirName++)) {
    if (ch == '/'  ||  ch == '\\')
      *dirNamePtr++ = '/';
    else
      *dirNamePtr++ = ch;
  }
  if (dirNamePtr[-1] != '/')
    *dirNamePtr++ = '/';
  *dirNamePtr = 0;

  DIR* dirp = opendir(dirName);
  if (dirp)
  {
    struct dirent* dp;
    while ((dp = readdir(dirp)) != NULL) {
      strcpy(dirNamePtr, dp->d_name);
      if (dp->d_type & DT_DIR) {
        int skipDir = dp->d_name[0] == '.'  &&  (dp->d_name[1] == 0  ||  (dp->d_name[1] == '.'  &&  dp->d_name[2] == 0));
        if (!skipDir) {
          strcat(dirNamePtr, "/");
          if (!path_destroy(dirName)) {
            ret = 0;
          }
        }
      } else {
        if (unlink(dirName) == -1) {
          ret = 0;
        }
      }
    }
  }

  closedir(dirp);

  *--dirNamePtr = 0;
  if (rmdir(dirName) == -1)
    ret = 0;
#endif

  return ret;
}


/* pathname access */
int ospath_access(lua_State *L)
{
  const char *pathname = luaL_checkstring(L, 1);
  const char *type = luaL_optstring(L, 2, "");

  int amode = 0;
  if (strcmp(type, "w") == 0)
    amode = 2;
  else if (strcmp(type, "r") == 0)
    amode = 4;
  else if (strcmp(type, "rw") == 0)
    amode = 6;

  lua_pushboolean(L, access(pathname, amode) == 0);
  return 1;
}


/* -- pathname/nil error */
static int ospath_getcwd(lua_State *L)
{
#if defined(_WIN32)
  char pathname[MAX_PATH + 1];
  size_t len = GetCurrentDirectory(sizeof pathname, pathname);
  if (len == 0) return push_error(L);
  lua_pushlstring(L, pathname, len);
  return 1;
#else
  char pathname[PATH_MAX + 1];
  if (!getcwd(pathname, sizeof pathname))
    return push_error(L);
  lua_pushstring(L, pathname);
  return 1;
#endif
}

/* pathname -- true/nil error */
static int ospath_chdir(lua_State *L)
{
  const char *pathname = luaL_checkstring(L, 1);
#if defined(_WIN32)
  if (!SetCurrentDirectory(pathname))
    return push_error(L);
#else
  if (-1 == chdir(pathname))
    return push_error(L);
#endif
  lua_pushboolean(L, 1);
  return 1;
}

/* pathname -- true/nil error */
static int ospath_chmod(lua_State *L)
{
  const char *pathname = luaL_checkstring(L, 1);
#if defined(_WIN32)
  const char *inmode = luaL_checkstring(L, 2);
  int pmode = 0;
  while (*inmode) {
    switch (*inmode) {
      case 'r':
        pmode |= _S_IREAD;
        break;
      case 'w':
        pmode |= _S_IWRITE;
        break;
    }
    ++inmode;
  }
  if (-1 == _chmod(pathname, pmode))
    return push_error(L);
#else
  int inmode = luaL_checkinteger(L, 2);
  int mode = (((inmode / 100) % 10) * 64) + (((inmode / 10) % 10) * 8) +
	(inmode % 10);
  if (-1 == chmod(pathname, mode))
    return push_error(L);
#endif
  lua_pushboolean(L, 1);
  return 1;
}

/* pathname -- true/nil error */
static int ospath_remove(lua_State *L)
{
  const char *pathname = luaL_checkstring(L, 1);
#if defined(_WIN32)  ||  defined(_WIN64)
  DWORD attr = GetFileAttributes(pathname);
  if (attr == (DWORD)-1)
    return push_error(L);
  if (attr & FILE_ATTRIBUTE_DIRECTORY) {
    if (!(attr & FILE_ATTRIBUTE_REPARSE_POINT)) {
      if (!path_destroy(pathname))
          return push_error(L);
    } else {
      if (!RemoveDirectory(pathname))
        return push_error(L);
    }

  } else {
    SetFileAttributes(pathname, FILE_ATTRIBUTE_ARCHIVE);
    if (!DeleteFile(pathname))
      return push_error(L);
  }
  lua_pushboolean(L, 1);
  return 1;
#else
  struct stat attr;
  if (stat(pathname, &attr) == -1)
    return push_error(L);
  if (attr.st_mode & S_IFDIR) {
    if (!path_destroy(pathname))
      return push_error(L);
  } else {
    if (remove(pathname) == -1)
      return push_error(L);
  }
  lua_pushboolean(L, 1);
  return 1;
#endif
}


static int ospath_copyfile(lua_State *L)
{
#if defined(_WIN32)  ||  defined(_WIN64)
  const char *srcfilename = luaL_checkstring(L, 1);
  const char *destfilename = luaL_checkstring(L, 2);
  lua_pushboolean(L, CopyFile(srcfilename, destfilename, FALSE) != FALSE);
  return 1;
#else
  int inputFile;
  int outputFile;

  // Operate in 64k buffers.
  const size_t BUFFER_SIZE = 64 * 1024;
  unsigned char* buffer;
  
  const char *srcfilename = luaL_checkstring(L, 1);
  const char *destfilename = luaL_checkstring(L, 2);
  
  ssize_t fileSize;

    inputFile = open(srcfilename, O_RDONLY);
  if (inputFile == -1) {
    lua_pushboolean(L, 0);
    return 1;
  }
    outputFile = open(destfilename, O_CREAT | O_TRUNC | O_WRONLY, 0666);
  if (outputFile == -1) {
    close(inputFile);
    lua_pushboolean(L, 0);
    return 1;
  }

  // Allocate the buffer space.
  buffer = malloc(BUFFER_SIZE);
  
  // Get the source file's size.
  fileSize = lseek(inputFile, 0, SEEK_END);
  lseek(inputFile, 0, SEEK_SET);
  
  // Keep copying until there is no more file left to copy.
  int ret = 1;
  while (fileSize > 0)
  {
    // Copy the minimum of BUFFER_SIZE or the fileSize.
        ssize_t readSize = BUFFER_SIZE < fileSize ? BUFFER_SIZE : fileSize;
    if (read(inputFile, buffer, readSize) != readSize) {
      ret = 0;
      break;
    }
    if (write(outputFile, buffer, readSize) != readSize) {
      ret = 0;
      break;
    }
    fileSize -= readSize;
  }
  
  close(outputFile);
  close(inputFile);

  lua_pushboolean(L, ret);
  return 1;
#endif
}


static int ospath_movefile(lua_State *L)
{
  const char *srcfilename = luaL_checkstring(L, 1);
  const char *destfilename = luaL_checkstring(L, 2);
#if defined(_WIN32)  ||  defined(_WIN64)
  lua_pushboolean(L, MoveFile(srcfilename, destfilename) != FALSE);
  return 1;
#else
  return 0;
#endif
}


/*
** Set access time and modification values for file
*/
static int ospath_touch(lua_State *L)
{
  const char *file = luaL_checkstring(L, 1);
  struct utimbuf utb, *buf;

  if (lua_gettop(L) == 1) /* set to current date/time */
    buf = NULL;
  else
  {
    utb.actime = (time_t)luaL_optnumber(L, 2, 0);
    utb.modtime = (time_t)luaL_optnumber(L, 3, (lua_Number)utb.actime);
    buf = &utb;
  }
  if (utime(file, buf))
  {
    lua_pushnil(L);
    lua_pushfstring(L, "%s", strerror(errno));
    return 2;
  }
  lua_pushboolean(L, 1);
  return 1;
}

/* pathname -- true/nil error */
int ospath_mkdir(lua_State *L)
{
  const char *pathname = luaL_checkstring(L, 1);
  if (!path_create(pathname))
    return push_error(L);
  lua_pushboolean(L, 1);
  return 1;
}


static int ospath_hardlink(lua_State *L)
{
  const char *hardlinkFilename = luaL_checkstring(L, 1);
  const char *targetFilename = luaL_checkstring(L, 2);
#if defined(_WIN32)  ||  defined(_WIN64)
  if (CreateHardLink(hardlinkFilename, targetFilename, NULL) == FALSE)
    return push_error(L);
  lua_pushboolean(L, 1);
  return 1;
#else
  return 0;
#endif
}


static lua_Integer luaL_checkboolean (lua_State *L, int narg) {
	lua_Integer d = lua_toboolean(L, narg);
	if (d == 0 && !lua_isboolean(L, narg)) {  /* avoid extra test when d is not 0 */
		const char *msg = lua_pushfstring(L, "%s expected, got %s",
				luaL_typename(L, narg), lua_typename(L, LUA_TBOOLEAN));
		return luaL_argerror(L, narg, msg);
	}
	return d;
}


static int ospath_symboliclink(lua_State *L)
{
  const char *symlinkFilename = luaL_checkstring(L, 1);
  const char *targetFilename = luaL_checkstring(L, 2);
  lua_Integer is_directory = luaL_checkboolean(L, 3);
#if defined(_WIN32)  ||  defined(_WIN64)
  if (CreateSymbolicLink(symlinkFilename, targetFilename, is_directory ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0) == FALSE)
    return push_error(L);
  lua_pushboolean(L, 1);
  return 1;
#else
  return 0;
#endif
}


static int path_append_helper(lua_State *L, int stackStart, int stackTop) {
  int concatBottom = stackTop;
  int newTop;
  int i;
  for (i = stackStart; i <= stackTop; ++i) {
    const char *path = luaL_checkstring(L, i);
    if (path[0] > 0) {
      if (*path == '/'  ||  *path == '\\'  ||  (path[0]  &&  path[1] == ':'))
        concatBottom = lua_gettop(L);

      lua_checkstack(L, 2);
      lua_pushvalue(L, i);
      lua_pushliteral(L, "/");
    }
  }

  newTop = lua_gettop(L);
  if (newTop - concatBottom > 0) {
    lua_remove(L, lua_gettop(L));
    --newTop;
  }
  lua_concat(L, newTop - concatBottom);
  return 1;
}


static int path_simplify_helper(lua_State *L, int index) {
  const char *path = luaL_checkstring(L, index);
  const char *pathEnd = path + strlen(path);
  char *file;
  char *filestart, *fileorg;

  filestart = fileorg = malloc(pathEnd - path + 1);
  memcpy(fileorg, path, pathEnd - path);
  fileorg[pathEnd - path] = 0;

  {
    char *ptr = filestart;
    char *endptr = filestart + (pathEnd - path);
    file = filestart;
    while ( ptr != endptr ) {
      // Skip './'
      if ( *ptr == '.' ) {
        if ( ptr[1] == 0  ||  ptr[1] == '/'  ||  ptr[1] == '\\' ) {
          int add = ptr[1] ? 1 : 0;
          ptr += 1 + add;
          while ( ptr != endptr  &&  ( *ptr == '/'  ||  *ptr == '\\' ) )
              ++ptr;
          if ( file == filestart ) {
            file += 1 + add;
            filestart += 1 + add;
          }
        } else if ( ptr[1] == '.'  &&  ( ptr[2] == 0  ||  ptr[2] == '/'  ||  ptr[2] == '\\' ) ) {
          // Go up a subdirectory.
          int add = ptr[2] ? 1 : 0;
          ptr += 2 + add;
          while ( ptr != endptr  &&  ( *ptr == '/'  ||  *ptr == '\\' ) )
              ++ptr;
          if ( file != filestart ) {
            file--;
            file -= (*file == '/' ? 1 : 0);
            if ( file - filestart == 1  &&  *file == ':' )
              file += 2;
            else {
              while ( file >= filestart  &&  ( *file != '/'  &&  *file != '\\' ) )
                file--;
              file++;
            }
          } else {
            file += 2 + add;
//            filestart += 2 + add;
          }
        } else {
          *file++ = *ptr++;
        }
      } else if ( *ptr == '\\'  ||  *ptr == '/' ) {
        if ( file > filestart  &&  ( file[-1] == '/'  ||  file[-1] == '\\' ) ) {
          ptr++;
        } else {
          /* is it a unc path? */
          if ( file == filestart  &&  (file[0] == '\\'  ||  file[0] == '/')  &&  (file[1] == '\\'  ||  file[1] == '/')) {
            file += 2;
            ptr += 2;
          } else {
            *file++ = '/';
            ptr++;
          }
        }
      } else {
        *file++ = *ptr++;
      }
    }
  }
  *file = 0;
//	file--;
//	if ( *file == '/' )
//    *file = 0;

  lua_pushstring(L, filestart);
  free(fileorg);
  return 1;
}


/**
    \internal
    \author Jack Handy

    Borrowed from http://www.codeproject.com/string/wildcmp.asp.
    Modified by Joshua Jensen.
 **/
int WildMatch( const char* pattern, const char *string, int caseSensitive )
{
  const char* mp;
  const char* cp;

  // Handle all the letters of the pattern and the string.
  while ( *string != 0  &&  *pattern != '*' ) {
    if ( *pattern != '?' ) {
      if ( caseSensitive ) {
        if ( *pattern != *string )
          return 0;
      } else {
        if ( toupper( *pattern ) != toupper( *string ) )
          return 0;
      }
    }

    pattern++;
    string++;
  }

  mp = NULL;
  cp = NULL;

  while ( *string != 0 ) {
    if (*pattern == '*') {
      // It's a match if the wildcard is at the end.
      if ( *++pattern == 0 )
        return 1;

      mp = pattern;
      cp = string + 1;
    } else {
      if ( caseSensitive ) {
        if ( *pattern == *string  ||  *pattern == '?' ) {
          pattern++;
          string++;
        } else {
          pattern = mp;
          string = cp++;
        }
      } else {
        if ( toupper( *pattern ) == toupper( *string )  ||  *pattern == '?' ) {
          pattern++;
          string++;
        } else {
          pattern = mp;
          string = cp++;
        }
      }
    }
  }

  // Collapse remaining wildcards.
  while ( *pattern == '*' )
    pattern++;

  return !*pattern;
}


static int ospath_add_slash(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);
  if (path[0] == 0) {
    lua_pushstring(L, "/");
    return 1;
  }
  lua_pushvalue(L, 1);
  if (pathEnd[-1] != '\\'  &&  pathEnd[-1] != '/') {
    lua_pushliteral(L, "/");
    lua_concat(L, 2);
    return 1;
  }
  return 1;
}


static int ospath_add_extension(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *ext = luaL_checkstring(L, 2);
  (void)path; /* avoid warning about unused parameter */
  lua_pushvalue(L, 1);
  if (*ext != '.')
    lua_pushliteral(L, ".");
  lua_pushvalue(L, 2);
  lua_concat(L, 2 + (*ext == '.' ? 0 : 1));
  return 1;
}


static int ospath_append(lua_State *L) {
  return path_append_helper(L, 1, lua_gettop(L));
}


static int ospath_escape(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *ptr = path;

#if defined(_WIN32)
  while (*ptr  &&  *ptr != ' '  &&  *ptr != '/')
    ++ptr;

  if (*ptr == ' '  ||  *ptr == '/') {
    lua_pushliteral(L, "\"");
    lua_pushvalue(L, 1);
    lua_pushliteral(L, "\"");
    lua_concat(L, 3);
  } else
    lua_pushvalue(L, 1);
#else
  luaL_Buffer b;
  luaL_buffinit(L, &b);

  while (*ptr) {
    if (*ptr == ' '  ||  *ptr == '\\')
      luaL_addchar(&b, '\\');
    luaL_addchar(&b, *ptr);
    ++ptr;
  }

  luaL_pushresult(&b);
#endif
  return 1;
}


static int ospath_exists(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  lua_pushboolean(L, access(path, 0) != -1);
  return 1;
}


static int ospath_find_extension(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);

  while (pathEnd > path  &&  *(pathEnd - 1) != '.'  &&  *(pathEnd - 1) != '\\'  &&  *(pathEnd - 1) != '/'  &&  *(pathEnd - 1) != ':')
    --pathEnd;

  if (*(pathEnd - 1) == '.')
    lua_pushinteger(L, pathEnd - path);
  else
    lua_pushnil(L);

  return 1;
}


static int ospath_find_filename(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);

  while (pathEnd > path  &&  *(pathEnd - 1) != '\\'  &&  *(pathEnd - 1) != '/'  &&  *(pathEnd - 1) != ':')
    --pathEnd;

  lua_pushinteger(L, pathEnd - path + 1);
  return 1;
}


static int ospath_get_extension(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);

  while (pathEnd > path  &&  *(pathEnd - 1) != '.'  &&  *(pathEnd - 1) != '\\'  &&  *(pathEnd - 1) != '/'  &&  *(pathEnd - 1) != ':')
    --pathEnd;

  if (*(pathEnd - 1) == '.')
    lua_pushstring(L, pathEnd - 1);
  else
    lua_pushliteral(L, "");

  return 1;
}


static int ospath_get_filename(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);

  while (pathEnd > path  &&  *(pathEnd - 1) != '\\'  &&  *(pathEnd - 1) != '/'  &&  *(pathEnd - 1) != ':')
    --pathEnd;

  lua_pushstring(L, pathEnd);
  return 1;
}


static int ospath_is_directory(lua_State *L) {
  char *trimmedPath;
  struct stat sbuff;
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);

  while (pathEnd > path  &&  (*(pathEnd - 1) == '/'  ||  *(pathEnd - 1) == '\\'))
    --pathEnd;

  trimmedPath = malloc(pathEnd - path + 1);
  memcpy(trimmedPath, path, pathEnd - path);
  trimmedPath[pathEnd - path] = 0;

  if (trimmedPath[0]  &&  stat(trimmedPath, &sbuff) == 0)
#if defined(_WIN32)
    lua_pushboolean(L, (sbuff.st_mode & _S_IFDIR) > 0);
#else
    lua_pushboolean(L, S_ISDIR(sbuff.st_mode));
#endif
  else
    lua_pushboolean(L, 0);

  free(trimmedPath);

  return 1;
}


static int ospath_is_file(lua_State *L) {
  struct stat sbuff;
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);
  if (*(pathEnd - 1) == '/'  ||  *(pathEnd - 1) == '\\')
    lua_pushnil(L);
  else if (path[0]  &&  stat(path, &sbuff) == 0)
#if defined(_WIN32)
    lua_pushboolean(L, (sbuff.st_mode & _S_IFDIR) == 0);
#else
    lua_pushboolean(L, !S_ISDIR(sbuff.st_mode));
#endif
  else
    lua_pushnil(L);
  return 1;
}


static int ospath_is_relative(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  if (*path == 0)
    lua_pushboolean(L, 1);
  else if (*path == '/'  ||  *path == '\\')
    lua_pushboolean(L, 0);
  else if (*(path + 1) == ':')
    lua_pushboolean(L, 0);
  else
    lua_pushboolean(L, 1);
  return 1;
}


static int ospath_is_root(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  if (*path == 0)
    lua_pushboolean(L, 0);
  else if (*path == '/'  ||  *path == '\\')
    lua_pushboolean(L, 1);
  else if (*(path + 1) == ':')
    lua_pushboolean(L, 1);
  else
    lua_pushboolean(L, 0);
  return 1;
}


static int ospath_is_unc(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);

  if (*path == 0)
    lua_pushboolean(L, 0);
  else if (strlen(path) < 3)
    lua_pushboolean(L, 0);
  else if ((path[0] == '/'  ||  path[0] == '\\')  &&  (path[1] == '/'  ||  path[1] == '\\'))
    lua_pushboolean(L, 1);
  else
    lua_pushboolean(L, 0);

  return 1;
}


static int ospath_is_writable(lua_State *L) {
  struct stat sbuff;
  const char *path = luaL_checkstring(L, 1);
  if (path[0]  &&  stat(path, &sbuff) == 0)
#if defined(_WIN32)
    lua_pushboolean(L, (sbuff.st_mode & _S_IWRITE) == _S_IWRITE);
#else
    lua_pushboolean(L, (sbuff.st_mode & S_IWRITE) == S_IWRITE);
#endif
  else
    lua_pushboolean(L, 0);
  return 1;
}


static int ospath_join(lua_State *L) {
  ospath_append(L);
  return path_simplify_helper(L, -1);
}


static int ospath_match(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *spec = luaL_checkstring(L, 2);
  int caseSensitive = lua_isboolean(L, 3) ? lua_toboolean(L, 3) : 0;

  lua_pushboolean(L, WildMatch(spec, path, caseSensitive));
  return 1;
}


static int ospath_make_absolute(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *cwd = getcwd(NULL, 0);
  int start = lua_gettop(L);
  (void)path; /* avoid warning about unused parameter */

  lua_pushstring(L, cwd);
  lua_pushvalue(L, 1);
  path_append_helper(L, start, start + 2);
  return path_simplify_helper(L, -1);
}


static int ospath_make_backslash(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);

  char *newPath = malloc(pathEnd - path + 1);
  char *ptr = newPath;
  while (*path) {
    if (*path == '/') {
      *ptr++ = '\\';
      ++path;
    } else
      *ptr++ = *path++;
  }
  *ptr = 0;

  lua_pushstring(L, newPath);
  free(newPath);
  return 1;
}


static int ospath_make_slash(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);

  char *newPath = malloc(pathEnd - path + 1);
  char *ptr = newPath;
  while (*path) {
    if (*path == '\\') {
      *ptr++ = '/';
      ++path;
    } else
      *ptr++ = *path++;
  }
  *ptr = 0;

  lua_pushstring(L, newPath);
  free(newPath);
  return 1;
}


static int ospath_make_writable(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);

#if defined(_WIN32)
  const char *pathname = luaL_checkstring(L, 1);
  int pmode = _S_IWRITE;
  if (-1 == _chmod(pathname, pmode))
    return push_error(L);
  lua_pushboolean(L, 1);
#else
  const char *pathname = luaL_checkstring(L, 1);
  int pmode = 0666;
  if (-1 == chmod(pathname, pmode))
    return push_error(L);
  lua_pushboolean(L, 1);
#endif
  return 1;
}


static int ospath_replace_extension(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);
  const char *ext = luaL_checkstring(L, 2);

  /* Search for extension */
  const char *ptr = pathEnd - 1;
  while (*ptr  &&  *ptr != '.')
    --ptr;
  if (*ptr == '.')
    pathEnd = ptr;

  lua_pushlstring(L, path, pathEnd - path);
  if (*ext != '.')
    lua_pushliteral(L, ".");
  lua_pushvalue(L, 2);
  lua_concat(L, 2 + (*ext == '.' ? 0 : 1));
  return 1;
}


static int ospath_remove_directory(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);

  while (pathEnd > path  &&  *(pathEnd - 1) != '\\'  &&  *(pathEnd - 1) != '/'  &&  *(pathEnd - 1) != ':')
    --pathEnd;

  if (path == pathEnd)
    lua_pushstring(L, path);
  else
    lua_pushstring(L, pathEnd);
  return 1;
}


static int ospath_remove_extension(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);

  /* Search for extension */
  const char *ptr = pathEnd - 1;
  while (*ptr  &&  *ptr != '.')
    --ptr;
  if (*ptr == '.')
    pathEnd = ptr;

  lua_pushlstring(L, path, pathEnd - path);
  return 1;
}


static int ospath_remove_filename(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);

  while (pathEnd > path  &&  *(pathEnd - 1) != '\\'  &&  *(pathEnd - 1) != '/'  &&  *(pathEnd - 1) != ':')
    --pathEnd;

  lua_pushlstring(L, path, pathEnd - path);
  return 1;
}


static int ospath_remove_slash(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);

  while (pathEnd > path  &&  (*(pathEnd - 1) == '\\'  ||  *(pathEnd - 1) == '/'))
    --pathEnd;

  lua_pushlstring(L, path, pathEnd - path);
  return 1;
}


static int ospath_simplify(lua_State *L) {
  return path_simplify_helper(L, 1);
}


static int ospath_split(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);

  while (pathEnd > path  &&  *(pathEnd - 1) != '\\'  &&  *(pathEnd - 1) != '/'  &&  *(pathEnd - 1) != ':')
    --pathEnd;

  lua_pushlstring(L, path, pathEnd - path);
  lua_pushstring(L, pathEnd);
  return 2;
}


static int ospath_trim(lua_State *L) {
  const char *pathStart = luaL_checkstring(L, 1);
  const char *pathEnd;

  /* trim the front */
  while (*pathStart == ' '  ||  *pathStart == '\t')
    ++pathStart;

  /* trim the end */
  pathEnd = pathStart + strlen(pathStart);
  while (pathEnd - 1 > pathStart  &&  (*(pathEnd - 1) == ' '  ||  *(pathEnd - 1) == '\t'))
    --pathEnd;

  lua_pushlstring(L, pathStart, pathEnd - pathStart);
  return 1;
}


static int ospath_unescape(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);

#if defined(_WIN32)
  if (path[0] == '"') {
    const char* lastquote = path + 1;
    while (*lastquote  &&  *lastquote != '"')
      ++lastquote;
    lua_pushlstring(L, path + 1, lastquote - (path + 1));
  } else {
    lua_pushvalue(L, 1);
  }
#else
  const char *ptr = path;
  luaL_Buffer b;
  luaL_buffinit(L, &b);

  while (*ptr) {
    if (*ptr != '\\')
      luaL_addchar(&b, *ptr);
    ++ptr;
  }

  luaL_pushresult(&b);
#endif
  return 1;
}


#define file_handle(fp) (HANDLE)_get_osfhandle(_fileno(fp))

#define absindex(L,i) ((i)>0?(i):lua_gettop(L)+(i)+1)

static FILE *check_file(lua_State *L, int idx, const char *argname)
{
#if LUA_VERSION_NUM <= 501
  FILE **pf;
  if (idx > 0) pf = luaL_checkudata(L, idx, LUA_FILEHANDLE);
  else {
    idx = absindex(L, idx);
    pf = lua_touserdata(L, idx);
    luaL_getmetatable(L, LUA_FILEHANDLE);
    if (!pf || !lua_getmetatable(L, idx) || !lua_rawequal(L, -1, -2))
      luaL_error(L, "bad %s option (%s expected, got %s)",
                 argname, LUA_FILEHANDLE, luaL_typename(L, idx));
    lua_pop(L, 2);
  }
  if (!*pf) return luaL_error(L, "attempt to use a closed file"), NULL;
  return *pf;
#else
  luaL_Stream* p;
  idx = absindex(L, idx);
  p = (luaL_Stream *)luaL_checkudata(L, idx, LUA_FILEHANDLE);
  if (!p || !p->f) return luaL_error(L, "attempt to use a closed file"), NULL;
  return p->f;
#endif
}


static int file_lock(lua_State *L,
                     FILE *f, const char *mode, long offset, long length)
{
#if defined(_WIN32)  ||  defined(_WIN64)
  static const ULARGE_INTEGER zero_len;
  static const OVERLAPPED zero_ov;
  HANDLE h = file_handle(f);
  ULARGE_INTEGER len = zero_len;
  OVERLAPPED ov = zero_ov;
  DWORD flags = 0;
  BOOL ret;
  if (length) len.LowPart = length;
  else len.LowPart = len.HighPart = -1;
  ov.Offset = offset;
  switch (*mode) {
    case 'w':
      flags = LOCKFILE_EXCLUSIVE_LOCK;
      /*FALLTHRU*/
    case 'r':
      flags |= LOCKFILE_FAIL_IMMEDIATELY;
      ret = LockFileEx(h, flags, 0, len.LowPart, len.HighPart, &ov);
      break;
    case 'u':
      ret = UnlockFileEx(h, 0, len.LowPart, len.HighPart, &ov);
      break;
    default:
      return luaL_error(L, "invalid mode");
  }
  if (!ret)
    return push_error(L);
#else
  struct flock k;
  switch (*mode) {
    case 'w': k.l_type = F_WRLCK; break;
    case 'r': k.l_type = F_RDLCK; break;
    case 'u': k.l_type = F_UNLCK; break;
    default: return luaL_error(L, "invalid mode");
  }
  k.l_whence = SEEK_SET;
  k.l_start = offset;
  k.l_len = length;
  if (-1 == fcntl(fileno(f), F_SETLK, &k))
    return push_error(L);
#endif
  /* return the file */
  lua_settop(L, 1);
  return 1;
}


static const char *opt_mode(lua_State *L, int *pidx)
{
  if (lua_type(L, *pidx) != LUA_TSTRING)
    return "u";
  return lua_tostring(L, (*pidx)++);
}


/* file [mode] [offset [length]] -- file/nil error */
static int ospath_lock(lua_State *L)
{
  FILE *f = check_file(L, 1, NULL);
  int argi = 2;
  const char *mode = opt_mode(L, &argi);
  long offset = (long)luaL_optnumber(L, argi, 0);
  long length = (long)luaL_optnumber(L, argi + 1, 0);
  return file_lock(L, f, mode, offset, length);
}


int luaopen_ospath_core(lua_State *L) {
  const luaL_Reg ospath_lib[] = {
    {"access",            ospath_access},
    {"getcwd",            ospath_getcwd},
    {"chdir",             ospath_chdir},
    {"chmod",             ospath_chmod},
    {"mkdir",             ospath_mkdir},
    {"remove",            ospath_remove},
    {"copy_file",         ospath_copyfile},
    {"move_file",         ospath_movefile},
    {"touch",             ospath_touch},
    {"hardlink",          ospath_hardlink},
    {"symboliclink",      ospath_symboliclink},
    {"lock",              ospath_lock},
    {"unlock",            ospath_lock},

    {"add_slash",         ospath_add_slash},
    {"add_extension",     ospath_add_extension},
    {"append",            ospath_append},
    {"escape",            ospath_escape},
    {"exists",            ospath_exists},
    {"find_extension",    ospath_find_extension},
    {"find_filename",     ospath_find_filename},
    {"get_extension",     ospath_get_extension},
    {"get_filename",      ospath_get_filename},
    {"is_directory",      ospath_is_directory},
    {"is_file",           ospath_is_file},
    {"is_relative",       ospath_is_relative},
    {"is_root",           ospath_is_root},
    {"is_unc",            ospath_is_unc},
    {"is_writable",       ospath_is_writable},
    {"join",              ospath_join},
    {"make_absolute",     ospath_make_absolute},
    {"make_backslash",    ospath_make_backslash},
    {"make_slash",        ospath_make_slash},
    {"make_writable",     ospath_make_writable},
    {"match",             ospath_match},
    {"remove_directory",  ospath_remove_directory},
    {"remove_extension",  ospath_remove_extension},
    {"remove_filename",   ospath_remove_filename},
    {"remove_slash",      ospath_remove_slash},
    {"replace_extension", ospath_replace_extension},
    {"simplify",          ospath_simplify},
    {"split",             ospath_split},
    {"trim",              ospath_trim},
    {"unescape",          ospath_unescape},
    {0,0} };

  lua_newtable(L);
  luaL_register(L, NULL, ospath_lib);
  return 1;
}
