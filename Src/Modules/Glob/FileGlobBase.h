/**
	\file FileGlobBase.cpp

	This file contains the class definition of the FileGlobBase class.
**/
#pragma once

#pragma warning( disable: 4702 )

#include <list>
#include <string>

/**
	The base class of all file glob matching classes.  Derived classes should
	provide an implementation for FoundMatch().

	\sa MatchPattern
**/
class FileGlobBase
{
public:
	FileGlobBase();
	virtual ~FileGlobBase()	{}

	void MatchPattern( const char* inPattern );
	void AddExclusivePattern( const char* name );
	void AddIgnorePattern( const char* name );

	virtual void FoundMatch( const char* name ) = 0;

protected:
	bool MatchExclusivePattern( const char* name );
	bool MatchIgnorePattern( const char* name );
	void GlobHelper( const char* inPattern );

private:
	typedef std::list< std::string > StringList;

	StringList m_exclusiveFilePatterns;
	StringList m_ignorePatterns;
};

