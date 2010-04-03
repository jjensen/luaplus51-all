/**
	\file FileGlobBase.cpp

	This file contains many of the definitions for parts of the FileGlobBase
	class.
**/
#include "FileGlobBase.h"
#if defined(WIN32)
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#define MAX_PATH 256
#endif
#include <assert.h>

// Forward declares.
bool WildMatch( const char* pattern, const char *string, bool caseSensitive );


/**
	Constructor.
**/
FileGlobBase::FileGlobBase()
{
	AddIgnorePattern( "./" );
	AddIgnorePattern( "../" );
}							


/**
	Finds all files matching [inPattern].  The wildcard expansion understands
	the following pattern constructs:

	- ?
		- Any single character.
	- *
		- Any character of which there are zero or more of them.
	- **
		- All subdirectories recursively.

	Additionally, if the pattern closes in a single slash, only directories
	are processed.  Forward slashes and backslashes may be used
	interchangeably.

	- *\
		- List all the directories in the current directory.  Can also be
		  *(forwardslash), but it turns off the C++ comment in this message.
	- **\
		- List all directories recursively.

	Wildcards may appear anywhere in the pattern, including directories.

	- *\*\*\*.c

	Note that *.* only matches files that have an extension.  This is different
	than standard DOS behavior.  Use * all by itself to match files, extension
	or not.

	Recursive wildcards can be used anywhere:

	c:/Dir1\**\A*\**\FileDirs*\**.mp3

	This matches all directories under c:\Dir1\ that start with A.  Under all
	of the directories that start with A, directories starting with FileDirs
	are matched recursively.  Finally, all files ending with an mp3 extension
	are matched.

	Any place that has more than two .. for going up a subdirectory is expanded
	a la 4DOS.

	...../StartSearchHere\**

	Expands to:

	../../../../StartSearchHere\**
		
	\param inPattern The pattern to use for matching.
**/
void FileGlobBase::MatchPattern( const char* inPattern )
{
	char pattern[MAX_PATH];

	// Give ourselves a local copy of the inPattern with all \ characters
	// changed to / characters and more than two periods expanded.
	const char* srcPtr = inPattern;

	// Is it a Windows network path?  If so, don't convert the opening \\.
	if ( srcPtr[ 0 ] == '\\'  &&  srcPtr[ 1 ] == '\\' )
		srcPtr += 2;

	const char* lastSlashPtr = srcPtr - 1;
	char* destPtr = pattern;
	int numPeriods = 0;
	while ( *srcPtr != '\0' )
	{
		char ch = *srcPtr;
		
		///////////////////////////////////////////////////////////////////////
		// Check for slashes or backslashes.
		if ( ch == '\\'  ||  ch == '/' )
		{
			*destPtr++ = '/';

			lastSlashPtr = srcPtr;
			numPeriods = 0;
		}

		///////////////////////////////////////////////////////////////////////
		// Check for .
		else if ( ch == '.' )
		{
			if ( srcPtr - numPeriods - 1 == lastSlashPtr )
			{
				numPeriods++;
				if ( numPeriods > 2 )
				{
					*destPtr++ = '/';
					*destPtr++ = '.';
					*destPtr++ = '.';
				}
				else
				{
					*destPtr++ = '.';
				}
			}
			else
			{
				*destPtr++ = '.';
			}
		}

		///////////////////////////////////////////////////////////////////////
		// Check for **
		else if ( ch == '*'  &&  srcPtr[ 1 ] == '*' )
		{
			if ( srcPtr - 1 != lastSlashPtr )
			{
				// Something like this:
				//
				// /Dir**/
				//
				// needs to be translated to:
				//
				// /Dir*/**/
				*destPtr++ = '*';
				*destPtr++ = '/';
			}

			srcPtr += 2;

			*destPtr++ = '*';
			*destPtr++ = '*';

			// Did we get a double star this round?
			if ( srcPtr[ 0 ] != '/'  &&  srcPtr[ 0 ] != '\\' )
			{
				// Handle the case that looks like this:
				//
				// /**Text
				//
				// Translate to:
				//
				// /**/*Text
				*destPtr++ = '/';
				*destPtr++ = '*';
			}
			else if ( srcPtr[ 1 ] == '\0'  ||  srcPtr[ 1 ] == '@' )
			{
				srcPtr++;

				*destPtr++ = '/';
				*destPtr++ = '*';
				*destPtr++ = '/';
			}

			// We added one too many in here... the compiler will optimize.
			srcPtr--;
		}

		///////////////////////////////////////////////////////////////////////
		// Check for @
		else if ( ch == '@' )
		{
			// Gonna finish this processing in another loop.
			break;
		}
			
		///////////////////////////////////////////////////////////////////////
		// Everything else.
		else
		{
			*destPtr++ = *srcPtr;
		}

		srcPtr++;
	}

	*destPtr = 0;

	// Check for the @.
	if ( *srcPtr == '@' )
	{
		// Clear out function registered entries.
		m_ignorePatterns.clear();
		AddIgnorePattern( "./" );
		AddIgnorePattern( "../" );

		m_exclusiveFilePatterns.clear();
	}

	while ( *srcPtr == '@' )
	{
		srcPtr++;

		char ch = *srcPtr++;
		if ( ch == '-'  ||  ch == '=' )
		{
			char buffer[1000];
			char* ptr = buffer;
			while ( *srcPtr != '@'  &&  *srcPtr != '\0' )
			{
				*ptr++ = *srcPtr++;
			}

			*ptr = 0;

			if ( ch == '-' )
				AddIgnorePattern( buffer );
			else if ( ch == '=' )
				AddExclusivePattern( buffer );
		}
		else
			break;		// Don't know what it is.
	}

	// Start globbing!
	GlobHelper( pattern );
}


/**
	Adds a pattern to the file glob database of exclusive patterns.  If any
	exclusive patterns are registered, the ignore database is completely
	ignored.  Only patterns matching the exclusive patterns will be
	candidates for matching.

	\param name The exclusive pattern.
**/
void FileGlobBase::AddExclusivePattern( const char* pattern )
{
	for ( StringList::iterator it = m_exclusiveFilePatterns.begin();
			it != m_exclusiveFilePatterns.end(); ++it )
	{
#if defined(WIN32)
		if ( stricmp( (*it).c_str(), pattern ) == 0 )
#else
		if ( strcasecmp( (*it).c_str(), pattern ) == 0 )
#endif
			return;
	}

	m_exclusiveFilePatterns.push_back( pattern );
}


/**
	Adds a pattern to ignore to the file glob database.  If a pattern of
	the given name is found, its contents will not be considered for further
	matching.  The result is as if the pattern did not exist for the search
	in the first place.

	\param name The pattern to ignore.
**/
void FileGlobBase::AddIgnorePattern( const char* pattern )
{
	for ( StringList::iterator it = m_ignorePatterns.begin();
			it != m_ignorePatterns.end(); ++it )
	{
#if defined(WIN32)
		if ( stricmp( (*it).c_str(), pattern ) == 0 )
#else
		if ( strcasecmp( (*it).c_str(), pattern ) == 0 )
#endif
			return;
	}

	m_ignorePatterns.push_back( pattern );
}


/**
	Match an exclusive pattern.

	\param text The text to match an ignore pattern against.
	\return Returns true if the directory should be ignored, false otherwise.
**/
bool FileGlobBase::MatchExclusivePattern( const char* text )
{
	for ( StringList::iterator it = m_exclusiveFilePatterns.begin();
			it != m_exclusiveFilePatterns.end(); ++it )
	{
		if ( WildMatch( (*it).c_str(), text, false  ) )
			return true;
	}

	return false;
}


/**
	Do a case insensitive find for the pattern.

	\param text The text to match an ignore pattern against.
	\return Returns true if the directory should be ignored, false otherwise.
**/
bool FileGlobBase::MatchIgnorePattern( const char* text )
{
	for ( StringList::iterator it = m_ignorePatterns.begin();
			it != m_ignorePatterns.end(); ++it )
	{
		if ( WildMatch( (*it).c_str(), text, false  ) )
			return true;
	}

	return false;
}


/**
	\internal
	\author Jack Handy

	Borrowed from http://www.codeproject.com/string/wildcmp.asp.
	Modified by Joshua Jensen.
**/
bool WildMatch( const char* pattern, const char *string, bool caseSensitive )
{
	// Handle all the letters of the pattern and the string.
	while ( *string != 0  &&  *pattern != '*' )
	{
		if ( *pattern != '?' )
		{
			if ( caseSensitive )
			{
				if ( *pattern != *string )
					return false;
			}
			else
			{
				if ( toupper( *pattern ) != toupper( *string ) )
					return false;
			}
		}

		pattern++;
		string++;
	}

	const char* mp = NULL;
	const char* cp = NULL;
	while ( *string != 0 )
	{
		if (*pattern == '*')
		{
			// It's a match if the wildcard is at the end.
			if ( *++pattern == 0 )
			{
				return true;
			}

			mp = pattern;
			cp = string + 1;
		}
		else
		{
			if ( caseSensitive )
			{
				if ( *pattern == *string  ||  *pattern == '?' )
				{
					pattern++;
					string++;
				}
				else 
				{
					pattern = mp;
					string = cp++;
				}
			}
			else
			{
				if ( toupper( *pattern ) == toupper( *string )  ||  *pattern == '?' )
				{
					pattern++;
					string++;
				}
				else
				{
					pattern = mp;
					string = cp++;
				}
			}
		}
	}

	// Collapse remaining wildcards.
	while ( *pattern == '*' )
		pattern++;

	return !*pattern;
}


/**
	\internal Simple path splicing (assumes no '/' in either part)
	\author Matthias Wandel (MWandel@rim.net) http://http://www.sentex.net/~mwandel/
**/
static void CatPath(char * dest, const char * p1, const char * p2)
{
	size_t len = strlen( p1 );
	if ( len == 0 )
	{
		strcpy( dest, p2 );
	}
	else
	{
		if ( len + strlen( p2 ) > 200 )
		{
			// Path too long.
			assert( 0 );
		}
		memcpy( dest, p1, len + 1 );
		if ( dest[ len - 1 ] != '/'  &&  dest[ len - 1 ] != ':' )
		{
			dest[ len++ ] = '/';
		}
		strcpy( dest + len, p2 );
	}
}


/**
	\internal Does all the actual globbing.
	\author Matthias Wandel (MWandel@rim.net) http://http://www.sentex.net/~mwandel/
	\author Joshua Jensen (jjensen@workspacewhiz.com)

	Matthias Wandel wrote the original C algorithm, which is contained in
	his Exif Jpeg header parser at http://www.sentex.net/~mwandel/jhead/ under
	the filename MyGlob.c.  It should be noted that the MAJORITY of this
	function is his, albeit rebranded style-wise.

	I have made the following extensions:

	-	Support for ignoring directories.
	-	Perforce-style (and DJGPP-style) ... for recursion, instead of **.
	-	Automatic conversion from ...Stuff to ...\*Stuff.  Allows lookup of
		files by extension, too: '....h' translates to '...\*.h'.
	-	Ability to handle forward slashes and backslashes.
	-	A minimal C++ class design.
	-	Wildcard matching not based on FindFirstFile().  Should allow greater
		control in the future and patching in of the POSIX fnmatch() function
		on systems that support it.
**/
void FileGlobBase::GlobHelper( const char* inPattern )
{
	char patternBuf[ MAX_PATH * 2 ];
	strcpy( patternBuf, inPattern );

DoRecursion:
	char basePath[ MAX_PATH ];
	char* basePathEndPtr = basePath;
	char* recurseAtPtr = NULL;

	// Split the path into base path and pattern to match against.
	bool hasWildcard = false;

	char* pattern;
	for ( pattern = patternBuf; *pattern != '\0'; ++pattern )
	{
		char ch = *pattern;

		// Is it a '?' ?
		if ( ch == '?' )
			hasWildcard = true;

		// Is it a '*' ?
		else if ( ch == '*' )
		{
			hasWildcard = true;

			// Is there a '**'?
			if ( pattern[ 1 ] == '*' )
			{
				// If we're just starting the pattern or the characters immediately
				// preceding the pattern are a drive letter ':' or a directory path
				// '/', then set up the internals for later recursion.
				if ( pattern == patternBuf  ||  pattern[ -1 ] == '/'  ||
					pattern[ -1 ] == ':')
				{
					char ch2 = pattern[ 2 ];
					if ( ch2 == '/' )
					{
						recurseAtPtr = pattern;
						memcpy(pattern, pattern + 3, strlen( pattern ) - 2 );
					}
					else if ( ch2 == '\0' )
					{
						recurseAtPtr = pattern;
						*pattern = '\0';
					}
				}
			}
		}

		// Is there a '/' or ':' in the pattern at this location?
		if ( ch == '/'  ||  ch == ':' )
		{
			if ( hasWildcard )
				break;
			basePathEndPtr = &basePath[ pattern - patternBuf + 1 ];
		}
	}

	// If there is no wildcard this time, then just add the current file and
	// get out of here.
	if ( !hasWildcard )
	{
		// This should refer to a file.
		FoundMatch( patternBuf );
		return;
	}

	// Did we make it to the end of the pattern?  If so, we should match files,
	// since there were no slashes encountered.
	bool matchFiles = *pattern == '\0';

	// Copy the directory down.
	size_t basePathLen = basePathEndPtr - basePath;
	strncpy( basePath, patternBuf, basePathLen );
	basePath[basePathLen] = 0;

	// Copy the wildcard matching string.
	char matchPattern[ MAX_PATH ];
	size_t matchLen = ( pattern - patternBuf ) - basePathLen;
	strncpy( matchPattern, patternBuf + basePathLen, matchLen + 1 );
	if ( matchPattern[ matchLen ] == '/' )
		matchPattern[ matchLen ] = 0;

	StringList fileList;

#if defined(WIN32)
	// Do the file search with *.* in the directory specified in basePattern.
	strcpy( basePathEndPtr, "*.*" );
	
	// Start the find.
	WIN32_FIND_DATA fd;
	HANDLE handle = FindFirstFile( basePath, &fd );

	// Clear out the *.* so we can use the original basePattern string.
	*basePathEndPtr = 0;

	// Any files found?
	if ( handle != INVALID_HANDLE_VALUE )
	{
		for ( ;; )
		{
			// Is the file a directory?
			if ( ( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )  &&  !matchFiles )
			{
				// Do a wildcard match.
				if ( WildMatch( matchPattern, fd.cFileName, false ) )
				{
					// It matched.  Let's see if the file should be ignored.
					bool ignore = false;
					
					// Knock out "." or ".." if they haven't already been.
					size_t len = strlen( fd.cFileName );
					fd.cFileName[ len ] = '/';
					fd.cFileName[ len + 1 ] = '\0';
					
					// See if this is a directory to ignore.
					ignore = MatchIgnorePattern( fd.cFileName );
					
					fd.cFileName[ len ] = 0;
					
					// Should this file be ignored?
					if ( !ignore )
					{
						// Nope.  Add it to the linked list.
						fileList.push_back( fd.cFileName );
					}
				}
			}
			else if ( !( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )  &&  matchFiles )
			{
				// Do a wildcard match.
				if ( WildMatch( matchPattern, fd.cFileName, false ) )
				{
					// It matched.  Let's see if the file should be ignored.
					bool ignore = MatchIgnorePattern( fd.cFileName );
					
					// Is this pattern exclusive?
					if ( !ignore  &&  m_exclusiveFilePatterns.begin() != m_exclusiveFilePatterns.end() )
					{
						ignore = !MatchExclusivePattern( fd.cFileName );
					}
					
					// Should this file be ignored?
					if ( !ignore )
					{
						// Nope.  Add it to the linked list.
						fileList.push_back( fd.cFileName );
					}
				}
			}
			
			// Look up the next file.
			if ( !FindNextFile( handle, &fd ) )
				break;
		}
		
		// Close down the file find handle.
		FindClose( handle );
	}
	
#else
	// Start the find.
	DIR* dirp = opendir(basePath[0] ? basePath : ".");
	if (!dirp)
		return;

	// Any files found?
	struct dirent* dp;
	while ((dp = readdir(dirp)) != NULL)
	{
		for ( ;; )
		{
			struct stat attr;
			strcpy(basePathEndPtr, dp->d_name);
			stat(basePath, &attr);
			*basePathEndPtr = 0;
			
			// Is the file a directory?
			if ( (attr.st_mode & S_IFDIR) != 0  &&  !matchFiles )
			{
				// Do a wildcard match.
				if ( WildMatch( matchPattern, dp->d_name, false ) )
				{
					// It matched.  Let's see if the file should be ignored.
					bool ignore = false;
					
					// Knock out "." or ".." if they haven't already been.
					size_t len = strlen( dp->d_name );
					dp->d_name[ len ] = '/';
					dp->d_name[ len + 1 ] = '\0';
					
					// See if this is a directory to ignore.
					ignore = MatchIgnorePattern( dp->d_name );
					
					dp->d_name[ len ] = 0;
					
					// Should this file be ignored?
					if ( !ignore )
					{
						// Nope.  Add it to the linked list.
						fileList.push_back( dp->d_name );
					}
				}
			}
			else if ( (attr.st_mode & S_IFDIR) == 0  &&  matchFiles )
			{
				// Do a wildcard match.
				if ( WildMatch( matchPattern, dp->d_name, false ) )
				{
					// It matched.  Let's see if the file should be ignored.
					bool ignore = MatchIgnorePattern( dp->d_name );
					
					// Is this pattern exclusive?
					if ( !ignore  &&  m_exclusiveFilePatterns.begin() != m_exclusiveFilePatterns.end() )
					{
						ignore = !MatchExclusivePattern( dp->d_name );
					}
					
					// Should this file be ignored?
					if ( !ignore )
					{
						// Nope.  Add it to the linked list.
						fileList.push_back( dp->d_name );
					}
				}
			}
			
			// Look up the next file.
			if ((dp = readdir(dirp)) == NULL)
				break;
		}
		
		// Close down the file find handle.
		closedir( dirp );
	}
	
#endif

	// Sort the list.
	fileList.sort();

	// Iterate the file list and either recurse or add the file as a found
	// file.
	if ( !matchFiles )
	{
		for ( StringList::iterator it = fileList.begin(); it != fileList.end(); ++it )
		{
			char combinedName[ MAX_PATH * 2 ];

			// Need more directories.
			CatPath( combinedName, basePath, (*it).c_str() );
			strcat( combinedName, pattern );
			GlobHelper( combinedName );
		}
	}
	else // if ( !matchFiles )
	{
		for ( StringList::iterator it = fileList.begin(); it != fileList.end(); ++it )
		{
			char combinedName[ MAX_PATH * 2 ];
			CatPath( combinedName, basePath, (*it).c_str());
			FoundMatch( combinedName );
		}
	}

	// Clear out the file list, so the goto statement below can recurse
	// internally.
	fileList.clear();

	// Do we need to recurse?
	if ( !recurseAtPtr )
		return;

	// Copy in the new recursive pattern to match.
	strcpy( matchPattern, recurseAtPtr );
	strcpy( recurseAtPtr, "*/**/" );
	strcat( patternBuf, matchPattern );

	// As this function context is no longer needed, we can just go back
	// to the top of it to avoid adding another context on the stack.
	goto DoRecursion;
}


