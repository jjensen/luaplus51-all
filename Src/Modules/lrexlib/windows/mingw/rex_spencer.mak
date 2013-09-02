# Project: rex_spencer

# User Settings ------------------------------------------------------------
# path of Spencer's include files
REGEXINC = s:\progr\work\system\include\rxspencer
# --------------------------------------------------------------------------

PROJECT  = rex_spencer
MYINCS   = -I$(REGEXINC)
MYLIBS   = -lrxspencer
OBJ      = lposix.o common.o
PROJDIR  = posix
TESTNAME = spencer

include _mingw.mak

lposix.o  : common.h algo.h
common.o  : common.h
