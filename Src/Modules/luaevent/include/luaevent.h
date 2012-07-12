/* LuaEvent - Copyright (C) 2007,2012 Thomas Harning <harningt@gmail.com>
 * Licensed as LGPL - See doc/COPYING for details */
#ifndef LUAEVENT_H
#define LUAEVENT_H

#include <lua.h>

/* Workarounds for Lua 5.2 */
#if (LUA_VERSION_NUM == 502)

#undef lua_equal
#define lua_equal(L,idx1,idx2) lua_compare(L, (idx1), (idx2), LUA_OPEQ)

#undef lua_getfenv
#define lua_getfenv lua_getuservalue
#undef lua_setfenv
#define lua_setfenv lua_setuservalue

#undef lua_objlen
#define lua_objlen lua_rawlen

#undef luaL_register
#define luaL_register(L, n, f) \
	{ if ((n) == NULL) luaL_setfuncs(L, f, 0); else luaL_newlib(L, f); }

#endif

#include <sys/types.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/time.h>
#endif
#include <event.h>

typedef struct {
	struct event_base* base;
	lua_State* loop_L;
} le_base;

le_base* event_base_get(lua_State* L, int idx);
void load_timeval(double time, struct timeval *tv);
int getSocketFd(lua_State* L, int idx);

int luaopen_luaevent(lua_State* L);

#endif
