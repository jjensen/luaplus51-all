///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2005 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://wwhiz.com/LuaPlus/.
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
#include "LuaStackObject.h"
#include "src/lua.h"
#ifndef LUAPLUS_ENABLE_INLINES
#include "LuaStackObject.inl"
#endif // LUAPLUS_ENABLE_INLINES

namespace LuaPlus {

} // namespace LuaPlus
