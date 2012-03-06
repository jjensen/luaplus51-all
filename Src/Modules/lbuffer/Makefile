# This Makefile is based on LuaSec's Makefile. Thanks to the LuaSec developers.
# Inform the location to intall the modules
LUAPATH  = /usr/share/lua/5.1
LUACPATH = /usr/lib/lua/5.1
INCDIR   = -I/usr/include/lua5.1
LIBDIR   = -L/usr/lib

# For Mac OS X: set the system version
MACOSX_VERSION = 10.4

PLAT = none
DEFS =
CMOD = buffer.so
OBJS = lbuffer.o lb_interface.o

LIBS = -llua
WARN = -Wall -pedantic

BSD_CFLAGS  = -O2 -fPIC
BSD_LDFLAGS = -O -shared -fPIC $(LIBDIR)

LNX_CFLAGS  = -O2 -fPIC
LNX_LDFLAGS = -O -shared -fPIC $(LIBDIR)

MAC_ENV     = env MACOSX_DEPLOYMENT_TARGET='$(MACVER)'
MAC_CFLAGS  = -O2 -fPIC -fno-common
MAC_LDFLAGS = -bundle -undefined dynamic_lookup -fPIC $(LIBDIR)

MGW_INCDIR = -Id:/lua/include
MGW_LIBS = d:/lua/lua51.dll
MGW_CMOD = buffer.dll
MGW_CFLAGS = -O2 -mdll -DLUA_BUILD_AS_DLL
MGW_LDFLAGS = -mdll

CC = gcc
LD = $(MYENV) gcc
CFLAGS  = $(MYCFLAGS) $(WARN) $(INCDIR) $(DEFS)
LDFLAGS = $(MYLDFLAGS) $(LIBDIR)

.PHONY: test clean install none linux bsd macosx

none:
	@echo "Usage: $(MAKE) <platform>"
	@echo "  * linux"
	@echo "  * bsd"
	@echo "  * macosx"
	@echo "  * mingw"

install: $(CMOD)
	cp $(CMOD) $(LUACPATH)

uninstall:
	-rm $(LUACPATH)/$(CMOD)

linux:
	@$(MAKE) $(CMOD) PLAT=linux MYCFLAGS="$(LNX_CFLAGS)" MYLDFLAGS="$(LNX_LDFLAGS)"

bsd:
	@$(MAKE) $(CMOD) PLAT=bsd MYCFLAGS="$(BSD_CFLAGS)" MYLDFLAGS="$(BSD_LDFLAGS)"

macosx:
	@$(MAKE) $(CMOD) PLAT=macosx MYCFLAGS="$(MAC_CFLAGS)" MYLDFLAGS="$(MAC_LDFLAGS)" MYENV="$(MAC_ENV)"

mingw:
	@$(MAKE) $(MGW_CMOD) PLAT=mingw MYCFLAGS="$(MGW_CFLAGS)" MYLDFLAGS="$(MGW_LDFLAGS)" INCDIR="$(MGW_INCDIR)" LIBS="$(MGW_LIBS)" CMOD="$(MGW_CMOD)"

test:
	lua ./test.lua

clean:
	-rm -f $(OBJS) $(CMOD) $(MGW_CMOD)

.c.o:
	$(CC) $(CFLAGS) $< -c -o $@

$(CMOD): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $@
