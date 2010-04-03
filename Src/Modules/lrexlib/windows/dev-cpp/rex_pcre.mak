# Project: rex_pcre

# User Settings ------------------------------------------------------------
# path of Lua include files
LUAINC = s:\progr\work\system\include

# path of PCRE include files
REGEXINC = s:\progr\work\system\include

# path of Lua DLL and pcre.dll
DLLPATH = c:\exe

# name of Lua DLL to link to (.dll should be omitted)
LUADLL = lua5.1

# path to install rex_pcre.dll
INSTALLPATH = s:\exe\lib\lua\5.1
# --------------------------------------------------------------------------

PROJECT     = rex_pcre
MYINCS      = -I$(REGEXINC) -I$(LUAINC) 
MYLIBS      = -L$(DLLPATH) -lpcre -l$(LUADLL)
OBJ         = lpcre.o lpcre_f.o common.o
MYCFLAGS    = -W -Wall -O2
EXPORTED    = 'luaopen_$(PROJECT)'
SRCPATH     = ..\..\src;..\..\src\pcre
TESTPATH    = ..\..\test
TESTNAME    = pcre

include _mingw.mak

lpcre.o   : common.h algo.h
lpcre_f.o : common.h
common.o  : common.h

