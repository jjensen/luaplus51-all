///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2011 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUAPLUS__LUASTATEOUTFILE_H
#define LUAPLUS__LUASTATEOUTFILE_H

#include "LuaPlusInternal.h"

#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus
{

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Output file helper class.
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
	The DumpObject() facility uses a LuaStateOutFile derived class to
	output data to.  The LuaStateOutFile class may be derived from to enable
	an application specific method of output.
**/
class LuaStateOutFile
{
public:
	LuaStateOutFile();
	LuaStateOutFile(const char* fileName);
	virtual ~LuaStateOutFile();
	virtual bool Open( const char* fileName );
	virtual void Close();
	virtual void Print( const char* str, ... );
	bool Assign( FILE* file );
	void Indent( unsigned int indentLevel );

protected:
	FILE* m_file;
	bool m_fileOwner;
};

} // namespace LuaPlus

namespace LuaPlus {

inline LuaStateOutFile::LuaStateOutFile() :
    m_file( NULL ),
    m_fileOwner( false )
{
}


inline LuaStateOutFile::LuaStateOutFile(const char* fileName) :
    m_file( NULL ),
    m_fileOwner( false )
{
	Open(fileName);
}


inline LuaStateOutFile::~LuaStateOutFile()
{
	if ( m_file  &&  m_fileOwner )
		fclose( m_file );
}


inline bool LuaStateOutFile::Open( const char* fileName )
{
	Close();

    if (fileName[0] == '+')
	    m_file = fopen( fileName + 1, "a+b" );
    else
	    m_file = fopen( fileName, "wb" );
	m_fileOwner = true;

	return m_file != NULL;
}


inline void LuaStateOutFile::Close()
{
	if ( m_file  &&  m_fileOwner )
		fclose( m_file );
}


inline void LuaStateOutFile::Print( const char* str, ... )
{
	char message[ 800 ];
	va_list arglist;

	va_start( arglist, str );
	vsprintf( message, str, arglist );
	va_end( arglist );

	fputs( message, m_file );
}


inline bool LuaStateOutFile::Assign( FILE* file )
{
	m_file = file;
	m_fileOwner = false;

	return true;
}

} // namespace LuaPlus

#endif // LUAPLUS__LUASTATEOUTFILE_H
