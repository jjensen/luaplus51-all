///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2011 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#define LUA_CORE
#include "LuaPlus.h"
#include <string.h>

namespace LuaPlus
{

#if LUAPLUS_EXCEPTIONS

LuaException::LuaException(const char* message)
	: m_message(NULL)
{
	if (message)
	{
		m_message = new char[strlen(message) + 1];
		strcpy(m_message, message);
	}
}


LuaException::LuaException(const LuaException& src)
{
    m_message = new char[strlen(src.m_message) + 1];
    strcpy(m_message, src.m_message);
}


LuaException& LuaException::operator=(const LuaException& src)
{
    delete[] m_message;
    m_message = new char[strlen(src.m_message) + 1];
    strcpy(m_message, src.m_message);
    return *this;
}


LuaException::~LuaException()
{
    delete[] m_message;
}

#endif // LUAPLUS_EXCEPTIONS


} // namespace LuaPlus

