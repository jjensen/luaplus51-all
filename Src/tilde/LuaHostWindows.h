
/****************************************************************************

Tilde

Copyright (c) 2008 Tantalus Media Pty

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

****************************************************************************/

#pragma once

#include <stdio.h>
#include <winsock2.h>

#include "tilde/LuaDebugger.h"

struct sockaddr_in;
class LuaMachine;

namespace tilde
{
	class LuaDebuggerComms;
}

extern const char * FormatAddress(sockaddr_in & address);
extern void print(const char * format, ...);
extern void warn(const char * format, ...);
extern void error(const char * format, ...);

#define strcmp _stricmp

class LuaHostWindows : public tilde::LuaDebuggerHost
{
public:
	LuaHostWindows(int port = 10000);
	virtual ~LuaHostWindows();

	bool	IsConnected() const;
	void	WaitForDebuggerConnection();
	void	Poll();

	virtual void RegisterState(const char* stateName, lua_State* lvm);
	virtual void SendDebuggerData(const void * data, int size);
	virtual void CloseDebuggerConnection();
	virtual bool AttachLuaHook(lua_State* lvm, lua_Hook hook, int mask, int count);
	virtual void DetachLuaHook(lua_State* lvm, lua_Hook hook);
	virtual void AttachPrintfHook( void (* hook)(const char *) );
	virtual void DetachPrintfHook( void (* hook)(const char *) );
	virtual void ReceiveExCommand(const char * command, int datalen, tilde::ReceiveMessageBuffer * data);
	virtual void OnIdle();
	virtual void OnRun();
	virtual const char * GetFunctionName(lua_State * lvm, int funcIndex, lua_Debug * ar);
	virtual const char *LookupSourceFile( const char *target );
	virtual const char *LookupTargetFile( const char *source);




private:
	void	InitialiseServerSocket();
	void	DestroyServerSocket();
	void	Close();

private:

	tilde::LuaDebuggerComms * m_debuggerComms;

	int		m_serverPort;

	SOCKET	m_serverSocket;
	SOCKET	m_debuggerSocket;

	tilde::u8		* m_netBuffer;
	int				m_netBufferSize;

	tilde::String	m_functionName;
	tilde::String	m_targetFileName;
};
