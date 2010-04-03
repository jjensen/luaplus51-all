///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2005 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://wwhiz.com/LuaPlus/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER
#ifndef LUASTATE_INL
#define LUASTATE_INL

#include <string.h>
#include <assert.h>

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus
{

LUAPLUS_INLINE lua_CFunction LuaState::AtPanic(lua_CFunction panicf)
{
	return lua_atpanic(LuaState_to_lua_State(this), panicf);
}

// Basic stack manipulation.
LUAPLUS_INLINE int LuaState::GetTop()
{
	return lua_gettop(LuaState_to_lua_State(this));
}

LUAPLUS_INLINE void LuaState::SetTop(int index)
{
	lua_settop(LuaState_to_lua_State(this), index);
}

LUAPLUS_INLINE void LuaState::PushValue(int index)
{
	lua_pushvalue(LuaState_to_lua_State(this), index);
}

LUAPLUS_INLINE void LuaState::PushValue(LuaStackObject& object)
{
	lua_pushvalue(LuaState_to_lua_State(this), object);
}

LUAPLUS_INLINE void LuaState::Remove(int index)
{
	lua_remove(LuaState_to_lua_State(this), index);
}

LUAPLUS_INLINE void LuaState::Insert(int index)
{
	lua_insert(LuaState_to_lua_State(this), index);
}

LUAPLUS_INLINE void LuaState::Replace(int index)
{
	lua_replace(LuaState_to_lua_State(this), index);
}


LUAPLUS_INLINE int LuaState::CheckStack(int size)
{
	return lua_checkstack(LuaState_to_lua_State(this), size);
}


LUAPLUS_INLINE void LuaState::XMove(LuaState* to, int n)
{
	lua_xmove(LuaState_to_lua_State(this), LuaState_to_lua_State(to), n);
}

	
// access functions (stack -> C)
LUAPLUS_INLINE int LuaState::Equal(int index1, int index2)
{
	return lua_equal(LuaState_to_lua_State(this), index1, index2);
}

LUAPLUS_INLINE int LuaState::RawEqual(int index1, int index2)
{
	return lua_rawequal(LuaState_to_lua_State(this), index1, index2);
}

LUAPLUS_INLINE int LuaState::LessThan(int index1, int index2)
{
	return lua_lessthan(LuaState_to_lua_State(this), index1, index2);
}


// push functions (C -> stack)
LUAPLUS_INLINE LuaStackObject LuaState::PushNil()
{
	lua_pushnil(LuaState_to_lua_State(this));
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushNumber(lua_Number n)
{
	lua_pushnumber(LuaState_to_lua_State(this), n);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushInteger(int n)
{
	lua_pushnumber(LuaState_to_lua_State(this), n);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushLString(const char *s, size_t len)
{
	lua_pushlstring(LuaState_to_lua_State(this), s, len);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushString(const char *s)
{
	lua_pushstring(LuaState_to_lua_State(this), s);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}

#if LUA_WIDESTRING
LUAPLUS_INLINE LuaStackObject LuaState::PushLWString(const lua_WChar* s, size_t len)
{
	lua_pushlwstring(LuaState_to_lua_State(this), s, len);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushWString(const lua_WChar* s)
{
	lua_pushwstring(LuaState_to_lua_State(this), s);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}
#endif /* LUA_WIDESTRING */

LUAPLUS_INLINE LuaStackObject LuaState::PushCClosure(lua_CFunction fn, int n)
{
	lua_pushcclosure(LuaState_to_lua_State(this), fn, n);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushCFunction(lua_CFunction f)
{
	lua_pushcclosure(LuaState_to_lua_State(this), f, 0);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushBoolean(bool value)
{
	lua_pushboolean(LuaState_to_lua_State(this), value);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushLightUserData(void* p)
{
	lua_pushlightuserdata(LuaState_to_lua_State(this), p);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushThread()
{
	lua_pushthread(LuaState_to_lua_State(this));
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}


LUAPLUS_INLINE LuaStackObject LuaState::CreateTable(int narr, int nrec)
{
	lua_createtable(LuaState_to_lua_State(this), narr, nrec);
	return LuaStackObject(*this, GetTop());
}


LUAPLUS_INLINE LuaStackObject LuaState::NewUserData(size_t size)
{
	lua_newuserdata(LuaState_to_lua_State(this), size);
	return LuaStackObject(*this, GetTop());
}

// get functions (Lua -> stack)
LUAPLUS_INLINE void LuaState::GetTable(int index)
{
	lua_gettable(LuaState_to_lua_State(this), index);
}

LUAPLUS_INLINE void LuaState::RawGet(int index)
{
	lua_rawget(LuaState_to_lua_State(this), index);
}

LUAPLUS_INLINE void LuaState::RawGetI(int index, int n)
{
	lua_rawgeti(LuaState_to_lua_State(this), index, n);
}

LUAPLUS_INLINE LuaStackObject LuaState::GetMetaTable(int index)
{
	lua_getmetatable(LuaState_to_lua_State(this), index);  return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}

LUAPLUS_INLINE LuaStackObject LuaState::GetGlobals_Stack()
{
	return LuaStackObject(*this, LUA_GLOBALSINDEX);
}

LUAPLUS_INLINE LuaStackObject LuaState::GetGlobal_Stack(const char *name)
{
	lua_getglobal(LuaState_to_lua_State(this), name);  return LuaStackObject(*this, GetTop());
}

LUAPLUS_INLINE LuaStackObject LuaState::GetRegistry_Stack()
{
	return LuaStackObject(*this, LUA_REGISTRYINDEX);  //{  lua_getregistry(LuaState_to_lua_State(this));
}


// set functions(stack -> Lua)
LUAPLUS_INLINE void LuaState::SetTable(int index)
{
	lua_settable(LuaState_to_lua_State(this), index);
}

LUAPLUS_INLINE void LuaState::RawSet(int index)
{
	lua_rawset(LuaState_to_lua_State(this), index);
}

LUAPLUS_INLINE void LuaState::RawSetI(int index, int n)
{
	lua_rawseti(LuaState_to_lua_State(this), index, n);
}

LUAPLUS_INLINE void LuaState::SetMetaTable(int index)
{
	lua_setmetatable(LuaState_to_lua_State(this), index);
}

LUAPLUS_INLINE void LuaState::SetFEnv(int index)
{
	lua_setfenv(LuaState_to_lua_State(this), index);
}

LUAPLUS_INLINE int LuaState::Ref(int t)
{
	return luaL_ref(LuaState_to_lua_State(this), t);
}

LUAPLUS_INLINE void LuaState::Unref(int t, int ref)
{
	luaL_unref(LuaState_to_lua_State(this), t, ref);
}


// `load' and `do' functions (load and run Lua code)
LUAPLUS_INLINE void LuaState::Call(int nargs, int nresults)
{
	lua_call(LuaState_to_lua_State(this), nargs, nresults);
}

LUAPLUS_INLINE int LuaState::PCall(int nargs, int nresults, int errf)
{
	return lua_pcall(LuaState_to_lua_State(this), nargs, nresults, errf);
}

LUAPLUS_INLINE int LuaState::CPCall(lua_CFunction func, void* ud)
{
	return lua_cpcall(LuaState_to_lua_State(this), func, ud);
}

LUAPLUS_INLINE int LuaState::Load(lua_Reader reader, void *dt, const char *chunkname)
{
	return lua_load(LuaState_to_lua_State(this), reader, dt, chunkname);
}

#if LUA_WIDESTRING
LUAPLUS_INLINE int LuaState::WLoad(lua_Reader reader, void *dt, const char *chunkname)
{
	return lua_wload(LuaState_to_lua_State(this), reader, dt, chunkname);
}
#endif /* LUA_WIDESTRING */

#if LUA_ENDIAN_SUPPORT
LUAPLUS_INLINE int LuaState::Dump(lua_Chunkwriter writer, void* data, int strip, char endian)
{
	return lua_dumpendian(LuaState_to_lua_State(this), writer, data, strip, endian);
}
#else
LUAPLUS_INLINE int LuaState::Dump(lua_Chunkwriter writer, void* data)
{
	return lua_dump(LuaState_to_lua_State(this), writer, data);
}
#endif /* LUA_ENDIAN_SUPPORT */

LUAPLUS_INLINE int LuaState::LoadFile(const char* filename)
{
	return luaL_loadfile(LuaState_to_lua_State(this), filename);
}

LUAPLUS_INLINE int LuaState::DoFile(const char *filename)
{
	return luaL_dofile(LuaState_to_lua_State(this), filename);
}

LUAPLUS_INLINE void callalert (lua_State *L, int status) {
  if (status != 0) {
    lua_getglobal(L, "_ALERT");
    if (lua_isfunction(L, -1)) {
      lua_insert(L, -2);
      lua_call(L, 1, 0);
    }
    else {  /* no _ALERT function; print it on stderr */
      fprintf(stderr, "%s\n", lua_tostring(L, -2));
      lua_pop(L, 2);  /* remove error message and _ALERT */
    }
  }
}

#if 0
static int traceback (lua_State *L)
{
	luaL_getfield(L, LUA_GLOBALSINDEX, "debug.traceback");
	if (!lua_isfunction(L, -1))
		lua_pop(L, 1);
	else
	{
		lua_pushvalue(L, 1);  /* pass error message */
		lua_pushinteger(L, 2);  /* skip this function and traceback */
		lua_call(L, 2, 1);  /* call debug.traceback */
	}
	return 1;
}
#endif


LUAPLUS_INLINE int aux_do (lua_State *L, int status) {
  if (status == 0) {  /* parse OK? */
    status = lua_pcall(L, 0, LUA_MULTRET, 0);  /* call main */
  }
  callalert(L, status);
  return status;
}


LUAPLUS_INLINE int LuaState::DoString(const char *str)
{
	return luaL_dostring(LuaState_to_lua_State(this), str);
}

LUAPLUS_INLINE int LuaState::LoadBuffer(const char* buff, size_t size, const char* name)
{
	return luaL_loadbuffer(LuaState_to_lua_State(this), buff, size, name);
}

LUAPLUS_INLINE int LuaState::DoBuffer(const char *buff, size_t size, const char *name)
{
	return (luaL_loadbuffer(LuaState_to_lua_State(this), buff, size, name) || lua_pcall(LuaState_to_lua_State(this), 0, 0, 0));
}

#if LUA_WIDESTRING
LUAPLUS_INLINE int LuaState::DoWString(const lua_WChar *str, const char* name)
{
	(void)name;
	return luaL_dowstring(LuaState_to_lua_State(this), str);
}

LUAPLUS_INLINE int LuaState::LoadWBuffer(const lua_WChar* buff, size_t size, const char* name)
{
	return luaL_loadwbuffer(LuaState_to_lua_State(this), buff, size, name);
}

LUAPLUS_INLINE int LuaState::DoWBuffer(const lua_WChar* buff, size_t size, const char *name)
{
	return (luaL_loadwbuffer(LuaState_to_lua_State(this), buff, size, name) || lua_pcall(LuaState_to_lua_State(this), 0, 0, 0));
}
#endif /* LUA_WIDESTRING */

// coroutine functions
LUAPLUS_INLINE int LuaState::CoYield(int nresults)
{
	return lua_yield(LuaState_to_lua_State(this), nresults);
}

LUAPLUS_INLINE int LuaState::CoResume(int narg)
{
	return lua_resume(LuaState_to_lua_State(this), narg);
}

LUAPLUS_INLINE int LuaState::CoStatus()
{
	return lua_status(LuaState_to_lua_State(this));
}

// Miscellaneous functions
LUAPLUS_INLINE int LuaState::Error()
{
	return lua_error(LuaState_to_lua_State(this));
}


LUAPLUS_INLINE int LuaState::Next(int index)
{
	return lua_next(LuaState_to_lua_State(this), index);
}

LUAPLUS_INLINE void LuaState::Concat(int n)
{
	lua_concat(LuaState_to_lua_State(this), n);
}


LUAPLUS_INLINE lua_Alloc LuaState::GetAllocF(void **ud)
{
	return lua_getallocf(LuaState_to_lua_State(this), ud);
}


LUAPLUS_INLINE void LuaState::SetAllocF(lua_Alloc f, void *ud)
{
	lua_setallocf(LuaState_to_lua_State(this), f, ud);
}

// Helper functions
LUAPLUS_INLINE void LuaState::Pop()
{
	lua_pop(LuaState_to_lua_State(this), 1);
}

LUAPLUS_INLINE void LuaState::Pop(int amount)
{
	lua_pop(LuaState_to_lua_State(this), amount);
}


LUAPLUS_INLINE int LuaState::GC(int what, int data)
{
	return lua_gc(LuaState_to_lua_State(this), what, data); 
}


// Debug functions.
LUAPLUS_INLINE int LuaState::GetStack(int level, lua_Debug* ar)
{
	return lua_getstack(LuaState_to_lua_State(this), level, ar);
}

LUAPLUS_INLINE int LuaState::GetInfo(const char* what, lua_Debug* ar)
{
	return lua_getinfo(LuaState_to_lua_State(this), what, ar);
}

LUAPLUS_INLINE const char* LuaState::GetLocal(const lua_Debug* ar, int n)
{
	return lua_getlocal(LuaState_to_lua_State(this), ar, n);
}

LUAPLUS_INLINE const char* LuaState::SetLocal(const lua_Debug* ar, int n)
{
	return lua_setlocal(LuaState_to_lua_State(this), ar, n);
}


LUAPLUS_INLINE int LuaState::SetHook(lua_Hook func, int mask, int count)
{
	return lua_sethook(LuaState_to_lua_State(this), func, mask, count);
}

LUAPLUS_INLINE lua_Hook LuaState::GetHook()
{
	return lua_gethook(LuaState_to_lua_State(this));
}

LUAPLUS_INLINE int LuaState::GetHookMask()
{
	return lua_gethookmask(LuaState_to_lua_State(this));
}


// Extra
LUAPLUS_INLINE LuaStackObject LuaState::BoxPointer(void* u)
{
	(*(void **)(lua_newuserdata(LuaState_to_lua_State(this), sizeof(void *))) = (u));  return LuaStackObject(*this, lua_gettop(LuaState_to_lua_State(this)));
}

LUAPLUS_INLINE void* LuaState::UnBoxPointer(int stackIndex)
{
	return (*(void **)(lua_touserdata(LuaState_to_lua_State(this), stackIndex)));
}



} // namespace LuaPlus

#endif // LUASTATE_INL
