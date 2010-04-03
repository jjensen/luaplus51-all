# Project: rex_onig

# User Settings ------------------------------------------------------------
# path of Lua include files
LUAINC = s:\progr\work\system\include

# path of Oniguruma include files
REGEXINC = s:\progr\work\system\include

# path of Lua DLL and onig.dll
DLLPATH = c:\exe

# name of Lua DLL to link to (.dll should be omitted)
LUADLL = lua5.1

# path to install rex_onig.dll
INSTALLPATH = s:\exe\lib\lua\5.1
# --------------------------------------------------------------------------

PROJECT     = rex_onig
MYINCS      = -I$(REGEXINC) -I$(LUAINC) 
MYLIBS      = -L$(DLLPATH) -lonig -l$(LUADLL) -Wl,--enable-auto-import
OBJ         = lonig.o lonig_f.o common.o
MYCFLAGS    = -W -Wall -O2
EXPORTED    = 'luaopen_$(PROJECT)'
SRCPATH     = ..\..\src;..\..\src\oniguruma
TESTPATH    = ..\..\test
TESTNAME    = onig

include _mingw.mak

lonig.o   : common.h algo.h
lonig_f.o : common.h
common.o  : common.h

