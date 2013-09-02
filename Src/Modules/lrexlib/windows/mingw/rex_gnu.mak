# Project: rex_gnu

# User Settings ------------------------------------------------------------
# path of GNU include files
REGEXINC = s:\progr\work\system\include\gnuregex
# --------------------------------------------------------------------------

PROJECT  = rex_gnu
MYINCS   = -I$(REGEXINC)
MYLIBS   = -lregex2
OBJ      = lgnu.o common.o
PROJDIR  = gnu
TESTNAME = gnu

include _mingw.mak

lgnu.o    : common.h algo.h
common.o  : common.h
