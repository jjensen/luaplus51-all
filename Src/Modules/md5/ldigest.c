/*
* ldigest.c
* message digest library for Lua 5.1 based on OpenSSL
* Luiz Henrique de Figueiredo <lhf@tecgraf.puc-rio.br>
* 24 Mar 2011 12:11:47
* This code is hereby placed in the public domain.
*/

/* build with
	make ldigest.o DEFS= ; make MYNAME=digest SUM=echo
*/

#include <stdio.h>
#include <stdlib.h>

#include "lua.h"
#include "lauxlib.h"

#define USE_MULTIPLE

#ifdef OPENSSL_NO_MD2
#define luaopen_md2(L)
#else
#define USE_MD2_OPENSSL
#include "lmd5.c"
#undef USE_MD2_OPENSSL
#endif

#ifdef OPENSSL_NO_MD4
#define luaopen_md4(L)
#else
#define USE_MD4_OPENSSL
#include "lmd5.c"
#undef USE_MD4_OPENSSL
#endif

#ifdef OPENSSL_NO_MD5
#define luaopen_md5(L)
#else
#define USE_MD5_OPENSSL
#include "lmd5.c"
#undef USE_MD5_OPENSSL
#endif

#ifdef OPENSSL_NO_SHA1
#define luaopen_sha1(L)
#else
#define USE_SHA1_OPENSSL
#include "lmd5.c"
#undef USE_SHA1_OPENSSL
#endif

#ifdef OPENSSL_NO_SHA256
#define luaopen_sha224(L)
#else
#define USE_SHA224_OPENSSL
#include "lmd5.c"
#undef USE_SHA224_OPENSSL
#endif

#ifdef OPENSSL_NO_SHA256
#define luaopen_sha256(L)
#else
#define USE_SHA256_OPENSSL
#include "lmd5.c"
#undef USE_SHA256_OPENSSL
#endif

#ifdef OPENSSL_NO_SHA512
#define luaopen_sha384(L)
#else
#define USE_SHA384_OPENSSL
#include "lmd5.c"
#undef USE_SHA384_OPENSSL
#endif

#ifdef OPENSSL_NO_SHA512
#define luaopen_sha512(L)
#else
#define USE_SHA512_OPENSSL
#include "lmd5.c"
#undef USE_SHA512_OPENSSL
#endif

#ifdef OPENSSL_NO_RIPEMD160
#define luaopen_ripemd160(L)
#else
#define USE_RIPEMD160_OPENSSL
#include "lmd5.c"
#undef USE_RIPEMD160_OPENSSL
#endif

#ifdef OPENSSL_NO_MDC2
#define luaopen_mdc2(L)
#else
#define USE_MDC2_OPENSSL
#include "lmd5.c"
#undef USE_MDC2_OPENSSL
#endif

#undef luaopen_md5

LUALIB_API int luaopen_digest(lua_State *L)
{
 luaopen_md2(L);
 luaopen_md4(L);
 luaopen_md5(L);
 luaopen_sha1(L);
 luaopen_sha224(L);
 luaopen_sha256(L);
 luaopen_sha384(L);
 luaopen_sha512(L);
 luaopen_ripemd160(L);
 luaopen_mdc2(L);
 return 0;
}
