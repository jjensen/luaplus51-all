# Project: rex_onig

# User Settings ------------------------------------------------------------
# path of Oniguruma include files
REGEXINC = s:\progr\work\system\include
# --------------------------------------------------------------------------

PROJECT  = rex_onig
MYINCS   = -I$(REGEXINC)
MYLIBS   = -lonig -Wl,--enable-auto-import
OBJ      = lonig.o lonig_f.o common.o
PROJDIR  = oniguruma
TESTNAME = onig

include _mingw.mak

lonig.o   : common.h algo.h
lonig_f.o : common.h
common.o  : common.h
