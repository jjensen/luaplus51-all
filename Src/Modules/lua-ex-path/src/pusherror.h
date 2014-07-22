/*
 * "ex" API implementation
 * http://lua-users.org/wiki/ExtensionProposal
 * Copyright 2007 Mark Edgar < medgar at student gc maricopa edu >
 */
#ifndef pusherror_h
#define pusherror_h

#if defined(_WIN32)
#include <windows.h>
#endif

#include "lua.h"

#if defined(_WIN32)

int windows_pusherror(lua_State *L, DWORD error, int nresults);
#define windows_pushlasterror(L) windows_pusherror(L, GetLastError(), -2)
#define push_error(L) windows_pushlasterror(L)

#else

#include <string.h>
#include <errno.h>

int push_error(lua_State *L);

#endif

#endif/*pusherror_h*/
