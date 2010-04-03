///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2005 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://wwhiz.com/LuaPlus/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef BUILDING_LUAPLUS
#define BUILDING_LUAPLUS
#endif
#include "LuaLink.h"
LUA_EXTERN_C_BEGIN
#include "src/lobject.h"
#include "src/lgc.h"
LUA_EXTERN_C_END
#include "LuaPlus.h"
#include "LuaCall.h"
#include <string.h>
#ifdef WIN32
#if defined(WIN32) && !defined(_XBOX) && !defined(_XBOX_VER) && !defined(_WIN32_WCE)
#include <windows.h>
#elif defined(_XBOX) || defined(_XBOX_VER)
#include <xtl.h>
#endif // WIN32
#undef GetObject
#endif // WIN32
#if defined(__GNUC__)
	#include <new>
#else
	#include <new.h>
#endif

#include <stdlib.h>
#include <wchar.h>
#include <assert.h>

#ifdef _MSC_VER
#pragma warning(disable: 4100)
#endif // _MSC_VER

//-----------------------------------------------------------------------------
LUA_EXTERN_C_BEGIN
#include "src/lualib.h"
#include "src/lstate.h"
LUA_EXTERN_C_END

namespace LuaPlus {

/*static*/ LuaState* LuaState::Create(bool initStandardLibrary)
{
	LuaState* state = LuaState::Create();
	if (initStandardLibrary)
		state->OpenLibs();
	return state;
}

LuaObject LuaState::CreateThread(LuaState* parentState)
{
    lua_State* L1 = lua_newthread(LuaState_to_lua_State(parentState));
    lua_TValue tobject;
#if LUA_REFCOUNT
    setnilvalue2n(L1, &tobject);
#else
    setnilvalue(&tobject);
#endif /* LUA_REFCOUNT */
    setthvalue(parentState->GetCState(), &tobject, L1);

//	lua_State* L = L1;
	LuaObject retObj = LuaObject(lua_State_To_LuaState(L1), &tobject);
    setnilvalue(&tobject);
    lua_pop(LuaState_to_lua_State(parentState), 1);
    return retObj;
}


static int LS_LOG( lua_State* L )
{
	int n = lua_gettop(L);  /* number of arguments */
	int i;
	lua_getglobal(L, "towstring");
	lua_getglobal(L, "tostring");
	for (i=1; i<=n; i++) {
		const char *s = NULL;
#if LUA_WIDESTRING
		const lua_WChar *ws = NULL;
#endif /* LUA_WIDESTRING */
		if (lua_type(L, i) == LUA_TSTRING)
		{
			s = lua_tostring(L, -1);
		}
#if LUA_WIDESTRING
		else if (lua_type(L, i) != LUA_TWSTRING)
		{
			lua_pushvalue(L, -1);  /* function to be called */
			lua_pushvalue(L, i);   /* value to print */
			lua_call(L, 1, 1);
			s = lua_tostring(L, -1);  /* get result */
			if (s == NULL)
				return luaL_error(L, "`tostring' must return a string to `print'");
		}
#endif /* LUA_WIDESTRING */
		else
		{
			lua_pushvalue(L, -2);  /* function to be called */
			lua_pushvalue(L, i);   /* value to print */
			lua_call(L, 1, 1);
#if LUA_WIDESTRING
			ws = lua_towstring(L, -1);  /* get result */
			if (ws == NULL)
				return luaL_error(L, "`tostring' must return a string to `print'");
#endif /* LUA_WIDESTRING */
		}
		if (i>1)
		{
#ifdef WIN32
			OutputDebugStringA("\t");
#else
			fputs("\t", stdout);
#endif
		}
		if (s)
		{
#ifdef WIN32
			OutputDebugStringA(s);
#else
			fputs(s, stdout);
#endif
		}
#if LUA_WIDESTRING
		else if (ws)
		{
            wchar_t out[512];
            wchar_t* outEnd = out + sizeof(out) - 2;
            while (*ws) {
                wchar_t* outPos = out;
                while (*ws && outPos != outEnd) {
                *outPos++ = *ws++;
                }
                *outPos++ = 0;
#ifdef WIN32
			    OutputDebugStringW(out);
#else
    			fputws(out, stdout);
#endif
            }
		}
#endif /* LUA_WIDESTRING */
		lua_pop(L, 1);  /* pop result */
	}

#ifdef WIN32
	OutputDebugStringA("\n");
#else
	fputs("\n", stdout);
#endif

	return 0;
}


static int LS_ALERT( lua_State* L )
{
	const char* err = lua_tostring(L, 1);
#ifdef WIN32
	OutputDebugString(err);
    OutputDebugString("\n");
#else // !WIN32
	puts(err);
#endif // WIN32

	return 0;
}


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


extern "C" void luaplus_dumptable(lua_State* L, int index)
{
	LuaState* state = lua_State_To_LuaState(L);
	LuaObject valueObj(state, index);
	LuaStateOutString stringFile;
	state->DumpObject(stringFile, NULL, valueObj, LuaState::DUMP_ALPHABETICAL | LuaState::DUMP_WRITEALL, 0, -1);
	state->PushString(stringFile.GetBuffer());
}


// LuaDumpObject(file, key, value, alphabetical, indentLevel, maxIndentLevel, writeAll)
int LS_LuaDumpObject( LuaState* state )
{
	LuaStateOutFile file;

	LuaStack args(state);
	LuaStackObject fileObj = args[1];
	if (fileObj.IsTable()  &&  state->GetTop() == 1)
	{
		LuaObject valueObj(fileObj);
		LuaObject nameObj;
		LuaStateOutString stringFile;
		state->DumpObject(stringFile, NULL, valueObj, LuaState::DUMP_ALPHABETICAL, 0, -1);
		state->PushString(stringFile.GetBuffer());
		return 1;
	}

	const char* fileName = NULL;
	if ( fileObj.IsUserData() )
	{	
		FILE* stdioFile = (FILE *)fileObj.GetUserData();
		file.Assign( stdioFile );
	}
	else if ( fileObj.IsString() )
	{
		fileName = fileObj.GetString();
	}

	LuaObject nameObj = args[2];
	LuaObject valueObj = args[3];
	LuaStackObject alphabeticalObj = args[4];
	LuaStackObject indentLevelObj = args[5];
	LuaStackObject maxIndentLevelObj = args[6];
	LuaStackObject writeAllObj = args[7];
	bool writeAll = writeAllObj.IsBoolean() ? writeAllObj.GetBoolean() : false;
	bool alphabetical = alphabeticalObj.IsBoolean() ? alphabeticalObj.GetBoolean() : true;
	unsigned int maxIndentLevel = maxIndentLevelObj.IsInteger() ? (unsigned int)maxIndentLevelObj.GetInteger() : 0xFFFFFFFF;

	unsigned int flags = (alphabetical ? LuaState::DUMP_ALPHABETICAL : 0) | (writeAll ? LuaState::DUMP_WRITEALL : 0);

	if (fileName)
	{
		if (strcmp(fileName, ":string") == 0)
		{
			LuaStateOutString stringFile;
			state->DumpObject(stringFile, nameObj, valueObj, flags, indentLevelObj.GetInteger(), maxIndentLevel);
			state->PushString(stringFile.GetBuffer());
			return 1;
		}
		else
		{
			state->DumpObject(fileName, nameObj, valueObj, flags, (unsigned int)indentLevelObj.GetInteger(), maxIndentLevel);
		}
	}
	else
	{
		state->DumpObject(file, nameObj, valueObj, flags, (unsigned int)indentLevelObj.GetInteger(), maxIndentLevel);
	}

	return 0;
}


// LuaDumpFile(file, key, value, alphabetical, indentLevel, maxIndentLevel, writeAll)
int LS_LuaDumpFile( LuaState* state )
{
	return LS_LuaDumpObject(state);
}


// LuaDumpGlobals(file, alphabetical, maxIndentLevel, writeAll)
int LS_LuaDumpGlobals(LuaState* state)
{
	LuaStateOutFile file;

	LuaStack args(state);
	LuaStackObject fileObj = args[1];
	const char* fileName = NULL;
	if ( fileObj.IsUserData() )
	{
		FILE* stdioFile = (FILE *)fileObj.GetUserData();
		file.Assign( stdioFile );
	}
	else if ( fileObj.IsString() )
	{
		fileName = fileObj.GetString();
	}

	LuaStackObject alphabeticalObj = args[2];
	LuaStackObject maxIndentLevelObj = args[3];
	LuaStackObject writeAllObj = args[4];
	bool alphabetical = alphabeticalObj.IsBoolean() ? alphabeticalObj.GetBoolean() : true;
	unsigned int maxIndentLevel = maxIndentLevelObj.IsInteger() ? (unsigned int)maxIndentLevelObj.GetInteger() : 0xFFFFFFFF;
	bool writeAll = writeAllObj.IsBoolean() ? writeAllObj.GetBoolean() : false;

	unsigned int flags = (alphabetical ? LuaState::DUMP_ALPHABETICAL : 0) | (writeAll ? LuaState::DUMP_WRITEALL : 0);

	if (fileName)
	{
		state->DumpGlobals(fileName, flags, maxIndentLevel);
	}
	else
	{
		state->DumpGlobals(file, flags, maxIndentLevel);
	}

	return 0;
}


static int pmain (lua_State *L)
{
	luaL_openlibs(L);
	return 0;
}


/**
**/
void LuaState::OpenLibs()
{
#if LUAPLUS_INCLUDE_STANDARD_LIBRARY
	LuaAutoBlock autoBlock(this);
	lua_cpcall(LuaState_to_lua_State(this), &pmain, NULL);
#endif // LUAPLUS_INCLUDE_STANDARD_LIBRARY
}

} // namespace LuaPlus
