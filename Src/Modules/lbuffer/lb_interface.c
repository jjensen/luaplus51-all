#define LUA_LIB
#include "lbuffer.h"


#include <string.h>


#ifdef LB_REPLACE_LUA_API
#  undef lua_isstring
#  undef lua_tolstring
#  undef luaL_checklstring
#  undef luaL_optlstring
#endif


/* buffer interface */

buffer *lb_rawtestbuffer(lua_State *L, int narg)
{
    void *p = lua_touserdata(L, narg);
    if (p != NULL) {  /* value is a userdata? */
        if (lua_getmetatable(L, narg)) {  /* does it have a metatable? */
            /* get correct metatable */
#if LUA_VERSION_NUM >= 502
            lua_rawgetp(L, LUA_REGISTRYINDEX, lb_libname);
#else
            lua_pushlightuserdata(L, (void*)lb_libname);
            lua_rawget(L, LUA_REGISTRYINDEX);
#endif
            if (!lua_rawequal(L, -1, -2))  /* not the same? */
                p = NULL;  /* value is a userdata with wrong metatable */
            lua_pop(L, 2);  /* remove both metatables */
            return (buffer*)p;
        }
    }
    return NULL;  /* value is not a userdata with a metatable */
}

buffer *lb_testbuffer(lua_State *L, int narg)
{
    buffer *b = lb_rawtestbuffer(L, narg);
    if (b != NULL && lb_isinvalidsub(b))
        luaL_error(L, "invalid subbuffer (%p)", b);
    return b;
}

static int typeerror(lua_State *L, int narg, const char *tname)
{
    const char *msg = lua_pushfstring(L, "%s expected, got %s",
                                      tname, luaL_typename(L, narg));
    return luaL_argerror(L, narg, msg);
}

buffer *lb_checkbuffer(lua_State *L, int narg)
{
    buffer *b = lb_testbuffer(L, narg);
    if (b == NULL)
        typeerror(L, narg, lb_libname);
    return b;
}

static void lb_setmetatable(lua_State *L)
{
#if LUA_VERSION_NUM >= 502
    lua_rawgetp(L, LUA_REGISTRYINDEX, lb_libname);
#else
    lua_pushlightuserdata(L, (void*)lb_libname);
    lua_rawget(L, LUA_REGISTRYINDEX);
#endif
    lua_setmetatable(L, -2);
}

#ifdef LB_SUBBUFFER
subbuffer *lb_initsubbuffer(subbuffer *b)
{
    b->str = NULL;
    b->len = 0;
    b->subtype = LB_INVALID_SUB;
    b->parent = NULL;
    b->subparent = NULL;
    return b;
}

static void register_subbuffer(lua_State *L, subbuffer *sb)
{
    lua_getfield(L, LUA_REGISTRYINDEX, LB_SBPTR_BOX); /* 2 */
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1); /* (2) */
        lua_createtable(L, 0, 1); /* 2 */
        lua_createtable(L, 0, 1); /* 3 */
        lua_pushliteral(L, "__mode"); /* 4 */
        lua_pushliteral(L, "v"); /* 5 */
        lua_rawset(L, -3); /* 4,5->3 */
        lua_setmetatable(L, -2); /* 3->2 */
        lua_pushvalue(L, -1); /* 2->3 */
        lua_setfield(L, LUA_REGISTRYINDEX, LB_SBPTR_BOX); /* 3->(reg) */
    }
    lua_pushlightuserdata(L, sb); /* 3 */
    lua_pushvalue(L, -3); /* 1->4 */
    lua_rawset(L, -3); /* 3,4->2 */
    lua_pop(L, 1); /* (2) */
}

static subbuffer *retrieve_subbuffer(lua_State *L, subbuffer *sb)
{
    lua_getfield(L, LUA_REGISTRYINDEX, LB_SBPTR_BOX);
    if (!lua_isnil(L, -1)) {
        lua_pushlightuserdata(L, sb);
        lua_rawget(L, -2);
        lua_remove(L, -2);
        if (!lua_isnil(L, -1))
            return sb;
    }
    lua_pop(L, 1);
    return NULL;
}

buffer *lb_newsubbuffer(lua_State *L, buffer *b, size_t begin, size_t end)
{
    subbuffer *sb, *subparent = NULL;
    int i;
    char *str;
    size_t len;
    begin = begin > b->len ? b->len : begin;
    end = end > b->len ? b->len : end;
    str = &b->str[begin];
    len = begin < end ? end - begin : 0;

    if (lb_issubbuffer(b)) {
        subparent = (subbuffer*)b;
        b = ((subbuffer*)b)->parent;
    }
    for (i = 0; i < b->subcount; ++i) {
        if (b->subs[i]->str == str
                && b->subs[i]->len == len
                && (sb = retrieve_subbuffer(L, b->subs[i])) != NULL)
            return (buffer*)sb;
    }

    sb = (subbuffer*)lua_newuserdata(L, sizeof(subbuffer)); /* 1 */
    lb_setmetatable(L);

    if (b->subcount == LB_SUBS_MAX)
        lb_removesubbuffer(b->subs[0]);
    b->subs[b->subcount++] = sb;

    sb->str = str;
    sb->len = len;
    sb->parent = b;
    sb->subparent = subparent;
    sb->subtype = LB_SUB;

    register_subbuffer(L, sb);
    return (buffer*)sb;
}

void lb_removesubbuffer(subbuffer *b)
{
    buffer *pb = b->parent;

    if (pb != NULL && lb_issubbuffer(b)) {
        size_t i, j;
        lb_initsubbuffer(b);
        for (i = j = 0; i < (size_t)pb->subcount; ++i) {
            subbuffer *sb = pb->subs[i];
            if (sb != NULL && !lb_isinvalidsub(sb))
                pb->subs[j++] = sb;
        }
        pb->subcount = j;
    }
}

static char *realloc_subbuffer(lua_State *L, subbuffer *sb, size_t len)
{
    if (sb != NULL && !lb_isinvalidsub(sb)) {
        buffer *pb = sb->parent;
        size_t begin = sb->str - pb->str;
        size_t pb_oldlen = pb->len;
        int dlen = len - sb->len;

        if (dlen == 0)
            return sb->str;

        else if (dlen > 0) {
            if (lb_realloc(L, pb, pb->len + dlen)) {
#define MAINTAIN_SUBBUFFER() do { \
        size_t sb_oldend = begin + sb->len; \
        size_t sb_newend = begin + len; \
        subbuffer *sp; \
        for (sp = sb->subparent; sp != NULL; sp = sp->subparent)\
            sp->len += dlen; \
        sb->len = len; \
        memmove(&pb->str[sb_newend], &pb->str[sb_oldend], \
                pb_oldlen - sb_oldend); } while (0)

                MAINTAIN_SUBBUFFER();
                sb->str = &pb->str[begin];
                return sb->str;
            }
        }

        /* we need to move data *before* the buffer is shorten */
        else { /* assume shorten length never fail */
            MAINTAIN_SUBBUFFER();
            if (lb_realloc(L, pb, pb->len + dlen)) {
                sb->str = &pb->str[begin];
                return sb->str;
            }
        }
    }
    return NULL;
}

static void redir_subbuffers(buffer *b, char *newstr, size_t len)
{
    size_t i, j;

    if (len == 0) {
        for (i = 0; i < (size_t)b->subcount; ++i) {
            if (b->subs[i] != NULL) {
                lb_initsubbuffer(b->subs[i]);
                b->subs[i] = NULL;
            }
        }
        b->subcount = 0;
    } else if (len >= b->len && newstr != b->str) {
        for (i = 0; i < (size_t)b->subcount; ++i) {
            subbuffer *sb = b->subs[i];
            if (sb != NULL) {
                size_t begin = sb->str - b->str;
                sb->str = &newstr[begin];
            }
        }
    } else if (len < b->len) {
        for (i = j = 0; i < (size_t)b->subcount; ++i) {
            subbuffer *sb = b->subs[i];
            if (sb != NULL) {
                size_t begin = sb->str - b->str;
                size_t end = begin + sb->len;
                if (begin > len)
                    lb_initsubbuffer(sb);
                else {
                    if (end > len && sb->len != 0)
                        sb->len = len - begin;
                    sb->str = &newstr[begin];
                    b->subs[j++] = b->subs[i];
                }
            }
        }
        b->subcount = j;
    }
}
#endif

char *lb_realloc(lua_State *L, buffer *b, size_t len)
{
#ifdef LB_SUBBUFFER
    if (b->subcount < 0)
        return realloc_subbuffer(L, (subbuffer*)b, len);
#endif

    if (len != b->len) {
        char *newstr = NULL;
        void *ud;
        lua_Alloc f;

        f = lua_getallocf(L, &ud);
#define REAL_LEN(len) ((len) == 0 ? 0 : (len) + 1)
        newstr = (char*)f(ud, b->str, REAL_LEN(b->len), REAL_LEN(len));
        if (len == 0 || newstr != NULL) {
            if (newstr != NULL)
                newstr[len] = '\0';
#ifdef LB_SUBBUFFER
            redir_subbuffers(b, newstr, len);
#endif
            b->str = newstr;
            b->len = len;
#undef REAL_LEN
        }
        return newstr;
    }
    return b->str;
}

buffer *lb_initbuffer(buffer *b)
{
    b->str = NULL;
    b->len = 0;
#ifdef LB_SUBBUFFER
    {
        size_t i;
        b->subcount = 0;
        for (i = 0; i < LB_SUBS_MAX; ++i)
            b->subs[i] = NULL;
    }
#endif
    return b;
}

buffer *lb_newbuffer(lua_State *L)
{
    buffer *b = lb_initbuffer((buffer*)lua_newuserdata(L, sizeof(buffer))); /* 1 */
    lb_setmetatable(L);
    return b;
}

buffer *lb_copybuffer(lua_State *L, buffer *b)
{
    buffer *nb = lb_newbuffer(L);
    if (lb_realloc(L, nb, b->len))
        memcpy(nb->str, b->str, b->len);
    return nb;
}

buffer *lb_pushbuffer(lua_State *L, const char *str, size_t len)
{
    buffer *b = lb_newbuffer(L);
    if (lb_realloc(L, b, len))
        memcpy(b->str, str, len);
    return b;
}

const char *lb_setbuffer(lua_State *L, buffer *b, const char *str, size_t len)
{
    if (b != NULL && lb_realloc(L, b, len))
        memcpy(b->str, str, len);
    return b->str;
}

/* compatible with lua api */

int lb_isbufferorstring(lua_State *L, int narg)
{
    return lua_isstring(L, narg) || lb_testbuffer(L, narg) != NULL;
}

static const char *tolstring(lua_State *L, buffer *b, size_t *plen)
{
    if (plen != NULL)
        *plen = (b == NULL ? 0 : b->len);
    if (b == NULL) return NULL;
    /* never return NULL with a valid buffer,
     * even if b->str is NULL. */
    return b->str != NULL ? b->str : "";
}

const char *lb_tolstring(lua_State *L, int narg, size_t *plen)
{
    const char *str = lua_tolstring(L, narg, plen);
    if (str != NULL) return str;
    return tolstring(L, lb_testbuffer(L, narg), plen);
}

const char *lb_checklstring(lua_State *L, int narg, size_t *plen)
{
    if (lua_isstring(L, narg))
        return lua_tolstring(L, narg, plen);
    else {
        buffer *b = lb_testbuffer(L, narg);
        if (b != NULL)
            return tolstring(L, b, plen);
        typeerror(L, narg, "buffer or string");
        return NULL; /* avoid warning */
    }
}

const char *lb_optlstring(lua_State *L, int narg, const char *def, size_t *plen)
{
    if (lua_isnoneornil(L, narg)) {
        if (plen != NULL) *plen = def ? strlen(def) : 0;
        return def;
    }
    return lb_checklstring(L, narg, plen);
}

/*
 * cc: lua='lua52' flags+='-s -O2 -Wall -pedantic -mdll -Id:/$lua/include' libs+='d:/$lua/$lua.dll'
 * cc: flags+='-DLB_SUBBUFFER=1 -DLB_REDIR_STRLIB=1 -DLB_FILEHANDLE'
 * cc: flags+='-DLUA_BUILD_AS_DLL' input='lb*.c' output='buffer.dll'
 * cc: run='$lua test.lua'
 */
