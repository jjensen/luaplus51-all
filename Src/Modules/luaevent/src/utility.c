/* LuaEvent - Copyright (C) 2007,2012 Thomas Harning <harningt@gmail.com>
 * Licensed as LGPL - See doc/COPYING for details */
#include "utility.h"
#include <lauxlib.h>

#define WEAK_REF_LOCATION le_register_utility

static void get_weakref_table(lua_State* L) {
	lua_pushlightuserdata(L, WEAK_REF_LOCATION);
	lua_gettable(L, LUA_REGISTRYINDEX);
}

void le_weak_ref(lua_State* L, void* ptr, int idx) {
	get_weakref_table(L);
	lua_pushlightuserdata(L, ptr);
	if(idx < 0) idx-=2;
	lua_pushvalue(L, idx);
	lua_settable(L, -3);
}
void le_weak_unref(lua_State* L, void* ptr) {
	get_weakref_table(L);
	lua_pushlightuserdata(L, ptr);
	lua_pushnil(L);
	lua_settable(L, -3);
}

void le_weak_get(lua_State* L, void* ptr) {
	get_weakref_table(L);
	lua_pushlightuserdata(L, ptr);
	lua_gettable(L, -2);
}

static void push_weak_table(lua_State* L, const char* mode) {
	lua_newtable(L);
	lua_createtable(L,0,1);
	lua_pushstring(L,mode);
	lua_setfield(L,-2,"__mode");
	lua_setmetatable(L,-2);
}

void le_register_utility(lua_State* L) {
	lua_pushlightuserdata(L, WEAK_REF_LOCATION);
	push_weak_table(L, "v");
	lua_settable(L, LUA_REGISTRYINDEX);
}
