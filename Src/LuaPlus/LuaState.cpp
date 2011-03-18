///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef BUILDING_LUAPLUS
#define BUILDING_LUAPLUS
#endif
#include "LuaLink.h"
LUA_EXTERN_C_BEGIN
#include "src/lobject.h"
LUA_EXTERN_C_END
#include "LuaPlus.h"
#include "LuaState.h"
//#include "LuaCall.h"
#include "LuaPlusCD.h"

#if defined(WIN32) && !defined(_XBOX) && !defined(_XBOX_VER)
#include <windows.h>
#undef GetObject
#undef LoadString
#elif defined(_XBOX) || defined(_XBOX_VER)
#include <xtl.h>
#endif // WIN32

#ifdef _MSC_VER
#pragma warning(disable: 4702)
#endif // _MSC_VER

#include <ctype.h>

LUA_EXTERN_C_BEGIN
#include "src/lstate.h"
#include "src/lvm.h"
#include "src/lgc.h"
LUA_EXTERN_C_END


#ifndef LUAPLUS_ENABLE_INLINES
#include "LuaState.inl"
#endif // LUAPLUS_ENABLE_INLINES

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


#if LUAPLUS_EXTENSIONS

LuaObject LuaState::GetLocalByName( int level, const char* name )
{
	lua_State * L = GetCState();
	lua_Debug ar;
	int i;
	const char *localName;
	if (lua_getstack(L, level, &ar) == 0)
		return LuaObject(this);  /* failure: no such level in the stack */
	i = 1;
	while ((localName = lua_getlocal(L, &ar, i++)) != NULL) {
		if (strcmp(name, localName) == 0)
		{
			LuaObject obj(this, -1);
			lua_pop(L, 1);
			return obj;
		}
		lua_pop(L, 1);  /* remove variable value */
	}
	return LuaObject(this);
}


namespace LuaHelper {

LUAPLUS_API void MergeObjects(LuaObject& mergeTo, LuaObject& mergeFrom, bool replaceDuplicates)
{
	if (mergeTo.GetState() == mergeFrom.GetState())
	{
		for (LuaTableIterator it(mergeFrom); it; ++it)
		{
			LuaObject toNodeKeyObj = mergeTo[it.GetKey()];
			if (it.GetValue().IsTable())
			{
				if (toNodeKeyObj.IsNil()  ||  replaceDuplicates)
				{
					toNodeKeyObj = mergeTo.CreateTable(it.GetKey());
				}
				MergeObjects(toNodeKeyObj, it.GetValue(), replaceDuplicates);
			}
			else if (toNodeKeyObj.IsNil()  ||  replaceDuplicates)
			{
				mergeTo.Set(it.GetKey(), it.GetValue());
			}
		}
	}
	else
	{
		for (LuaTableIterator it(mergeFrom); it; ++it)
		{
			LuaObject obj;
			switch (it.GetKey().Type())
			{
				case LUA_TBOOLEAN:	obj.Assign(mergeTo.GetState(), it.GetKey().GetBoolean());		break;
				case LUA_TNUMBER:	obj.Assign(mergeTo.GetState(), it.GetKey().GetNumber());			break;
				case LUA_TSTRING:	obj.Assign(mergeTo.GetState(), it.GetKey().GetString());			break;
#if LUA_WIDESTRING
				case LUA_TWSTRING:	obj.Assign(mergeTo.GetState(), it.GetKey().GetWString());		break;
#endif /* LUA_WIDESTRING */
			}

			LuaObject toNodeKeyObj = mergeTo[obj];

			if (it.GetValue().IsTable())
			{
				if (toNodeKeyObj.IsNil()  ||  replaceDuplicates)
				{
					toNodeKeyObj = mergeTo.CreateTable(it.GetKey());
				}
				MergeObjects(toNodeKeyObj, it.GetValue(), replaceDuplicates);
			}
			else if (toNodeKeyObj.IsNil()  ||  replaceDuplicates)
			{
				LuaObject toKeyObj;
				switch (it.GetKey().Type())
				{
					case LUA_TBOOLEAN:	toKeyObj.Assign(mergeTo.GetState(), it.GetKey().GetBoolean());		break;
					case LUA_TNUMBER:	toKeyObj.Assign(mergeTo.GetState(), it.GetKey().GetNumber());			break;
					case LUA_TSTRING:	toKeyObj.Assign(mergeTo.GetState(), it.GetKey().GetString());			break;
#if LUA_WIDESTRING
					case LUA_TWSTRING:	toKeyObj.Assign(mergeTo.GetState(), it.GetKey().GetWString());		break;
#endif /* LUA_WIDESTRING */
				}

				switch (it.GetValue().Type())
				{
					case LUA_TBOOLEAN:	mergeTo.Set(toKeyObj, it.GetValue().GetBoolean());	break;
					case LUA_TNUMBER:	mergeTo.Set(toKeyObj, it.GetValue().GetNumber());		break;
					case LUA_TSTRING:	mergeTo.Set(toKeyObj, it.GetValue().GetString());		break;
#if LUA_WIDESTRING
					case LUA_TWSTRING:	mergeTo.Set(toKeyObj, it.GetValue().GetWString());	break;
#endif /* LUA_WIDESTRING */
				}
			}
		}
	}
}

} // namespace LuaHelper

#endif // LUAPLUS_EXTENSIONS

} // namespace LuaPlus


