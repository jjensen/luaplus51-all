/*
 * LuaMessage. Wrapper around errors and warnings.
 *
 * Copyright (c) 2010, Perforce Software, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTR
 * IBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL PERFORCE SOFTWARE, INC. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: //depot/r11.1/p4-python/LuaMessage.cpp#1 $
 *
 */

/*******************************************************************************
 * Name		: LuaMessage.cpp
 *
 * Author	: Sven Erik Knop <sknop@perforce.com>
 *
 * Description	: Class for bridging Perforce's Error class to Python
 *
 ******************************************************************************/

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include <p4/clientapi.h>
#include "LuaMessage.h"

void LuaMessage::getSeverity()
{
	lua_pushinteger( L, err.GetSeverity() );
}

void LuaMessage::getGeneric()
{
	lua_pushnumber( L, err.GetGeneric() );
}

void LuaMessage::getMsgid()
{
    ErrorId *id = err.GetId( 0 );
    if( !id )
		lua_pushinteger( L, 0 );
	else
		lua_pushnumber( L, id->UniqueCode() );
}

void LuaMessage::getText()
{
    StrBuf buf;
    err.Fmt(buf, EF_PLAIN);
	lua_pushlstring( L, buf.Text(), buf.Length() );
}

void LuaMessage::getRepr()
{
    StrBuf a;
    StrBuf b;

    err.Fmt( a, EF_PLAIN );
    b << "[";
    b << "Gen:" << err.GetGeneric();
    b << "/Sev:" << err.GetSeverity();
    b << "]: ";
    b << a;

	lua_pushlstring( L, b.Text(), b.Length() );
}
