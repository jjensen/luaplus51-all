# Copyright (C) 2007-2012 LuaDist.
# Created by Peter Kapec, David Manura, Peter Drahoš
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the COPYRIGHT file distributed with LuaDist.
# Please note that the package source code is licensed under its own license.

project ( luacom C CXX )
cmake_minimum_required ( VERSION 2.8 )
include ( cmake/dist.cmake )
include ( lua )
include ( CheckCSourceCompiles )
include ( CheckSymbolExists )
# note: requires 'C' in PROJECT.

# Build
include_directories ( include src/dll src/library ${CMAKE_CURRENT_BINARY_DIR} )

# Detect htmlhelp (not normally available in Cygwin 1.7, MinGW, and Wine).
# Use CHECK_C_SOURCE_COMPILES because these two aren't reliable:
#   CHECK_INCLUDE_FILES("windows.h;htmlhelp.h" HAVE_HTMLHELP)
#          # On MinGW and Wine, htmlhelp.h may exist but not htmlhelp.lib.
#   CHECK_LIBRARY_EXISTS(htmlhelp HtmlHelpA "" HAVE_HTMLHELP) # detection fails in MSVC
set ( CMAKE_REQUIRED_LIBRARIES htmlhelp )
check_c_source_compiles ( " #include <windows.h> #include <htmlhelp.h> int main() { HtmlHelp(NULL, NULL, HH_HELP_CONTEXT, 0); return 0; } " 
  HAVE_HTMLHELP )
set ( CMAKE_REQUIRED_LIBRARIES )
if ( NOT HAVE_HTMLHELP )
  add_definitions ( -DNO_HTMLHELP )
endif ( )

# _stricmp (is there a simpler way of doing this?)
check_symbol_exists ( _stricmp string.h HAVE__STRICMP )
if ( HAVE__STRICMP )
  set ( STRICMP _stricmp )
endif ( )
if ( NOT STRICMP )
  check_symbol_exists ( stricmp string.h HAVE_STRICMP )
  if ( HAVE_STRICMP )
    set ( STRICMP stricmp )
  endif ( )
endif ( )
if ( NOT STRICMP )
  check_symbol_exists ( strcasecmp string.h HAVE_STRCASECMP )
  if ( HAVE_STRCASECMP )
    set ( STRICMP strcasecmp )
  endif ( )
endif ( )
if ( NOT STRICMP )
  message ( FATAL_ERROR "_stricmp, stricmp, or strcasecmp not found" )
endif ( )
add_definitions ( -D_stricmp=${STRICMP} )

set ( SRC_LIB src/library/LuaAux.cpp src/library/luabeans.cpp src/library/luacom.cpp 
  src/library/tLuaCOM.cpp src/library/tLuaCOMException.cpp src/library/tLuaCOMTypeHandler.cpp 
  src/library/tLuaDispatch.cpp src/library/tLuaObjList.cpp src/library/tLuaVector.cpp 
  src/library/tStringBuffer.cpp src/library/tUtil.cpp src/library/tCOMUtil.cpp src/library/tLuaCOMClassFactory.cpp 
  src/library/tLuaCOMConnPoints.cpp src/library/LuaCompat.cpp src/library/tLuaCOMEnumerator.cpp 
  src/library/tLuaObject.cpp src/library/tLuaControl.cpp src/library/tLuaTLB.cpp )

set ( LIBS gdi32 shell32 advapi32 ole32 winspool uuid oleaut32 shlwapi )
# kernel32 user32
if ( HAVE_HTMLHELP )
  set ( LIBS ${LIBS} htmlhelp )
endif ( )

add_lua_bin2c ( ${CMAKE_CURRENT_BINARY_DIR}/luacom5.loh src/library/luacom5.lua ${CMAKE_CURRENT_SOURCE_DIR}/mak/bin2c.lua 
  ${CMAKE_CURRENT_SOURCE_DIR}/mak/luac.lua )
set_source_files_properties ( src/library/luacom.cpp PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/luacom5.loh )

set ( SRC_DLL src/dll/luacom_dll.cpp )
add_definitions ( -DLUACOM_DLL="luacom.dll" )

install_lua_module ( luacom ${SRC_DLL} ${SRC_LIB} src/dll/luacom_dll.def LINK ${LIBS} )
install_data ( README COPYRIGHT announce.txt todo.txt )
install_doc ( doc/luacom.gif doc/luacom.pdf www/index.html )
install_example ( demo/ )

add_lua_test ( ${CMAKE_CURRENT_SOURCE_DIR}/src/test/luacom_tests5.lua )
