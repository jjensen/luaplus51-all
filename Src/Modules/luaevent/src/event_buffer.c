/* LuaEvent - Copyright (C) 2007,2012 Thomas Harning <harningt@gmail.com>
 * Licensed as LGPL - See doc/COPYING for details */
#include <stdlib.h>
#include "event_buffer.h"
#include <lauxlib.h>

#define EVENT_BUFFER_MT "EVENT_BUFFER_MT"

#define BUFFER_ADD_CHECK_INPUT_FIRST 1

/* Obtains an le_buffer structure from a given index */
static le_buffer* event_buffer_get(lua_State* L, int idx) {
	return (le_buffer*)luaL_checkudata(L, idx, EVENT_BUFFER_MT);
}

/* Obtains an le_buffer structure from a given index
	AND checks that it hadn't been prematurely freed
*/
le_buffer* event_buffer_check(lua_State* L, int idx) {
	le_buffer* buf = (le_buffer*)luaL_checkudata(L, idx, EVENT_BUFFER_MT);
	if(!buf->buffer)
		luaL_argerror(L, idx, "Attempt to use closed event_buffer object");
	return buf;
}

/* Checks if the given index contains an le_buffer object */
int is_event_buffer(lua_State* L, int idx) {
	int ret;
	lua_getmetatable(L, idx);
	luaL_getmetatable(L, EVENT_BUFFER_MT);
	ret = lua_rawequal(L, -2, -1);
	lua_pop(L, 2);
	return ret;
}

/* TODO: Use lightuserdata mapping to locate hanging object instances */
/* Pushes the specified evbuffer object onto the stack, attaching a metatable to it */
int event_buffer_push(lua_State* L, struct evbuffer* buffer) {
	le_buffer *buf = (le_buffer*)lua_newuserdata(L, sizeof(le_buffer));
	buf->buffer = buffer;
	luaL_getmetatable(L, EVENT_BUFFER_MT);
	lua_setmetatable(L, -2);
	return 1;
}

/* LUA: new()
	Pushes a new evbuffer instance on the stack
*/
static int event_buffer_push_new(lua_State* L) {
	return event_buffer_push(L, evbuffer_new());
}

/* LUA: __gc and buffer:close()
	Releases the buffer resources
*/
static int event_buffer_gc(lua_State* L) {
	le_buffer* buf = event_buffer_get(L, 1);
	if(buf->buffer) {
		evbuffer_free(buf->buffer);
		buf->buffer = NULL;
	}
	return 0;
}

/* LUA: buffer:add(...)
	progressively adds items to the buffer
		if arg[*] is string, treat as a string:format call
		if arg[*] is a buffer, perform event_add_buffer
	expects at least 1 other argument
	returns number of bytes added
*/
static int event_buffer_add(lua_State* L) {
	le_buffer* buf = event_buffer_check(L, 1);
	struct evbuffer* buffer = buf->buffer;
	int oldLength = EVBUFFER_LENGTH(buffer);
	int last = lua_gettop(L);
	int i;
	if(last == 1) luaL_error(L, "Not enough arguments to add: expects at least 1 additional operand");
	for(i = 2; i <= last; i++) {
		if(!lua_isstring(L, i) && !is_event_buffer(L, i))
			luaL_argerror(L, i, "Argument is not a string or buffer object");
		if(lua_equal(L, 1, i))
			luaL_argerror(L, i, "Cannot add buffer to itself");
/* Optionally perform checks and data loading separately to avoid overfilling the buffer */
#if BUFFER_ADD_CHECK_INPUT_FIRST
	}
	for(i = 2; i <= last; i++) {
#endif
		if(lua_isstring(L, i)) {
			size_t len;
			const char* data = lua_tolstring(L, i, &len);
			if(0 != evbuffer_add(buffer, data, len))
				luaL_error(L, "Failed to add data to the buffer");
		} else {
			le_buffer* buf2 = event_buffer_check(L, i);
			if(0 != evbuffer_add_buffer(buffer, buf2->buffer))
				luaL_error(L, "Failed to move buffer-data to the buffer");
		}
	}
	lua_pushinteger(L, EVBUFFER_LENGTH(buffer) - oldLength);
	return 1;
}

/* LUA: buffer:length()
	Returns the length of the buffer contents
*/
static int event_buffer_get_length(lua_State* L) {
	le_buffer* buf = event_buffer_check(L, 1);
	lua_pushinteger(L, EVBUFFER_LENGTH(buf->buffer));
	return 1;
}

/* MAYBE: Could add caching */
/* LUA: buffer:get_data
	() - Returns all data in buffer
	(len) - Returns data up to 'len' bytes long
	(begin,len) - Returns data beginning at 'begin' up to 'len' bytes long
	If begin < 0, wraps at data length
		ex: (-1, 1) returns last character
		(-2,2) returns last 2 chars [length meaning does not get inverted]
*/
static int event_buffer_get_data(lua_State* L) {
	le_buffer* buf = event_buffer_check(L, 1);
	int begin, len;
	switch(lua_gettop(L)) {
	case 1:
		/* Obtain full data */
		begin = 0;
		len = EVBUFFER_LENGTH(buf->buffer);
		break;
	case 2:
		begin = 0;
		len = luaL_checkinteger(L, 2);
		if(len > EVBUFFER_LENGTH(buf->buffer))
			len = EVBUFFER_LENGTH(buf->buffer);
		break;
	case 3:
	default:
		/* - 1 to map it to Lua's 1-based indexing
		 * If begin < 0 add length to cause position wrapping
		 */
		begin = luaL_checkinteger(L, 2);
		if(begin < 0)
			begin += EVBUFFER_LENGTH(buf->buffer);
		else
			begin--;
		len = luaL_checkinteger(L, 3);
		/* If length is less than zero, capture entire remaining string */

		if(len < 0) len = EVBUFFER_LENGTH(buf->buffer);
		if(begin > EVBUFFER_LENGTH(buf->buffer))
			begin = EVBUFFER_LENGTH(buf->buffer);
		if(begin + len > EVBUFFER_LENGTH(buf->buffer))
			len = EVBUFFER_LENGTH(buf->buffer) - begin;
		break;
	}
	lua_pushlstring(L, (const char*)EVBUFFER_DATA(buf->buffer) + begin, len);
	return 1;
}

/* LUA: buffer:readline()
	Returns a line terminated by either '\r\n','\n\r' or '\r' or '\n'
	Returns nil and leaves data alone if no terminator is found
	Newline is not present in the captured string.
*/
static int event_buffer_readline(lua_State* L) {
	le_buffer* buf = event_buffer_check(L, 1);
	char* line = evbuffer_readline(buf->buffer);
	if(!line)
		return 0;
	lua_pushstring(L, line);
	free(line);
	return 1;
}

/* LUA: buffer:drain(amt)
	Drains 'amt' bytes from the buffer
	If amt < 0, drains all data
		(Due to auto-casting to unsigned int and automatic capping)
*/
static int event_buffer_drain(lua_State* L) {
	le_buffer* buf = event_buffer_check(L, 1);
	size_t len = luaL_checkinteger(L, 2);
	evbuffer_drain(buf->buffer, len);
	return 0;
}

/* LUA: buffer:write
	(integer/lightuserdata fd) - Attempts to write all the data out to the FD
	(socket) - Attempts to write all the data out to the socket object
*/
static int event_buffer_write(lua_State* L) {
	le_buffer* buf = event_buffer_check(L, 1);
	int ret;
	if(lua_isnumber(L, 2)) {
		ret = evbuffer_write(buf->buffer, lua_tointeger(L, 2));
	} else if(lua_islightuserdata(L, 2)) {
		ret = evbuffer_write(buf->buffer, (int)(long)lua_touserdata(L, 2));
	} else if(lua_isuserdata(L, 2)) {
		ret = evbuffer_write(buf->buffer, getSocketFd(L, 2));
	} else {
		ret = 0; /* Shush uninitialized warning */
		luaL_argerror(L, 2, "Unexpected data type.  Expects: integer/lightuserdata/socket");
	}
	lua_pushinteger(L, ret);
	return 1;
}

/* LUA: buffer:read
	(integer/lightuserdata fd, len) - Attempts to read up to 'len' out of the FD
	(socket, len) - Attempts to read up to 'len' out of the socket object
*/
static int event_buffer_read(lua_State* L) {
	le_buffer* buf = event_buffer_check(L, 1);
	int len = luaL_checkinteger(L, 3);
	int ret;
	if(lua_isnumber(L, 2)) {
		ret = evbuffer_read(buf->buffer, lua_tointeger(L, 2), len);
	} else if(lua_islightuserdata(L, 2)) {
		ret = evbuffer_read(buf->buffer, (int)(long)lua_touserdata(L, 2), len);
	} else if(lua_isuserdata(L, 2)) {
		ret = evbuffer_read(buf->buffer, getSocketFd(L, 2), len);
	} else {
		ret = 0; /* Shush uninitialized warning */
		luaL_argerror(L, 2, "Unexpected data type.  Expects: integer/lightuserdata/socket");
	}
	lua_pushinteger(L, ret);
	return 1;
}
static luaL_Reg buffer_funcs[] = {
	{"add", event_buffer_add},
	{"length", event_buffer_get_length},
	{"get_data", event_buffer_get_data},
	{"readline", event_buffer_readline},
	{"drain", event_buffer_drain},
	{"close", event_buffer_gc},
	{"read", event_buffer_read},
	{"write", event_buffer_write},
	{NULL, NULL}
};
static luaL_Reg funcs[] = {
	{"new", event_buffer_push_new},
	{NULL, NULL}
};
 
void event_buffer_register(lua_State* L, int coreIndex) {
	luaL_newmetatable(L, EVENT_BUFFER_MT);
	lua_pushcfunction(L, event_buffer_gc);
	lua_setfield(L, -2, "__gc");
	lua_pushcfunction(L, event_buffer_get_length);
	lua_setfield(L, -2, "__len");
	lua_pushcfunction(L, event_buffer_get_data);
	lua_setfield(L, -2, "__tostring");
	lua_newtable(L);
	luaL_register(L, NULL, buffer_funcs);
	lua_setfield(L, -2, "__index");
	lua_pop(L, 1);

	lua_newtable(L);
	luaL_register(L, NULL, funcs);
	lua_setfield(L, coreIndex, "buffer");
}
