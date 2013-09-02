# makefile for message digest library for Lua

# change these to reflect your Lua installation
LUA= /tmp/lhf/lua-5.2.1
LUAINC= $(LUA)/src
LUALIB= $(LUA)/src
LUABIN= $(LUA)/src

# these will probably work if Lua has been installed globally
#LUA= /usr/local
#LUAINC= $(LUA)/include
#LUALIB= $(LUA)/lib
#LUABIN= $(LUA)/bin

# probably no need to change anything below here
CC= gcc
CFLAGS= $(INCS) $(DEFS) $(WARN) -O2 $G
WARN= -ansi -pedantic -Wall -Wextra
INCS= -I$(LUAINC)
MAKESO= $(CC) -shared
#MAKESO= $(CC) -bundle -undefined dynamic_lookup

# default is md5
MYNAME=md5
DEFS=-DUSE_MD5_OPENSSL
SUM=$(MYNAME)sum

default: all

digests: md2 md4 md5 sha1 sha224 sha256 sha384 sha512 ripemd160

md2:
	$(MAKE) all MYNAME=$@ DEFS=-DUSE_MD2_OPENSSL SUM="openssl $@"

md4:
	$(MAKE) all MYNAME=$@ DEFS=-DUSE_MD4_OPENSSL SUM="openssl $@"

md5:
	$(MAKE) all MYNAME=$@ DEFS=-DUSE_MD5_OPENSSL

sha1:
	$(MAKE) all MYNAME=$@ DEFS=-DUSE_SHA1_OPENSSL

sha224:
	$(MAKE) all MYNAME=$@ DEFS=-DUSE_SHA224_OPENSSL

sha256:
	$(MAKE) all MYNAME=$@ DEFS=-DUSE_SHA256_OPENSSL

sha384:
	$(MAKE) all MYNAME=$@ DEFS=-DUSE_SHA384_OPENSSL

sha512:
	$(MAKE) all MYNAME=$@ DEFS=-DUSE_SHA512_OPENSSL

ripemd160:
	$(MAKE) all MYNAME=$@ DEFS=-DUSE_RIPEMD160_OPENSSL SUM="openssl rmd160"

mdc2:
	$(MAKE) all MYNAME=$@ DEFS=-DUSE_MDC2_OPENSSL SUM="openssl mdc2"

MYLIB= l$(MYNAME)
T= $(MYNAME).so
OBJS= $(MYLIB).o
TEST= test.lua

all:	test

test:	$T
	$(LUABIN)/lua -l$(MYNAME) $(TEST)
	-@echo -n "$(MYNAME)	" ; $(SUM) < README

o:	$(MYLIB).o

so:	$T

$T:	$(OBJS)
	$(MAKESO) -o $@ $(OBJS) -lcrypto

clean:
	rm -f $(OBJS) $T core core.* l*.o *.so

doc:
	@echo "digest library:"
	@fgrep '/**' lmd5.c | cut -f2 -d/ | tr -d '*' | sort | column

$(MYLIB).o:	lmd5.c lmd5.h
	$(CC) $(CFLAGS) -o $@ -c lmd5.c

# eof
