PACKAGE=lua-memoryfile
VERSION=$(shell head -1 Changes | sed 's/ .*//')
RELEASEDATE=$(shell head -1 Changes | sed 's/.* //')
PREFIX=/usr/local
DISTNAME=$(PACKAGE)-$(VERSION)

# The path to where the module's source files should be installed.
LUA_CPATH:=$(shell pkg-config lua5.1 --define-variable=prefix=$(PREFIX) \
                              --variable=INSTALL_CMOD)

LIBDIR = $(PREFIX)/lib

# Uncomment this to run the regression tests with valgrind.
#VALGRIND = valgrind -q --leak-check=yes --show-reachable=yes --num-callers=10

OBJECTS = memoryfile.lo
SOURCES := $(OBJECTS:.lo=.c)

LIBTOOL := libtool --quiet

CC := gcc
CFLAGS := -ansi -pedantic -Wall -W -Wshadow -Wpointer-arith \
          -Wcast-align -Wwrite-strings -Wstrict-prototypes \
          -Wmissing-prototypes -Wnested-externs -Wno-long-long \
          $(shell pkg-config --cflags lua5.1) \
          -DVERSION=\"$(VERSION)\"
LDFLAGS := $(shell pkg-config --libs lua5.1)

# Uncomment this line to enable optimization.  Comment it out when running
# the test suite because it makes the assert() errors clearer and avoids
# warnings about ridiculously long string constants with some versions of gcc.
CFLAGS := $(CFLAGS) -O2
#CFLAGS := $(CFLAGS) -O3 -fomit-frame-pointer

# Uncomment this line to enable debugging.
#DEBUG := -g

# Uncomment this line to prevent the module from being unloaded when Lua exits,
# so that Valgrind can still access the debugging symbols.
#DEBUG := $(DEBUG) -DVALGRIND_LUA_MODULE_HACK

# Uncomment one of these lines to enable profiling and/or gcov coverage testing.
#DEBUG := $(DEBUG) -pg
#DEBUG := $(DEBUG) -fprofile-arcs -ftest-coverage

all: liblua-memoryfile.la manpages

manpages: doc/lua-memoryfile.3
doc/lua-memoryfile.3: doc/lua-memoryfile.pod Changes
	sed 's/E<copy>/(c)/g' <$< | sed 's/E<ndash>/-/g' | \
	    pod2man --center="In-memory file handles for Lua" \
	            --name="LUA-MEMORYFILE" --section=3 \
	            --release="$(VERSION)" --date="$(RELEASEDATE)" >$@

test: all
	echo 'lunit.main({...})' | $(VALGRIND) lua -llunit - test/*.lua

install: all
	mkdir -p $(LUA_CPATH)
	install --mode=644 .libs/liblua-memoryfile.so.0.0.0 $(LUA_CPATH)/memoryfile.so
	mkdir -p $(PREFIX)/share/man/man3
	gzip -c doc/lua-memoryfile.3 >$(PREFIX)/share/man/man3/lua-memoryfile.3.gz;


checktmp:
	@if [ -e tmp ]; then \
	    echo "Can't proceed if file 'tmp' exists"; \
	    false; \
	fi
dist: all checktmp
	mkdir -p tmp/$(DISTNAME)
	tar cf - --files-from MANIFEST | (cd tmp/$(DISTNAME) && tar xf -)
	cd tmp && tar cf - $(DISTNAME) | gzip -9 >../$(DISTNAME).tar.gz
	cd tmp && tar cf - $(DISTNAME) | bzip2 -9 >../$(DISTNAME).tar.bz2
	rm -rf tmp


%.lo: %.c
	@echo 'CC>' $@
	@$(LIBTOOL) --mode=compile $(CC) $(CFLAGS) $(DEBUG) -c -o $@ $<
memoryfile.lo: memoryfile.c memoryfile.h
liblua-memoryfile.la: memoryfile.lo
	@echo 'LD>' $@
	@$(LIBTOOL) --mode=link $(CC) $(LDFLAGS) $(DEBUG) -o $@ $< -rpath $(LIBDIR)

clean:
	rm -f *.o *.lo
	rm -rf liblua-memoryfile.la .libs
	rm -f gmon.out *.bb *.bbg *.da *.gcov
realclean: clean
	rm -f doc/lua-memoryfile.3

.PHONY: all test install checktmp dist clean realclean
