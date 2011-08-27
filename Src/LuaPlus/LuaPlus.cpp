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
#include <string.h>
#ifdef WIN32
#if defined(WIN32) && !defined(_XBOX) && !defined(_XBOX_VER) && !defined(_WIN32_WCE)
#include <windows.h>
#elif defined(_XBOX) || defined(_XBOX_VER)
#include <xtl.h>
#endif // WIN32
#endif // WIN32

//-----------------------------------------------------------------------------
LUA_EXTERN_C_BEGIN

#if LUA_MEMORY_STATS
static void *luaplus_alloc (void *ud, void *ptr, size_t osize, size_t nsize, const char* allocName, unsigned int flags) {
  (void)allocName;
  (void)flags;
#else
static void *luaplus_alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
#endif /* LUA_MEMORY_STATS */
  (void)osize;
  (void)ud;
  if (nsize == 0) {
    free(ptr);
    return NULL;
  }
  else
    return realloc(ptr, nsize);
}


static lua_Alloc luaHelper_defaultAlloc = luaplus_alloc;
static void* luaHelper_ud = NULL;

void lua_getdefaultallocfunction(lua_Alloc* allocFunc, void** ud)
{
	*allocFunc = luaHelper_defaultAlloc;
	*ud = luaHelper_ud;
}


void lua_setdefaultallocfunction(lua_Alloc allocFunc, void* ud)
{
	luaHelper_defaultAlloc = allocFunc ? allocFunc : luaplus_alloc;
	luaHelper_ud = ud;
}


int LuaState_FatalError( lua_State* );

LUA_EXTERN_C_END

namespace LuaPlus
{

#if LUAPLUS_EXCEPTIONS

LuaException::LuaException(const char* message)
	: m_message(NULL)
{
	if (message)
	{
		m_message = new char[strlen(message) + 1];
		strcpy(m_message, message);
	}
}


LuaException::LuaException(const LuaException& src)
{
    m_message = new char[strlen(src.m_message) + 1];
    strcpy(m_message, src.m_message);
}


LuaException& LuaException::operator=(const LuaException& src)
{
    delete[] m_message;
    m_message = new char[strlen(src.m_message) + 1];
    strcpy(m_message, src.m_message);
    return *this;
}


LuaException::~LuaException()
{
    delete[] m_message;
}

#endif // LUAPLUS_EXCEPTIONS


/*static*/ LuaState* LuaState::Create()
{
	lua_State* L = lua_newstate(luaHelper_defaultAlloc, luaHelper_ud);
	lua_atpanic(L, LuaState_FatalError);
	return lua_State_To_LuaState(L);
}


/*static*/ void LuaState::Destroy( LuaState* state )
{
	lua_State* L = LuaState_to_lua_State(state);
	lua_close(L);
}


#if 0

namespace LuaPlus {

LuaStackObject LuaState::PushVFString(const char *fmt, va_list argp) {
	lua_State* L = LuaState_to_lua_State(this);
	lua_lock(L);
	luaC_checkGC(L);
	luaO_pushvfstring(L, fmt, argp);
	lua_unlock(L);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}


LuaStackObject LuaState::PushFString(const char *fmt, ...) {
	lua_State* L = LuaState_to_lua_State(this);
	va_list argp;
	lua_lock(L);
	luaC_checkGC(L);
	va_start(argp, fmt);
	luaO_pushvfstring(L, fmt, argp);
	va_end(argp);
	lua_unlock(L);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}

} // namespace LuaPlus

#endif

} // namespace LuaPlus

LUA_EXTERN_C_BEGIN

/**
**/
int LuaState_FatalError( lua_State* L )
{
	const char* err = lua_tostring(L, 1);
#ifdef WIN32
	if (err)
		OutputDebugString(err);
#else // !WIN32
	if (err)
		puts(err);
#endif // WIN32

#ifndef _WIN32_WCE
	luaplus_throw(err);
#endif // _WIN32_WCE

	return -1;
}

LUA_EXTERN_C_END
