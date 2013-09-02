#define LUA_LIB
#include "lua.h"
#include "lauxlib.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

struct tagBuffer {
  size_t      size;
  size_t      top;
  char      * arr;
  lua_State * L;
};

typedef struct tagBuffer TBuffer;

void buffer_free (TBuffer *buf) {
  free (buf->arr);
}

void buffer_init (TBuffer *buf, size_t sz, lua_State *L) {
  buf->arr = (char*) malloc (sz);
  if (!buf->arr) {
	buffer_free(buf);
    luaL_error (L, "malloc failed");
  }
  buf->size = sz;
  buf->top = 0;
  buf->L = L;
}

void buffer_clear (TBuffer *buf) {
  buf->top = 0;
}

void buffer_pushresult (TBuffer *buf) {
  lua_pushlstring (buf->L, buf->arr, buf->top);
}

void buffer_addlstring (TBuffer *buf, const void *src, size_t sz) {
  size_t newtop = buf->top + sz;
  if (newtop > buf->size) {
    char *p = (char*) realloc (buf->arr, 2 * newtop);   /* 2x expansion */
    if (!p) {
      buffer_free (buf);
      luaL_error (buf->L, "realloc failed");
    }
    buf->arr = p;
    buf->size = 2 * newtop;
  }
  if (src)
    memcpy (buf->arr + buf->top, src, sz);
  buf->top = newtop;
}

void buffer_addvalue (TBuffer *buf, int stackpos) {
  size_t len;
  const char *p = lua_tolstring (buf->L, stackpos, &len);
  buffer_addlstring (buf, p, len);
}



























static const char *lmemfind (const char *s1, size_t l1,
                               const char *s2, size_t l2) {
  if (l2 == 0) return s1;  /* empty strings are everywhere */
  else if (l2 > l1) return NULL;  /* avoids a negative `l1' */
  else {
    const char *init;  /* to search for a `*s2' inside `s1' */
    l2--;  /* 1st char will be checked by `memchr' */
    l1 = l1-l2;  /* `s2' cannot be found after that */
    while (l1 > 0 && (init = (const char *)memchr(s1, *s2, l1)) != NULL) {
      init++;   /* 1st char is already checked */
      if (memcmp(init, s2+1, l2) == 0)
        return init-1;
      else {  /* correct `l1' and `s1' to try again */
        l1 -= init-s1;
        s1 = init;
      }
    }
    return NULL;  /* not found */
  }
}


static int str_replace(lua_State *L) {
    size_t l1, l2, l3;
    const char *src = luaL_checklstring(L, 1, &l1);
    const char *p = luaL_checklstring(L, 2, &l2);
    const char *p2 = luaL_checklstring(L, 3, &l3);
	int init = luaL_optinteger(L, 4, 1) - 1;
    const char *s2;
    int n = 0;

    TBuffer b;
	
	buffer_init (&b, l1, L);

	if (init > 0) {
		buffer_addlstring(&b, src, init);
	}

    while (1) {
        s2 = lmemfind(src+init, l1-init, p, l2);
        if (s2) {
            buffer_addlstring(&b, src+init, s2-(src+init));
            buffer_addlstring(&b, p2, l3);
            init = init + (s2-(src+init)) + l2;
            n++;
        } else {
            buffer_addlstring(&b, src+init, l1-init);
            break;
        }
    }

	buffer_pushresult(&b);
	buffer_free (&b);
    lua_pushnumber(L, (lua_Number)n);  /* number of substitutions */
    return 2;
}
/*
static int str_replace(lua_State *L) {
    size_t l1, l2, l3;
    const char *src = luaL_checklstring(L, 1, &l1);
    const char *p = luaL_checklstring(L, 2, &l2);
    const char *p2 = luaL_checklstring(L, 3, &l3);
    const char *s2;
    int n = 0;
    int init = 0;

    luaL_Buffer b;
    luaL_buffinit(L, &b);

    while (1) {
        s2 = lmemfind(src+init, l1-init, p, l2);
        if (s2) {
            luaL_addlstring(&b, src+init, s2-(src+init));
            luaL_addlstring(&b, p2, l3);
            init = init + (s2-(src+init)) + l2;
            n++;
        } else {
            luaL_addlstring(&b, src+init, l1-init);
            break;
        }
    }

    luaL_pushresult(&b);
    lua_pushnumber(L, (lua_Number)n); 
    return 2;
}
*/
static const struct luaL_Reg str_funcs[] = {
  { "replace",	str_replace },
  { NULL, NULL }
};


LUALIB_API int luaopen_replace(lua_State *L) {
	lua_newtable(L);
#if LUA_VERSION_NUM >= 502
	luaL_setfuncs(L, str_funcs, 0);
#else
	luaL_register(L, NULL, str_funcs);
#endif
	return 1;
}
