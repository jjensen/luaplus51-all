/*
* lmd5.h
* message digest library for Lua 5.1 based on OpenSSL
* Luiz Henrique de Figueiredo <lhf@tecgraf.puc-rio.br>
* 11 Nov 2010 22:58:59
* This code is hereby placed in the public domain.
*/

#include <openssl/opensslv.h>

#undef MYNAME
#undef luaopen_md5
#undef AUTHOR
#undef MD5_CTX
#undef MD5Init
#undef MD5Update
#undef MD5Final
#undef N
#undef Pget
#undef Pnew
#undef Lclone
#undef Ldigest
#undef Lnew
#undef Lreset
#undef Ltostring
#undef Lupdate
#undef Lupdatefile
#undef R

#ifdef USE_MD2_OPENSSL
#include <openssl/md2.h>
#define MYNAME			"md2"
#define luaopen_md5		luaopen_md2
#define AUTHOR			"OpenSSL " SHLIB_VERSION_NUMBER
#define MD5_CTX			MD2_CTX
#define MD5Init			MD2_Init
#define MD5Update		MD2_Update
#define MD5Final		MD2_Final
#define N			MD2_DIGEST_LENGTH
#ifdef USE_MULTIPLE
#define Pget			md2_Pget
#define Pnew			md2_Pnew
#define Lclone			md2_Lclone
#define Ldigest			md2_Ldigest
#define Lnew			md2_Lnew
#define Lreset			md2_Lreset
#define Ltostring		md2_Ltostring
#define Lupdate			md2_Lupdate
#define Lupdatefile		md2_Lupdatefile
#define R			md2_R
#endif
#endif

#ifdef USE_MD4_OPENSSL
#include <openssl/md4.h>
#define MYNAME			"md4"
#define luaopen_md5		luaopen_md4
#define AUTHOR			"OpenSSL " SHLIB_VERSION_NUMBER
#define MD5_CTX			MD4_CTX
#define MD5Init			MD4_Init
#define MD5Update		MD4_Update
#define MD5Final		MD4_Final
#define N			MD4_DIGEST_LENGTH
#ifdef USE_MULTIPLE
#define Pget			md4_Pget
#define Pnew			md4_Pnew
#define Lclone			md4_Lclone
#define Ldigest			md4_Ldigest
#define Lnew			md4_Lnew
#define Lreset			md4_Lreset
#define Ltostring		md4_Ltostring
#define Lupdate			md4_Lupdate
#define Lupdatefile		md4_Lupdatefile
#define R			md4_R
#endif
#endif

#ifdef USE_MD5_OPENSSL
#include <openssl/md5.h>
#define MYNAME			"md5"
#define luaopen_md5		luaopen_md5
#define AUTHOR			"OpenSSL " SHLIB_VERSION_NUMBER
#define MD5_CTX			MD5_CTX
#define MD5Init			MD5_Init
#define MD5Update		MD5_Update
#define MD5Final		MD5_Final
#define N			MD5_DIGEST_LENGTH
#ifdef USE_MULTIPLE
#define Pget			md5_Pget
#define Pnew			md5_Pnew
#define Lclone			md5_Lclone
#define Ldigest			md5_Ldigest
#define Lnew			md5_Lnew
#define Lreset			md5_Lreset
#define Ltostring		md5_Ltostring
#define Lupdate			md5_Lupdate
#define Lupdatefile		md5_Lupdatefile
#define R			md5_R
#endif
#endif

#ifdef USE_SHA1_OPENSSL
#include <openssl/sha.h>
#define MYNAME			"sha1"
#define luaopen_md5		luaopen_sha1
#define AUTHOR			"OpenSSL " SHLIB_VERSION_NUMBER
#define MD5_CTX			SHA_CTX
#define MD5Init			SHA1_Init
#define MD5Update		SHA1_Update
#define MD5Final		SHA1_Final
#define N			SHA_DIGEST_LENGTH
#ifdef USE_MULTIPLE
#define Pget			sha1_Pget
#define Pnew			sha1_Pnew
#define Lclone			sha1_Lclone
#define Ldigest			sha1_Ldigest
#define Lnew			sha1_Lnew
#define Lreset			sha1_Lreset
#define Ltostring		sha1_Ltostring
#define Lupdate			sha1_Lupdate
#define Lupdatefile		sha1_Lupdatefile
#define R			sha1_R
#endif
#endif

#ifdef USE_SHA224_OPENSSL
#define SHA224_CTX		SHA256_CTX
#include <openssl/sha.h>
#define MYNAME			"sha224"
#define luaopen_md5		luaopen_sha224
#define AUTHOR			"OpenSSL " SHLIB_VERSION_NUMBER
#define MD5_CTX			SHA224_CTX
#define MD5Init			SHA224_Init
#define MD5Update		SHA224_Update
#define MD5Final		SHA224_Final
#define N			SHA224_DIGEST_LENGTH
#ifdef USE_MULTIPLE
#define Pget			sha224_Pget
#define Pnew			sha224_Pnew
#define Lclone			sha224_Lclone
#define Ldigest			sha224_Ldigest
#define Lnew			sha224_Lnew
#define Lreset			sha224_Lreset
#define Ltostring		sha224_Ltostring
#define Lupdate			sha224_Lupdate
#define Lupdatefile		sha224_Lupdatefile
#define R			sha224_R
#endif
#endif

#ifdef USE_SHA256_OPENSSL
#include <openssl/sha.h>
#define MYNAME			"sha256"
#define luaopen_md5		luaopen_sha256
#define AUTHOR			"OpenSSL " SHLIB_VERSION_NUMBER
#define MD5_CTX			SHA256_CTX
#define MD5Init			SHA256_Init
#define MD5Update		SHA256_Update
#define MD5Final		SHA256_Final
#define N			SHA256_DIGEST_LENGTH
#ifdef USE_MULTIPLE
#define Pget			sha256_Pget
#define Pnew			sha256_Pnew
#define Lclone			sha256_Lclone
#define Ldigest			sha256_Ldigest
#define Lnew			sha256_Lnew
#define Lreset			sha256_Lreset
#define Ltostring		sha256_Ltostring
#define Lupdate			sha256_Lupdate
#define Lupdatefile		sha256_Lupdatefile
#define R			sha256_R
#endif
#endif

#ifdef USE_SHA384_OPENSSL
#define SHA384_CTX		SHA512_CTX
#include <openssl/sha.h>
#define MYNAME			"sha384"
#define luaopen_md5		luaopen_sha384
#define AUTHOR			"OpenSSL " SHLIB_VERSION_NUMBER
#define MD5_CTX			SHA384_CTX
#define MD5Init			SHA384_Init
#define MD5Update		SHA384_Update
#define MD5Final		SHA384_Final
#define N			SHA384_DIGEST_LENGTH
#ifdef USE_MULTIPLE
#define Pget			sha384_Pget
#define Pnew			sha384_Pnew
#define Lclone			sha384_Lclone
#define Ldigest			sha384_Ldigest
#define Lnew			sha384_Lnew
#define Lreset			sha384_Lreset
#define Ltostring		sha384_Ltostring
#define Lupdate			sha384_Lupdate
#define Lupdatefile		sha384_Lupdatefile
#define R			sha384_R
#endif
#endif

#ifdef USE_SHA512_OPENSSL
#include <openssl/sha.h>
#define MYNAME			"sha512"
#define luaopen_md5		luaopen_sha512
#define AUTHOR			"OpenSSL " SHLIB_VERSION_NUMBER
#define MD5_CTX			SHA512_CTX
#define MD5Init			SHA512_Init
#define MD5Update		SHA512_Update
#define MD5Final		SHA512_Final
#define N			SHA512_DIGEST_LENGTH
#ifdef USE_MULTIPLE
#define Pget			sha512_Pget
#define Pnew			sha512_Pnew
#define Lclone			sha512_Lclone
#define Ldigest			sha512_Ldigest
#define Lnew			sha512_Lnew
#define Lreset			sha512_Lreset
#define Ltostring		sha512_Ltostring
#define Lupdate			sha512_Lupdate
#define Lupdatefile		sha512_Lupdatefile
#define R			sha512_R
#endif
#endif

#ifdef USE_RIPEMD160_OPENSSL
#include <openssl/ripemd.h>
#define MYNAME			"ripemd160"
#define luaopen_md5		luaopen_ripemd160
#define AUTHOR			"OpenSSL " SHLIB_VERSION_NUMBER
#define MD5_CTX			RIPEMD160_CTX
#define MD5Init			RIPEMD160_Init
#define MD5Update		RIPEMD160_Update
#define MD5Final		RIPEMD160_Final
#define N			RIPEMD160_DIGEST_LENGTH
#ifdef USE_MULTIPLE
#define Pget			ripemd160_Pget
#define Pnew			ripemd160_Pnew
#define Lclone			ripemd160_Lclone
#define Ldigest			ripemd160_Ldigest
#define Lnew			ripemd160_Lnew
#define Lreset			ripemd160_Lreset
#define Ltostring		ripemd160_Ltostring
#define Lupdatefile		ripemd160_Lupdatefile
#define R			ripemd160_R
#endif
#endif

#ifdef USE_MDC2_OPENSSL
#include <openssl/mdc2.h>
#define MYNAME			"mdc2"
#define luaopen_md5		luaopen_mdc2
#define AUTHOR			"OpenSSL " SHLIB_VERSION_NUMBER
#define MD5_CTX			MDC2_CTX
#define MD5Init			MDC2_Init
#define MD5Update		MDC2_Update
#define MD5Final		MDC2_Final
#define N			MDC2_DIGEST_LENGTH
#ifdef USE_MULTIPLE
#define Pget			mdc2_Pget
#define Pnew			mdc2_Pnew
#define Lclone			mdc2_Lclone
#define Ldigest			mdc2_Ldigest
#define Lnew			mdc2_Lnew
#define Lreset			mdc2_Lreset
#define Ltostring		mdc2_Ltostring
#define Lupdate			mdc2_Lupdate
#define Lupdatefile		mdc2_Lupdatefile
#define R			mdc2_R
#endif
#endif

