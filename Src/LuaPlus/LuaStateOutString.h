///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2011 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUAPLUS__LUASTATEOUTSTRING_H
#define LUAPLUS__LUASTATEOUTSTRING_H

#include "LuaPlus.h"
#include <string.h>
#if defined(__GNUC__)
	#include <new>
#else
	#include <new.h>
#endif

#include <stdlib.h>
#include <assert.h>

namespace LuaPlus {

class LuaStateOutString : public LuaStateOutFile
{
public:
	LuaStateOutString(size_t growBy = 10000) :
		m_buffer(NULL),
		m_growBy(growBy),
		m_curPos(0),
		m_size(0)
	{
	}

	virtual ~LuaStateOutString()
	{
		free(m_buffer);
	}

	virtual void Print(const char* str, ...)
	{
		char message[ 800 ];
		va_list arglist;

		va_start( arglist, str );
		vsprintf( message, str, arglist );
		va_end( arglist );

		size_t len = strlen(message);
		if (len != 0)
		{
			if (m_curPos + len + 1 > m_size)
			{
				size_t newSize = m_size;
				while (newSize < m_curPos + len + 1)
					newSize += m_growBy;
				m_buffer = (char*)realloc(m_buffer, newSize);
				m_size = newSize;
			}

			strncpy(m_buffer + m_curPos, message, len);
			m_curPos += len;
			m_buffer[m_curPos] = 0;
		}
	}

	const char* GetBuffer() const
	{
		return m_buffer;
	}

protected:
	char* m_buffer;
	size_t m_growBy;
	size_t m_curPos;
	size_t m_size;
};

#if 0

NOT AS FAST, BUT BETTER MEMORY MANAGEMENT.

class LuaStateOutString : public LuaStateOutFile
{
public:
	LuaStateOutString(size_t growBy = 10000)
		: m_tail(&m_head)
		, m_bufferDirty(true)
		, m_buffer(NULL)
		, m_bufferSize(0)
	{
		m_head.next = NULL;
	}

	virtual ~LuaStateOutString()
	{
		delete[] m_buffer;

		LineNode* node = m_head.next;
		while (node)
		{
			LineNode* oldNode = node;
			node = node->next;
			delete[] (unsigned char*)oldNode;
		}
	}

	virtual void Print(const char* str, ...)
	{
		char message[ 800 ];
		va_list arglist;

		va_start( arglist, str );
		vsprintf( message, str, arglist );
		va_end( arglist );

		size_t len = strlen(message);
		if (len != 0)
		{
			LineNode* newNode = (LineNode*)new unsigned char[sizeof(LineNode) + len];	// Already includes the +1 for \0
			newNode->next = NULL;
			m_tail->next = newNode;
			m_tail = newNode;

			newNode->len = len;
			strncpy(newNode->line, message, len);
			newNode->line[len] = 0;

			m_bufferSize += len;
			m_bufferDirty = true;
		}
	}

	const char* GetBuffer()
	{
		if (m_bufferDirty)
		{
			delete[] m_buffer;
			m_buffer = new char[m_bufferSize + 1];

			char* bufferPos = m_buffer;
			for (LineNode* node = m_head.next; node; node = node->next)
			{
				memcpy(bufferPos, node->line, node->len);
				bufferPos += node->len;
			}
			*bufferPos = 0;
		}

		return m_buffer;
	}

protected:
	struct LineNode
	{
		LineNode* next;
		size_t len;
		char line[1];
	};

	LineNode m_head;
	LineNode* m_tail;

	bool m_bufferDirty;
	char* m_buffer;
	size_t m_bufferSize;
};

#endif

} // namespace LuaPlus

#endif // LUAPLUS__LUASTATEOUTSTRING_H
