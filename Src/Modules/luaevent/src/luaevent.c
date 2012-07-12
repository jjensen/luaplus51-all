/* LuaEvent - Copyright (C) 2007,2012 Thomas Harning <harningt@gmail.com>
 * Licensed as LGPL - See doc/COPYING for details */

#include "event_callback.h"
#include "event_buffer.h"
#include "buffer_event.h"

#include <lauxlib.h>
#include <assert.h>
#include <string.h>

#define EVENT_BASE_MT "EVENT_BASE_MT"

le_base* event_base_get(lua_State* L, int idx) {
	return (le_base*)luaL_checkudata(L, idx, EVENT_BASE_MT);
}

int luaevent_newbase(lua_State* L) {
	le_base *base = (le_base*)lua_newuserdata(L, sizeof(le_base));
	base->loop_L = NULL; /* No running loop */
	base->base = event_init();
	luaL_getmetatable(L, EVENT_BASE_MT);
	lua_setmetatable(L, -2);
	return 1;
}

int luaevent_libevent_version(lua_State* L) {
	lua_pushstring(L, event_get_version());
	return 1;
}

static int luaevent_base_gc(lua_State* L) {
	le_base *base = event_base_get(L, 1);
	if(base->base) {
		event_base_free(base->base);
		base->base = NULL;
	}
	return 0;
}

int getSocketFd(lua_State* L, int idx) {
	int fd;
	if(lua_isnumber(L, idx)) {
		fd = lua_tonumber(L, idx);
	} else {
		luaL_checktype(L, idx, LUA_TUSERDATA);
		lua_getfield(L, idx, "getfd");
		if(lua_isnil(L, -1))
			return luaL_error(L, "Socket type missing 'getfd' method");
		lua_pushvalue(L, idx);
		lua_call(L, 1, 1);
		fd = lua_tointeger(L, -1);
		lua_pop(L, 1);
	}
	return fd;
}

void load_timeval(double time, struct timeval *tv) {
	tv->tv_sec = (int) time;
	tv->tv_usec = (int)( (time - tv->tv_sec) * 1000000 );
}

/* sock, event, callback, timeout */
static int luaevent_addevent(lua_State* L) {
	int fd, event;
	le_callback* arg = event_callback_push(L, 1, 4);
	struct timeval *tv = &arg->timeout;
	if(lua_isnil(L, 2) && lua_isnumber(L, 5)) {
		fd = -1; /* Per event_timer_set.... */
	} else {
		fd = getSocketFd(L, 2);
	}
	event = luaL_checkinteger(L, 3);
	if(lua_isnumber(L, 5)) {
		double time = lua_tonumber(L, 5);
		load_timeval(time, tv);
	} else {
		tv = NULL;
	}

	/* Setup event... */
	event_set(&arg->ev, fd, event | EV_PERSIST, luaevent_callback, arg);
	event_base_set(arg->base->base, &arg->ev);
	event_add(&arg->ev, tv);
	return 1;
}

static int luaevent_loop(lua_State* L) {
	int ret;
	le_base *base = event_base_get(L, 1);
	base->loop_L = L;
	ret = event_base_loop(base->base, 0);
	lua_pushinteger(L, ret);
	return 1;
}

static int luaevent_loopexit(lua_State*L) {
	int ret;
	le_base *base = event_base_get(L, 1);
	struct timeval tv = { 0, 0 };
	if(lua_gettop(L) >= 2) /* Optional timeout before exiting the loop */
		load_timeval(luaL_checknumber(L, 2), &tv);
	ret = event_base_loopexit(base->base, &tv);
	lua_pushinteger(L, ret);
	return 1;
}

static int luaevent_method(lua_State* L) {
	#ifdef _EVENT_VERSION
	le_base *base = event_base_get(L, 1);
	if(strcmp(_EVENT_VERSION, "1.3")<0)
		lua_pushstring(L, event_base_get_method(base->base));
	else
	#endif
		lua_pushstring(L, event_get_method());
	return 1;
}

static luaL_Reg base_funcs[] = {
	{ "addevent", luaevent_addevent },
	{ "loop", luaevent_loop },
	{ "loopexit", luaevent_loopexit },
	{ "method", luaevent_method },
	{ NULL, NULL }
};

static luaL_Reg funcs[] = {
	{ "new", luaevent_newbase },
	{ "libevent_version", luaevent_libevent_version },
	{ NULL, NULL }
};

typedef struct {
	const char* name;
	int value;
} namedInteger;

static namedInteger consts[] = {
	{"LEAVE", -1},
	{"EV_READ", EV_READ},
	{"EV_WRITE", EV_WRITE},
	{"EV_TIMEOUT", EV_TIMEOUT},
	{"EV_SIGNAL", EV_SIGNAL},
	{"EV_PERSIST", EV_PERSIST},
	/* bufferevent */
	{"EVBUFFER_READ", EVBUFFER_READ},
	{"EVBUFFER_WRITE", EVBUFFER_WRITE},
	{"EVBUFFER_EOF", EVBUFFER_EOF},
	{"EVBUFFER_ERROR", EVBUFFER_ERROR},
	{"EVBUFFER_TIMEOUT", EVBUFFER_TIMEOUT},
	{NULL, 0}
};

void setNamedIntegers(lua_State* L, namedInteger* p) {
	while(p->name) {
		lua_pushinteger(L, p->value);
		lua_setfield(L, -2, p->name);
		p++;
	}
}

/* Verified ok */
int luaopen_luaevent_core(lua_State* L) {
#ifdef _WIN32
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	WSAStartup(wVersionRequested, &wsaData);
#endif
	event_init( );
	/* Setup metatable */
	luaL_newmetatable(L, EVENT_BASE_MT);
	lua_newtable(L);
	luaL_register(L, NULL, base_funcs);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, luaevent_base_gc);
	lua_setfield(L, -2, "__gc");
	lua_pop(L, 1);

	luaL_register(L, "luaevent.core", funcs);
	setNamedIntegers(L, consts);

	/* Register external items */
	event_callback_register(L);
	event_buffer_register(L, lua_gettop(L));
	buffer_event_register(L, lua_gettop(L));

	return 1;
}

