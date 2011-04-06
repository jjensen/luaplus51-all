
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

#include "LuaHostWindows.h"

#include <stdio.h>
#include <tchar.h>

#include "LuaHostWindows.h"

#include "tilde/LuaDebugger.h"
#include "tilde/LuaDebuggerComms.h"
#include "tilde/ReceiveMessageBuffer.h"

#include "lua.h"
#include "lobject.h"

#pragma comment(lib, "wsock32.lib")

namespace tilde {

	void OsSleep(unsigned int millisecs)
	{
		Sleep(millisecs);
	}

} // namespace tilde

const char * FormatAddress(sockaddr_in & address)
{
	static char buf[256];
	sprintf(buf, "%d.%d.%d.%d:%d", address.sin_addr.S_un.S_un_b.s_b1, address.sin_addr.S_un.S_un_b.s_b2, address.sin_addr.S_un.S_un_b.s_b3, address.sin_addr.S_un.S_un_b.s_b4, address.sin_port);
	return buf;
}

char s_printBuffer[1024];

void print(const char * format, ...)
{
	if (1) return;

	va_list	ap;

	va_start(ap,format);
	vsnprintf(s_printBuffer, 1024, format, ap);
	va_end(ap);

	OutputDebugString(s_printBuffer);
	printf(s_printBuffer);
}

void warn(const char * format, ...)
{
	if (1) return;

	va_list	ap;

	print("WARNING: ");

	va_start(ap,format);
	vsnprintf(s_printBuffer, 1024, format, ap);
	va_end(ap);

	OutputDebugString(s_printBuffer);
	printf(s_printBuffer);

	print("\n");
}

void error(const char * format, ...)
{
	if (1) return;

	va_list	ap;

	print("FATAL ERROR: ");

	va_start(ap,format);
	vsnprintf(s_printBuffer, 1024, format, ap);
	va_end(ap);

	OutputDebugString(s_printBuffer);
	printf(s_printBuffer);

	print("\n");

	DebugBreak();
	exit(1);
}


LuaHostWindows::LuaHostWindows(int port)
	:
		m_serverPort(port),
		m_serverSocket(SOCKET_ERROR),
		m_debuggerSocket(SOCKET_ERROR)
{
	m_debuggerComms = new tilde::LuaDebuggerComms(this);

	m_netBufferSize = 4096;
	m_netBuffer = new tilde::u8[m_netBufferSize];

	InitialiseServerSocket();
}

LuaHostWindows::~LuaHostWindows()
{

}

void LuaHostWindows::RegisterState(const char* name, lua_State* lvm)
{
	m_debuggerComms->RegisterState(name, lvm);
}

void LuaHostWindows::InitialiseServerSocket()
{
	// Initialize Winsock.
	WSADATA wsaData;
	int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
	if(iResult != NO_ERROR)
		error("WSAStartup() failed (error %d)", WSAGetLastError());

	// Create a socket.
	m_serverSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if(m_serverSocket == INVALID_SOCKET) 
	{
		error("socket() failed (error %d)", WSAGetLastError());
		WSACleanup();
		return;
	}

	// Bind the socket.
	sockaddr_in service;

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr( "127.0.0.1" );
	service.sin_port = htons( m_serverPort );

	if (bind(m_serverSocket, (SOCKADDR*) &service, sizeof(service) ) == SOCKET_ERROR ) 
	{
		error("bind() failed (error %d)", WSAGetLastError());
		closesocket(m_serverSocket);
		return;
	}

	// Listen on the socket.
	if (listen( m_serverSocket, 1 ) == SOCKET_ERROR)
		error("listen() failed (error %d)", WSAGetLastError());
	else
		fprintf(stderr, "Listening for debugger connection on port %d...\n", m_serverPort);
}

void LuaHostWindows::DestroyServerSocket()
{
	if(m_serverSocket != SOCKET_ERROR)
	{
		closesocket(m_serverSocket);
		m_serverSocket = SOCKET_ERROR;
	}
}

bool LuaHostWindows::IsConnected() const
{
	return m_debuggerComms->GetDebugger()->IsConnected();
}

void LuaHostWindows::WaitForDebuggerConnection()
{
	while (!IsConnected())
	{
		Poll();
		Sleep(50);
	}
}

void LuaHostWindows::Poll()
{
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(m_serverSocket, &readfds);

	if(m_debuggerSocket != SOCKET_ERROR)
		FD_SET(m_debuggerSocket, &readfds);

	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	int count = select(0, &readfds, NULL, NULL, &timeout);

	if(count == SOCKET_ERROR)
		error("select() failed (error %d)", WSAGetLastError());

	else if(count >= 1)
	{
		if(FD_ISSET(m_debuggerSocket, &readfds) && m_debuggerSocket != SOCKET_ERROR)
		{
			int bytes = recv(m_debuggerSocket, (char *) m_netBuffer, m_netBufferSize, 0);
			if(bytes == 0)
				Close();
			else if(bytes == SOCKET_ERROR)
			{
				warn("recv() failed (error %d)", WSAGetLastError());
				Close();
			}
			else
			{
				m_debuggerComms->Receive(m_netBuffer, bytes);
			}
		}

		if(FD_ISSET(m_serverSocket, &readfds) && m_debuggerSocket == SOCKET_ERROR && !m_debuggerComms->GetDebugger()->IsConnected())
		{
			sockaddr_in address;
			int addressLen = sizeof(address);
			m_debuggerSocket = accept(m_serverSocket, (sockaddr *) &address, &addressLen);

			if(m_debuggerSocket == SOCKET_ERROR)
				error ("accept() failed (error %d)", WSAGetLastError());

			print("Connection accepted from %s!\n", FormatAddress(address));

			m_debuggerComms->Open();
		}
	}
}

void LuaHostWindows::Close()
{
	m_debuggerComms->Close();
	m_debuggerSocket = SOCKET_ERROR;
}

void LuaHostWindows::SendDebuggerData( const void * data, int size )
{
	if(m_debuggerSocket != SOCKET_ERROR)
	{
		send(m_debuggerSocket, (char *) data, (int) size, 0);
	}
}

void LuaHostWindows::CloseDebuggerConnection()
{
	Close();
}

bool LuaHostWindows::AttachLuaHook( lua_State* lvm, lua_Hook hook, int mask, int count )
{
	lua_sethook(lvm, hook, mask, count);
	return true;
}

void LuaHostWindows::DetachLuaHook( lua_State* lvm, lua_Hook hook )
{
	lua_sethook(lvm, NULL, 0, 0);
}

void LuaHostWindows::AttachPrintfHook( void (* hook)(const char *) )
{

}

void LuaHostWindows::DetachPrintfHook( void (* hook)(const char *) )
{

}

void LuaHostWindows::ReceiveExCommand(const char * command, int datalen, tilde::ReceiveMessageBuffer * data)
{
	data->Skip(datalen);
}

void LuaHostWindows::OnIdle()
{
	Poll();
}

void LuaHostWindows::OnRun()
{
	Poll();
}

const char * LuaHostWindows::GetFunctionName( lua_State * lvm, int funcIndex, lua_Debug * ar )
{
	m_functionName.clear();

	// Start with the name the function was called by
	if (ar->name)
		m_functionName.append(ar->name);

	// If it's a C function, then add its details
	if(lua_iscfunction(lvm, funcIndex))
	{
		if(!m_functionName.empty())
			m_functionName.append(" ");

		CClosure * closure = (CClosure *) lua_topointer(lvm, funcIndex);
		char buf[16];
		sprintf(buf, "%p", closure->f);
		m_functionName.append("(C function ");
		m_functionName.append(buf);
		m_functionName.append(")");
	}

	// If it's a lua function then add the file/line
	if (ar->source && ar->linedefined >= 0)
	{
		char buf[16];

		if(ar->currentline >= 0)
			sprintf(buf, "%d", ar->currentline);
		else
			sprintf(buf, "%d", ar->linedefined);

		if(!m_functionName.empty())
			m_functionName.append(" ");

		m_functionName.append("(");
		m_functionName.append(ar->source);
		m_functionName.append(":");
		m_functionName.append(buf);
		m_functionName.append(")");
	}

	if (ar->what)
	{
		if(!m_functionName.empty())
			m_functionName.append(" ");

		m_functionName.append("[");
		m_functionName.append(ar->what);
		m_functionName.append("]");
	}

	return m_functionName.c_str();
}

const char * LuaHostWindows::LookupSourceFile(const char *target)
{
	if(target[0] == '@')
		return target + 1;
	else
		return target;
}

const char * LuaHostWindows::LookupTargetFile(const char *source)
{
	m_targetFileName.clear();
	m_targetFileName.append("@");
	m_targetFileName.append(source);
	return m_targetFileName.c_str();
}
