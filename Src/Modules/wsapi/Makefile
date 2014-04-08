# $Id: Makefile,v 1.14 2008/04/04 02:01:01 mascarenhas Exp $

include config

DESTDIR := /
LDIR := $(DESTDIR)/$(LUA_DIR)
WDIR := $(DESTDIR)/$(LUA_DIR)/wsapi
CDIR := $(DESTDIR)/$(LUA_LIBDIR)
BDIR := $(DESTDIR)/$(BIN_DIR)

all: cgi fastcgi

cgi:

config:
	touch config

fastcgi: src/fastcgi/lfcgi.so

fcgi: fastcgi

src/fastcgi/lfcgi.so: src/fastcgi/lfcgi.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIB_OPTION) -o src/fastcgi/lfcgi.so src/fastcgi/lfcgi.c -lfcgi $(INC)

install:
	@mkdir -p $(LDIR) $(WDIR) $(BDIR)
	@cp src/*.lua $(LDIR)
	@cp src/wsapi/*.lua $(WDIR)
	@cp src/launcher/wsapi.cgi $(BDIR)
	@cp src/launcher/wsapi.fcgi $(BDIR)
	@echo "Installing of Lua WSAPI part is done!"

install-fcgi:
	@mkdir -p $(CDIR)
	@cp src/fastcgi/lfcgi.so $(CDIR)
	@echo "Installing of bundled Lua-fcgi lib is done!"

install-rocks: install
	mkdir -p $(PREFIX)/samples
	cp -r samples/* $(PREFIX)/samples
	mkdir -p $(PREFIX)/doc
	cp -r doc/* $(PREFIX)/doc

clean:
	@rm -f config src/fastcgi/lfcgi.so
	@echo "Cleaning is done!"

snapshot:
	git archive --format=tar --prefix=wsapi-$(VERSION)/ HEAD | gzip > wsapi-$(VERSION).tar.gz

rockspecs:
	for pkg in wsapi wsapi-fcgi wsapi-xavante ; do cp rockspec/$$pkg-$(VERSION_OLD)-1.rockspec rockspec/$$pkg-$(VERSION_NEW)-1.rockspec ; done
	for pkg in wsapi wsapi-fcgi wsapi-xavante; do sed -e "s/$(VERSION_OLD)/$(VERSION_NEW)/g" -i "" rockspec/$$pkg-$(VERSION_NEW)-1.rockspec ; done
	for pkg in wsapi wsapi-fcgi wsapi-xavante; do git add rockspec/$$pkg-$(VERSION_NEW)-1.rockspec ; done
