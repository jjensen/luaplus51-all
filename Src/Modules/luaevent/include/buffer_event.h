/* LuaEvent - Copyright (C) 2007,2012 Thomas Harning <harningt@gmail.com>
 * Licensed as LGPL - See doc/COPYING for details */
#ifndef BUFFER_EVENT_H
#define BUFFER_EVENT_H
 
#include "luaevent.h"

typedef struct {
	struct bufferevent* ev;
	le_base* base;
} le_bufferevent;

void buffer_event_register(lua_State* L, int coreIndex);
int is_buffer_event(lua_State* L, int idx);
le_bufferevent* buffer_event_check(lua_State* L, int idx);

#endif
