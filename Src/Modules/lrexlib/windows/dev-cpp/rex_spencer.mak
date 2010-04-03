# Project: rex_spencer

# User Settings ------------------------------------------------------------
# path of Lua include files
LUAINC = s:\progr\work\system\include

# path of Spencer's include files
REGEXINC = s:\progr\work\system\include\rxspencer

# path of Lua DLL and rxspencer.dll
DLLPATH = c:\exe

# name of Lua DLL to link to (.dll should be omitted)
LUADLL = lua5.1

# path to install rex_spencer.dll
INSTALLPATH = s:\exe\lib\lua\5.1
# --------------------------------------------------------------------------

PROJECT     = rex_spencer
MYINCS      = -I$(REGEXINC) -I$(LUAINC) 
MYLIBS      = -L$(DLLPATH) -lrxspencer -l$(LUADLL)
OBJ         = lposix.o common.o
MYCFLAGS    = -W -Wall -O2
EXPORTED    = 'luaopen_$(PROJECT)'
SRCPATH     = ..\..\src;..\..\src\posix
TESTPATH    = ..\..\test
TESTNAME    = spencer

include _mingw.mak

lposix.o  : common.h algo.h
common.o  : common.h

