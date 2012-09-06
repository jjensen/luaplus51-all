///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2011 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#include <string.h>
#include "src/lua.h"
#include "src/lauxlib.h"
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


static void tag_error (lua_State *L, int narg, int tag) {
	luaL_typerror(L, narg, lua_typename(L, tag));
}


LUALIB_API lua_Integer luaL_checkboolean (lua_State *L, int narg) {
	lua_Integer d = lua_toboolean(L, narg);
	if (d == 0 && !lua_isboolean(L, narg))  /* avoid extra test when d is not 0 */
		tag_error(L, narg, LUA_TBOOLEAN);
	return d;
}


LUALIB_API lua_Integer luaL_optboolean (lua_State *L, int narg, int def) {
	return luaL_opt(L, luaL_checkboolean, narg, def);
}


int luaplus_base_createtable (lua_State *L) {
  lua_createtable(L, (int)luaL_optinteger(L, 1, 0), (int)luaL_optinteger(L, 2, 0));
  return 1;
}


#if LUAPLUS_DUMPOBJECT

int luaplus_ls_LuaDumpObject(lua_State*);

#endif // LUAPLUS_DUMPOBJECT


LUA_EXTERN_C void LuaPlus_ScriptFunctionsRegister(lua_State* L)
{
#if LUAPLUS_DUMPOBJECT
	lua_pushliteral(L, "LuaDumpObject");
	lua_pushcfunction(L, luaplus_ls_LuaDumpObject);
	lua_settable(L, LUA_GLOBALSINDEX);
#endif // LUAPLUS_DUMPOBJECT

#if LUA_STRING_FORMAT_EXTENSIONS
	lua_getglobal(L, "string");
	lua_pushliteral(L, "formatx");
	lua_pushcfunction(L, luaplus_str_format);
	lua_settable(L, -3);
	lua_pop(L, 1);
#endif /* LUA_STRING_FORMAT_EXTENSIONS */

	lua_pushliteral(L, "createtable");
	lua_pushcfunction(L, luaplus_base_createtable);
	lua_settable(L, LUA_GLOBALSINDEX);
}
