///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://wwhiz.com/LuaPlus/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUALINK_H
#define LUALINK_H

#ifndef LUA_TILDE_DEBUGGER
#if defined(_MSC_VER)
#define LUA_TILDE_DEBUGGER 1
#endif
#endif /* LUA_TILDE_DEBUGGER */

#ifndef LUA_FASTREF_SUPPORT
#define LUA_FASTREF_SUPPORT 1
#endif /* LUA_FASTREF_SUPPORT */

#ifndef LUA_WIDESTRING
#define LUA_WIDESTRING 0
#endif /* LUA_WIDESTRING */

#ifndef LUA_WIDESTRING_FILE
#define LUA_WIDESTRING_FILE 0
#endif /* LUA_WIDESTRING_FILE */

#ifndef LUA_BITFIELD_OPS
#define LUA_BITFIELD_OPS 1
#endif /* LUA_BITFIELD_OPS */

#ifndef LUA_MUTATION_OPERATORS
#define LUA_MUTATION_OPERATORS 0
#endif /* LUA_MUTATION_OPERATORS */

#ifndef LUA_EXT_HEXADECIMAL
#define LUA_EXT_HEXADECIMAL 1
#endif /* LUA_EXT_HEXADECIMAL */

#ifndef LUA_ENDIAN_SUPPORT
#define LUA_ENDIAN_SUPPORT 1
#endif /* LUA_ENDIAN_SUPPORT */

#ifndef LUA_MEMORY_STATS
#define LUA_MEMORY_STATS 1
#endif /* LUA_MEMORY_STATS */

#ifndef LUA_AUTO_INCLUDE_STRING_PACK_LIBRARY
#define LUA_AUTO_INCLUDE_STRING_PACK_LIBRARY 1
#endif /* LUA_AUTO_INCLUDE_STRING_PACK_LIBRARY */

#ifndef LUA_STRING_FORMAT_EXTENSIONS
#define	LUA_STRING_FORMAT_EXTENSIONS 1
#endif /* LUA_STRING_FORMAT_EXTENSIONS */


#ifndef LUAPLUS_EXTENSIONS
#define LUAPLUS_EXTENSIONS 0
#endif /* LUAPLUS_EXTENSIONS */

#ifndef LUAPLUS_DUMPOBJECT
#define LUAPLUS_DUMPOBJECT 1
#endif /* LUAPLUS_DUMPOBJECT */

#ifndef LUA_EXT_CONTINUE
#define LUA_EXT_CONTINUE 1
#endif /* LUA_EXT_CONTINUE */

#ifndef LUA_EXT_RESUMABLEVM
#define LUA_EXT_RESUMABLEVM 0
#endif /* LUA_EXT_RESUMABLEVM */

/* Still has bugs... */
#ifndef LUA_REFCOUNT
#define LUA_REFCOUNT 0
#endif /* LUA_REFCOUNT */

#ifndef LUAPLUS_EXCEPTIONS
#define LUAPLUS_EXCEPTIONS 0
#endif // LUAPLUS_EXCEPTIONS

#if LUA_WIDESTRING
#define lua_wstr2number(s,p)    triow_to_double((s), (p))
#endif /* LUA_WIDESTRING */

#ifndef LUA_PACK_VALUE
#define LUA_PACK_VALUE 1
#endif /* LUA_PACK_VALUE */

#ifndef LNUM_PATCH
#define LNUM_PATCH 0
#define LNUM_INT32
#define LNUM_FLOAT
#endif /* LNUM_PATCH */

#if !defined(LUA_PACK_VALUE)
/* on platform with lua number double we could use nan packing for value */
#if (defined(__i386) || defined (_M_IX86) || defined(__i386__)) && defined(LUA_NUMBER_DOUBLE)
/* currently try it on known little endian platform :) */
#define LUA_PACK_VALUE 1
#else
#define LUA_PACK_VALUE 0
#endif
#endif

#ifndef LUAPLUS_DLL
#ifndef LUAPLUS_LIB
#define LUAPLUS_LIB
#endif // LUAPLUS_LIB
#endif // LUAPLUS_DLL

#ifndef LUAPLUS_INCLUDE_STANDARD_LIBRARY
#define LUAPLUS_INCLUDE_STANDARD_LIBRARY 1
#endif // LUAPLUS_INCLUDE_STANDARD_LIBRARY

#undef LUA_API
#undef LUALIB_API

#ifdef _MSC_VER
	#ifndef LUAPLUS_LIB
		#ifdef LUAPLUS_BUILDDLL
			#define LUA_API __declspec(dllexport)
			#define LUAPLUS_CLASS __declspec(dllexport)
			#define LUAPLUS_API	__declspec(dllexport)
			#define LUAPLUS_CLASS_API __declspec(dllexport)
		#else
//			#define LUA_API __declspec(dllimport)
//			#define LUAPLUS_CLASS __declspec(dllimport)
//			#define LUAPLUS_API	__declspec(dllimport)
//			#define LUAPLUS_CLASS_API __declspec(dllimport)
			#define LUA_API
			#define LUAPLUS_CLASS
			#define LUAPLUS_API
			#define LUAPLUS_CLASS_API
		#endif

		#ifdef LUAMODULE_BUILDDLL
			#define LUAMODULE_API __declspec(dllexport)
		#else
			#define LUAMODULE_API __declspec(dllimport)
		#endif
	#else // LUAPLUS_LIB
		#define LUA_API
		#define LUAPLUS_CLASS
		#define LUAPLUS_CLASS_API
		#define LUAPLUS_API extern
		#define LUAMODULE_API
	#endif
#else // !_MSC_VER
	#ifdef __cplusplus
		#define LUA_API extern "C"
	#else
		#define LUA_API extern
	#endif
    #define LUAPLUS_CLASS
    #define LUAPLUS_CLASS_API
    #define LUAPLUS_API extern
	#ifdef LUAMODULE_BUILDDLL
		#define LUAMODULE_API
	#else
		#define LUAMODULE_API
	#endif
#endif // _MSC_VER

#if defined(PLATFORM_IOS)
#elif defined(__APPLE__)  ||  defined(macintosh)
#define LUA_USE_MACOSX
#endif

#if defined(__cplusplus)
#define LUA_EXTERN_C extern "C"
#define LUA_EXTERN_C_BEGIN extern "C" {
#define LUA_EXTERN_C_END }
#else
#define LUA_EXTERN_C
#define LUA_EXTERN_C_BEGIN
#define LUA_EXTERN_C_END
#endif

#endif /* LUALINK_H */

