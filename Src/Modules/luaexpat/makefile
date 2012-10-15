T= lxp
V= 1.3.0
CONFIG= ./config

include $(CONFIG)

OBJS= src/lxplib.o
lib: src/$(LIBNAME)

src/$(LIBNAME) : $(OBJS)
	export MACOSX_DEPLOYMENT_TARGET="10.3"; $(CC) -o src/$(LIBNAME) $(LIB_OPTION) $(OBJS) -lexpat

install:
	mkdir -p $(LUA_LIBDIR)
	cp src/$(LIBNAME) $(LUA_LIBDIR)
	cd $(LUA_LIBDIR); ln -f -s $(LIBNAME) $T.so
	mkdir -p $(LUA_DIR)/$T
	cp src/$T/lom.lua $(LUA_DIR)/$T

clean:
	rm -f src/$(LIBNAME) $(OBJS)

# $Id: makefile,v 1.33 2006/06/08 20:41:48 tomas Exp $
