#define LUA_LIB
#include "lbuffer.h"
#include "lualib.h" /* for LUA_FILEHANDLE */


#include <stdarg.h>
#include <ctype.h>
#include <string.h>


#ifdef LB_REPLACE_LUA_API
#  undef lua_isstring
#  undef lua_tolstring
#  undef luaL_checklstring
#  undef luaL_optlstring
#endif

#define uchar(ch) ((unsigned char)(ch))


/* buffer maintenance */

static char *grow_buffer(lua_State *L, buffer *b, size_t len) {
    if (len > b->len) return lb_realloc(L, b, len);
    return b->str;
}

static void fill_str(buffer *b, int pos, size_t fill_len, const char *str, size_t len) {
    if (str == NULL || len <= 1)
        memset(&b->str[pos], len == 1 ? str[0] : 0, fill_len);
    else if (fill_len != 0) {
        int i, count = fill_len / len;
        for (i = (str == &b->str[pos] ? 1 : 0); i < count; ++i) {
            memcpy(&b->str[pos + i * len], str, len);
        }
        memcpy(&b->str[pos + count * len], str, fill_len % len);
    }
}

static size_t real_offset(int offset, size_t len) {
    if (offset >= 1 && (size_t)offset <= len)
        return offset - 1;
    else if (offset <= -1 && (size_t) - offset <= len)
        return offset + len;
    return offset > 0 && len != 0 ? len : 0;
}

static size_t real_range(lua_State *L, int narg, size_t *plen) {
    if (lua_gettop(L) >= narg) {
        size_t i = real_offset((int)luaL_optinteger(L, narg, 1), *plen);
        int sj = (int)luaL_optinteger(L, narg + 1, -1);
        size_t j = real_offset(sj, *plen);
        *plen = i <= j ? j - i + (sj != 0 && j != *plen) : 0;
        return i;
    }
    return 0;
}

/* buffer information */

static int lbE_isbuffer(lua_State *L) {
    buffer *b;
    return (b = lb_rawtestbuffer(L, 1)) != NULL
           && !lb_isinvalidsub(b);
}

static int lbE_tostring(lua_State *L) {
    buffer *b;
    if ((b = lb_rawtestbuffer(L, 1)) == NULL) {
        size_t len;
        const char *str = lua_tolstring(L, 1, &len);
        lua_pushlstring(L, str, len);
    }
    else if (lb_isinvalidsub(b))
        lua_pushfstring(L, "(invalid subbuffer): %p", b);
    else
        lua_pushlstring(L, b->str, b->len);
    return 1;
}

static int lbE_tohex(lua_State *L) {
    size_t i, len, seplen, gseplen = 0;
    const char *str = lb_tolstring(L, 1, &len);
    const char *sep, *gsep = NULL;
    int upper, group = -1, col = 0;
    int has_group = lua_type(L, 2) == LUA_TNUMBER, arg = 2;
    luaL_Buffer b;
    if (has_group) group = lua_tointeger(L, arg++);
    if (group == 0) group = -1;
    sep = lb_optlstring(L, arg++, "", &seplen);
    if (has_group) gsep = lb_optlstring(L, arg++, "\n", &gseplen);
    upper = lua_toboolean(L, arg++);
    luaL_buffinit(L, &b);
    for (i = 0; i < len; ++i, ++col) {
        char *hexa = upper ? "0123456789ABCDEF" : "0123456789abcdef";
        if (col == group)
            col = 0, luaL_addlstring(&b, gsep, gseplen);
        else if (i != 0)
            luaL_addlstring(&b, sep, seplen);
        luaL_addchar(&b, hexa[uchar(str[i]) >> 4]);
        luaL_addchar(&b, hexa[uchar(str[i]) & 0xF]);
    }
    luaL_pushresult(&b);
    return 1;
}

static int lbE_quote(lua_State *L) {
    size_t i, len;
    const char *str = lb_tolstring(L, 1, &len);
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    luaL_addchar(&b, '"');
    for (i = 0; i < len; ++i) {
        if (str[i] != '"' && str[i] != '\\' && isprint(str[i]))
            luaL_addchar(&b, uchar(str[i]));
        else {
            char *numa = "0123456789";
            switch (uchar(str[i])) {
            case '\a': luaL_addstring(&b, "\\a"); break;
            case '\b': luaL_addstring(&b, "\\b"); break;
            case '\f': luaL_addstring(&b, "\\f"); break;
            case '\n': luaL_addstring(&b, "\\n"); break;
            case '\r': luaL_addstring(&b, "\\r"); break;
            case '\t': luaL_addstring(&b, "\\t"); break;
            case '\v': luaL_addstring(&b, "\\v"); break;
            case '\\': luaL_addstring(&b, "\\\\"); break;
            default:
                       luaL_addchar(&b, '\\');
                       luaL_addchar(&b, numa[uchar(str[i])/100%10]);
                       luaL_addchar(&b, numa[uchar(str[i])/10%10]);
                       luaL_addchar(&b, numa[uchar(str[i])%10]);
                       break;
            }
        }
    }
    luaL_addchar(&b, '"');
    luaL_pushresult(&b);
    return 1;
}

static int lbE_topointer(lua_State *L) {
    buffer *b = lb_checkbuffer(L, 1);
    size_t offset = real_offset((int)luaL_optinteger(L, 2, 1), b->len);
    lua_pushlightuserdata(L, offset == b->len ? NULL : &b->str[offset]);
    return 1;
}

static int lbE_cmp(lua_State *L) {
    size_t l1, l2;
    const char *s1 = lb_checklstring(L, 1, &l1);
    const char *s2 = lb_checklstring(L, 2, &l2);
    int res;
    if ((res = memcmp(s1, s2, l1 < l2 ? l1 : l2)) == 0)
        res = l1 - l2;
    lua_pushinteger(L, res > 0 ? 1 : res < 0 ? -1 : 0);
    return 1;
}

static int lbE_eq(lua_State *L) {
    /* We can do this slightly faster than lb_cmp() by comparing
     * string length first.  */
    size_t l1, l2;
    const char *s1 = lb_checklstring(L, 1, &l1);
    const char *s2 = lb_checklstring(L, 2, &l2);
    lua_pushboolean(L, l1 == l2 && memcmp(s1, s2, l1) == 0);
    return 1;
}

static int lb_map(lua_State *L, int (*f)(int)) {
    buffer *b = lb_checkbuffer(L, 1);
    size_t i, len = b->len, pos = real_range(L, 2, &len);
    for (i = 0; i < len; ++i)
        b->str[pos + i] = uchar(f(b->str[pos + i]));
    lua_settop(L, 1);
    return 1;
}
static int lbE_lower(lua_State *L) { return lb_map(L, tolower); }
static int lbE_upper(lua_State *L) { return lb_map(L, toupper); }

static int auxipairs(lua_State *L) {
    buffer *b = lb_checkbuffer(L, 1);
    int key = (int)luaL_checkinteger(L, 2) + 1;
    if (key <= 0 || (size_t)key > b->len) return 0;
    lua_pushinteger(L, key);
    lua_pushinteger(L, uchar(b->str[key - 1]));
    return 2;
}

static int lbE_ipairs(lua_State *L) {
    buffer *b = lb_checkbuffer(L, 1);
    int pos = real_offset((int)luaL_optinteger(L, 2, 0), b->len);
    lua_pushcfunction(L, auxipairs);
    lua_insert(L, 1);
    lua_pushinteger(L, pos);
    return 3;
}

static int lbE_len(lua_State *L) {
    if (
#if LUA_VERSION_NUM >= 502
            !lua_rawequal(L, 1, 2) &&
#endif
            !lua_isnoneornil(L, 2)) {
        buffer *b = lb_checkbuffer(L, 1);
        size_t oldlen = b->len;
        int newlen = (int)luaL_checkinteger(L, 2);
        if (newlen < 0) newlen += b->len;
        if (newlen < 0) newlen = 0;
        if (lb_realloc(L, b, newlen) && (size_t)newlen > oldlen)
            memset(&b->str[oldlen], 0, newlen - oldlen);
        lua_pushinteger(L, b->len);
    }
    else {
        size_t len;
        lb_checklstring(L, 1, &len);
        lua_pushinteger(L, len);
    }
    return 1;
}

static int lbE_alloc(lua_State *L) {
    buffer *b = lb_checkbuffer(L, 1);
    if (lb_realloc(L, b, (int)luaL_checkinteger(L, 2))) {
        size_t len = 0;
        const char *str = lb_optlstring(L, 3, NULL, &len);
        size_t pos = real_range(L, 4, &len);
        fill_str(b, 0, b->len, &str[pos], len);
    }
    lua_settop(L, 1);
    lua_pushinteger(L, b->len);
    return 2;
}

static int lbE_free(lua_State *L) {
    buffer *b = lb_checkbuffer(L, 1);
    lb_realloc(L, b, 0);
    return 0;
}

static int lbE_byte(lua_State *L) {
    buffer *b = lb_checkbuffer(L, 1);
    size_t i;
    size_t len = b->len, pos = real_range(L, 2, &len);
    if (lua_isnoneornil(L, 3)) len = 1;
    luaL_checkstack(L, len, "string slice too long");
    for (i = 0; i < len; ++i)
        lua_pushinteger(L, uchar(b->str[pos + i]));
    return len;
}

static int lbE_char(lua_State *L) {
    buffer *b = NULL;
    int invalid = 0;
    int i, n = lua_gettop(L);
    if ((b = lb_testbuffer(L, 1)) == NULL) {
        b = lb_newbuffer(L);
        lua_insert(L, 1);
        n += 1;
    }
    if (lb_realloc(L, b, n - 1)) {
        for (i = 2; i <= n; ++i) {
            int c = (int)luaL_checkinteger(L, i);
            b->str[i - 2] = uchar(c);
            if (c != uchar(c) && !invalid)
                invalid = i - 1;
        }
    }
    lua_settop(L, 1);
    if (invalid) {
        lua_pushinteger(L, invalid);
        return 2;
    }
    return 1;
}

/* buffer operations */

enum cmd {
    cmd_append,
    cmd_assign,
    cmd_insert,
    cmd_set,
    cmd_last
};

static char *prepare_cmd(lua_State *L, buffer *b, enum cmd c, int pos, int len) {
    size_t oldlen = b->len;
    char *newstr = NULL;
    if (c == cmd_assign)
        return lb_realloc(L, b, pos + len);
    else if (c != cmd_insert)
        return grow_buffer(L, b, pos + len);
    else if ((newstr = lb_realloc(L, b, b->len + len)) != NULL)
        memmove(&b->str[pos + len], &b->str[pos], oldlen - pos);
    return newstr;
}

#ifdef LB_FILEHANDLE
static void* testudata(lua_State *L, int narg, const char *tname) {
    void *p = lua_touserdata(L, narg);
    if (p != NULL) {  /* value is a userdata? */
        if (lua_getmetatable(L, narg)) {  /* does it have a metatable? */
            lua_getfield(L, LUA_REGISTRYINDEX, tname);  /* get correct metatable */
            if (!lua_rawequal(L, -1, -2))  /* not the same? */
                p = NULL;  /* value is a userdata with wrong metatable */
            lua_pop(L, 2);  /* remove both metatables */
            return p;
        }
    }
    return NULL;  /* value is not a userdata with a metatable */
}

static const char *readfile(lua_State *L, int narg, size_t *plen) {
    /* narg must absolute index */
    if (testudata(L, narg, LUA_FILEHANDLE) != NULL) {
        int top = lua_gettop(L);
        lua_getfield(L, narg, "read");
        lua_insert(L, narg);
        if (top == narg) {
            lua_pushliteral(L, "*a");
            top += 1;
        }
        lua_call(L, top - narg + 1, 1);
        return lua_tolstring(L, narg, plen);
    }
    return NULL;
}
#endif /* LB_FILEHANDLE */

static const char *udtolstring(lua_State *L, int narg, size_t *plen) {
    void *u = NULL;
#ifdef LB_FILEHANDLE
    if ((u = (void*)readfile(L, narg, plen)) != NULL)
        return (const char*)u;
#endif /* LB_FILEHANDLE */
    if ((u = lua_touserdata(L, narg)) != NULL) {
        int len;
        if (!lua_isnumber(L, narg + 1)) {
            luaL_typeerror(L, narg + 1, "number");
            return NULL; /* avoid warning */
        }
        len = (int)lua_tointeger(L, narg + 1);
        if (plen != NULL) *plen = len >= 0 ? len : 0;
#ifdef LB_COMPAT_TOLUA
        if (!lua_islightuserdata(L, narg)) /* compatble with tolua */
            u = *(void**)u;
#endif /* LB_COMPAT_TOLUA */
    }
    if (u == NULL && plen != NULL) *plen = 0;
    return (const char*)u;
}

static int do_cmd(lua_State *L, buffer *b, int narg, enum cmd c) {
    int pos;
    int orig_narg = narg;
    switch (c) {
    case cmd_append:
        pos = b->len; break;
    case cmd_insert: case cmd_set:
        pos = real_offset((int)luaL_checkinteger(L, narg++), b->len); break;
    default:
        pos = 0; break;
    }

    if (lua_type(L, narg) == LUA_TNUMBER) {
        size_t fill_len = lua_tointeger(L, narg);
        size_t len = 0;
        const char *str = NULL;
        if (!lua_isnoneornil(L, narg + 1)) {
            if ((str = lb_tolstring(L, narg + 1, &len)) != NULL)
                str += real_range(L, narg + 2, &len);
            else if ((str = udtolstring(L, narg + 1, &len)) == NULL)
                luaL_typeerror(L, narg + 1, "string/buffer/userdata");
        }
        if (prepare_cmd(L, b, c, pos, fill_len))
            fill_str(b, pos, fill_len, str, len);
    }
    else if (lb_isbufferorstring(L, narg)) {
        size_t len;
        const char *str = lb_tolstring(L, narg, &len);
        size_t i = real_range(L, narg + 1, &len);
        if (prepare_cmd(L, b, c, pos, len))
            memcpy(&b->str[pos], &str[i], len);
    }
    else if (lua_isuserdata(L, narg)) {
        size_t len = 0;
        const char *str = udtolstring(L, narg, &len);
        if (prepare_cmd(L, b, c, pos, len))
            memcpy(&b->str[pos], str, len);
    }
    else if (!lua_isnoneornil(L, narg))
        luaL_typeerror(L, narg, "string/buffer/number/pointer");
    lua_settop(L, orig_narg - 1);
    return 1;
}

static int lbE_new(lua_State *L) {
    buffer b;
    lb_initbuffer(&b);
    do_cmd(L, &b, 1, cmd_assign);
    memcpy(lb_newbuffer(L), &b, sizeof(buffer));
    return 1;
}

#define DEFINE_DOCMD_FUNC(name) \
    static int lbE_##name(lua_State *L) { \
        return do_cmd(L, lb_checkbuffer(L, 1), 2, cmd_##name); \
    }
DEFINE_DOCMD_FUNC(append)
DEFINE_DOCMD_FUNC(assign)
DEFINE_DOCMD_FUNC(insert)
DEFINE_DOCMD_FUNC(set)
#undef DEFINE_DOCMD_FUNC

#ifdef LB_SUBBUFFER
static int lbE_sub(lua_State *L) {
    buffer *b = lb_checkbuffer(L, 1);
    size_t len = b->len, pos = real_range(L, 2, &len);
    lb_newsubbuffer(L, b, pos, pos+len);
    return 1;
}

static int lbE_subcount(lua_State *L) {
    buffer *b = lb_rawtestbuffer(L, 1);
    if (b == NULL)
        return luaL_typeerror(L, 1, lb_libname);
    if (lb_isinvalidsub(b))
        lua_pushliteral(L, "invalid");
    else if (b->subcount == LB_SUB)
        lua_pushliteral(L, "sub");
    else if (b->subcount >= 0)
        lua_pushinteger(L, b->subcount);
    else
        lua_pushnil(L);
    return 1;
}

static int lbE_offset(lua_State *L) {
    buffer *b = lb_checkbuffer(L, 1);
    if (b->subcount >= 0)
        return 0;
    else {
        subbuffer *sb = (subbuffer*)b;
        buffer *parent = sb->parent;
        int pos = sb->str - parent->str + 1;
        lua_pushinteger(L, pos);
        lua_pushinteger(L, pos + sb->len - 1);
        return 2;
    }
}
#endif

static int lbE_rep(lua_State *L) {
    buffer *b = lb_checkbuffer(L, 1);
    size_t len = b->len;
    const char *str = NULL;
    int rep = 0;
    if (lua_type(L, 2) == LUA_TNUMBER)
        rep = lua_tointeger(L, 2);
    else if ((str = lb_tolstring(L, 2, &len)) != NULL)
        rep = (int)luaL_checkinteger(L, 3);
    else luaL_typeerror(L, 2, "number/buffer/string");
    if (lb_realloc(L, b, len * (rep >= 0 ? rep : 0)))
        fill_str(b, 0, b->len, str != NULL ? str : b->str, len);
    lua_settop(L, 1);
    return 1;
}

static int lbE_clear(lua_State *L) {
    buffer *b = lb_checkbuffer(L, 1);
    size_t len = b->len, pos = real_range(L, 2, &len);
    memset(&b->str[pos], 0, len);
    lua_settop(L, 1);
    return 1;
}

static int lbE_copy(lua_State *L) {
    buffer *b = lb_checkbuffer(L, 1);
    size_t len = b->len, pos = real_range(L, 2, &len);
    lb_pushbuffer(L, &b->str[pos], len);
    return 1;
}

static int lbE_move(lua_State *L) {
    buffer *b = lb_checkbuffer(L, 1);
    int dst = (int)luaL_checkinteger(L, 2);
    size_t len = b->len, pos = real_range(L, 3, &len);
    size_t oldlen = b->len;

    if (dst > 0) dst -= 1;
    if (dst < 0) dst += b->len;
    if (dst < 0) dst = 0;

    if (grow_buffer(L, b, dst + len)) {
        memmove(&b->str[dst], &b->str[pos], len);
        if ((size_t)dst > oldlen)
            memset(&b->str[oldlen], 0, dst - oldlen);
    }
    lua_settop(L, 1);
    return 1;
}

static int lbE_remove(lua_State *L) {
    buffer *b = lb_checkbuffer(L, 1);
    size_t len = b->len, pos = real_range(L, 2, &len);
    size_t end = pos + len;
    if (len != 0)
        memmove(&b->str[pos], &b->str[end], b->len - end);
    b->len -= len;
    lua_settop(L, 1);
    return 1;
}

static void my_strrev(char *p1, char *p2) {
    while (p1 < --p2) {
        char t = *p1;
        *p1++ = *p2;
        *p2 = t;
    }
}

static int lbE_reverse(lua_State *L) {
    buffer *b = lb_checkbuffer(L, 1);
    size_t len = b->len, pos = real_range(L, 2, &len);
    my_strrev(&b->str[pos], &b->str[pos + len]);
    lua_settop(L, 1);
    return 1;
}

static void exchange(char *p1, char *p2, char *p3) {
    my_strrev(p1, p2);
    my_strrev(p2, p3);
    my_strrev(p1, p3);
}

static void exchange_split(char *p1, char *p2, char *p3, char *p4) {
    my_strrev(p1,  p2);
    my_strrev(p2,  p3);
    my_strrev(p3,  p4);
    my_strrev(p1,  p4);
}

static int lbE_swap(lua_State *L) {
    size_t p1, l1, p2, l2;
    buffer *b = lb_checkbuffer(L, 1);
    if (lua_isnoneornil(L, 3)) {
        p2 = real_offset((int)luaL_checkinteger(L, 2), b->len);
        l2 = b->len - p2;
        p1 = 0, l1 = p2;
    }
    else {
        l1 = b->len, p1 = real_range(L, 2, &l1);
        l2 = b->len, p2 = real_range(L, 4, &l2);
        if (lua_isnoneornil(L, 5)) {
            size_t new_p2 = p1 + l1;
            l2 = p2 - new_p2 + 1;
            p2 = new_p2;
        }
        else if (p1 + l1 > p2) {
            size_t old_l1 = l1, old_p2 = p2;
            l1 = p2 - p1;
            p2 = p1 + old_l1;
            l2 -= p2 - old_p2;
        }
    }
    if (p1 + l1 == p2)
        exchange(&b->str[p1], &b->str[p2], &b->str[p2 + l2]);
    else
        exchange_split(&b->str[p1], &b->str[p1 + l1],
                       &b->str[p2], &b->str[p2 + l2]);
    lua_settop(L, 1);
    return 1;
}

/* pack/unpack bianry buffer */

#ifndef _MSC_VER
#  include <stdint.h>
#else
#  define uint32_t unsigned long
#  define uint64_t unsigned __int64
#  define int32_t signed long
#  define int64_t signed __int64
#endif
#if defined( __sparc__ ) || defined( __ppc__ )
#  define CPU_BIG_ENDIAN 1
#else
#  define CPU_BIG_ENDIAN 0
#endif

#define swap32(x) \
    (((x) >> 24) | (((x) >> 8) & 0xFF00) | (((x) & 0xFF00) << 8) | ((x) << 24))

typedef union numcast_t {
    uint32_t i32;
    uint64_t i64;
    float f;
    double d;
#ifndef LB_ARTHBIT
    uint32_t i32s[2];
    char c[8];
#endif /* LB_ARTHBIT */
} numcast_t;

typedef struct parse_info {
    lua_State *L;       /* lua state */
    buffer *b;          /* working buffer */
    size_t pos;         /* current working position in buffer */
    unsigned int flags; /* see PIF_* flags below */
    int narg, nret;     /* numbers of arguments/return values */
    int level, index;   /* the level/index of nest table */
    int fmtpos;         /* the pos of fmt */
    const char *fmt;    /* the format string pointer */
} parse_info;

#define I(field) (info->field)

#define pif_test(i, flag)      (((i)->flags & (flag)) != 0)
#define pif_clr(i, flag)       ((i)->flags &= ~(flag))
#define pif_set(i, flag)       ((i)->flags |= (flag))

/* flags */
#define PIF_PACK         0x01
#define PIF_BIGENDIAN    0x02
#define PIF_STRINGKEY    0x04


#ifndef LB_ARTHBIT
static void swap_binary(int bigendian, numcast_t *buf, size_t wide) {
    if (CPU_BIG_ENDIAN != !!bigendian) {
        buf->i32s[0] = swap32(buf->i32s[0]);
        if (wide > 4) {
            buf->i32s[1] = swap32(buf->i32s[1]);
            buf->i64 = (buf->i64 >> 32) | (buf->i64 << 32);
        }
    }
}
#else
static uint32_t read_int32(const char *str, int bigendian, int wide) {
    int n = 0;
    switch (wide) {
    default: return 0;
    case 4:          n |= *str++ & 0xFF;
    case 3: n <<= 8; n |= *str++ & 0xFF;
    case 2: n <<= 8; n |= *str++ & 0xFF;
    case 1: n <<= 8; n |= *str++ & 0xFF;
    }
    if (!bigendian)
        n = swap32(n) >> ((4 - wide) << 3);
    return n;
}

static void write_int32(char *str, int bigendian, uint32_t n, int wide) {
    if (bigendian)
        n = swap32(n) >> ((4 - wide) << 3);
    switch (wide) {
    default: return;
    case 4: *str++ = n & 0xFF; n >>= 8;
    case 3: *str++ = n & 0xFF; n >>= 8;
    case 2: *str++ = n & 0xFF; n >>= 8;
    case 1: *str++ = n & 0xFF;
    }
}
#endif /* LB_ARTHBIT */

static void read_binary(const char *str, int bigendian, numcast_t *buf, size_t wide) {
#ifndef LB_ARTHBIT
    buf->i64 = 0;
    memcpy(&buf->c[bigendian ? (8 - wide)&3 : 0], str, wide);
    swap_binary(bigendian, buf, wide);
#else
    if (wide <= 4) buf->i32 = read_int32(str, bigendian, wide);
    else {
        uint32_t lo, hi; /* in big bigendian */
        hi = read_int32(str, bigendian, 4);
        lo = read_int32(&str[4], bigendian, wide - 4);
        buf->i64 = bigendian ?
                   ((uint64_t)hi << ((wide - 4) << 3)) | lo :
                   ((uint64_t)lo << 32) | hi;
    }
#endif /* LB_ARTHBIT */
}

static void write_binary(char *str, int bigendian, numcast_t *buf, size_t wide) {
#ifndef LB_ARTHBIT
    swap_binary(bigendian, buf, wide);
    memcpy(str, &buf->c[bigendian ? (8 - wide)&3 : 0], wide);
#else
    if (wide <= 4) write_int32(str, bigendian, buf->i32, wide);
    else if (bigendian) {
        write_int32(str, bigendian, (uint32_t)(buf->i64 >> 32), wide - 4);
        write_int32(&str[wide - 4], bigendian, (uint32_t)buf->i64, 4);
    }
    else {
        write_int32(str, bigendian, (uint32_t)buf->i64, 4);
        write_int32(&str[4], bigendian, (uint32_t)(buf->i64 >> 32), wide - 4);
    }
#endif /* LB_ARTHBIT */
}

static void expand_sign(numcast_t *buf, size_t wide) {
    int shift = wide<<3;
    if (wide <= 4) {
        if (wide != 4 && ((uint32_t)1 << (shift - 1) & buf->i32) != 0)
            buf->i32 |= ~(uint32_t)0 << shift;
    }
    else {
        if (wide != 8 && ((uint64_t)1 << (shift - 1) & buf->i64) != 0)
            buf->i64 |= ~(uint64_t)0 << shift;
    }
}

static int source(parse_info *info) {
    if (I(level) == 0)
        lua_pushvalue(I(L), I(narg)++);
    else {
        if (!pif_test(info, PIF_STRINGKEY))
            lua_pushinteger(I(L), I(index)++);
        lua_gettable(I(L), -2);
    }
    return -1;
}

static const char *source_lstring(parse_info *info, size_t *plen) {
    int narg = source(info);
    if (lb_isbufferorstring(I(L), narg))
        return lb_tolstring(I(L), narg, plen);
    else if (I(level) == 0)
        luaL_typeerror(I(L), I(narg) - 1, "string");
    else {
        lua_pushfstring(I(L),
                "buffer/string expected in [%d], got %s",
                I(index) - 1, luaL_typename(I(L), narg));
        luaL_argerror(I(L), I(narg) - 1, lua_tostring(I(L), -1));
    }
    return NULL;
}

static lua_Number source_number(parse_info *info) {
    int narg = source(info);
    if (lua_isnumber(I(L), narg))
        return lua_tonumber(I(L), narg);
    else if (I(level) == 0)
        luaL_typeerror(I(L), I(narg) - 1, "number");
    else {
        lua_pushfstring(I(L),
                "number expected in [%d], got %s",
                I(index) - 1, luaL_typename(I(L), narg));
        luaL_argerror(I(L), I(narg) - 1, lua_tostring(I(L), -1));
    }
    return 0;
}

static void sink(parse_info *info) {
    if (I(level) == 0)
        ++I(nret);
    else {
        if (!pif_test(info, PIF_STRINGKEY)) {
            lua_pushinteger(I(L), I(index)++);
            lua_insert(I(L), -2);
        }
        lua_settable(I(L), -3);
    }
}

#define pack_checkstack(n) luaL_checkstack(I(L), (n), "too much top level formats")

static int fmterror(parse_info *info, const char *msgfmt, ...) {
    const char *msg;
    va_list list;
    va_start(list, msgfmt);
    msg = lua_pushvfstring(I(L), msgfmt, list);
    va_end(list);
    return luaL_argerror(I(L), I(fmtpos), msg);
}

static const char *lb_pushlstring(lua_State *L, const char *str, size_t len) {
    return lb_pushbuffer(L, str, len)->str;
}

#if LUA_VERSION_NUM < 502
static const char *my_lua_pushlstring(lua_State *L, const char *str, size_t len) {
    lua_pushlstring(L, str, len);
    return str;
}
#else
#  define my_lua_pushlstring lua_pushlstring
#endif

static int do_packfmt(parse_info *info, char fmt, size_t wide, int count) {
    numcast_t buf;
    size_t pos;
    int top = lua_gettop(I(L));
    typedef const char *(*pushlstring_t)(lua_State * L, const char * str, size_t len);
    pushlstring_t pushlstring = isupper(fmt) ?  lb_pushlstring : my_lua_pushlstring;

#define SINK() do { if (I(level) == 0 && count < 0) pack_checkstack(1); \
        sink(info); } while (0)
#define BEGIN_PACK(n) \
    if (pif_test(info, PIF_PACK)) { \
        if (count < 0 || n <= 0 || lb_realloc(I(L), I(b), (n))) { \
            while ((count >= 0 || I(narg) <= top) && count--)
#define BEGIN_UNPACK() \
        } \
    } else { \
        size_t blen = I(b)->len; \
        if (count > 0) pack_checkstack(count); \
        while (count--)
#define END_PACK() \
    } break

    switch (fmt) {
    case 's': case 'S': /* zero-terminated string */
    case 'z': case 'Z': /* zero-terminated string */
        BEGIN_PACK(0) {
            size_t len;
            const char *str = source_lstring(info, &len);
            if (wide != 0 && len > wide) len = wide;
            if (lb_realloc(I(L), I(b), I(pos) + len + 1)) {
                memcpy(&I(b)->str[I(pos)], str, len);
                I(b)->str[I(pos) + len] = '\0';
                I(pos) += len + 1;
            }
            lua_pop(I(L), 1); /* pop source */
        }
        BEGIN_UNPACK() {
            size_t len = 0;
            while ((wide == 0 || len < wide)
                    && I(pos) + len < blen
                    && I(b)->str[I(pos) + len] != '\0')
                ++len;
            if ((fmt == 'z' || fmt == 'Z')
                    && (I(pos) + len) >= blen
                    && (wide == 0 || len < wide))
                return 0;
            pushlstring(I(L), &I(b)->str[I(pos)], len); SINK();
            I(pos) += len;
            if (I(pos) < blen && I(b)->str[I(pos)] == '\0') ++I(pos);
        }
        END_PACK();

    case 'b': case 'B': /* byte */
    case 'c': case 'C': /* char */
        if (wide == 0) wide = 1;
        BEGIN_PACK(I(pos) + wide * count) {
            size_t len;
            const char *str = source_lstring(info, &len);
            if (wide != 0 && len > wide) len = wide;
            if (lb_realloc(I(L), I(b), I(pos) + wide)) {
                memcpy(&I(b)->str[I(pos)], str, len);
                if (len < wide)
                    memset(&I(b)->str[I(pos) + len], 0, wide - len);
                I(pos) += wide;
            }
            lua_pop(I(L), 1); /* pop source */
        }
        BEGIN_UNPACK() {
            if ((fmt == 'b' || fmt == 'B')
                    && (I(pos) + wide >= blen))
                return 0;
            if (I(pos) + wide > blen) wide = blen - I(pos);
            pushlstring(I(L), &I(b)->str[I(pos)], wide); SINK();
            I(pos) += wide;
        }
        END_PACK();

    case 'd': case 'D': /* length preceded data */
    case 'p': case 'P': /* length preceded string */
        if (wide == 0) wide = 4;
        if (wide > 8) fmterror(
                info,
                "invalid wide of format '%c': only 1 to 8 supported.", fmt);
        BEGIN_PACK(0) {
            size_t len;
            const char *str = source_lstring(info, &len);
            if (lb_realloc(I(L), I(b), I(pos) + wide + len)) {
                if (wide <= 4) buf.i32 = len;
                else buf.i64 = len;
                write_binary(&I(b)->str[I(pos)],
                             pif_test(info, PIF_BIGENDIAN), &buf, wide);
                memcpy(&I(b)->str[I(pos) + wide], str, len);
                I(pos) += wide + len;
            }
            lua_pop(I(L), 1); /* pop source */
        }
        BEGIN_UNPACK() {
            size_t len;
            if (I(pos) + wide > blen) return 0;
            read_binary(&I(b)->str[I(pos)],
                        pif_test(info, PIF_BIGENDIAN), &buf, wide);
            if (wide <= 4)
                len = buf.i32;
            else if ((len = (size_t)buf.i64) != buf.i64)
                fmterror(info, "string too big in format '%c'", fmt);
            if ((fmt == 'd' || fmt == 'D') && I(pos) + wide + len > blen)
                return 0;
            I(pos) += wide;
            if (I(pos) + len > blen)
                len = blen - I(pos);
            pushlstring(I(L), &I(b)->str[I(pos)], len); SINK();
            I(pos) += len;
        }
        END_PACK();

    case 'i': case 'I': /* int */
    case 'u': case 'U': /* unsigend int */
        if (wide == 0) wide = 4;
        if (wide > 8) fmterror(
                info,
                "invalid wide of format '%c': only 1 to 8 supported.", fmt);
        BEGIN_PACK(I(pos) + wide * count) {
            if (wide <= 4)
                buf.i32 = (int32_t)source_number(info);
            else
                buf.i64 = (int64_t)source_number(info);
            if (count >= 0
                    || lb_realloc(I(L), I(b), I(pos) + wide)) {
                write_binary(&I(b)->str[I(pos)],
                             pif_test(info, PIF_BIGENDIAN), &buf, wide);
                I(pos) += wide;
            }
            lua_pop(I(L), 1); /* pop source */
        }
        BEGIN_UNPACK() {
            if (I(pos) + wide > blen) return 0;
            read_binary(&I(b)->str[I(pos)],
                        pif_test(info, PIF_BIGENDIAN), &buf, wide);
            I(pos) += wide;
            if (fmt == 'u' || fmt == 'U')
                lua_pushnumber(I(L), wide <= 4 ? buf.i32 :
                               (lua_Number)buf.i64);
            else {
                expand_sign(&buf, wide);
                lua_pushnumber(I(L), wide <= 4 ? (int32_t)buf.i32 :
                               (lua_Number)(int64_t)buf.i64);
            }
            SINK();
        }
        END_PACK();

    case 'f': case 'F': /* float */
        if (wide == 0) wide = 4;
        if (wide != 4 && wide != 8) fmterror(
                info,
                "invalid wide of format '%c': only 4 or 8 supported.", fmt);
        BEGIN_PACK(I(pos) + wide * count) {
            buf.d = source_number(info);
            if (wide == 4) buf.f = (float)buf.d;
            if (count >= 0 || lb_realloc(I(L), I(b), I(pos) + wide)) {
                write_binary(&I(b)->str[I(pos)],
                             pif_test(info, PIF_BIGENDIAN), &buf, wide);
                I(pos) += wide;
            }
            lua_pop(I(L), 1); /* pop source */
        }
        BEGIN_UNPACK() {
            if (I(pos) + wide > blen) return 0;
            read_binary(&I(b)->str[I(pos)],
                        pif_test(info, PIF_BIGENDIAN), &buf, wide);
            I(pos) += wide;
            lua_pushnumber(I(L), wide == 4 ? buf.f : buf.d); SINK();
        }
        END_PACK();

    case '@': /* seek for absolute address */
        pos = wide * count - 1; goto check_seek;
    case '+': /* seek for positive address */
        pos = I(pos) + wide * count; goto check_seek;
    case '-': /* seek for negitive address */
        pos = wide * count;
        if (I(pos) > pos)
            pos = I(pos) - pos;
        else
            pos = 0;
check_seek:
        if (count < 0)
            fmterror(info, "invalid count of format '%c'", fmt);
        if (pos > I(b)->len) pos = I(b)->len;
        I(pos) = pos;
        break;

    default:
        fmterror(info, "invalid format '%c'", fmt);
        break;
    }
    return 1;
#undef SINK
#undef END_PACK
#undef BEGIN_UNPACK
#undef BEGIN_PACK
}

static int do_delimiter(parse_info *info, char fmt) {
    switch (fmt) {
    case '{':
        /* when meet a open-block, 3 value will be pushed onto stack:
         * the current index, the string key (or nil), and a new table
         * of block.  so the extra used stack space equals level * 3.
         * stack: [args] [[index][stringkey][table]] ... [stringkey]
         * NOTE: if you changed stack structure, you *MUST* change the
         * pop stack behavior in parse_fmt !!  */
        luaL_checkstack(I(L), 4, "table level too big");
        if (!pif_test(info, PIF_PACK)) {
            if (!pif_test(info, PIF_STRINGKEY))
                lua_pushnil(I(L));
            lua_pushinteger(I(L), I(index));
            lua_insert(I(L), -2);
            lua_newtable(I(L));
        }
        else {
            source(info);
            lua_pushinteger(I(L), I(index));
            lua_insert(I(L), -2);
            lua_pushnil(I(L));
            lua_insert(I(L), -2);
        }
        I(level) += 1;
        I(index) = 1;
        break;

    case '}':
        if (I(level) <= 0)
            fmterror(info, "unbalanced '}' in format near "LUA_QS, I(fmt) - 1);
        I(index) = lua_tointeger(I(L), -3);
        I(level) -= 1;
        lua_remove(I(L), -3);
        if (pif_test(info, PIF_PACK)) {
            lua_pop(I(L), 2);
        }
        else {
            if (!lua_isnil(I(L), -2))
                pif_set(info, PIF_STRINGKEY);
            else {
                lua_remove(I(L), -2);
                pif_clr(info, PIF_STRINGKEY);
            }
            pack_checkstack(1);
            sink(info);
        }
        break;

    case '#': /* current pos */
        if (I(level) != 0)
            fmterror(info, "can only retrieve position out of block");
        lua_pushinteger(I(L), I(pos) + 1);
        pack_checkstack(1);
        sink(info);
        break;

    case '<': /* little bigendian */
        pif_clr(info, PIF_BIGENDIAN); break;
    case '>': /* big bigendian */
        pif_set(info, PIF_BIGENDIAN); break;
    case '=': /* native bigendian */
#if CPU_BIG_ENDIAN
        pif_set(info, PIF_BIGENDIAN);
#else
        pif_clr(info, PIF_BIGENDIAN);
#endif
        break;

    default:
        return 0;
    }
    return 1;
}

#define skip_white(s) do { while (*(s) == ' ' || *(s) == '\t' \
                || *(s) == '\r'|| *(s) == '\n' || *(s) == ',') ++(s); } while(0)

static int parse_optint(const char **str, unsigned int *pn) {
    unsigned int n = 0;
    const char *oldstr = *str;
    while (isdigit(**str)) n = n * 10 + uchar(*(*str)++ - '0');
    if (*str != oldstr) *pn = n;
    return n;
}

static void parse_fmtargs(parse_info *info, size_t *wide, int *count) {
    skip_white(I(fmt));
    parse_optint(&I(fmt), wide);
    skip_white(I(fmt));
    if (*I(fmt) == '*') {
        size_t ucount = *count;
        ++I(fmt);
        skip_white(I(fmt));
        parse_optint(&I(fmt), &ucount);
        *count = ucount;
    }
    else if (*I(fmt) == '$') {
        ++I(fmt);
        *count = -1;
    }
    skip_white(I(fmt));
}

static void parse_stringkey(parse_info *info) {
    skip_white(I(fmt));
    if (isalpha(*I(fmt)) || *I(fmt) == '_') {
        const char *curpos = I(fmt)++, *end;
        while (isalnum(*I(fmt)) || *I(fmt) == '_')
            ++I(fmt);
        end = I(fmt);
        skip_white(I(fmt));
        if (*I(fmt) != '=')
            I(fmt) = curpos;
        else {
            ++I(fmt);
            skip_white(I(fmt));
            if (*I(fmt) == '}' || *I(fmt) == '\0')
                fmterror(info, "key without format near "LUA_QS, curpos);
            if (I(level) == 0)
                fmterror(info, "key at top level near "LUA_QS, curpos);
            lua_pushlstring(I(L), curpos, end - curpos);
            pif_set(info, PIF_STRINGKEY);
            return;
        }
    }
    pif_clr(info, PIF_STRINGKEY);
}

static int parse_fmt(parse_info *info) {
    int fmt, insert_pos = 0;
    skip_white(I(fmt));
    if (*I(fmt) == '!') {
        insert_pos = 1; /* only enabled in unpack */
        ++I(fmt);
    }
    while (parse_stringkey(info), (fmt = *I(fmt)++) != '\0') {
        if (!do_delimiter(info, fmt)) {
            size_t wide = 0;
            int count = 1;
            parse_fmtargs(info, &wide, &count);
            if (!do_packfmt(info, fmt, wide, count)) {
                if (pif_test(info, PIF_STRINGKEY))
                    lua_pop(I(L), 1);
                lua_pop(I(L), I(level) * 3); /* 3 values per level */
                I(level) = 0;
                lua_pushnil(I(L)); ++I(nret);
                skip_white(I(fmt));
                /* skip any block */
                while (*I(fmt) == '{' || *I(fmt) == '}') {
                    ++I(fmt);
                    skip_white(I(fmt));
                }
                if ((fmt = *I(fmt)++) == '#')
                    do_delimiter(info, fmt);
                break;
            }
        }
    }
    if (I(level) != 0)
        fmterror(info, "unbalanced '{' in format");
    if (insert_pos) {
        lua_pushinteger(I(L), I(pos) + 1);
        lua_insert(I(L), -(++I(nret)));
    }
    return I(nret);
}

static int do_pack(lua_State *L, buffer *b, int narg, int pack) {
    parse_info info = {NULL};
    info.L = L;
    info.b = b;
    info.narg = narg;
    if (pack) pif_set(&info, PIF_PACK);
#if CPU_BIG_ENDIAN
    pif_set(&info, PIF_BIGENDIAN);
#endif
    if (lua_type(L, info.narg) == LUA_TNUMBER)
        info.pos = real_offset(lua_tointeger(L, info.narg++), info.b->len);
    info.fmtpos = info.narg++;
    info.fmt = lb_checklstring(L, info.fmtpos, NULL);
    parse_fmt(&info);
    if (pack) {
        lua_pushinteger(L, info.pos + 1);
        lua_insert(L, -(++info.nret));
    }
    return info.nret;
}

static int lbE_pack(lua_State *L) {
    int res;
    buffer *b;
    if ((b = lb_testbuffer(L, 1)) != NULL) {
        res = do_pack(L, b, 2, 1);
        lua_pushvalue(L, 1);
    }
    else {
        buffer local_b;
        lb_initbuffer(&local_b);
        res = do_pack(L, &local_b, 1, 1);
        memcpy(lb_newbuffer(L), &local_b, sizeof(buffer));
    }
    lua_insert(L, -res-1);
    return res+1;
}

static int lbE_unpack(lua_State *L) {
    if (lua_type(L, 1) == LUA_TSTRING) {
        buffer b; /* a fake buffer */
        lb_initbuffer(&b);
        /* in unpack, all functions never changed the content of
         * buffer, so use force cast is safe */
        b.str = (char*)lua_tolstring(L, 1, &b.len);
        return do_pack(L, &b, 2, 0);
    }
    return do_pack(L, lb_checkbuffer(L, 1), 2, 0);
}

static size_t check_giargs(lua_State *L, int narg, size_t len, size_t *wide, int *bigendian) {
    size_t pos = real_offset((int)luaL_optinteger(L, narg, 1), len);
    *wide = (int)luaL_optinteger(L, narg + 1, 4);
    if (*wide < 1 || *wide > 8)
        luaL_argerror(L, narg + 1, "only 1 to 8 wide support");
    switch (*luaL_optlstring(L, narg + 2, "native", NULL)) {
    case 'b': case 'B': case '>': *bigendian = 1; break;
    case 'l': case 'L': case '<': *bigendian = 0; break;
    case 'n': case 'N': case '=': *bigendian = CPU_BIG_ENDIAN; break;
    default: luaL_argerror(L, 4, "only \"big\" or \"little\" or \"native\" endian support");
    }
    return pos;
}

static int lbE_getint(lua_State *L) {
    numcast_t buf;
    size_t len;
    const char *str = lb_checklstring(L, 1, &len);
    int bigendian;
    size_t wide, pos = check_giargs(L, 2, len, &wide, &bigendian);
    if (pos + wide > len) return 0;
    read_binary(&str[pos], bigendian, &buf, wide);
    expand_sign(&buf, wide);
    lua_pushnumber(L, wide <= 4 ? (int32_t)buf.i32 :
                   (lua_Number)(int64_t)buf.i64);
    return 1;
}

static int lbE_getuint(lua_State *L) {
    numcast_t buf;
    size_t len;
    const char *str = lb_checklstring(L, 1, &len);
    int bigendian;
    size_t wide, pos = check_giargs(L, 2, len, &wide, &bigendian);
    if (pos + wide > len) return 0;
    read_binary(&str[pos], bigendian, &buf, wide);
    lua_pushnumber(L, wide <= 4 ? buf.i32 : (lua_Number)buf.i64);
    return 1;
}

static int lbE_setuint(lua_State *L) {
    numcast_t buf;
    buffer *b = lb_checkbuffer(L, 1);
    int bigendian;
    size_t wide, pos = check_giargs(L, 3, b->len, &wide, &bigendian);
    if (grow_buffer(L, b, pos + wide)) {
        /* we use int64_t with i64, because if we use uint64_t, the
         * high 32 bit of 64bit integer will be stripped, don't know
         * why it happened.  */
        if (wide <= 4) buf.i32 = (/*u*/int32_t)luaL_checknumber(L, 2);
        else buf.i64 = (/*u*/int64_t)luaL_checknumber(L, 2);
        write_binary(&b->str[pos], bigendian, &buf, wide);
    }
    lua_settop(L, 1);
    return 1;
}

#undef I

/* meta methods */

static int lbM_gc(lua_State *L) {
    buffer *b;
    if ((b = lb_rawtestbuffer(L, 1)) != NULL && !lb_isinvalidsub(b)) {
#ifdef LB_SUBBUFFER
        if (lb_issubbuffer(b)) {
            lb_removesubbuffer((subbuffer*)b);
            return 0;
        }
#endif
        lb_realloc(L, b, 0);
    }
    return 0;
}

static int lbM_concat(lua_State *L) {
    size_t l1, l2;
    const char *s1 = lb_checklstring(L, 1, &l1);
    const char *s2 = lb_checklstring(L, 2, &l2);
    buffer *b = lb_pushbuffer(L, s1, l1);
    lua_replace(L, 1);
    if (lb_realloc(L, b, l1 + l2))
        memcpy(&b->str[l1], s2, l2);
    lua_settop(L, 1);
    return 1;
}

static int check_offset(int offset, int len, int extra) {
    if (offset >= 0)
        offset -= 1;
    else
        offset += len;
    if (offset < 0 || offset >= len + extra)
        return -1;
    return offset;
}

static int lbM_index(lua_State *L) {
    buffer *b = lb_rawtestbuffer(L, 1);
    int pos;

    if (b == NULL) {
        luaL_typeerror(L, 1, lb_libname);
        return 0; /* avoid warning */
    }

    switch (lua_type(L, 2)) {
    case LUA_TSTRING:
        lua_rawget(L, lua_upvalueindex(1));
        break;
    case LUA_TNUMBER:
        if (!lb_isinvalidsub(b)
                && (pos = check_offset((int)luaL_checkinteger(L, 2), b->len, 0)) >= 0)
            lua_pushinteger(L, uchar(b->str[pos]));
        else
            lua_pushnil(L);
        break;
    default:
        lua_pushnil(L);
        break;
    }

    return 1;
}

static int lbM_newindex(lua_State *L) {
    buffer *b = lb_checkbuffer(L, 1);
    int pos;
    if ((pos = check_offset((int)luaL_checkinteger(L, 2), b->len, 1)) < 0)
        return 0;

    if (pos == b->len && !grow_buffer(L, b, b->len + 1))
        return 0;

    if (lua_isnumber(L, 3)) {
        int value = (int)lua_tointeger(L, 3);
        b->str[pos] = uchar(value);
    }
    else if (lb_isbufferorstring(L, 3)) {
        size_t len, oldlen = b->len;
        const char *str = lb_tolstring(L, 3, &len);
        if (len == 1)
            b->str[pos] = str[0];
        else if (grow_buffer(L, b, b->len + len - 1)) {
            memmove(&b->str[pos + len], &b->str[pos + 1], oldlen - pos - 1);
            memcpy(&b->str[pos], str, len);
        }
    }
    else if (pos == b->len-1 && lua_isnil(L, 3))
        lb_realloc(L, b, pos);
    else luaL_typeerror(L, 3, "string/buffer/number");
    return 0;
}

static int lbM_call(lua_State *L) {
    buffer b;
    lua_remove(L, 1);
    lb_initbuffer(&b);
    do_cmd(L, &b, 1, cmd_assign);
    memcpy(lb_newbuffer(L), &b, sizeof(buffer));
    return 1;
}

#ifdef LB_REDIR_STRLIB
static int redir_to_strlib(lua_State *L, const char *name) {
    buffer *b = lb_testbuffer(L, 1);
    int i, base = 1, top = lua_gettop(L);
    if (b != NULL) {
        lua_pushlstring(L, b->str != NULL ? b->str : "", b->len);
        lua_insert(L, 2);
        base += 1;
        top += 1;
    }
    for (i = base; i <= top; ++i) {
        buffer *b = lb_testbuffer(L, i);
        if (b != NULL) {
            lua_pushlstring(L, b->str != NULL ? b->str : "", b->len);
            lua_replace(L, i);
        }
    }
    lua_getglobal(L, "string");
    lua_getfield(L, -1, name);
    lua_remove(L, -2);
    if (lua_isnil(L, -1))
        return luaL_error(L, "can not find function "LUA_QS" in "LUA_QS,
                          name, "string");
    lua_insert(L, base);
    lua_call(L, top - base + 1, LUA_MULTRET);
    if (lua_isstring(L, 2) && b != NULL) {
        size_t len;
        const char *str = lua_tolstring(L, 2, &len);
        lb_setbuffer(L, b, str, len);
        lua_remove(L, 2);
    }
    return lua_gettop(L);
}

#define redir_functions(X) \
    X(dump)   X(find)   X(format) X(gmatch) X(gsub)   X(match)

#define X(name) \
    static int lbR_##name (lua_State *L) \
    { return redir_to_strlib(L, #name); }
redir_functions(X)
#undef X
#endif /* LB_REDIR_STRLIB */

/* module registration */

LB_API const char lb_libname[] = "buffer";

static const luaL_Reg funcs[] = {
#ifdef LB_REDIR_STRLIB
#define ENTRY(name) { #name, lbR_##name },
    redir_functions(ENTRY)
#undef  ENTRY
#endif /* LB_REDIR_STRLIB */
#define ENTRY(name) { #name, lbE_##name },
    ENTRY(byte)    ENTRY(append) ENTRY(getuint)  ENTRY(remove)
    ENTRY(char)    ENTRY(assign) ENTRY(insert)   ENTRY(set)
    ENTRY(len)     ENTRY(clear)  ENTRY(ipairs)   ENTRY(setuint)
    ENTRY(lower)   ENTRY(cmp)    ENTRY(isbuffer) ENTRY(swap)
    ENTRY(rep)     ENTRY(copy)   ENTRY(move)     ENTRY(tohex)
    ENTRY(reverse) ENTRY(eq)     ENTRY(new)      ENTRY(topointer)
    ENTRY(upper)   ENTRY(free)   ENTRY(pack)     ENTRY(tostring)
    ENTRY(alloc)   ENTRY(getint) ENTRY(quote)    ENTRY(unpack)
#define lbE_setint lbE_setuint
    ENTRY(setint)
#undef  lbE_setint
#ifdef LB_SUBBUFFER
    ENTRY(sub)
    ENTRY(offset)
    ENTRY(subcount)
#endif /* LB_SUBBUFFER */
#undef ENTRY
    { NULL, NULL }
};

static const luaL_Reg mt[] = {
    { "__concat",  lbM_concat    },
    { "__len",     lbE_len       },
    { "__tostring",lbE_tostring  },
    { "__index",   lbM_index     },
    { "__newindex",lbM_newindex  },
    { "__gc",      lbM_gc        },
#if LUA_VERSION_NUM >= 502
    { "__ipairs",  lbE_ipairs    },
    { "__pairs",   lbE_ipairs    },
#endif
    { NULL,        NULL          },
};

int luaopen_buffer(lua_State *L) {
    luaL_newlib(L, funcs); /* 1 */
#if LUA_VERSION_NUM < 502
    lua_pushvalue(L, -1); /* 2 */
    lua_setglobal(L, lb_libname); /* (2) */
#endif
    lua_createtable(L, 0, 1); /* 2 */
    lua_pushcfunction(L, lbM_call); /* 3 */
    lua_setfield(L, -2, "__call"); /* 3->2 */
    lua_setmetatable(L, -2); /* 2->1 */

    lua_pushliteral(L, LB_VERSION); /* 2 */
    lua_setfield(L, -2, "_VERSION"); /* 2->1 */
#ifdef LB_SUBBUFFER
    lua_pushinteger(L, LB_SUBS_MAX); /* 2 */
    lua_setfield(L, -2, "_SUBS_MAX"); /* 2->1 */
#endif

    /* create metatable */
    lua_rawgetp(L, LUA_REGISTRYINDEX, lb_libname);
    if (lua_isnil(L, -1)) { /* 2 */
        lua_pop(L, 1); /* pop 2 */
        luaL_newlibtable(L, mt); /* 2 */
        lua_pushvalue(L, -2); /* 1->3 */
        luaL_setfuncs(L, mt, 1); /* 3->2 */
        lua_pushvalue(L, -1); /* 3 */
        lua_rawsetp(L, LUA_REGISTRYINDEX, (void*)lb_libname); /* 3->env */
        lua_setfield(L, LUA_REGISTRYINDEX, lb_libname); /* 2->env */
    }

    return 1;
}

/*
 * cc: lua='lua52' flags+='-s -O2 -Wall -pedantic -mdll -Id:/$lua/include' libs+='$lua.dll'
 * cc: flags+='-DLB_SUBBUFFER=1 -DLB_REDIR_STRLIB=1 -DLB_FILEHANDLE'
 * cc: flags+='-DLUA_BUILD_AS_DLL' input='lb*.c' output='buffer.dll'
 * cc: run='$lua test.lua'
 */
