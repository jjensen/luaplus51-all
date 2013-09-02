#include <stdio.h>
#include <stdlib.h>

#include "City.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#define MYVERSION	"CityHash library for " LUA_VERSION

static int Lhash64(lua_State *L) {
    size_t len;
    const char *key = luaL_checklstring(L, 1, &len);
    uint64 result = CityHash64WithSeed(key, luaL_optinteger(L, 2, len), (uint64)luaL_optnumber(L, 3, 0));
    lua_pushlstring(L, (const char*)&result, sizeof(result));
    return 1;
}

static const struct luaL_Reg lcityhashlib[] = {
    { "hash64", Lhash64 },
    {NULL, NULL},
};


extern "C" int luaopen_cityhash(lua_State *L) {
    lua_newtable(L);
#if LUA_VERSION_NUM >= 502
    luaL_setfuncs(L, lcityhashlib, 0);
#else
    luaL_register(L, NULL, lcityhashlib);
#endif
    lua_pushliteral(L, "version");
    lua_pushliteral(L, MYVERSION);
    lua_settable(L, -3);

    return 1;
}
