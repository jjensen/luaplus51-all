#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include "LuaPlus/LuaPlus.h"
extern "C" {
#include "LuaPlus/src/lua.h"
#include "LuaPlus/src/lualib.h"
}

using namespace LuaPlus;

// From some MSDN sample, I think.
static int CreatePipeChild(HANDLE& child, DWORD& childId, HANDLE* inH, HANDLE* outH, HANDLE* errH, LPCTSTR Command, bool show)
{
    static int PCount = 0;

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);                        // Security descriptor for INHERIT.
    sa.lpSecurityDescriptor = 0;
    sa.bInheritHandle       = 0;

    SECURITY_ATTRIBUTES lsa;
    HANDLE ChildIn;
	HANDLE ChildOut;
	HANDLE ChildErr;
	lsa.nLength=sizeof(SECURITY_ATTRIBUTES);
	lsa.lpSecurityDescriptor=NULL;
	lsa.bInheritHandle=TRUE;

	// Create Parent_Write to ChildStdIn Pipe
	if (!CreatePipe(&ChildIn,inH,&lsa,0))
	{
		// Error.
	}

	// Create ChildStdOut to Parent_Read pipe
	if (!CreatePipe(outH,&ChildOut,&lsa,0))
	{
		// Error.
	}

	// Create ChildStdOut to Parent_Read pipe
	if (!CreatePipe(errH,&ChildErr,&lsa,0))
	{
		// Error.
	}

	// Lets Redirect Console StdHandles - easy enough
    PROCESS_INFORMATION pi;
    STARTUPINFO             si;
    HANDLE hNul;

	// Dup the child handle to get separate handles for stdout and err,
    hNul = CreateFile("NUL",
                      GENERIC_READ | GENERIC_WRITE,
                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                      NULL, OPEN_EXISTING,
                      0,
                      NULL);

    if (hNul != NULL)
	{
        // Set up members of STARTUPINFO structure.
        memset(&si, 0, sizeof(si));
        si.cb = sizeof(STARTUPINFO);
        si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		si.wShowWindow = show ? SW_SHOW : SW_HIDE;
        si.hStdOutput = ChildOut;
        si.hStdError    = ChildErr;
        si.hStdInput    = ChildIn;
        if (CreateProcess(NULL, (char*)Command, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi) == TRUE)
        {
            CloseHandle(pi.hThread);        // Thread handle not needed
            //fprintf(stderr, "create process success\n");
            child = pi.hProcess;  // Return process handle (to get RC)
			childId = pi.dwProcessId;
        } else
			return -1;
        CloseHandle(hNul);                                      // Close error handle,
		CloseHandle(ChildOut);
		CloseHandle(ChildErr);
		CloseHandle(ChildIn);
    }
    else
	{
		// Error.
	}

    return 0;
}


static LuaStackObject newfile (LuaState* state, FILE *f) {
  FILE **pf = (FILE **)lua_newuserdata(*state, sizeof(FILE *));
  *pf = f;
  LuaStackObject obj = state->StackTop();
  luaL_getmetatable(*state, LUA_FILEHANDLE);
  lua_setmetatable(*state, obj);
  return obj;
}


static int LS_popen( lua_State* L )
{
	LuaState* state = lua_State_To_LuaState(L);
	LuaStack args(state);
	if (!args[1].IsString())
	{
		state->PushNil();
		return 1;
	}

	LPCSTR str = args[1].GetString();

	bool show = args[2].IsBoolean() ? args[2].GetBoolean() : true;

	char* cmd = new char[7 + strlen(str) + 1];
	strcpy(cmd, "cmd /C ");
	strcat(cmd, str);

	HANDLE child;
	DWORD childId;
	HANDLE hIn, hOut, hErr;
	int rc = CreatePipeChild(child, childId, &hIn, &hOut, &hErr, cmd, show);
	delete[] cmd;
	if (rc == -1)
	{
		state->PushNil();
		return 1;
	}

	state->CreateTable();
    LuaStackObject outTableObj = state->StackTop();
	int in = _open_osfhandle((long)hIn, _O_WRONLY | _O_TEXT);
	FILE* fIn = fdopen(in, "wt");
	int err = _open_osfhandle((long)hErr, _O_RDONLY | _O_TEXT);
	FILE* fErr = fdopen(err, "rt");
	int out = _open_osfhandle((long)hOut, _O_RDONLY | _O_TEXT);
	FILE* fOut = fdopen(out, "rt");

	setvbuf(fIn, NULL, _IONBF, 0);
	setvbuf(fOut, NULL, _IONBF, 0);
	setvbuf(fErr, NULL, _IONBF, 0);

	{
		LuaAutoBlock autoBlock(state);
		LuaStackObject inObj = newfile(state, fIn);
		outTableObj.SetObject("stdin", inObj);
		LuaStackObject outObj = newfile(state, fOut);
		outTableObj.SetObject("stdout", outObj);
		LuaStackObject errObj = newfile(state, fErr);
		outTableObj.SetObject("stderr", errObj);
	}

	outTableObj.SetLightUserData("__handle", child);
	outTableObj.SetNumber("__processId", childId);

	return 1;
}


static int LS_pclose( lua_State* L )
{
	LuaState* state = lua_State_To_LuaState(L);
	LuaStack args(state);
	LuaStackObject pipeObj = args[1];
	if (!pipeObj.IsTable())
		return 0;

	LuaObject childObj = pipeObj["__handle"];
	if (!childObj.IsUserData())
		return 0;
	HANDLE child = (HANDLE)childObj.GetLightUserData();

	DWORD processReturnCode = -1;
	GetExitCodeProcess(child, &processReturnCode);
	CloseHandle(child);
	pipeObj.SetInteger("exitcode", processReturnCode);

	// Close down the pipe.
	/*
	state->PushString("close");
	LuaStackObject closeObj(state, state->GetTop());

	closeObj.Push();
	LuaStackObject inObj = pipeObj["stdin"];
	*/

	state->PushBoolean(true);
	return 1;
}

#define topfile(L)	((FILE **)luaL_checkudata(L, 1, LUA_FILEHANDLE))

static FILE *tofile (lua_State *L) {
  FILE **f = topfile(L);
  if (*f == NULL)
    luaL_error(L, "attempt to use a closed file");
  return *f;
}



static int pipe_aux_close (lua_State *L) {
  lua_getfenv(L, 1);
  lua_getfield(L, -1, "__close");
  return (lua_tocfunction(L, -1))(L);
}


static int pipe_io_close (lua_State *L) {
	FILE* f = tofile(L);
	if (f)
		fclose(f);
	return 0;
}


static luaL_reg func[] = {
    { "popen", LS_popen },
    { "pclose", LS_pclose },
    { NULL, NULL }
};


extern "C" int luaopen_pipe_core(lua_State *L)
{
	luaL_openlib(L, "pipe", func, 0);

	lua_pushcfunction(L, pipe_io_close);
	lua_setfield(L, LUA_ENVIRONINDEX, "__close");

	return 1;
}


