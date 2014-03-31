# ----------------------------------------------------------------------------
# Purpose : Makefile for wxstedit sample - UNIX or cygwin using gmake (gcc)
# Author : John Labenski
# Copyright : John Labenski 2006
# License : wxWidgets license
# ----------------------------------------------------------------------------

# ----------------------------------------------------------------------------
# wxWidgets build settings

WXCONFIG   := wx-config

WXDIR      := $(shell $(WXCONFIG) --prefix)
WXEXECDIR  := $(shell $(WXCONFIG) --exec-prefix)
WXLIBDIR   := $(WXEXECDIR)/lib

WXBASENAME := $(shell $(WXCONFIG) --basename)
WXVERSION  := $(shell $(WXCONFIG) --version)
WXRELEASE  := $(shell $(WXCONFIG) --release)

WXCXX      := $(shell $(WXCONFIG) --cxx)
WXCXXFLAGS := $(shell $(WXCONFIG) --cxxflags)
WXLIBS     := $(shell $(WXCONFIG) --libs stc,html,adv,core,base)

WXSTC_LIB  := $(WXBASENAME)_stc-$(WXRELEASE)

# Try to get the lib path, "/usr/local/lib64" from wx-config --libs = "-L/usr/local/lib64 -pthread ..."
WXLIBDIR   := $(shell $(WXCONFIG) --libs | awk '{for(i=1;i<=NF;i++) if ($$i~/\-L/){ print substr($$i,3); break } }')

ifeq (x$(WXLIBDIR),x)
    WXLIBDIR_RPATH := 
else
    WXLIBDIR_RPATH := -Xlinker -rpath $(WXLIBDIR)
endif


# ----------------------------------------------------------------------------
# Settings for the stedit library

#WXSTEDIT_LIBDIR  = $(WXLIBDIR)
WXSTEDIT_LIBDIR  = ../../lib
WXSTEDIT_LIBDIR_ABS := $(realpath $(WXSTEDIT_LIBDIR))
WXSTEDIT_LIBNAME = $(WXBASENAME)_stedit-$(WXRELEASE)
WXSTEDIT_LIBNAME := $(patsubst wx_%,wxcode_%,$(WXSTEDIT_LIBNAME))

LIBVERSION_CURRENT  = 1
LIBVERSION_REVISION = 6
LIBVERSION_AGE      = 0

TARGET_LIBNAME = lib$(WXSTEDIT_LIBNAME)
TARGET_LIBNAME_STATIC = $(TARGET_LIBNAME).a
TARGET_LIBNAME_SHARED = $(TARGET_LIBNAME).so.$(LIBVERSION_CURRENT).$(LIBVERSION_REVISION).$(LIBVERSION_AGE)
TARGET_LIB_STATIC = $(WXSTEDIT_LIBDIR)/$(TARGET_LIBNAME_STATIC)
TARGET_LIB_SHARED = $(WXSTEDIT_LIBDIR)/$(TARGET_LIBNAME_SHARED)
TARGET_LIB_LINK1  = $(TARGET_LIBNAME).so.$(LIBVERSION_CURRENT)
TARGET_LIB_LINK2  = $(TARGET_LIBNAME).so

# ----------------------------------------------------------------------------
# CXX flags and libs

CXXFLAGS = $(WXCXXFLAGS) -MMD -g  -Wall -Wunused -Wunused-parameter -fPIC -DPIC
LDLIBS   = $(WXLIBS) -Wl,--as-needed
CXX      = $(WXCXX)

# -pg in libs and defs for profiling
PROFILE = #-pg
APPEXTRADEFS = -I../../include -I$(WXDIR)/contrib/include $(PROFILE)
APPEXTRALIBS = $(PROFILE)

# ----------------------------------------------------------------------------
# Source files, objects to link

PROGRAM = wxstedit

OBJECTS = \
    wxstedit.o \
    stedoc.o

DEPFILES := $(OBJECTS:.o=.d)

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(APPEXTRADEFS) -o $@ $<

# ---------------------------------------------------------------------------
# Targets

all: stedit_lib wxstedit

stedit_lib :
	@(cd ../../src && $(MAKE))

# wxstedit program using shared stedit.so library
wxstedit: stedit_lib $(OBJECTS)
	$(CXX) $(LDFLAGS_EXE) $(WXLIBDIR_RPATH) -Xlinker -rpath $(WXSTEDIT_LIBDIR_ABS) -o $@ $(OBJECTS) $(APPEXTRALIBS) -L${WXSTEDIT_LIBDIR} -l$(WXSTEDIT_LIBNAME) $(LDLIBS)

# wxstedit program using static stedit.a library
wxstedit-static: stedit_lib $(OBJECTS)
	$(CXX) $(LDFLAGS_EXE) $(WXLIBDIR_RPATH) -o $@ $(PROGRAM).o $(APPEXTRALIBS) $(TARGET_LIB_STATIC) $(LDLIBS) 

docs:
	# requires wxLua's bin2c.lua program where $(WXLUA) is environment var
	# of the path to the base dir of wxLua
	$(WXLUA)/bin/lua $(WXLUA)/util/bin2c/bin2c.lua -t -o wxstedit_htm.hpp ../../docs/wxstedit.htm
	$(WXLUA)/bin/lua $(WXLUA)/util/bin2c/bin2c.lua -t -o readme_htm.hpp ../../docs/readme.htm

clean:
	rm -f $(OBJECTS) $(DEPFILES) $(PROGRAM) $(PROGRAM)-static core

cleanall: clean
	@(cd ../../src && make clean)


-include $(DEPFILES)
