///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://wwhiz.com/LuaPlus/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUAPLUS__LUAPLUS_CONFIG_H
#define LUAPLUS__LUAPLUS_CONFIG_H

#ifndef LUA_TILDE_DEBUGGER
#if defined(_WIN32)
#define LUA_TILDE_DEBUGGER 1
#endif
#endif /* LUA_TILDE_DEBUGGER */

#ifndef LUA_FASTREF_SUPPORT
#if LUA_VERSION_NUM <= 501
#define LUA_FASTREF_SUPPORT 1
#else
#define LUA_FASTREF_SUPPORT 0
#endif
#endif /* LUA_FASTREF_SUPPORT */

#ifndef LUA_EXT_HEXADECIMAL
#define LUA_EXT_HEXADECIMAL 1
#endif /* LUA_EXT_HEXADECIMAL */

#ifndef LUA_ENDIAN_SUPPORT
#define LUA_ENDIAN_SUPPORT 1
#endif /* LUA_ENDIAN_SUPPORT */

#ifndef LUA_MEMORY_STATS
#define LUA_MEMORY_STATS 1
#endif /* LUA_MEMORY_STATS */

#ifndef LUA_STRING_FORMAT_EXTENSIONS
#define	LUA_STRING_FORMAT_EXTENSIONS 1
#endif /* LUA_STRING_FORMAT_EXTENSIONS */


#ifndef LUAPLUS_EXTENSIONS
#define LUAPLUS_EXTENSIONS 1
#endif /* LUAPLUS_EXTENSIONS */

#ifndef LUAPLUS_DUMPOBJECT
#define LUAPLUS_DUMPOBJECT 1
#endif /* LUAPLUS_DUMPOBJECT */

#ifndef LUAPLUS_EXCEPTIONS
#define LUAPLUS_EXCEPTIONS 0
#endif // LUAPLUS_EXCEPTIONS

#ifndef LUA_PACK_VALUE
#define LUA_PACK_VALUE 0
#endif /* LUA_PACK_VALUE */

#if !defined(LUA_PACK_VALUE)
/* on platform with lua number double we could use nan packing for value */
#if (defined(__i386) || defined (_M_IX86) || defined(__i386__)) && defined(LUA_NUMBER_DOUBLE)
/* currently try it on known little endian platform :) */
#define LUA_PACK_VALUE 1
#else
#define LUA_PACK_VALUE 0
#endif
#endif

#ifndef LUAPLUS_INCLUDE_STANDARD_LIBRARY
#define LUAPLUS_INCLUDE_STANDARD_LIBRARY 1
#endif // LUAPLUS_INCLUDE_STANDARD_LIBRARY

#if defined(PLATFORM_IOS)
#elif defined(__APPLE__)  ||  defined(macintosh)
#define LUA_USE_MACOSX
#endif

#endif /* LUAPLUS__LUAPLUS_CONFIG_H */

