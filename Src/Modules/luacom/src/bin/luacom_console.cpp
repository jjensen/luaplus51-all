/*
** $Id: luacom_console.cpp,v 1.3 2008/01/06 04:57:48 dmanura Exp $
** Lua stand-alone interpreter
** See Copyright Notice in lua.h
*/


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <sys/timeb.h>

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include "LuaCompat.h"
#include "luacom.h"


#ifdef _POSIX_SOURCE
#include <unistd.h>
#else
#define isatty(x)       (x==0)  /* assume stdin is a tty */
#endif


typedef void (*handler)(int);  /* type for signal actions */

static void laction (int i);


static lua_Hook old_linehook = NULL;
static lua_Hook old_callhook = NULL;

static lua_State *global_lua_state = NULL;


/////////////////////////////////////////////////////////////////////
// Lua compatibility functions

lua_Hook lua_setcallhook (lua_State *L, lua_Hook func)
{
  lua_Hook old_hook = lua_gethook(L);

  lua_sethook (L, func, LUA_MASKCALL, 0);


  return old_hook;
}

lua_Hook lua_setlinehook (lua_State *L, lua_Hook func)
{
  lua_Hook old_hook = lua_gethook(L);
  lua_sethook (L, func, LUA_MASKLINE, 0);

  return old_hook;
}

void make_lua_error(lua_State* L, const char *error)
{
  lua_pushstring(L, error);
  lua_error(L);
}

static int lua_dostring(lua_State* L, const char* code) {
  return luaL_dostring(L, code);
}
static int lua_dofile(lua_State* L, const char* filename) {
  return luaL_dofile(L, filename);
}


/////////////////////////////////////////////////////////////////////
// Windows Utilities

#include <windows.h>
static int _get_time(lua_State *lua_state)
{
   struct _timeb tval;
   _ftime(&tval);
   double t =  ((double)tval.time) + 0.001*tval.millitm;
   lua_pushnumber(lua_state, t);

   return 1; /* returns 1 value to lua */
}

static void fill_env(lua_State *lua_state, char* buffer)
{
   int i = 1, j = 0;
   
   lua_pushnumber(lua_state, (double)i);

   lua_gettable(lua_state, -2);
   for (i = 2; !lua_isnil(lua_state, -1); i++)
   {
      const char* str = lua_tostring(lua_state, -1);
      lua_remove(lua_state, -1);

      strcpy(&buffer[j],str);
      j += (strlen(str) +1 );

     lua_pushnumber(lua_state, (float)i);
     lua_gettable(lua_state, -2);
   }
}

static WORD getShowMode(const char* str_mode)
{
   if (!strcmp("SW_SHOWMAXIMIZED", str_mode))
      return SW_SHOWMAXIMIZED;
   else if (!strcmp("SW_SHOWMINIMIZED", str_mode))
      return SW_SHOWMINIMIZED;
   else if (!strcmp("SW_SHOWMINNOACTIVE", str_mode))
      return SW_SHOWMINNOACTIVE;
   else if (!strcmp("SW_RESTORE", str_mode))
      return SW_RESTORE;
   else
      return SW_SHOWNORMAL;
}

static int _create_process (lua_State *lua_state)
{
   char* commandline;
   WORD show_mode = SW_SHOWNORMAL;
   WORD flags = NORMAL_PRIORITY_CLASS;
   char* dir = NULL;
   char* env = NULL;
   static char buffer[1024];

   luaL_argcheck(lua_state,
                  lua_istable(lua_state, 1) || lua_isstring(lua_state, 1),1,
                  "must be a table or string");

   if (lua_isstring(lua_state, 1))
      commandline = _strdup(lua_tostring(lua_state, 1));
   else
   {
      lua_pushstring(lua_state, "cmd");
      lua_gettable(lua_state, 1);
      if (lua_isstring(lua_state, -1))
      {
        commandline = _strdup(lua_tostring(lua_state, -1));
      }
      else
         make_lua_error(lua_state, "The cmd field must be provided");

      lua_pushstring(lua_state, "console");
      lua_gettable(lua_state, 1);
      if (!lua_isnil(lua_state, -1))
         flags |= CREATE_NEW_CONSOLE;

      lua_pushstring(lua_state, "show_mode");
      lua_gettable(lua_state, 1);
      if (lua_isstring(lua_state, -1))
         show_mode = getShowMode(lua_tostring(lua_state, -1));

      lua_pushstring(lua_state, "dir");
      lua_gettable(lua_state, 1);
      if (lua_isstring(lua_state, -1))
         dir = _strdup(lua_tostring(lua_state, -1));

      lua_pushstring(lua_state, "env");
      lua_gettable(lua_state, 1);
      if (lua_istable(lua_state, -1))
      {
         fill_env(lua_state, buffer);
         env = buffer;
      }
   }

   STARTUPINFO si;
   PROCESS_INFORMATION pi;
   ZeroMemory(&si, sizeof(STARTUPINFO));
   si.cb = sizeof(STARTUPINFO);
   si.dwFlags = STARTF_USESHOWWINDOW;
   si.wShowWindow = show_mode;
   BOOL pr = CreateProcess(NULL,commandline,NULL,NULL,FALSE,
                           flags,env,dir,&si,&pi);

   free(commandline);
   free(dir);

   CloseHandle(pi.hThread);
   CloseHandle(pi.hProcess);
   if (!pr)
      lua_pushnil(lua_state);
   else
      lua_pushnumber(lua_state, pi.dwProcessId);

   /* number of results */
   return 1;
}

static int _kill_process(lua_State *lua_state)
{
   DWORD pid = (DWORD)luaL_checknumber(lua_state, 1);
   HANDLE hProcess = OpenProcess(PROCESS_TERMINATE,0,pid);

   if (hProcess != NULL)
   {
      TerminateProcess(hProcess, 0);
      CloseHandle(hProcess);
   }

   return 0;
}

static int _sleep(lua_State *lua_state)
{
   double dtime = luaL_checknumber(lua_state, 1);
   DWORD time = (DWORD) (dtime*1000);
   Sleep(time);

   return 0; /* returns no values to lua */
}

static void init_windows(lua_State *lua_state)
{
   lua_register(lua_state, "create_process",_create_process);
   lua_register(lua_state, "kill_process",_kill_process);
   lua_register(lua_state, "sleep",_sleep);
   lua_register(lua_state, "get_time",_get_time);
}

static handler lreset (void) {
  return signal(SIGINT, laction);
}


static void lstop (lua_State *lua_state, lua_Debug *ar) {
  lua_setlinehook(lua_state, old_linehook);
  lua_setcallhook(lua_state, old_callhook);
  lreset();
  make_lua_error(lua_state, "interrupted!");
}


static void laction (int i) {
  signal(SIGINT, SIG_DFL); /* if another SIGINT happens before lstop,
                              terminate process (default action) */
  old_linehook = lua_setlinehook(global_lua_state, (lua_Hook)lstop);
  old_callhook = lua_setcallhook(global_lua_state, (lua_Hook)lstop);
}


static int ldo (lua_State *lua_state, int (*f)(lua_State *,const char *), char *name) {
  int res;
  global_lua_state = lua_state;
  handler h = lreset();
  res = f(lua_state, name);  /* dostring | dofile */
  signal(SIGINT, h);  /* restore old action */
  global_lua_state = NULL;
  return res;
}


static void print_message (void) {
  fprintf(stderr,
"LuaCom: command line options:\n"
"  -v       print version information\n"
"  -d       turn debug on\n"
"  -e stat  dostring `stat'\n"
"  -q       interactive mode without prompt\n"
"  -i       interactive mode with prompt\n"
"  -        executes stdin as a file\n"
"  a=b      sets global `a' with string `b'\n"
"  name     dofile `name'\n\n");
}


static void assign (lua_State *lua_state, char *arg) {
  if (strlen(arg) >= 500)
    fprintf(stderr, "lua: shell argument too long");
  else {
    char buffer[500];
    char *eq = strchr(arg, '=');
    lua_pushstring(lua_state, eq+1);
    strncpy(buffer, arg, eq-arg);
    buffer[eq-arg] = 0;
    lua_setglobal(lua_state, buffer);
  }
}


static void manual_input (lua_State *lua_state, int prompt) {
  int cont = 1;
  while (cont) {
    char buffer[BUFSIZ];
    int i = 0;

    if (prompt)
    {
      lua_getglobal(lua_state, "_PROMPT");
      printf("%s", lua_tostring(lua_state, -1));
      lua_remove(lua_state, -1);
    }
    for(;;) {
      int c = getchar();
      if (c == EOF) {
        cont = 0;
        break;
      }
      else if (c == '\n') {
        if (i>0 && buffer[i-1] == '\\')
          buffer[i-1] = '\n';
        else break;
      }
      else if (i >= BUFSIZ-1) {
        fprintf(stderr, "lua: argument line too long\n");
        break;
      }
      else buffer[i++] = (char)c;
    }
    buffer[i] = '\0';
    ldo(lua_state, lua_dostring, buffer);
  }
  printf("\n");
}


int main (int argc, char *argv[])
{
  int i;

  OleInitialize(NULL);

  SetConsoleTitle(LUACOM_VERSION);

  lua_State *lua_state = lua_open();
  luaL_openlibs(lua_state);

  lua_pushstring(lua_state, "> "); lua_setglobal(lua_state, "_PROMPT");

  luacom_open(lua_state);
  init_windows(lua_state);
  if (argc < 2) {  /* no arguments? */
    if (isatty(0)) {
      printf("%s  %s\n", LUACOM_VERSION, LUACOM_COPYRIGHT);
      manual_input(lua_state, 1);
    }
    else
      ldo(lua_state, lua_dofile, NULL);  /* executes stdin as a file */
  }
  else for (i=1; i<argc; i++) {
    if (argv[i][0] == '-') {  /* option? */
      switch (argv[i][1]) {
        case 0:
          ldo(lua_state, lua_dofile, NULL);  /* executes stdin as a file */
          break;
        case 'i':
          manual_input(lua_state, 1);
          break;
        case 'q':
          manual_input(lua_state, 0);
          break;
        case 'v':
          printf("%s  %s\n(written by %s)\n\n",
                 LUACOM_VERSION, LUACOM_COPYRIGHT, LUACOM_AUTHORS);
          break;
        case 'e':
          i++;
          if (ldo(lua_state, lua_dostring, argv[i]) != 0) {
            fprintf(stderr, "lua: error running argument `%s'\n", argv[i]);
            return 1;
          }
          break;
        default:
          print_message();
          exit(1);
      }
    }
    else if (strchr(argv[i], '='))
      assign(lua_state, argv[i]);
    else {
      int result = ldo(lua_state, lua_dofile, argv[i]);
      if (result) {
        if (result == 2) {
          fprintf(stderr, "lua: cannot execute file ");
          perror(argv[i]);
        }
        exit(1);
      }
    }
  }

  luacom_close(lua_state);
  lua_close(lua_state);

  OleUninitialize();

  return 0;
}

