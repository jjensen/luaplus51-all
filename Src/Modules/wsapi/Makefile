# $Id: Makefile,v 1.14 2008/04/04 02:01:01 mascarenhas Exp $

include config

all: cgi fastcgi

cgi:

config:
	touch config

fastcgi: src/fastcgi/lfcgi.so

fcgi: fastcgi

src/fastcgi/lfcgi.so: src/fastcgi/lfcgi.o src/fastcgi/lfcgi.h
	$(CC) $(CFLAGS) $(LIB_OPTION) -o src/fastcgi/lfcgi.so src/fastcgi/lfcgi.o -lfcgi 

install:
	mkdir -p $(LUA_DIR)/wsapi
	cp src/wsapi/*.lua $(LUA_DIR)/wsapi
	cp src/launcher/wsapi.cgi $(BIN_DIR)/
	cp src/launcher/wsapi.fcgi $(BIN_DIR)/

install-fcgi:
	cp src/fastcgi/lfcgi.so $(LUA_LIBDIR)/

install-rocks: install
	mkdir -p $(PREFIX)/samples
	cp -r samples/* $(PREFIX)/samples
	mkdir -p $(PREFIX)/doc
	cp -r doc/* $(PREFIX)/doc

clean:
	rm src/fastcgi/lfcgi.o src/fastcgi/lfcgi.so

snapshot:
	cvs export -r HEAD -d wsapi-1.0-snapshot wsapi
	tar czf wsapi-1.0-snapshot.tar.gz wsapi-1.0-snapshot
	rm -rf wsapi-1.0-snapshot
	scp wsapi-1.0-snapshot.tar.gz mascarenhas@139.82.100.4:public_html/

