# $Id: Makefile,v 1.3 2008/01/22 18:39:18 mascarenhas Exp $

config_file:=config

DESTDIR ?= /

ifneq '$(wildcard $(config_file))' ''
include $(config_file)
endif

$(config_file):
	chmod +x configure

install: $(config_file)
	mkdir -p $(DESTDIR)$(LUA_DIR)
	cp src/coxpcall.lua $(DESTDIR)$(LUA_DIR)/

install-doc: install
	mkdir -p $(DESTDIR)$(DOC_PREFIX)/doc
	cp -r doc/* $(DESTDIR)$(DOC_PREFIX)/doc
	echo "Go to $(DOC_PREFIX) for docs!"

