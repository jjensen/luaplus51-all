/*
 * PythonMessage. Wrapper around errors and warnings.
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
 * $Id: //depot/r11.1/p4-python/PythonMessage.h#1 $
 *
 */

/*******************************************************************************
 * Name		: PythonMessage.h
 *
 * Author	: Sven Erik Knop <sknop@perforce.com>
 *
 * Description	: Class for bridging Perforce's Error class to Python
 *
 ******************************************************************************/

#ifndef LUAMESSAGE_H_
#define LUAMESSAGE_H_

class LuaMessage
{
private:
	lua_State* L;
    Error err;

public:
    LuaMessage(lua_State* _L, const Error * other) {
		L = _L;
		err = *other;
    }

public:
    void getText();
    void getRepr();
    void getSeverity();
    void getGeneric();
    void getMsgid();

};

int p4_message_new(lua_State* L, const Error * message);

#endif /* PYTHONMESSAGE_H_ */
