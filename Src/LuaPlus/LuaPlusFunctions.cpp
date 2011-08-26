///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2011 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#include "LuaPlus.h"
#include "LuaPlus_Libs.h"

namespace LuaPlus {

void ScriptFunctionsRegister(struct lua_State* L)
{
#if LUAPLUS_EXTENSIONS
	LuaState* state = lua_State_To_LuaState(L);
	LuaObject globalsObj = state->GetGlobals();

#if LUAPLUS_DUMPOBJECT
    state->GetGlobals().Register("LuaDumpGlobals", LS_LuaDumpGlobals);
	state->GetGlobals().Register("LuaDumpObject", LS_LuaDumpObject);
	state->GetGlobals().Register("LuaDumpFile", LS_LuaDumpFile);
#endif // LUAPLUS_DUMPOBJECT
#endif // LUAPLUS_EXTENSIONS
}

} // namespace LuaPlus


LUA_EXTERN_C void LuaPlus_ScriptFunctionsRegister(lua_State* L)
{
	LuaPlus::ScriptFunctionsRegister(L);
}
