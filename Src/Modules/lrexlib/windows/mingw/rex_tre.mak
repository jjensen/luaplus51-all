# Project: rex_tre

# User Settings ------------------------------------------------------------
# path of TRE include files
REGEXINC = s:\progr\work\system\include
# --------------------------------------------------------------------------

PROJECT  = rex_tre
MYINCS   = -I$(REGEXINC)
MYLIBS   = -ltre
OBJ      = ltre.o common.o
PROJDIR  = tre
TESTNAME = tre

# Uncomment the following line to add wide-character functions (in alpha state).
# ADDWIDECHARFUNCS = 1
ifdef ADDWIDECHARFUNCS
  OBJ += ltre_w.o
  MYCFLAGS += -DREX_ADDWIDECHARFUNCS
endif

include _mingw.mak

ltre.o    : common.h algo.h
ltre_w.o  : common.h algo.h
common.o  : common.h
