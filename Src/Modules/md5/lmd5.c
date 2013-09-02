/*
* lmd5.c
* MD5 library for Lua 5.1 based on Rivest's API
* Luiz Henrique de Figueiredo <lhf@tecgraf.puc-rio.br>
* 28 Feb 2013 21:15:09
* This code is hereby placed in the public domain.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"

#include "lmd5.h"

#define MYVERSION	MYNAME " library for " LUA_VERSION " / Feb 2013 / "\
			"using " AUTHOR
#define MYTYPE		MYNAME " context"

#if LUA_VERSION_NUM <= 501
void luaL_setmetatable (lua_State *L, const char *tname) {
  luaL_getmetatable(L, tname);
  lua_setmetatable(L, -2);
}
#endif

static MD5_CTX *Pget(lua_State *L, int i)
{
 return luaL_checkudata(L,i,MYTYPE);
}

static MD5_CTX *Pnew(lua_State *L)
{
 MD5_CTX *c=lua_newuserdata(L,sizeof(MD5_CTX));
 luaL_setmetatable(L,MYTYPE);
 return c;
}

static int Lnew(lua_State *L)			/** new() */
{
 MD5_CTX *c=Pnew(L);
 MD5Init(c);
 return 1;
}

static int Lclone(lua_State *L)			/** clone(c) */
{
 MD5_CTX *c=Pget(L,1);
 MD5_CTX *d=Pnew(L);
 *d=*c;
 return 1;
}

static int Lreset(lua_State *L)			/** reset(c) */
{
 MD5_CTX *c=Pget(L,1);
 MD5Init(c);
 lua_settop(L,1);
 return 1;
}

static int Lupdate(lua_State *L)		/** update(c,s,...) */
{
 MD5_CTX *c=Pget(L,1);
 int i,n=lua_gettop(L);
 for (i=2; i<=n; i++)
 {
  size_t l;
  const char *s=luaL_checklstring(L,i,&l);
  MD5Update(c,s,l);
 }
 lua_settop(L,1);
 return 1;
}
static int Ldigest(lua_State *L)		/** digest(c or s,[raw]) */
{
 unsigned char digest[N];
 if (lua_isuserdata(L,1))
 {
  MD5_CTX c=*Pget(L,1);
  MD5Final(digest,&c);
 }
 else
 {
  size_t l;
  const char *s=luaL_checklstring(L,1,&l);
  MD5_CTX c;
  MD5Init(&c);
  MD5Update(&c,s,l);
  MD5Final(digest,&c);
 }
 if (lua_toboolean(L,2))
  lua_pushlstring(L,(char*)digest,sizeof(digest));
 else
 {
  char *digit="0123456789abcdef";
  char hex[2*N],*h;
  int i;
  for (h=hex,i=0; i<N; i++)
  {
   *h++=digit[digest[i] >> 4];
   *h++=digit[digest[i] & 0x0F];
  }
  lua_pushlstring(L,hex,sizeof(hex));
 }
 return 1;
}

static int Ltostring(lua_State *L)		/** __tostring(c) */
{
 MD5_CTX *c=Pget(L,1);
 lua_pushfstring(L,"%s %p",MYTYPE,(void*)c);
 return 1;
}

/**
**/
static int Lupdatefile(lua_State *L)
{
	MD5_CTX *c=Pget(L,1);
	const char *fileName=luaL_checkstring(L,2);
	const size_t BLOCK_SIZE = 32768;
	unsigned char* buffer;
	size_t fileLen;
	size_t bytesToDo;
	FILE* file = fopen(fileName, "rb");
	if (!file)
		return 0;
	buffer = malloc(BLOCK_SIZE);
	fseek(file, 0, SEEK_END);
	fileLen = ftell(file);
    fseek(file, 0, SEEK_SET);
	bytesToDo = fileLen;
	while (bytesToDo > 0)
	{
		size_t bytesToRead = bytesToDo < BLOCK_SIZE ? bytesToDo : BLOCK_SIZE;

        fread(buffer, bytesToRead, 1, file);
		bytesToDo -= bytesToRead;
		MD5Update(c, buffer, bytesToRead);
	}

    fseek(file, 0, SEEK_SET);
	fclose(file);

	free(buffer);
	return 0;
}


static const luaL_Reg R[] =
{
	{ "__tostring",	Ltostring},
	{ "clone",	Lclone	},
	{ "digest",	Ldigest	},
	{ "new",	Lnew	},
	{ "reset",	Lreset	},
	{ "update",	Lupdate	},
	{ "updatefile", Lupdatefile },
	{ NULL,		NULL	}
};

LUALIB_API int luaopen_md5(lua_State *L)
{
 luaL_newmetatable(L,MYTYPE);
#if LUA_VERSION_NUM >= 502
 luaL_setfuncs(L,R,0);
#else
 lua_pushvalue(L,-1);
 luaL_openlib(L,NULL,R,0);
#endif
 lua_pushliteral(L,"version");			/** version */
 lua_pushliteral(L,MYVERSION);
 lua_settable(L,-3);
 lua_pushliteral(L,"__index");
 lua_pushvalue(L,-2);
 lua_settable(L,-3);
 return 1;
}
