///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2011 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUAPLUS__LUAPLUSINTERNAL_H
#define LUAPLUS__LUAPLUSINTERNAL_H

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#if defined(LUA_BUILD_AS_DLL)
#define LUAPLUS_API LUA_API
#else
#define LUAPLUS_API
#endif

#if LUA_FASTREF_SUPPORT

#define LUA_FASTREFNIL	(-1999999)

LUA_API int lua_fastref (lua_State *L);
LUA_API int lua_fastrefindex (lua_State *L, int idx);
LUA_API void lua_fastunref (lua_State *L, int ref);
LUA_API void lua_getfastref (lua_State *L, int ref);

#else

#if !defined(LUA_FASTREFNIL)
#define LUA_FASTREFNIL LUA_REFNIL

#define lua_fastref(L) luaL_ref(L, LUA_REGISTRYINDEX)
#define lua_fastrefindex(L, idx) (lua_pushvalue(L, idx), luaL_ref(L, LUA_REGISTRYINDEX))
#define lua_fastunref(L, ref) luaL_unref(L, LUA_REGISTRYINDEX, ref)
#define lua_getfastref(L, ref) lua_rawgeti(L, LUA_REGISTRYINDEX, ref)
#endif

#endif /* LUA_FASTREF_SUPPORT */

#define LUAPLUS_INLINE inline

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus
{

class LuaException
{
public:
	LuaException(const char* message);
	~LuaException();
    LuaException(const LuaException& src);
    LuaException& operator=(const LuaException& src);

	const char* GetErrorMessage() const			{  return m_message;  }

protected:
	char* m_message;
};

//#define luaplus_assert(e) /* empty */
#if defined(__CELLOS_LV2__)  &&  !defined(LUAPLUS_EXCEPTIONS)
#define LUAPLUS_EXCEPTIONS 0
#endif

class LuaStateOutFile;
class LuaState;
class LuaStackObject;
class LuaObject;
class LuaCall;

struct LuaArgNil {};

} // namespace LuaPlus

#if !LUAPLUS_EXCEPTIONS
#include <assert.h>
#define luaplus_assert(e) if (!(e)) assert(0)
//(void)0
#define luaplus_throw(e) assert(0)
//(void)0
#else
#define luaplus_assert(e) if (!(e)) throw LuaPlus::LuaException(#e)
#define luaplus_throw(e) throw LuaPlus::LuaException(e)
#endif

#define LuaState_to_lua_State(state) ((lua_State*)(state))
#define lua_State_to_LuaState(L) ((LuaPlus::LuaState*)L)

#endif // LUAPLUS__LUAPLUSINTERNAL_H
