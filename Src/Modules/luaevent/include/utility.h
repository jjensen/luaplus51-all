/* LuaEvent - Copyright (C) 2007,2012 Thomas Harning <harningt@gmail.com>
 * Licensed as LGPL - See doc/COPYING for details */
#ifndef UTILITY_H
#define UTILITY_H

#include <lua.h>

void le_weak_ref(lua_State* L, void* ptr, int idx);
void le_weak_unref(lua_State* L, void* ptr);
void le_weak_get(lua_State* L, void* ptr);

void le_register_utility(lua_State* L);
#endif
