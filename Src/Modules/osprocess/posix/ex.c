/*
 * "ex" API implementation
 * http://lua-users.org/wiki/ExtensionProposal
 * Copyright 2007 Mark Edgar < medgar at student gc maricopa edu >
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#include <unistd.h>
#ifdef __APPLE__
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#else
ENVIRON_DECL
#endif
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>
#include <utime.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#if LUA_VERSION_NUM >= 502
#define luaL_register(a, b, c) luaL_setfuncs(a, c, 0)
#define lua_objlen lua_rawlen

static int luaL_typerror (lua_State *L, int narg, const char *tname) {
  const char *msg = lua_pushfstring(L, "%s expected, got %s",
                                    tname, luaL_typename(L, narg));
  return luaL_argerror(L, narg, msg);
}

#endif

#define absindex(L,i) ((i)>0?(i):lua_gettop(L)+(i)+1)

#include "spawn.h"

/* -- nil error */
extern int push_error(lua_State *L)
{
  lua_pushnil(L);
  lua_pushstring(L, strerror(errno));
  return 2;
}

/* name -- value/nil */
static int ex_getenv(lua_State *L)
{
  const char *nam = luaL_checkstring(L, 1);
  char *val = getenv(nam);
  if (!val)
    return push_error(L);
  lua_pushstring(L, val);
  return 1;
}

/* name value -- true/nil error
 * name nil -- true/nil error */
static int ex_setenv(lua_State *L)
{
  const char *nam = luaL_checkstring(L, 1);
  const char *val = lua_tostring(L, 2);
  int err = val ? setenv(nam, val, 1) : unsetenv(nam);
  if (err == -1) return push_error(L);
  lua_pushboolean(L, 1);
  return 1;
}

/* -- environment-table */
static int ex_environ(lua_State *L)
{
  const char *nam, *val, *end;
  const char **env;
  lua_newtable(L);
  for (env = (const char **)environ; (nam = *env); env++) {
    end = strchr(val = strchr(nam, '=') + 1, '\0');
    lua_pushlstring(L, nam, val - nam - 1);
    lua_pushlstring(L, val, end - val);
    lua_settable(L, -3);
  }
  return 1;
}


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

static int pipe_close (lua_State *L);

static FILE **new_file(lua_State *L, int fd, const char *mode)
{
#if LUA_VERSION_NUM <= 501
  FILE **pf = lua_newuserdata(L, sizeof *pf);
  *pf = 0;
  luaL_getmetatable(L, LUA_FILEHANDLE);
  lua_setmetatable(L, -2);
  *pf = fdopen(fd, mode);
  return pf;
#else
  luaL_Stream *p = (luaL_Stream *)lua_newuserdata(L, sizeof(luaL_Stream));
  p->f = NULL;
  p->closef = pipe_close;  /* mark file handle as 'closed' */
  luaL_setmetatable(L, LUA_FILEHANDLE);
  p->f = fdopen(fd, mode);
  return &p->f;
#endif
}


static int closeonexec(int d)
{
  int fl = fcntl(d, F_GETFD);
  if (fl != -1)
    fl = fcntl(d, F_SETFD, fl | FD_CLOEXEC);
  return fl;
}

/* -- in out/nil error */
static int ex_pipe(lua_State *L)
{
  int fd[2];
  if (-1 == pipe(fd))
    return push_error(L);
  closeonexec(fd[0]);
  closeonexec(fd[1]);
  new_file(L, fd[0], "r");
  new_file(L, fd[1], "w");
  return 2;
}


/* seconds --
 * interval units -- */
static int ex_sleep(lua_State *L)
{
  lua_Number interval = luaL_checknumber(L, 1);
  lua_Number units = luaL_optnumber(L, 2, 1);
  usleep(1e6 * interval / units);
  return 0;
}


static void get_redirect(lua_State *L,
                         int idx, const char *stdname, struct spawn_params *p)
{
  lua_getfield(L, idx, stdname);
  if (!lua_isnil(L, -1))
    spawn_param_redirect(p, stdname, fileno(check_file(L, -1, stdname)));
  lua_pop(L, 1);
}

/* filename [args-opts] -- proc/nil error */
/* args-opts -- proc/nil error */
static int ex_spawn(lua_State *L)
{
  struct spawn_params *params;
  int have_options;
  switch (lua_type(L, 1)) {
  default: return luaL_typerror(L, 1, "string or table");
  case LUA_TSTRING:
    switch (lua_type(L, 2)) {
    default: return luaL_typerror(L, 2, "table");
    case LUA_TNONE: have_options = 0; break;
    case LUA_TTABLE: have_options = 1; break;
    }
    break;
  case LUA_TTABLE:
    have_options = 1;
    lua_getfield(L, 1, "command");      /* opts ... cmd */
    if (!lua_isnil(L, -1)) {
      /* convert {command=command,arg1,...} to command {arg1,...} */
      lua_insert(L, 1);                 /* cmd opts ... */
    }
    else {
      /* convert {arg0,arg1,...} to arg0 {arg1,...} */
      size_t i, n = lua_objlen(L, 1);
      lua_rawgeti(L, 1, 1);             /* opts ... nil cmd */
      lua_insert(L, 1);                 /* cmd opts ... nil */
      for (i = 2; i <= n; i++) {
        lua_rawgeti(L, 2, i);           /* cmd opts ... nil argi */
        lua_rawseti(L, 2, i - 1);       /* cmd opts ... nil */
      }
      lua_rawseti(L, 2, n);             /* cmd opts ... */
    }
    if (lua_type(L, 1) != LUA_TSTRING)
      return luaL_error(L, "bad command option (string expected, got %s)",
                        luaL_typename(L, 1));
    break;
  }
  params = spawn_param_init(L);
  /* get filename to execute */
  spawn_param_filename(params, lua_tostring(L, 1));
  /* get arguments, environment, and redirections */
  if (have_options) {
    lua_getfield(L, 2, "args");         /* cmd opts ... argtab */
    switch (lua_type(L, -1)) {
    default:
      return luaL_error(L, "bad args option (table expected, got %s)",
                        luaL_typename(L, -1));
    case LUA_TNIL:
      lua_pop(L, 1);                    /* cmd opts ... */
      lua_pushvalue(L, 2);              /* cmd opts ... opts */
      if (0) /*FALLTHRU*/
    case LUA_TTABLE:
      if (lua_objlen(L, 2) > 0)
        return
          luaL_error(L, "cannot specify both the args option and array values");
      spawn_param_args(params);         /* cmd opts ... */
      break;
    }
    lua_getfield(L, 2, "env");          /* cmd opts ... envtab */
    switch (lua_type(L, -1)) {
    default:
      return luaL_error(L, "bad env option (table expected, got %s)",
                        luaL_typename(L, -1));
    case LUA_TNIL:
      break;
    case LUA_TTABLE:
      spawn_param_env(params);          /* cmd opts ... */
      break;
    }

    lua_getfield(L, 2, "show");          /* cmd opts ... envtab */
    spawn_param_show(params, lua_type(L, -1) == LUA_TBOOLEAN ? lua_toboolean(L, -1) : 0);

    lua_getfield(L, 2, "shell");          /* cmd opts ... envtab */
    spawn_param_useshell(params, lua_type(L, -1) == LUA_TBOOLEAN ? lua_toboolean(L, -1) : 1);

    get_redirect(L, 2, "stdin", params);    /* cmd opts ... */
    get_redirect(L, 2, "stdout", params);   /* cmd opts ... */
    get_redirect(L, 2, "stderr", params);   /* cmd opts ... */
  }
  return spawn_param_execute(params);   /* proc/nil error */
}


#define tofilep(L)	((FILE **)luaL_checkudata(L, 1, LUA_FILEHANDLE))

static int pushresult (lua_State *L, int i, const char *filename) {
  int en = errno;  /* calls to Lua API may change this value */
  if (i) {
    lua_pushboolean(L, 1);
    return 1;
  }
  else {
    lua_pushnil(L);
    if (filename)
      lua_pushfstring(L, "%s: %s", filename, strerror(en));
    else
      lua_pushfstring(L, "%s", strerror(en));
    lua_pushinteger(L, en);
    return 3;
  }
}


/*
** function to close 'popen' files
*/
static int pipe_close (lua_State *L) {
  FILE **p = tofilep(L);
  int ok = fclose(*p);
  *p = NULL;
  return pushresult(L, ok, NULL);
}


static void newfenv (lua_State *L, lua_CFunction cls) {
  lua_createtable(L, 0, 1);
  lua_pushcfunction(L, cls);
  lua_setfield(L, -2, "__close");
}


int luaopen_osprocess_core(lua_State *L)
{
  const char *name = lua_tostring(L, 1);
  int ex;
  const luaL_Reg osprocess_lib[] = {
    {"pipe",       ex_pipe},
    /* environment */
    {"getenv",     ex_getenv},
    {"setenv",     ex_setenv},
    {"environ",    ex_environ},
    /* process control */
    {"sleep",      ex_sleep},
    {"spawn",      ex_spawn},
    {0,0} };
  const luaL_Reg ex_process_methods[] = {
    {"__tostring", process_tostring},
#define ex_process_functions (ex_process_methods + 1)
    {"wait",       process_wait},
    {0,0} };
  /* proc metatable */
  luaL_newmetatable(L, PROCESS_HANDLE);       /* . P */
  luaL_register(L, 0, ex_process_methods);    /* . P */
  lua_pushvalue(L, -1);                       /* . P P */
  lua_setfield(L, -2, "__index");             /* . P */
  /* make all functions available via ex.process namespace */
  lua_newtable(L);
  luaL_register(L, 0, osprocess_lib);         /* . P ex */
  ex = lua_gettop(L);
#if LUA_VERSION_NUM <= 501
  lua_getfield(L, ex, "pipe");                /* . io ex_pipe */
  newfenv(L, pipe_close);  /* create environment for 'popen' */
  lua_setfenv(L, -2);  /* set fenv for 'popen' */
  lua_pop(L, 1);  /* pop 'popen' */
#endif
  return 1;
}
