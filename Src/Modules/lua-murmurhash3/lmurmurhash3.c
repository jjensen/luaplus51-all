#include <stdio.h>
#include <stdlib.h>

#include "MurmurHash3.h"
#include "PMurHash.h"

#include "lua.h"
#include "lauxlib.h"

#define MYVERSION	"MurmurHash library for " LUA_VERSION
#define MURMURHASH3_METATABLE		"murmurhash context"

typedef struct pmurhash_context {
    MH_UINT32 seed;
    MH_UINT32 carry;
    size_t len;
} pmurhash_context;

static pmurhash_context *Pget(lua_State *L, int i) {
    return luaL_checkudata(L, i, MURMURHASH3_METATABLE);
}

static pmurhash_context *Pnew(lua_State *L, MH_UINT32 seed, MH_UINT32 carry)
{
    pmurhash_context *context = lua_newuserdata(L, sizeof(pmurhash_context));
    context->seed = seed;
    context->carry = carry;
    context->len = 0;
    luaL_getmetatable(L, MURMURHASH3_METATABLE);
    lua_setmetatable(L, -2);
    return context;
}

static int Lnew(lua_State *L) {
    Pnew(L, luaL_optinteger(L, 1, 0), luaL_optinteger(L, 2, 0));
    return 1;
}

static int Lclone(lua_State *L) {
    pmurhash_context *c = Pget(L, 1);
    pmurhash_context *newContext = Pnew(L, c->seed, c->carry);
    newContext->len = c->len;
    return 1;
}

static int Lreset(lua_State *L) {
    pmurhash_context *c = Pget(L, 1);
    c->seed = luaL_optinteger(L, 1, 0);
    c->carry = luaL_optinteger(L, 2, 0);
    c->len = 0;
    return 0;
}

static int Lupdate(lua_State *L) {
    size_t len;
    pmurhash_context *c = Pget(L, 1);
    const char *key = luaL_checklstring(L, 2, &len);
    len = luaL_optinteger(L, 3, len);
    PMurHash32_Process(&c->seed, &c->carry, key, len);
    c->len += len;
    return 0;
}

static int Ldigest(lua_State *L) {
    pmurhash_context *c = Pget(L, 1);
    lua_pushnumber(L, (lua_Number)PMurHash32_Result(c->seed, c->carry, c->len));
    return 1;
}

static int Ltostring(lua_State *L) {
    pmurhash_context *c = Pget(L, 1);
    lua_pushfstring(L, "%s %p", MURMURHASH3_METATABLE, (void*)c);
    return 1;
}

static const luaL_Reg R[] = {
    { "clone",	Lclone	},
    { "digest",	Ldigest	},
    { "reset",	Lreset	},
    { "tostring",	Ltostring},
    { "update",	Lupdate	},
    { NULL,		NULL	}
};

static int Lhash32(lua_State *L) {
    size_t len;
    const char *key = luaL_checklstring(L, 1, &len);
    lua_pushnumber(L, (lua_Number)PMurHash32((MH_UINT32)luaL_optnumber(L, 3, 0), key, luaL_optinteger(L, 2, len)));
    return 1;
}

static int Lfasthash32(lua_State *L) {
    size_t len;
    const char *key = luaL_checklstring(L, 1, &len);

    unsigned int out;
    MurmurHash3_x86_32(key, luaL_optinteger(L, 2, len), (MH_UINT32)luaL_optnumber(L, 3, 0), &out);
    lua_pushnumber(L, (lua_Number)out);
    return 1;
}

static int Lfasthash128(lua_State *L) {
    size_t len;
    const char *key = luaL_checklstring(L, 1, &len);

    unsigned char out[16];
    MurmurHash3_x86_128(key, luaL_optinteger(L, 2, len), (MH_UINT32)luaL_optnumber(L, 3, 0), &out);
    lua_pushlstring(L, out, 16);
    return 1;
}


static const struct luaL_Reg lmurmurhash3lib[] = {
    { "new", Lnew },
    { "hash32", Lhash32 },
    { "fasthash32", Lfasthash32 },
    { "fasthash128", Lfasthash128 },
    {NULL, NULL},
};


int luaopen_murmurhash3(lua_State *L) {
    lua_newtable(L);
#if LUA_VERSION_NUM >= 502
    luaL_setfuncs(L, lmurmurhash3lib, 0);
#else
    luaL_register(L, NULL, lmurmurhash3lib);
#endif
    lua_pushliteral(L, "version");
    lua_pushliteral(L, MYVERSION);
    lua_settable(L, -3);

    luaL_newmetatable(L, MURMURHASH3_METATABLE);
#if LUA_VERSION_NUM >= 502
    luaL_setfuncs(L, R, 0);
#else
    luaL_openlib(L, NULL, R, 0);
#endif
    lua_pushliteral(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    lua_pushliteral(L, "__tostring");
    lua_pushliteral(L, "tostring");
    lua_gettable(L, -3);
    lua_settable(L, -3);
    lua_pop(L, 1);

    return 1;
}
