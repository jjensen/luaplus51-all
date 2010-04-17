#include "LuaPlus/LuaPlus.h"
#include "LuaPlus/src/lauxlib.h"
#include <shlwapi.h>
#include <windows.h>
#include <io.h>
#include <time.h>

using namespace LuaPlus;

#pragma comment(lib, "shlwapi.lib")

bool PathCreate(LPCSTR inPath)
{
	char path[MAX_PATH];
	char* pathPtr = path;

	if ((inPath[0] == '\\'  ||  inPath[0] == '/')  &&  (inPath[1] == '\\'  ||  inPath[1] == '/'))
	{
		*pathPtr++ = '\\';
		*pathPtr++ = '\\';
		inPath += 2;
		while (char ch = *inPath++)
		{
			*pathPtr++ = ch;
			if (ch == '/'  ||  ch == '\\')
				break;
		}
	}

	while (char ch = *inPath++)
	{
		if (ch == '/'  ||  ch == '\\')
		{
			*pathPtr = 0;
			if (!::CreateDirectory(path, NULL)  &&  (::GetLastError() != ERROR_ALREADY_EXISTS  &&  ::GetLastError() != ERROR_ACCESS_DENIED))
				return false;
			*pathPtr++ = '\\';
		}
		else
			*pathPtr++ = ch;
	}

	return true;
}


bool PathDestroy(LPCSTR inDirName)
{
	if (*inDirName == 0)
		return true;

	char dirName[MAX_PATH];
	char* dirNamePtr = dirName;
	while (char ch = *inDirName++)
	{
		if (ch == '/'  ||  ch == '\\')
			*dirNamePtr++ = '\\';
		else
			*dirNamePtr++ = ch;
	}
	if (dirNamePtr[-1] != '\\')
		*dirNamePtr++ = '\\';
	*dirNamePtr = 0;

	strcpy(dirNamePtr, "*.*");

	WIN32_FIND_DATA fd;
	HANDLE handle = ::FindFirstFile(dirName, &fd);
	if (handle == INVALID_HANDLE_VALUE)
		return false;

	*dirNamePtr = 0;

	do
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			bool skipDir = fd.cFileName[0] == '.'  &&
				(fd.cFileName[1] == 0  ||  (fd.cFileName[1] == '.'  &&  fd.cFileName[2] == 0));
			if (!skipDir)
			{
				strcpy(dirNamePtr, fd.cFileName);
				strcat(dirNamePtr, "\\");
				if (!PathDestroy(dirName))
					return false;
			}
		}
		else
		{
			strcpy(dirNamePtr, fd.cFileName);
			::SetFileAttributes(dirName, FILE_ATTRIBUTE_ARCHIVE);
			if (!::DeleteFile(dirName))
			{
				FindClose(handle);
				return false;
			}
		}
	} while (::FindNextFile(handle, &fd));

	FindClose(handle);

	*dirNamePtr = 0;
	if (!::RemoveDirectory(dirName))
		return false;

	return true;
}


static int LS_GetCurrentDirectory(LuaState* state)
{
	char path[MAX_PATH];
	if (::GetCurrentDirectory(MAX_PATH, path) == FALSE)
		return 0;

	state->PushString(path);
	return 1;
}


static int LS_SetCurrentDirectory(LuaState* state)
{
	LPCSTR path = luaL_checkstring(*state, 1);
	if (!::SetCurrentDirectory(path))
		return 0;
	state->PushBoolean(true);
	return 1;
}


static int LS_PathAddBackslash(LuaState* state)
{
	char lpszPath[MAX_PATH];
	strcpy(lpszPath, luaL_checkstring(*state, 1));
	::PathAddBackslash(lpszPath);
	state->PushString(lpszPath);
    return 1;
}


static int LS_PathAddExtension(LuaState* state)
{
	char pszPath[MAX_PATH];
	strcpy(pszPath, luaL_checkstring(*state, 1));
	if (!::PathAddExtension(pszPath, luaL_checkstring(*state, 2)))
		return 0;
	state->PushString(pszPath);
    return 1;
}


static int LS_PathAppend(LuaState* state)
{
	char pszPath[MAX_PATH];
	strcpy(pszPath, luaL_checkstring(*state, 1));
	if (!::PathAppend(pszPath, luaL_checkstring(*state, 2)))
		return 0;
	state->PushString(pszPath);
    return 1;
}


static int LS_PathBuildRoot(LuaState* state)
{
	char szRoot[MAX_PATH];
	strcpy(szRoot, luaL_checkstring(*state, 1));
	::PathBuildRoot(szRoot, (int)luaL_checkint(*state, 2));
	state->PushString(szRoot);
    return 1;
}


static int LS_PathCanonicalize(LuaState* state)
{
	char lpszDest[MAX_PATH];
	if (!::PathCanonicalize(lpszDest, luaL_checkstring(*state, 1)))
		return 0;
	state->PushString(lpszDest);
    return 1;
}


static int LS_PathCombine(LuaState* state)
{
	char lpszDest[MAX_PATH];
	if (!::PathCombine(lpszDest, luaL_checkstring(*state, 1), luaL_checkstring(*state, 2)))
		return 0;
	state->PushString(lpszDest);
    return 1;
}


static int LS_PathCommonPrefix(LuaState* state)
{
	char pszPath[MAX_PATH];
	state->PushInteger(::PathCommonPrefix(luaL_checkstring(*state, 1), luaL_checkstring(*state, 2), pszPath));
	state->PushString(pszPath);
    return 1;
}


static int LS_PathCompactPathEx(LuaState* state)
{
	char pszOut[MAX_PATH];
	if (!::PathCompactPathEx(pszOut, luaL_checkstring(*state, 1), (UINT)luaL_checkint(*state, 2), 0))
		return 0;
	state->PushString(pszOut);
    return 1;
}


static int LS_PathCreateFromURL(LuaState* state)
{
	char pszPath[MAX_PATH];
	DWORD cchPath;
	if (FAILED(::PathCreateFromUrl(luaL_checkstring(*state, 1), pszPath, &cchPath, NULL)))
		return 0;
	state->PushString(pszPath);
    return 1;
}


static int LS_PathFileExists(LuaState* state)
{
	state->PushBoolean(::PathFileExists(luaL_checkstring(*state, 1)) != FALSE);
    return 1;
}


static int LS_PathFindExtension(LuaState* state)
{
	LPCSTR pszPath = luaL_checkstring(*state, 1);
	LPTSTR pszFound = ::PathFindExtension(luaL_checkstring(*state, 1));
	if (!pszFound)
		return 0;
	state->PushInteger((pszFound - pszPath) + 1);
    return 1;
}


static int LS_PathFindFileName(LuaState* state)
{
	LPCSTR pszPath = luaL_checkstring(*state, 1);
	LPTSTR pszFound = ::PathFindFileName(pszPath);
	if (!pszFound)
		return 0;
	state->PushInteger((pszFound - pszPath) + 1);
    return 1;
}


static int LS_PathFindNextComponent(LuaState* state)
{
	LPCSTR pszPath = luaL_checkstring(*state, 1);
	LPTSTR pszFound = ::PathFindNextComponent(luaL_checkstring(*state, 1));
	if (!pszFound)
		return 0;
	state->PushInteger((pszFound - pszPath) + 1);
    return 1;
}


static int LS_PathFindOnPath(LuaState* state)
{
	char pszFile[MAX_PATH];
	strcpy(pszFile, luaL_checkstring(*state, 1));
	// Do the table.
	if (!::PathFindOnPath(pszFile, NULL))
		return 0;
	state->PushString(pszFile);
    return 1;
}


static int LS_PathFindSuffixArray(LuaState* state)
{
	LPCTSTR found;
	if (!(found = ::PathFindSuffixArray(luaL_checkstring(*state, 1), NULL, 0)))
		return 0;
	state->PushString(found);
    return 1;
}


static int LS_PathGetArgs(LuaState* state)
{
	LPCSTR pszPath = luaL_checkstring(*state, 1);
	LPTSTR pszFound;
	if (!(pszFound = ::PathGetArgs(pszPath)))
		return 0;
	state->PushInteger((pszFound - pszPath) + 1);
    return 1;
}


static int LS_PathGetCharType(LuaState* state)
{
	return 0;
}


static int LS_PathGetDriveNumber(LuaState* state)
{
	state->PushInteger(::PathGetDriveNumber(luaL_checkstring(*state, 1)));
	return 1;
}


static int LS_PathIsContentType(LuaState* state)
{
	state->PushBoolean(::PathIsContentType(luaL_checkstring(*state, 1), luaL_checkstring(*state, 2)) != FALSE);
    return 1;
}


static int LS_PathIsDirectory(LuaState* state)
{
	state->PushBoolean(::PathIsDirectory(luaL_checkstring(*state, 1)) != FALSE);
    return 1;
}


static int LS_PathIsDirectoryEmpty(LuaState* state)
{
	state->PushBoolean(::PathIsDirectoryEmpty(luaL_checkstring(*state, 1)) != FALSE);
    return 1;
}


static int LS_PathIsFileSpec(LuaState* state)
{
	state->PushBoolean(::PathIsFileSpec(luaL_checkstring(*state, 1)) != FALSE);
    return 1;
}


static int LS_PathIsHTMLFile(LuaState* state)
{
	state->PushBoolean(::PathIsHTMLFile(luaL_checkstring(*state, 1)) != FALSE);
    return 1;
}


static int LS_PathIsLFNFileSpec(LuaState* state)
{
	state->PushBoolean(::PathIsLFNFileSpec(luaL_checkstring(*state, 1)) != FALSE);
    return 1;
}


static int LS_PathIsNetworkPath(LuaState* state)
{
	state->PushBoolean(::PathIsNetworkPath(luaL_checkstring(*state, 1)) != FALSE);
    return 1;
}


static int LS_PathIsPrefix(LuaState* state)
{
	state->PushBoolean(::PathIsPrefix(luaL_checkstring(*state, 1), luaL_checkstring(*state, 2)) != FALSE);
    return 1;
}


static int LS_PathIsRelative(LuaState* state)
{
	state->PushBoolean(::PathIsRelative(luaL_checkstring(*state, 1)) != FALSE);
    return 1;
}


static int LS_PathIsRoot(LuaState* state)
{
	state->PushBoolean(::PathIsRoot(luaL_checkstring(*state, 1)) != FALSE);
    return 1;
}


static int LS_PathIsSameRoot(LuaState* state)
{
	state->PushBoolean(PathIsSameRoot(luaL_checkstring(*state, 1), luaL_checkstring(*state, 2)) != FALSE);
    return 1;
}


static int LS_PathIsSystemFolder(LuaState* state)
{
	state->PushBoolean(::PathIsSystemFolder(luaL_checkstring(*state, 1), GetFileAttributes(luaL_checkstring(*state, 1))) != FALSE);
    return 1;
}


static int LS_PathIsUNC(LuaState* state)
{
	state->PushBoolean(::PathIsUNC(luaL_checkstring(*state, 1)) != FALSE);
    return 1;
}


static int LS_PathIsUNCServer(LuaState* state)
{
	state->PushBoolean(::PathIsUNCServer(luaL_checkstring(*state, 1)) != FALSE);
    return 1;
}


static int LS_PathIsUNCServerShare(LuaState* state)
{
	state->PushBoolean(PathIsUNCServerShare(luaL_checkstring(*state, 1)) != FALSE);
    return 1;
}


static int LS_PathIsURL(LuaState* state)
{
	state->PushBoolean(PathIsURL(luaL_checkstring(*state, 1)) != FALSE);
    return 1;
}


static int LS_PathMakePretty(LuaState* state)
{
	char lpPath[MAX_PATH];
	strcpy(lpPath, luaL_checkstring(*state, 1));
	if (!::PathMakePretty(lpPath))
		return 0;
	state->PushString(lpPath);
    return 1;
}


static int LS_PathMakeSystemFolder(LuaState* state)
{
	char pszPath[MAX_PATH];
	strcpy(pszPath, luaL_checkstring(*state, 1));
	state->PushBoolean(::PathMakeSystemFolder(pszPath) != 0);
    return 1;
}


static int LS_PathMatchSpec(LuaState* state)
{
	state->PushBoolean(::PathMatchSpec(luaL_checkstring(*state, 1), luaL_checkstring(*state, 2)) != 0);
    return 1;
}


static int LS_PathParseIconLocation(LuaState* state)
{
	char pszIconFile[MAX_PATH];
	strcpy(pszIconFile, luaL_checkstring(*state, 1));
	state->PushInteger(::PathParseIconLocation(pszIconFile));
    return 1;
}


static int LS_PathQuoteSpaces(LuaState* state)
{
	char lpsz[MAX_PATH];
	strcpy(lpsz, luaL_checkstring(*state, 1));
	::PathQuoteSpaces(lpsz);
	state->PushString(lpsz);
    return 1;
}


static int LS_PathRelativePathTo(LuaState* state)
{
	LuaStack args(state);
	char pszPath[MAX_PATH];
	if (!::PathRelativePathTo(pszPath, luaL_checkstring(*state, 1), args[2].GetBoolean(), luaL_checkstring(*state, 3), args[4].GetBoolean()))
		return 0;
	state->PushString(pszPath);
    return 1;
}


static int LS_PathRemoveArgs(LuaState* state)
{
	char pszPath[MAX_PATH];
	strcpy(pszPath, luaL_checkstring(*state, 1));
	::PathRemoveArgs(pszPath);
	state->PushString(pszPath);
    return 1;
}


static int LS_PathRemoveBackslash(LuaState* state)
{
	char lpszPath[MAX_PATH];
	strcpy(lpszPath, luaL_checkstring(*state, 1));
	::PathRemoveBackslash(lpszPath);
	state->PushString(lpszPath);
    return 1;
}


static int LS_PathRemoveBlanks(LuaState* state)
{
	char lpszString[MAX_PATH];
	strcpy(lpszString, luaL_checkstring(*state, 1));
	::PathRemoveBlanks(lpszString);
	state->PushString(lpszString);
    return 1;
}


static int LS_PathRemoveExtension(LuaState* state)
{
	char pszPath[MAX_PATH];
	strcpy(pszPath, luaL_checkstring(*state, 1));
	PathRemoveExtension(pszPath);
	state->PushString(pszPath);
    return 1;
}


static int LS_PathRemoveFileSpec(LuaState* state)
{
	char pszPath[MAX_PATH];
	strcpy(pszPath, luaL_checkstring(*state, 1));
	if (!::PathRemoveFileSpec(pszPath))
		return 0;
	state->PushString(pszPath);
    return 1;
}


static int LS_PathRenameExtension(LuaState* state)
{
	char pszPath[MAX_PATH];
	strcpy(pszPath, luaL_checkstring(*state, 1));
	PathRenameExtension(pszPath, luaL_checkstring(*state, 2));
	state->PushString(pszPath);
    return 1;
}


static int LS_PathSearchAndQualify(LuaState* state)
{
	char pszFullyQualifiedPath[MAX_PATH];
	if (!::PathSearchAndQualify(luaL_checkstring(*state, 1), pszFullyQualifiedPath, MAX_PATH))
		return 0;
	state->PushString(pszFullyQualifiedPath);
    return 1;
}


static int LS_PathSkipRoot(LuaState* state)
{
	LPCSTR pszPath = luaL_checkstring(*state, 1);
	LPTSTR pszFound;
	if (!(pszFound = ::PathSkipRoot(pszPath)))
		return 0;
	state->PushInteger((pszFound - pszPath) + 1);
    return 1;
}


static int LS_PathStripPath(LuaState* state)
{
	char pszPath[MAX_PATH];
	strcpy(pszPath, luaL_checkstring(*state, 1));
	::PathStripPath(pszPath);
	state->PushString(pszPath);
    return 1;
}


static int LS_PathStripToRoot(LuaState* state)
{
	char szRoot[MAX_PATH];
	strcpy(szRoot, luaL_checkstring(*state, 1));
	if (!::PathStripToRoot(szRoot))
		return 0;
	state->PushString(szRoot);
    return 1;
}


static int LS_PathUndecorate(LuaState* state)
{
	char pszPath[MAX_PATH];
	strcpy(pszPath, luaL_checkstring(*state, 1));
	::PathUndecorate(pszPath);
	state->PushString(pszPath);
    return 1;
}


static int LS_PathUnExpandEnvStrings(LuaState* state)
{
	char pszBuf[MAX_PATH];
	if (!::PathUnExpandEnvStrings(luaL_checkstring(*state, 1), pszBuf, MAX_PATH))
		return 0;
	state->PushString(pszBuf);
    return 1;
}


static int LS_PathUnmakeSystemFolder(LuaState* state)
{
	state->PushBoolean(::PathUnmakeSystemFolder(luaL_checkstring(*state, 1)) != FALSE);
    return 1;
}


static int LS_PathUnquoteSpaces(LuaState* state)
{
	char lpsz[MAX_PATH];
	strcpy(lpsz, luaL_checkstring(*state, 1));
	::PathUnquoteSpaces(lpsz);
	state->PushString(lpsz);
    return 1;
}


static int LS_PathMakeAbsolute(LuaState* state)
{
	// Make the intermediate directory an absolute path.
	char currentDirectory[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, currentDirectory);

	LPCSTR rootPath = luaL_optstring(*state, 2, currentDirectory);
	SetCurrentDirectory(rootPath);

	char path[MAX_PATH];
	_fullpath(path, luaL_checkstring(*state, 1), MAX_PATH);

	SetCurrentDirectory(currentDirectory);
	state->PushString(path);
	return 1;
}


static int LS_PathMakeForwardSlash(LuaState* state)
{
	char path[MAX_PATH];
	strcpy(path, luaL_checkstring(*state, 1));
	TCHAR* p = path;
	while (*p)
	{
		if (*p == '\\')
			*p = '/';
		p++;
	}
	state->PushString(path);
    return 1;
}


static int LS_PathMakeBackSlash(LuaState* state)
{
	char path[MAX_PATH];
	strcpy(path, luaL_checkstring(*state, 1));
	TCHAR* p = path;
	while (*p)
	{
		if (*p == '/')
			*p = '\\';
		p++;
	}
	state->PushString(path);
    return 1;
}


#ifdef WIN32

struct FindFileInfo
{
	FindFileInfo() : m_handle(NULL) {}
	~FindFileInfo() {  CloseHandle(m_handle);  }
	HANDLE m_handle;
	WIN32_FIND_DATA m_fd;
};

#endif WIN32

static int LS_Sleep(LuaState* state)
{
	LuaStack args(state);
	if (!args[1].IsInteger())
		return 0;

	int sleepValue = args[1].GetInteger();	sleepValue;
#ifdef WIN32
	Sleep((DWORD)sleepValue);
#endif WIN32
	return 0;
}


static int LS_FindFirstFile(LuaState* state)
{
	LuaStack args(state);
	LPCSTR wildcard = args[ 1 ].GetString();	wildcard;

#ifdef WIN32
	FindFileInfo* info = new FindFileInfo;
	info->m_handle = FindFirstFile(wildcard, &info->m_fd);
	if (info->m_handle == INVALID_HANDLE_VALUE)
	{
		delete info;
		return 0;
	}

	state->NewUserDataBox(info);
	state->GetGlobal("FileFind");
	state->SetMetaTable(-2);

	return 1;
#else !WIN32
	return 0;
#endif WIN32
}


static int LS_FindNextFile(LuaState* state)
{
	LuaStack args(state);
	if (!args[ 1 ].IsUserData())
		return 0;

#ifdef WIN32
	FindFileInfo* info = static_cast<FindFileInfo*>(args[ 1 ].GetUserData());
	BOOL ret = FindNextFile(info->m_handle, &info->m_fd);
	if (!ret)
	{
		FindClose(info->m_handle);
		info->m_handle = NULL;
		return 0;
	}

	state->PushBoolean(true);
	return 1;
#else !WIN32
	return 0;
#endif WIN32
}


static int LS_FindClose(LuaState* state)
{
	LuaStack args(state);
	if (!args[ 1 ].IsUserData())
		return 0;

#ifdef WIN32
	FindFileInfo* info = static_cast<FindFileInfo*>(args[ 1 ].GetUserData());
	delete info;
#endif WIN32
	return 0;
}


static int LS_FindGetFileName(LuaState* state)
{
	LuaStack args(state);
	if (!args[ 1 ].IsUserData())
		return 0;

#ifdef WIN32
	FindFileInfo* info = static_cast<FindFileInfo*>(args[ 1 ].GetUserData());
	state->PushString(info->m_fd.cFileName);
#endif WIN32

	return 1;
}


static int LS_FindIsDirectory(LuaState* state)
{
	LuaStack args(state);
	if (!args[ 1 ].IsUserData())
		return 0;

#ifdef WIN32
	FindFileInfo* info = static_cast<FindFileInfo*>(args[ 1 ].GetUserData());
	state->PushBoolean((info->m_fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);

	return 1;
#else !WIN32
	return 0;
#endif WIN32
}


static int LS_FindGetLastWriteTime(LuaState* state)
{
	LuaStack args(state);
	if (!args[ 1 ].IsUserData())
		return 0;

#ifdef WIN32
	FindFileInfo* info = static_cast<FindFileInfo*>(args[ 1 ].GetUserData());
	state->PushInteger(info->m_fd.ftLastWriteTime.dwLowDateTime);
	state->PushInteger(info->m_fd.ftLastWriteTime.dwHighDateTime);
	return 2;
#else !WIN32
	return 0;
#endif WIN32
}


static int LS_GetFileSize(LuaState* state)
{
	LuaStack args(state);
	if (!args[ 1 ].IsString())
		return 0;

#ifdef WIN32
	WIN32_FIND_DATA fd;
	HANDLE handle = FindFirstFile(args[ 1 ].GetString(), &fd);
	if (handle == INVALID_HANDLE_VALUE)
		return 0;
	state->PushInteger(fd.nFileSizeLow);
	FindClose(handle);
	return 1;
#else !WIN32
	return 0;
#endif WIN32

}

static int LS_GetFileWriteTime(LuaState* state)
{
	LuaStackObject fileNameObj(state, 1);
	if (!fileNameObj.IsString())
		return 0;

#ifdef WIN32
	WIN32_FIND_DATA fd;
	HANDLE handle = FindFirstFile(fileNameObj.GetString(), &fd);
	if (handle == INVALID_HANDLE_VALUE)
		return 0;
	state->PushNumber(fd.ftLastWriteTime.dwLowDateTime);
	state->PushNumber(fd.ftLastWriteTime.dwHighDateTime);
	FindClose(handle);
	return 2;
#else !WIN32
	return 0;
#endif WIN32
}

static int LS_SetFileWriteTime(LuaState* state)
{
	LuaStack args(state);
	if (!args[ 1 ].IsString())
		return 0;

#ifdef WIN32
	FILETIME createTime;
	FILETIME accessTime;
	FILETIME writeTime;
	LPCSTR fileName = args[ 1 ].GetString();
	HANDLE handle = CreateFile(fileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	GetFileTime(handle, &createTime, &accessTime, &writeTime);
	writeTime.dwLowDateTime = args[ 2 ].GetInteger();
	writeTime.dwHighDateTime = args[ 3 ].GetInteger();
	SetFileTime(handle, &createTime, &accessTime, &writeTime);
	CloseHandle(handle);
	return 0;
#else !WIN32
	return 0;
#endif WIN32
}

static int LS_MakeWritable( LuaState* state, LuaStackObject* args )
{
	LuaStackObject fileNameObj(state, 1);
	if (!fileNameObj.IsString())
		return 0;

#ifdef WIN32
	SetFileAttributes(fileNameObj.GetString(), FILE_ATTRIBUTE_NORMAL);
#else !WIN32
#endif WIN32
	return 0;
}

static int LS_access(LuaState* state)
{
	LuaStack args(state);
	LuaStackObject& fileNameObj = args[1];
	if (!fileNameObj.IsString())
		return 0;
	LuaStackObject& typeObj = args[2];
	LPCSTR type = NULL;
	if (typeObj.IsString())
		type = typeObj.GetString();

	int access = 0;
	if (type)
	{
		if (strcmp(type, "w") == 0)
			access = 2;
		else if (strcmp(type, "r") == 0)
			access = 4;
		else if (strcmp(type, "rw") == 0)
			access = 6;
	}

	state->PushInteger(_access(fileNameObj.GetString(), access));
	return 1;
}


static int LS_CopyFile(LuaState* state)
{
	LuaStack args(state);
	if (!args[1].IsString())
	{
		state->PushBoolean(false);
		return 1;
	}
	if (!args[2].IsString())
	{
		state->PushBoolean(false);
		return 1;
	}

	LPCSTR existingFileName = args[1].GetString();
	LPCSTR newFileName = args[2].GetString();
	state->PushBoolean(::CopyFile(existingFileName, newFileName, FALSE) != FALSE);
	return 1;
}


static int LS_DeleteFile(LuaState* state)
{
	LuaStack args(state);
	if (!args[1].IsString())
	{
		state->PushBoolean(false);
		return 1;
	}

	LPCSTR fileName = args[1].GetString();
	SetFileAttributes(fileName, FILE_ATTRIBUTE_ARCHIVE);
	state->PushBoolean(DeleteFile(fileName) != FALSE);
	return 1;
}


static int LS_FileTimeToTime_t(LuaState* state)
{
	LuaStack args(state);

	FILETIME fileTime;

	if (args[1].IsInteger())
	{
		fileTime.dwLowDateTime = (DWORD)args[1].GetInteger();
		fileTime.dwHighDateTime = (DWORD)args[2].GetInteger();
	}
	else if (args[1].IsString())
	{
        ULARGE_INTEGER largeInteger;
		const char* fileTimeStr = args[1].GetString();

        if (sscanf( fileTimeStr, "%I64u", &largeInteger) != 1)
		{
			return 0;
		}

        fileTime.dwLowDateTime = largeInteger.LowPart;
        fileTime.dwHighDateTime = largeInteger.HighPart;
	}

	// first convert file time (UTC time) to local time
	FILETIME localTime;
	if (!FileTimeToLocalFileTime(&fileTime, &localTime))
	{
		return 0;
	}

	// then convert that time to system time
	SYSTEMTIME sysTime;
	if (!FileTimeToSystemTime(&localTime, &sysTime))
	{
		return 0;
	}

	// then convert the system time to a time_t (C-runtime local time)
	if (sysTime.wYear < 1900)
	{
		return 0;
	}

	struct tm atm;
	atm.tm_sec = sysTime.wSecond;
	atm.tm_min = sysTime.wMinute;
	atm.tm_hour = sysTime.wHour;
	atm.tm_mday = sysTime.wDay;
	atm.tm_mon = sysTime.wMonth - 1;        // tm_mon is 0 based
	atm.tm_year = sysTime.wYear - 1900;     // tm_year is 1900 based
	atm.tm_isdst = -1;
	state->PushNumber(mktime(&atm));
	return 1;
}


BOOL (WINAPI *fnTzSpecificLocalTimeToSystemTime)(LPTIME_ZONE_INFORMATION lpTimeZoneInformation, LPSYSTEMTIME lpLocalTime, LPSYSTEMTIME lpUniversalTime);

static int LS_Time_t_ToFileTime(LuaState* state)
{
	LuaStack args(state);

	if (!args[1].IsInteger())
		return 0;

	time_t theTime = (time_t)args[1].GetNumber();

	FILETIME localFILETIME;
	LONGLONG ll = Int32x32To64(theTime, 10000000) + 116444736000000000;
	localFILETIME.dwLowDateTime = (DWORD) ll;
	localFILETIME.dwHighDateTime = (DWORD)(ll >>32);

	SYSTEMTIME localSystemTime;
	FILETIME theFILETIME;
	LocalFileTimeToFileTime(&localFILETIME, &theFILETIME);
	FileTimeToSystemTime(&theFILETIME, &localSystemTime);

	SYSTEMTIME universalSystemTime;

	if (!fnTzSpecificLocalTimeToSystemTime)
	{
		HMODULE aLib = LoadLibraryA("kernel32.dll");
		if (aLib == NULL)
			return 0;

		*(void**)&fnTzSpecificLocalTimeToSystemTime = (void*)GetProcAddress(aLib, "TzSpecificLocalTimeToSystemTime");
	}
	fnTzSpecificLocalTimeToSystemTime(NULL, &localSystemTime, &universalSystemTime);

	SystemTimeToFileTime(&localSystemTime, &theFILETIME);

	state->PushNumber(theFILETIME.dwLowDateTime);
	state->PushNumber(theFILETIME.dwHighDateTime);
	return 2;
}


static int LS_FileTimeToUnixTimeUTC(LuaState* state)
{
	FILETIME fileTime;

	LuaStack args(state);
	if (args[1].IsInteger())
	{
		fileTime.dwLowDateTime = (DWORD)args[1].GetInteger();
		fileTime.dwHighDateTime = (DWORD)args[2].GetInteger();
	}
	else if (args[1].IsString())
	{
        ULARGE_INTEGER largeInteger;
		const char* fileTimeStr = args[1].GetString();

        if (sscanf( fileTimeStr, "%I64u", &largeInteger) != 1)
		{
			return 0;
		}

        fileTime.dwLowDateTime = largeInteger.LowPart;
        fileTime.dwHighDateTime = largeInteger.HighPart;
	}

	LONGLONG ll; // 64 bit value
	ll = (((LONGLONG)(fileTime.dwHighDateTime)) << 32) + fileTime.dwLowDateTime;
	state->PushNumber((ll - 116444736000000000ui64)/10000000ui64);
	return 1;
}


static int LS_UnixTimeToFileTimeUTC(LuaState* state)
{
	LuaStack args(state);

	if (!args[1].IsInteger())
		return 0;

	time_t unixTime = (time_t)args[1].GetNumber();
	LONGLONG ll; // 64 bit value
	ll = Int32x32To64(unixTime, 10000000) + 116444736000000000ui64;
	state->PushNumber((DWORD)ll);
	state->PushNumber((DWORD)(ll >> 32));
	return 2;
}


static int LS_LoadLibrary(LuaState* state)
{
	LuaStack args(state);
	if (!args[1].IsString())
		return 0;
	HMODULE handle = LoadLibrary(args[1].GetString());
	state->PushLightUserData(handle);
	return 1;
}


static int LS_FreeLibrary(LuaState* state)
{
	LuaStack args(state);
	if (!args[1].IsLightUserData())
		return 0;
	FreeLibrary((HMODULE)args[1].GetLightUserData());
	return 0;
}



extern "C" int luaopen_iox(lua_State* L)
{
	LuaState* state = lua_State_To_LuaState(L);
	LuaObject ioxObj = state->GetGlobals().CreateTable("iox");
	ioxObj.RegisterDirect("PathCreate", PathCreate);
	ioxObj.RegisterDirect("PathDestroy", PathDestroy);
	ioxObj.RegisterDirect("CreateDirectory", PathCreate);
	ioxObj.RegisterDirect("RemoveDirectory", PathDestroy);
	ioxObj.Register("GetCurrentDirectory", LS_GetCurrentDirectory);
	ioxObj.Register("SetCurrentDirectory", LS_SetCurrentDirectory);

	ioxObj.Register("PathAddBackslash", LS_PathAddBackslash);
	ioxObj.Register("PathAddExtension", LS_PathAddExtension);
	ioxObj.Register("PathAppend", LS_PathAppend);
    ioxObj.Register("PathCanonicalize", LS_PathCanonicalize);
	ioxObj.Register("PathCombine", LS_PathCombine);
	ioxObj.Register("PathCommonPrefix", LS_PathCommonPrefix);
	ioxObj.Register("PathFileExists", LS_PathFileExists);
	ioxObj.Register("PathFindExtension", LS_PathFindExtension);
	ioxObj.Register("PathFindFileName", LS_PathFindFileName);
	ioxObj.Register("PathFindNextComponent", LS_PathFindNextComponent);
	ioxObj.Register("PathFindOnPath", LS_PathFindOnPath);
	ioxObj.Register("PathGetArgs", LS_PathGetArgs);
	ioxObj.Register("PathIsDirectory", LS_PathIsDirectory);
	ioxObj.Register("PathIsDirectoryEmpty", LS_PathIsDirectoryEmpty);
	ioxObj.Register("PathIsFileSpec", LS_PathIsFileSpec);
	ioxObj.Register("PathIsHTMLFile", LS_PathIsHTMLFile);
	ioxObj.Register("PathIsLFNFileSpec", LS_PathIsLFNFileSpec);
	ioxObj.Register("PathIsNetworkPath", LS_PathIsNetworkPath);
	ioxObj.Register("PathIsPrefix", LS_PathIsPrefix);
	ioxObj.Register("PathIsRelative", LS_PathIsRelative);
	ioxObj.Register("PathIsRoot", LS_PathIsRoot);
	ioxObj.Register("PathIsSameRoot", LS_PathIsSameRoot);
	ioxObj.Register("PathIsSystemFolder", LS_PathIsSystemFolder);
	ioxObj.Register("PathIsUNC", LS_PathIsUNC);
	ioxObj.Register("PathIsUNCServer", LS_PathIsUNCServer);
	ioxObj.Register("PathIsUNCServerShare", LS_PathIsUNCServerShare);
	ioxObj.Register("PathIsURL", LS_PathIsURL);
	ioxObj.Register("PathMakePretty", LS_PathMakePretty);
	ioxObj.Register("PathMakeSystemFolder", LS_PathMakeSystemFolder);
	ioxObj.Register("PathMatchSpec", LS_PathMatchSpec);
	ioxObj.Register("PathQuoteSpaces", LS_PathQuoteSpaces);
	ioxObj.Register("PathRelativePathTo", LS_PathRelativePathTo);
	ioxObj.Register("PathRemoveArgs", LS_PathRemoveArgs);
	ioxObj.Register("PathRemoveBackslash", LS_PathRemoveBackslash);
	ioxObj.Register("PathRemoveBlanks", LS_PathRemoveBlanks);
	ioxObj.Register("PathRemoveExtension", LS_PathRemoveExtension);
	ioxObj.Register("PathRemoveFileSpec", LS_PathRemoveFileSpec);
	ioxObj.Register("PathRenameExtension", LS_PathRenameExtension);
	ioxObj.Register("PathSearchAndQualify", LS_PathSearchAndQualify);
	ioxObj.Register("PathSkipRoot", LS_PathSkipRoot);
	ioxObj.Register("PathStripPath", LS_PathStripPath);
	ioxObj.Register("PathStripToRoot", LS_PathStripToRoot);
	ioxObj.Register("PathUndecorate", LS_PathUndecorate);
	ioxObj.Register("PathUnExpandEnvStrings", LS_PathUnExpandEnvStrings);
	ioxObj.Register("PathUnmakeSystemFolder", LS_PathUnmakeSystemFolder);
	ioxObj.Register("PathUnquoteSpaces", LS_PathUnquoteSpaces);

	ioxObj.Register("PathMakeAbsolute", LS_PathMakeAbsolute);
	ioxObj.Register("PathMakeBackSlash", LS_PathMakeBackSlash);
	ioxObj.Register("PathMakeForwardSlash", LS_PathMakeForwardSlash);

	ioxObj.Register("FindFirstFile",		LS_FindFirstFile);
	ioxObj.Register("FindNextFile",			LS_FindNextFile);
	ioxObj.Register("FindClose",			LS_FindClose);
	ioxObj.Register("FindGetFileName",		LS_FindGetFileName);
	ioxObj.Register("FindIsDirectory",		LS_FindIsDirectory);
	ioxObj.Register("FindGetLastWriteTime",	LS_FindGetLastWriteTime);
	ioxObj.Register("GetFileSize",			LS_GetFileSize);
	ioxObj.Register("access",				LS_access);

	ioxObj.Register("CopyFile",				LS_CopyFile);
	ioxObj.Register("DeleteFile",			LS_DeleteFile);
	ioxObj.Register("GetWriteTime",			LS_GetFileWriteTime);
	ioxObj.Register("SetWriteTime",			LS_SetFileWriteTime);
	ioxObj.Register("MakeWritable",			LS_MakeWritable);

	ioxObj.Register("Sleep",				LS_Sleep);
	ioxObj.Register("FileTimeToTime_t",		LS_FileTimeToTime_t);
	ioxObj.Register("Time_t_ToFileTime",	LS_Time_t_ToFileTime);
	ioxObj.Register("UnixTimeToFileTimeUTC",LS_UnixTimeToFileTimeUTC);
	ioxObj.Register("FileTimeToUnixTimeUTC",LS_FileTimeToUnixTimeUTC);

	ioxObj.Register("LoadLibrary",			LS_LoadLibrary);
	ioxObj.Register("FreeLibrary",			LS_FreeLibrary);

	return 0;
}


