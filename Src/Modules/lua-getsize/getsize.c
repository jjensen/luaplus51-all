/* lua-getsize
   Author: (C) 2009 Matthew Wild
   License: MIT/X11 license
   Description: Adds a debug.getsize() function which
                returns the size in bytes of a Lua object
*/

#include <stdio.h>

#include <lua.h>
#include <lstate.h>
#include <lobject.h>
#include <lfunc.h>

int debug_getsize(lua_State* L)
{
  TValue* o = L->base;
  switch (o->tt) {
    /* Container types */
    case LUA_TTABLE: {
      Table *h = hvalue(o);
      lua_pushinteger(L, sizeof(Table) + sizeof(TValue) * h->sizearray +
                             sizeof(Node) * sizenode(h));
      break;
    }
    case LUA_TFUNCTION: {
      Closure *cl = clvalue(o);
      lua_pushinteger(L, (cl->c.isC) ? sizeCclosure(cl->c.nupvalues) :
                           sizeLclosure(cl->l.nupvalues));
      break;
    }
    case LUA_TTHREAD: {
      lua_State *th = thvalue(o);
      lua_pushinteger(L, sizeof(lua_State) + sizeof(TValue) * th->stacksize +
                                 sizeof(CallInfo) * th->size_ci);
      break;
    }
    case LUA_TPROTO: {
      Proto *p = pvalue(o);
      lua_pushinteger(L, sizeof(Proto) + sizeof(Instruction) * p->sizecode +
                             sizeof(Proto *) * p->sizep +
                             sizeof(TValue) * p->sizek + 
                             sizeof(int) * p->sizelineinfo +
                             sizeof(LocVar) * p->sizelocvars +
                             sizeof(TString *) * p->sizeupvalues);
     break;
    }
    /* Non-containers */
    case LUA_TUSERDATA: {
      lua_pushnumber(L, uvalue(o)->len);
      break;
    }
    case LUA_TLIGHTUSERDATA: {
      lua_pushnumber(L, sizeof(void*));
      break;
    }
    case LUA_TSTRING: {
      TString *s = rawtsvalue(o);
      lua_pushinteger(L, sizeof(TString) + s->tsv.len + 1);
      break;
    }
    case LUA_TNUMBER: {
      lua_pushinteger(L, sizeof(lua_Number));
      break;
    }
    case LUA_TBOOLEAN: {
      lua_pushinteger(L, sizeof(int));
      break;
    }
    default: return 0;
  }
  return 1;
}

int luaopen_getsize(lua_State* L)
{
	lua_getglobal(L, "debug");
	lua_pushcfunction(L, debug_getsize);
	lua_setfield(L, -2, "getsize");
	return 0;
}
