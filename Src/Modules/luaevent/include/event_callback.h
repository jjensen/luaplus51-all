/* LuaEvent - Copyright (C) 2007,2012 Thomas Harning <harningt@gmail.com>
 * Licensed as LGPL - See doc/COPYING for details */
#ifndef EVENT_CALLBACK
#define EVENT_CALLBACK

#include "luaevent.h"

typedef struct {
	struct event ev;
	le_base* base;
	int callbackRef;
	struct timeval timeout;
} le_callback;

void event_callback_register(lua_State* L);

le_callback* event_callback_push(lua_State* L, int baseIdx, int callbackIdx);

void luaevent_callback(int fd, short event, void* p);

#endif
