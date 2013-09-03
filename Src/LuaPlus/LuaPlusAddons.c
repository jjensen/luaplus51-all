///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2011 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#define LUA_CORE
#include <string.h>
#include "lua.h"
#include "lauxlib.h"
#include <assert.h>

#if LUA_STRING_FORMAT_EXTENSIONS

int str_format_helper (luaL_Buffer *b, lua_State *L, int arg);

int luaplus_str_format (lua_State *L) {
  int arg = 1;
  luaL_Buffer b;
  str_format_helper(&b, L, arg);
  luaL_pushresult(&b);
  return 1;
}

#endif /* LUA_STRING_FORMAT_EXTENSIONS */


int luaplus_base_createtable (lua_State *L) {
  lua_createtable(L, (int)luaL_optinteger(L, 1, 0), (int)luaL_optinteger(L, 2, 0));
  return 1;
}


#if LUAPLUS_DUMPOBJECT

int luaplus_ls_LuaDumpObject(lua_State*);

#endif // LUAPLUS_DUMPOBJECT


