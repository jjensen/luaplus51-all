///////////////////////////////////////////////////////////////////////////////
// Name:        wxldefs.h
// Purpose:     wxLua common defines
// Author:      John Labenski
// Created:     5/28/2005
// Copyright:   (c) John Labenski
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#ifndef __WX_WXLDEFS_H__
#define __WX_WXLDEFS_H__

//-----------------------------------------------------------------------------
// Include the Lua headers
//-----------------------------------------------------------------------------

extern "C"
{
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"

    // To not include "lua.h" use these
    //typedef struct lua_State lua_State;
    //typedef struct lua_Debug lua_Debug;
    //typedef int (*lua_CFunction)(lua_State *);
}

#include "wx/defs.h"

//-----------------------------------------------------------------------------
// The version of wxLua - for convenience we use the current version of
// wxWidgets which wxLua is most compatible with.
//-----------------------------------------------------------------------------

#define wxLUA_MAJOR_VERSION       2
#define wxLUA_MINOR_VERSION       8
#define wxLUA_RELEASE_NUMBER      10
#define wxLUA_SUBRELEASE_NUMBER   0
#define wxLUA_VERSION_STRING      wxT("wxLua 2.8.10.0")

// For non-Unix systems (i.e. when building without a configure script),
// users of this component can use the following macro to check if the
// current version is at least major.minor.release
#define wxLUA_CHECK_VERSION(major,minor,release) \
    (wxLUA_MAJOR_VERSION > (major) || \
    (wxLUA_MAJOR_VERSION == (major) && wxLUA_MINOR_VERSION > (minor)) || \
    (wxLUA_MAJOR_VERSION == (major) && wxLUA_MINOR_VERSION == (minor) && wxLUA_RELEASE_NUMBER >= (release)))

// the same but check the subrelease also
#define wxLUA_CHECK_VERSION_FULL(major,minor,release,subrel) \
    (wxLUA_CHECK_VERSION(major, minor, release) && \
        ((major) != wxLUA_MAJOR_VERSION || \
            (minor) != wxLUA_MINOR_VERSION || \
                (release) != wxLUA_RELEASE_NUMBER || \
                    (subrel) <= wxLUA_SUBRELEASE_NUMBER))

//-----------------------------------------------------------------------------
// This is an internal use binding generator version whos number is
//   incremented every time something changes that requires a regeneration
//   of the bindings. The check is written into the generated bindings to
//   give a compile time error.
// If this number is incremented the variable by the same name must be updated
//   in genwxbind.lua must be updated as well.
//-----------------------------------------------------------------------------

#define WXLUA_BINDING_VERSION 27

// ----------------------------------------------------------------------------
// If you're using stdcall in Lua, then override this with
//   "LUACALL = __stdcall" in your makefile or project.
// ----------------------------------------------------------------------------

#ifndef LUACALL
    #define LUACALL
#endif

// ----------------------------------------------------------------------------
// WXDLLIMPEXP macros
// ----------------------------------------------------------------------------

// These are our DLL macros (see the contrib libs like wxPlot)
#ifdef WXMAKINGDLL_WXLUA
    #define WXDLLIMPEXP_WXLUA WXEXPORT
    #define WXDLLIMPEXP_DATA_WXLUA(type) WXEXPORT type
#elif defined(WXUSINGDLL)
    #define WXDLLIMPEXP_WXLUA WXIMPORT
    #define WXDLLIMPEXP_DATA_WXLUA(type) WXIMPORT type
#else // not making nor using DLL
    #define WXDLLIMPEXP_WXLUA
    #define WXDLLIMPEXP_DATA_WXLUA(type) type
#endif

// ----------------------------------------------------------------------------
// Useful macros to make coding easier
// ----------------------------------------------------------------------------

#if wxUSE_UNICODE
    #define wxLUA_UNICODE_ONLY(x) x
#else /* !Unicode */
    #define wxLUA_UNICODE_ONLY(x)
#endif // wxUSE_UNICODE


#define WXLUA_HASBIT(value, bit)      (((value) & (bit)) != 0)
#define WXLUA_SETBIT(value, bit, set) ((set) ? (value)|(bit) : (value)&(~(bit)))

// ----------------------------------------------------------------------------
// Lua defines for making the code more readable
// ----------------------------------------------------------------------------

// initializes a lua_debug by nulling everything before use since the
//  functions that take it do not initialize it properly
#define INIT_LUA_DEBUG { 0, 0, 0, 0, 0, 0, 0, 0, 0, {0}, 0 }

// Create a wxString from the lua_Debug struct for debugging
#define lua_Debug_to_wxString(ld) \
    wxString::Format(wxT("%p event=%d name='%s' namewhat='%s' what='%s' source='%s' currentline=%d nups=%d linedefined=%d lastlinedefined=%d short_src='%s' i_ci=%d"), \
    &ld, ld.event, lua2wx(ld.name).c_str(), lua2wx(ld.namewhat).c_str(), lua2wx(ld.what).c_str(), lua2wx(ld.source).c_str(), ld.currentline, ld.nups, ld.linedefined, ld.lastlinedefined, lua2wx(ld.short_src).c_str(), ld.i_ci)

// ----------------------------------------------------------------------------
// Convert from wxWidgets wxT('') to wxT(""), a string. Copied from wx/filefn.h

// platform independent versions
#if defined(__UNIX__) && !defined(__OS2__)
  // CYGWIN also uses UNIX settings
  #define wxLua_FILE_SEP_PATH     wxT("/")
#elif defined(__MAC__)
  #define wxLua_FILE_SEP_PATH     wxT(":")
#else   // Windows and OS/2
  #define wxLua_FILE_SEP_PATH     wxT("\\")
#endif  // Unix/Windows

// ----------------------------------------------------------------------------
// wxWidgets compatibility defines
// ----------------------------------------------------------------------------

#ifndef wxUSE_WAVE
    #define wxUSE_WAVE 0
#endif
#ifndef wxUSE_SOUND
    #define wxUSE_SOUND 0
#endif

// ----------------------------------------------------------------------------
// Forward declared classes
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_WXLUA wxLuaState;


#endif  // __WX_WXLDEFS_H__
