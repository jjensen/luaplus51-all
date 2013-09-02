///////////////////////////////////////////////////////////////////////////////
// File:        setup0.h
// Purpose:     wxSTEditor setup definitions (rename to setup.h)
// Maintainer:
// Created:     2003-04-04
// RCS-ID:
// Copyright:   (c) John Labenski
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

/// @file setup0.h
/// @brief This file is distributed as wx/stedit/setup0.h and should be copied to wx/stedit/setup.h.
/// You may modify the copied setup.h file to control the behavior of the
/// wxSTEditor or override the default values of the #defines, where allowed,
/// using the compiler flag "-D STE_USE_HTML_PRINT=1" (for example).

#ifndef _STESETUP_H_
#define _STESETUP_H_

// --------------------------------------------------------------------------
// Track the version of this file to alert people when changes have been made
// so they can update their copy to wx/stedit/setup0.h.
// The STE_SETUP_VERSION is simply an integer that is incremented whenever
// wx/stedit/setup0.h is modified in a way that breaks compatibility.
// --------------------------------------------------------------------------

#include "wx/stedit/stedefs.h" // Get the current version this should be updated to.

#if STE_SETUP_VERSION != 2
#   error "Your wx/stedit/setup.h file is out of date, please update to wx/stedit/setup0.h."
#endif

// --------------------------------------------------------------------------
/// Set the maximum file size that the editor can try to load in bytes.
// --------------------------------------------------------------------------

#ifndef STE_MaxFileSize
    #define STE_MaxFileSize 100000000
#endif

//-----------------------------------------------------------------------------
/// Starting wxWindowID for the wxSTEdit menu items
//-----------------------------------------------------------------------------

#ifndef ID_STE__FIRST
    #define ID_STE__FIRST 100 // first menu/window ID value
#endif

//-----------------------------------------------------------------------------
/// Number of notebook pages allowed before showing dialog saying max # pages reached.
/// Use wxSTEditorNotebook::SetMaxPageCount() to change it dynamically, but
/// there can never be more pages than STN_NOTEBOOK_PAGES_MAX.
//-----------------------------------------------------------------------------

#ifndef STN_NOTEBOOK_PAGES_ALLOWED
    #define STN_NOTEBOOK_PAGES_ALLOWED 200  // default max number of pages
#endif

//-----------------------------------------------------------------------------
/// Use wxHtmlEasyPrinting instead of normal printing.
//-----------------------------------------------------------------------------

#ifndef STE_USE_HTML_PRINT
    #define STE_USE_HTML_PRINT 0
#endif

//-----------------------------------------------------------------------------
/// Set default initial values for the wxSTEditor font.
//-----------------------------------------------------------------------------

// A smallish font size that is nicely readable (your mileage may vary)
#ifndef STE_DEFAULT_FONT_SIZE
    #ifdef __WXGTK__
        #define STE_DEFAULT_FONT_SIZE 12
    #else
        #define STE_DEFAULT_FONT_SIZE 10
    #endif
#endif // #ifndef STE_DEFAULT_FONT_SIZE

// A fixed width font - courier is not great, but a reasonable start
#ifndef STE_DEFAULT_FONT_FACENAME
    #ifdef __WXMSW__
        // Use a TrueType/ClearType font on Windows
        #define STE_DEFAULT_FONT_FACENAME wxT("Courier New")
    #else
        #define STE_DEFAULT_FONT_FACENAME wxT("Courier")
    #endif
#endif // #ifndef STE_DEFAULT_FONT_FACENAME

// --------------------------------------------------------------------------
/// @name STE_USE_LANG_XXX determines whether or not the language information will be compiled in or not.
///
/// Note that the wxSTEditorLangs always has the
/// languages in the order given by enum STE_LangTypes STE_LANG_XXX
/// (wxSTC_LEX_XXX), however when the STE_USE_LANG_XXX is 0 the language struct
/// is NULL and the language will not be shown in the preference dialog.
/// Turning off unused languages serves two purposes, making a smaller binary
/// and a simplier interface by stripping out esoteric languages.
/// In order to make it easy to exclude languages be sure to use
/// wxSTEditorLangs::HasLanguage(lang_n) before accessing any values.
// --------------------------------------------------------------------------
/// @{

#define STE_USE_LANG_CONTAINER   0 // 0  probably never want this shown
#define STE_USE_LANG_NULL        1 // 1
#define STE_USE_LANG_PYTHON      1 // 2
#define STE_USE_LANG_CPP         1 // 3
#define STE_USE_LANG_HTML        1 // 4
#define STE_USE_LANG_XML         1 // 5
#define STE_USE_LANG_PERL        1 // 6
#define STE_USE_LANG_SQL         1 // 7
#define STE_USE_LANG_VB          1 // 8
#define STE_USE_LANG_PROPERTIES  1 // 9
#define STE_USE_LANG_ERRORLIST   1 // 10
#define STE_USE_LANG_MAKEFILE    1 // 11
#define STE_USE_LANG_BATCH       1 // 12
#define STE_USE_LANG_XCODE       1 // 13
#define STE_USE_LANG_LATEX       1 // 14
#define STE_USE_LANG_LUA         1 // 15
#define STE_USE_LANG_DIFF        1 // 16
#define STE_USE_LANG_CONF        1 // 17
#define STE_USE_LANG_PASCAL      1 // 18
#define STE_USE_LANG_AVE         1 // 19
#define STE_USE_LANG_ADA         1 // 20
#define STE_USE_LANG_LISP        1 // 21
#define STE_USE_LANG_RUBY        1 // 22
#define STE_USE_LANG_EIFFEL      1 // 23
#define STE_USE_LANG_EIFFELKW    1 // 24
#define STE_USE_LANG_TCL         1 // 25
#define STE_USE_LANG_NNCRONTAB   1 // 26
#define STE_USE_LANG_BULLANT     1 // 27
#define STE_USE_LANG_VBSCRIPT    1 // 28
#define STE_USE_LANG_ASP         1 // 29
#define STE_USE_LANG_PHP         1 // 30
#define STE_USE_LANG_BAAN        1 // 31
#define STE_USE_LANG_MATLAB      1 // 32
#define STE_USE_LANG_SCRIPTOL    1 // 33
#define STE_USE_LANG_ASM         1 // 34
#define STE_USE_LANG_CPPNOCASE   1 // 35
#define STE_USE_LANG_FORTRAN     1 // 36
#define STE_USE_LANG_F77         1 // 37
#define STE_USE_LANG_CSS         1 // 38
#define STE_USE_LANG_POV         1 // 39
#define STE_USE_LANG_LOUT        1 // 40
#define STE_USE_LANG_ESCRIPT     1 // 41
#define STE_USE_LANG_PS          1 // 42
#define STE_USE_LANG_NSIS        1 // 43
#define STE_USE_LANG_MMIXAL      1 // 44
#define STE_USE_LANG_CLW         1 // 45
#define STE_USE_LANG_CLWNOCASE   1 // 46
#define STE_USE_LANG_LOT         1 // 47
#define STE_USE_LANG_YAML        1 // 48
#define STE_USE_LANG_TEX         1 // 49
#define STE_USE_LANG_METAPOST    1 // 50
#define STE_USE_LANG_POWERBASIC  1 // 51
#define STE_USE_LANG_FORTH       1 // 52
#define STE_USE_LANG_ERLANG      1 // 53
#define STE_USE_LANG_OCTAVE      1 // 54
#define STE_USE_LANG_MSSQL       1 // 55
#define STE_USE_LANG_VERILOG     1 // 56
#define STE_USE_LANG_KIX         1 // 57
#define STE_USE_LANG_GUI4CLI     1 // 58
#define STE_USE_LANG_SPECMAN     1 // 59
#define STE_USE_LANG_AU3         1 // 60
#define STE_USE_LANG_APDL        1 // 61
#define STE_USE_LANG_BASH        1 // 62
#define STE_USE_LANG_ASN1        1 // 63
#define STE_USE_LANG_VHDL        1 // 64

// Derived languages
#define STE_USE_LANG_JAVA        1 // 65
#define STE_USE_LANG_JAVASCRIPT  1 // 66
#define STE_USE_LANG_RC          1 // 67
#define STE_USE_LANG_CS          1 // 68
#define STE_USE_LANG_D           1 // 69
#define STE_USE_LANG_IDL         1 // 70
#define STE_USE_LANG_PLSQL       1 // 71
#define STE_USE_LANG_SQUIRREL    1 // 72

/// @}

#endif // _STESETUP_H_
