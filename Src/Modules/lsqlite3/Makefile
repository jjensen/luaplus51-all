# Makefile for lsqlite3 library for Lua

LIBNAME= lsqlite3

LUAEXE= lua

ROCKSPEC= $(shell find . -name $(LIBNAME)-*-*.rockspec)

all: install

install:
	luarocks make $(ROCKSPEC)

test: 
	$(LUAEXE) test/test.lua
	$(LUAEXE) test/tests-sqlite3.lua

.PHONY: all test install
