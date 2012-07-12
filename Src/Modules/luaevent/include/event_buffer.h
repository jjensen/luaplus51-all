/* LuaEvent - Copyright (C) 2007,2012 Thomas Harning <harningt@gmail.com>
 * Licensed as LGPL - See doc/COPYING for details */
#ifndef EVENT_BUFFER_H
#define EVENT_BUFFER_H

#include "luaevent.h"

typedef struct {
	struct evbuffer* buffer;
} le_buffer;

void event_buffer_register(lua_State* L, int coreIndex);
int is_event_buffer(lua_State* L, int idx);
le_buffer* event_buffer_check(lua_State* L, int idx);
int event_buffer_push(lua_State* L, struct evbuffer* buffer);

#endif
