/*
 * "ex" API implementation
 * http://lua-users.org/wiki/ExtensionProposal
 * Copyright 2007 Mark Edgar < medgar at student gc maricopa edu >
 */
#include <stdlib.h>
#include <windows.h>
#include <io.h>

#include "lua.h"
#include "lauxlib.h"

#include "spawn.h"
#include "pusherror.h"

#if LUA_VERSION_NUM >= 502
#define luaL_register(a, b, c) luaL_setfuncs(a, c, 0)
#define lua_objlen lua_rawlen

static int luaL_argerror (lua_State *L, int narg, const char *extramsg) {
  lua_Debug ar;
  if (!lua_getstack(L, 0, &ar))  /* no stack frame? */
    return luaL_error(L, "bad argument #%d (%s)", narg, extramsg);
  lua_getinfo(L, "n", &ar);
  if (strcmp(ar.namewhat, "method") == 0) {
    narg--;  /* do not count `self' */
    if (narg == 0)  /* error is in the self argument itself? */
      return luaL_error(L, "calling " LUA_QS " on bad self (%s)",
                           ar.name, extramsg);
  }
  if (ar.name == NULL)
    ar.name = "?";
  return luaL_error(L, "bad argument #%d to " LUA_QS " (%s)",
                        narg, ar.name, extramsg);
}


static int luaL_typerror (lua_State *L, int narg, const char *tname) {
  const char *msg = lua_pushfstring(L, "%s expected, got %s",
                                    tname, luaL_typename(L, narg));
  return luaL_argerror(L, narg, msg);
}

#endif

#if _MSC_VER  &&  _MSC_VER <= 1300
#define debug() /* fprintf(stderr, __VA_ARGS__) */
#define debug_stack(L) /* #include "../lds.c" */
#else
#define debug(...) /* fprintf(stderr, __VA_ARGS__) */
#define debug_stack(L) /* #include "../lds.c" */
#endif

#define file_handle(fp) (HANDLE)_get_osfhandle(fileno(fp))

struct spawn_params {
  lua_State *L;
  const char *cmdline;
  const char *environment;
  STARTUPINFO si;
  int detach;
  int suspended;
  int can_terminate;
  int useshell;
};

/* quotes and adds argument string to b */
static int add_argument(luaL_Buffer *b, const char *s) {
  int oddbs = 0;
//  luaL_addchar(b, '"');
  for (; *s; s++) {
    switch (*s) {
//    case '\\':
//      luaL_addchar(b, '\\');
//      oddbs = !oddbs;
//      break;
//    case '"':
//      luaL_addchar(b, '\\');
//      oddbs = 0;
//      break;
    default:
      oddbs = 0;
      break;
    }
    luaL_addchar(b, *s);
  }
//  luaL_addchar(b, '"');
  return oddbs;
}

struct spawn_params *spawn_param_init(lua_State *L)
{
  static const STARTUPINFO si = {sizeof si, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, STARTF_USESHOWWINDOW, SW_HIDE, 0, NULL, 0, 0, 0 };
  struct spawn_params *p = lua_newuserdata(L, sizeof *p);
  p->L = L;
  p->cmdline = p->environment = 0;
  p->si = si;
  p->detach = 0;
  p->suspended = 0;
  p->can_terminate = 0;
  p->useshell = 1;
  return p;
}

/* cmd ... -- cmd ... */
void spawn_param_filename(struct spawn_params *p)
{
  lua_State *L = p->L;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  if (add_argument(&b, lua_tostring(L, 1))) {
    luaL_error(L, "argument ends in odd number of backslashes");
    return;
  }
  luaL_pushresult(&b);
  lua_replace(L, 1);
  p->cmdline = lua_tostring(L, 1);
}

/* cmd ... argtab -- cmdline ... */
void spawn_param_args(struct spawn_params *p)
{
  lua_State *L = p->L;
  int argtab = lua_gettop(L);
  size_t i, n = lua_objlen(L, argtab);
  luaL_Buffer b;
  debug("spawn_param_args:"); debug_stack(L);
  lua_pushnil(L);                 /* cmd opts ... argtab nil */
  luaL_buffinit(L, &b);           /* cmd opts ... argtab nil b... */
  lua_pushvalue(L, 1);            /* cmd opts ... argtab nil b... cmd */
  luaL_addvalue(&b);              /* cmd opts ... argtab nil b... */
  /* concatenate the arg array to a string */
  for (i = 1; i <= n; i++) {
    const char *s;
    lua_rawgeti(L, argtab, i);    /* cmd opts ... argtab nil b... arg */
    lua_replace(L, argtab + 1);   /* cmd opts ... argtab arg b... */
    luaL_addchar(&b, ' ');
    /* XXX checkstring is confusing here */
    s = lua_tostring(L, argtab + 1);
    if (!s) {
      luaL_error(L, "expected string for argument %d, got %s",
                 i, lua_typename(L, lua_type(L, argtab + 1)));
      return;
    }
    add_argument(&b, luaL_checkstring(L, argtab + 1));
  }
  luaL_pushresult(&b);            /* cmd opts ... argtab arg cmdline */
  lua_replace(L, 1);              /* cmdline opts ... argtab arg */
  lua_pop(L, 2);                  /* cmdline opts ... */
  p->cmdline = lua_tostring(L, 1);
}

void spawn_param_detach(struct spawn_params *p, int detach)
{
  p->detach = detach;
}

void spawn_param_suspended(struct spawn_params *p, int suspended)
{
  p->suspended = suspended;
}

void spawn_param_can_terminate(struct spawn_params *p, int can_terminate)
{
  p->can_terminate = can_terminate;
}

void spawn_param_show(struct spawn_params *p, int show)
{
  p->si.wShowWindow = show ? SW_SHOW : SW_HIDE;
}

void spawn_param_useshell(struct spawn_params *p, int shell)
{
  p->useshell = shell;
}

/* ... tab nil nil [...] -- ... tab envstr */
static char *add_env(lua_State *L, int tab, size_t where) {
  char *t;
  lua_checkstack(L, 2);
  lua_pushvalue(L, -2);
  if (lua_next(L, tab)) {
    size_t klen, vlen;
    const char *key = lua_tolstring(L, -2, &klen);
    const char *val = lua_tolstring(L, -1, &vlen);
    t = add_env(L, tab, where + klen + vlen + 2);
    memcpy(&t[where], key, klen);
    t[where += klen] = '=';
    memcpy(&t[where + 1], val, vlen + 1);
  }
  else {
    t = lua_newuserdata(L, where + 1);
    t[where] = '\0';
    lua_replace(L, tab + 1);
  }
  return t;
}

/* ... envtab -- ... envtab envstr */
void spawn_param_env(struct spawn_params *p)
{
  lua_State *L = p->L;
  int envtab = lua_gettop(L);
  lua_pushnil(L);
  lua_pushnil(L);
  p->environment = add_env(L, envtab, 0);
  lua_settop(L, envtab + 1);
}

void spawn_param_redirect(struct spawn_params *p, const char *stdname, HANDLE h)
{
  SetHandleInformation(h, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
  if (!(p->si.dwFlags & STARTF_USESTDHANDLES)) {
    p->si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
    p->si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    p->si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);
    p->si.dwFlags |= STARTF_USESTDHANDLES;
  }
  switch (stdname[3]) {
  case 'i': p->si.hStdInput = h; break;
  case 'o': p->si.hStdOutput = h; break;
  case 'e': p->si.hStdError = h; break;
  }
}

struct process {
  int status;
  PROCESS_INFORMATION pi;
  HANDLE jobHandle;
};

int spawn_param_execute(struct spawn_params *p)
{
  lua_State *L = p->L;
  char *c, *e;
  const char* comspec;
  BOOL ret;
  DWORD flags;
  struct process *proc = lua_newuserdata(L, sizeof *proc);
  luaL_getmetatable(L, PROCESS_HANDLE);
  lua_setmetatable(L, -2);
  proc->status = -1;

  if (p->useshell) {
    comspec = getenv("COMSPEC");
    if (!comspec)
      comspec = "cmd.exe";

    c = (char*)malloc(1 + strlen(comspec) + 1 + 4 + strlen(p->cmdline) + 2 + 1);
    strcpy(c, "\"");
    strcat(c, comspec);
    strcat(c, "\" /C ");
    strcat(c, "\"");
    strcat(c, p->cmdline);
    strcat(c, "\"");
  } else {
    c = strdup(p->cmdline);
  }

  e = (char *)p->environment; /* strdup(p->environment); */
  /* XXX does CreateProcess modify its environment argument? */
  flags = 0;
  if (p->detach)
	  flags |= DETACHED_PROCESS;
  if (p->suspended)
	  flags |= CREATE_SUSPENDED;
  proc->jobHandle = INVALID_HANDLE_VALUE;
  if (p->can_terminate) {
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION info;
	BOOL ret;

    proc->jobHandle = CreateJobObject(NULL, NULL);

    info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE | JOB_OBJECT_LIMIT_BREAKAWAY_OK;
	ret = SetInformationJobObject(proc->jobHandle, JobObjectExtendedLimitInformation, &info, sizeof(info));

	flags |= CREATE_BREAKAWAY_FROM_JOB | CREATE_SUSPENDED;
	//flags |= CREATE_SUSPENDED;
  }
  ret = CreateProcess(0, c, 0, 0, p->detach ? FALSE : TRUE, flags, e, 0, &p->si, &proc->pi);
  free(c);
  if (!ret)
    return windows_pushlasterror(L);
  if (proc->jobHandle != INVALID_HANDLE_VALUE) {
	  BOOL ret = AssignProcessToJobObject(proc->jobHandle, proc->pi.hProcess);
	  DWORD lastError = GetLastError();
	  if (!p->suspended) {
		  ResumeThread(proc->pi.hThread);
	  }
  }
  return 1;
}

/* proc -- exitcode/nil error */
int process_wait(lua_State *L)
{
  struct process *p = luaL_checkudata(L, 1, PROCESS_HANDLE);
  DWORD timeout = luaL_optinteger(L, 2, INFINITE);
  if (p->status == -1) {
    DWORD exitcode;
	DWORD result = WaitForSingleObject(p->pi.hProcess, timeout);
	if (WAIT_TIMEOUT == result)
		return 0;
    if (WAIT_FAILED == result
        || !GetExitCodeProcess(p->pi.hProcess, &exitcode))
      return windows_pushlasterror(L);
    p->status = exitcode;
  }
  process_close(L);
  lua_pushnumber(L, p->status);
  return 1;
}

/* proc -- string */
int process_tostring(lua_State *L)
{
  struct process *p = luaL_checkudata(L, 1, PROCESS_HANDLE);
  char buf[40];
  lua_pushlstring(L, buf,
    sprintf(buf, "process (%lu, %s)", (unsigned long)p->pi.dwProcessId,
        p->pi.dwProcessId ? (p->status==-1 ? "running" : "terminated") : "terminated"));
  return 1;
}

/* proc -- exitcode/nil error */
int process_close(lua_State *L)
{
  struct process *p = luaL_checkudata(L, 1, PROCESS_HANDLE);
  if (p->pi.hProcess != INVALID_HANDLE_VALUE) {
	if (p->jobHandle != INVALID_HANDLE_VALUE) {
		TerminateJobObject(p->jobHandle, 1);
		CloseHandle(p->jobHandle);
		p->jobHandle = INVALID_HANDLE_VALUE;
	}
    if (p->status == -1) {
      DWORD exitcode;
      GetExitCodeProcess(p->pi.hProcess, &exitcode);
      p->status = exitcode;
	}

	CloseHandle(p->pi.hThread);
	p->pi.hThread = INVALID_HANDLE_VALUE;
    CloseHandle(p->pi.hProcess);
    p->pi.hProcess = INVALID_HANDLE_VALUE;
  }
  return 0;
}


/* proc -- exitcode/nil error */
int process_resume(lua_State *L)
{
  struct process *p = luaL_checkudata(L, 1, PROCESS_HANDLE);
  if (p->pi.hProcess != INVALID_HANDLE_VALUE) {
    lua_pushboolean(L, ResumeThread(p->pi.hThread) != FALSE);
	lua_pushinteger(L, GetLastError());
	return 2;
  }
  return 0;
}


/* proc -- exitcode/nil error */
int process_getinfo(lua_State *L)
{
  struct process *p = luaL_checkudata(L, 1, PROCESS_HANDLE);
  lua_pushlightuserdata(L, p->pi.hProcess);
  lua_newtable(L);
  lua_pushstring(L, "process_handle");
  lua_pushlightuserdata(L, p->pi.hProcess);
  lua_rawset(L, -3);
  lua_pushstring(L, "thread_handle");
  lua_pushlightuserdata(L, p->pi.hThread);
  lua_rawset(L, -3);
  lua_pushstring(L, "process_id");
  lua_pushinteger(L, p->pi.dwProcessId);
  lua_rawset(L, -3);
  lua_pushstring(L, "thread_id");
  lua_pushinteger(L, p->pi.dwThreadId);
  lua_rawset(L, -3);
  lua_pushstring(L, "job");
  lua_pushlightuserdata(L, p->jobHandle);
  lua_rawset(L, -3);
  return 1;
}


int process_terminate(lua_State *L)
{
  HANDLE jobHandle = INVALID_HANDLE_VALUE;
  if (lua_isuserdata(L, 1)) {
    jobHandle = (HANDLE)lua_touserdata(L, 1);
  } else if (lua_isnumber(L, 1)) {
    jobHandle = (HANDLE)(intptr_t)lua_tonumber(L, 1);
  } else if (lua_isstring(L, 1)) {
  } else if (lua_istable(L, 1)) {
    lua_getfield(L, 1, "job");
	if (lua_isuserdata(L, -1)) {
      jobHandle = (HANDLE)lua_touserdata(L, -1);
	}
	lua_pop(L, 1);
  }
  if (jobHandle != INVALID_HANDLE_VALUE) {
    lua_pushboolean(L, TerminateJobObject(jobHandle, (UINT)luaL_optnumber(L, 2, 1)) != FALSE);
    return 1;
  }
  return 0;
}

