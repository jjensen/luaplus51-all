///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2011 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUAPLUS__LUAPLUS_H
#define LUAPLUS__LUAPLUS_H

#include "LuaPlusInternal.h"
#include "LuaStackObject.h"
#include "LuaObject.h"
#include "LuaState.h"
#include "LuaTableIterator.h"
#include "LuaObject.inl"
#include "LuaStateOutFile.h"
#include "LuaStateOutString.h"
#include "LuaHelper.h"
#include "LuaAutoBlock.h"
#include "LuaStackTableIterator.h"
#include "LuaCall.h"
#include "LuaFunction.h"
#include "LuaPlusCD.h"

extern "C" {
#define LUA_ALLOC_TEMP 1
LUAPLUS_API void lua_getdefaultallocfunction(lua_Alloc* allocFunc, void** ud);
LUAPLUS_API void lua_setdefaultallocfunction(lua_Alloc reallocFunc, void* ud);
}

#endif // LUAPLUS__LUAPLUS_H
