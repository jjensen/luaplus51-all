#include "memoryfile.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define MEMORYFILE_MT_NAME ("3a4d95f2-66e7-11dc-ad52-00e081225ce5")

#define MEMORYFILE_MIN_BUF_SIZE 1024

typedef struct MemoryFile_ {
    char *buf;
    size_t buf_size, buf_max_size, buf_pos;
    int append;     /* if true, all output goes at end of file */
} MemoryFile;

static void
ensure_buf_size (MemoryFile *f, lua_State *L, size_t min_size) {
    size_t new_size;
    lua_Alloc alloc;
    void *alloc_ud;

    /* Leave space for putting a NUL byte at the end for read_number() */
    assert(min_size > 0);
    ++min_size;

    if (f->buf_max_size >= min_size)
        return;

    new_size = f->buf_max_size * 2;
    if (new_size < min_size)
        new_size = min_size;
    if (new_size < MEMORYFILE_MIN_BUF_SIZE)
        new_size = MEMORYFILE_MIN_BUF_SIZE;

    alloc = lua_getallocf(L, &alloc_ud);
    f->buf = alloc(alloc_ud, f->buf, f->buf_max_size, new_size);
    assert(f->buf);
}

static void
delete_memoryfile_buffer (MemoryFile *f, lua_State *L) {
    lua_Alloc alloc;
    void *alloc_ud;

    if (f->buf) {
        alloc = lua_getallocf(L, &alloc_ud);
        f->buf = alloc(alloc_ud, f->buf, f->buf_max_size, 0);
    }

    f->buf_size = f->buf_max_size = f->buf_pos = 0;
}

static int
nil_and_error_message (lua_State *L, const char *msg) {
    lua_pushnil(L);
    lua_pushstring(L, msg);
    return 2;
}

static int
read_number (MemoryFile *f, lua_State *L) {
    lua_Number d;
    int bytes;
    if (!f->buf)
        return 0;       /* empty file */
    ensure_buf_size(f, L, f->buf_size + 1);
    f->buf[f->buf_size] = '\0';
    if (sscanf(f->buf + f->buf_pos, LUA_NUMBER_SCAN "%n", &d, &bytes) == 1) {
        assert(bytes > 0);
        lua_pushnumber(L, d);
        f->buf_pos += bytes;
        return 1;
    }
    else
        return 0;       /* read fails, invalid number syntax */
}

static int
read_line (MemoryFile *f, lua_State *L) {
    const char *e = memchr(f->buf + f->buf_pos, '\n', f->buf_size - f->buf_pos);
    size_t len = e ? (size_t) (e - (f->buf + f->buf_pos))
                   : f->buf_size - f->buf_pos;
    size_t new_pos = f->buf_pos + len;
    int success;
    if (e)
        ++new_pos;      /* skip newline */
    success = f->buf_pos != f->buf_size;
    lua_pushlstring(L, f->buf + f->buf_pos, len);
    f->buf_pos = new_pos;
    return success;
}

static int
test_eof (MemoryFile *f, lua_State *L) {
    lua_pushlstring(L, 0, 0);
    return f->buf_pos != f->buf_size;
}

static int
read_chars (MemoryFile *f, lua_State *L, size_t n) {
    size_t rlen = f->buf_size - f->buf_pos;
    if (rlen > n)
        rlen = n;
    lua_pushlstring(L, f->buf + f->buf_pos, rlen);
    f->buf_pos += rlen;
    return 1;
}

static int
lines_iter (lua_State *L) {
    MemoryFile *f = lua_touserdata(L, lua_upvalueindex(1));
    return read_line(f, L) ? 1 : 0;
}

static int
memfile_open (lua_State *L) {
    size_t data_len;
    const char *data, *mode;
    char modechar;
    MemoryFile *f;

    if (lua_gettop(L) > 2)
        return luaL_error(L, "too many arguments to memoryfile.open()");

    data = lua_tolstring(L, 1, &data_len);
    if (!data) {
        luaL_argcheck(L, lua_isnoneornil(L, 1), 1,
                      "input data must be string or nil");
        data_len = 0;
    }

    mode = luaL_optstring(L, 2, "r");
    modechar = mode ? mode[0] : 'r';
    if (modechar != 'r' && modechar != 'w' && modechar != 'a')
        luaL_argerror(L, 2, "mode must start with 'r', 'w', or 'a'");

    f = lua_newuserdata(L, sizeof(MemoryFile));
    f->buf = 0;
    f->buf_size = f->buf_max_size = f->buf_pos = 0;
    f->append = modechar == 'a';

    luaL_getmetatable(L, MEMORYFILE_MT_NAME);
    lua_setmetatable(L, -2);

    if (modechar != 'w' && data_len > 0) {
        ensure_buf_size(f, L, data_len);
        memcpy(f->buf, data, data_len);
        f->buf_size = data_len;
    }

    return 1;
}

static int
memfile_noop (lua_State *L) {
    lua_pushboolean(L, 1);
    return 1;
}

static int
memfile_close (lua_State *L) {
    MemoryFile *f = luaL_checkudata(L, 1, MEMORYFILE_MT_NAME);

    delete_memoryfile_buffer(f, L);

    lua_pushboolean(L, 1);      /* always successful */
    return 1;
}

static int
memfile_lines (lua_State *L) {
    lua_pushcclosure(L, lines_iter, 1);
    return 1;
}

static int
memfile_read (lua_State *L) {
    MemoryFile *f = luaL_checkudata(L, 1, MEMORYFILE_MT_NAME);
    int nargs = lua_gettop(L) - 1;
    int success;
    int n;

    if (nargs == 0) {   /* no arguments? */
        success = read_line(f, L);
        n = 3;          /* to return 1 result */
    }
    else {  /* ensure stack space for all results and for auxlib's buffer */
        luaL_checkstack(L, nargs + LUA_MINSTACK, "too many arguments");
        success = 1;
        for (n = 2; nargs-- && success; n++) {
            if (lua_type(L, n) == LUA_TNUMBER) {
                size_t l = (size_t) lua_tointeger(L, n);
                success = (l == 0) ? test_eof(f, L) : read_chars(f, L, l);
            }
            else {
                const char *p = lua_tostring(L, n);
                luaL_argcheck(L, p && p[0] == '*', n, "invalid option");
                switch (p[1]) {
                    case 'n':  /* number */
                        success = read_number(f, L);
                        break;
                    case 'l':  /* line */
                        success = read_line(f, L);
                        break;
                    case 'a':  /* all the rest of the file */
                        read_chars(f, L, ~((size_t)0)); /* MAX_SIZE_T bytes */
                        success = 1; /* always success */
                        break;
                    default:
                        return luaL_argerror(L, n, "invalid format");
                }
            }
        }
    }

    if (!success) {
        lua_pop(L, 1);      /* remove last result */
        lua_pushnil(L);     /* push nil instead */
    }

    return n - 2;
}

static int
memfile_seek (lua_State *L) {
    static const char *const modenames[] = { "set", "cur", "end", 0 };
    MemoryFile *f = luaL_checkudata(L, 1, MEMORYFILE_MT_NAME);
    int op = luaL_checkoption(L, 2, "cur", modenames);
    long new_pos = (long)luaL_optinteger(L, 3, 0);

    if (op == 1)            /* SEEK_CUR */
        new_pos = (long) f->buf_pos + new_pos;
    else if (op == 2)       /* SEEK_END */
        new_pos = (long) f->buf_size + new_pos;

    if (new_pos < 0)
        return nil_and_error_message(L, "seek to before start of memory file");
    else if ((size_t) new_pos > f->buf_size)
        return nil_and_error_message(L, "seek to after end of memory file");

    f->buf_pos = (size_t) new_pos;
    lua_pushinteger(L, f->buf_pos);
    return 1;
}

static int
memfile_write (lua_State *L) {
    size_t new_bytes, tmpsize, end_pos;
    int i, num_args = lua_gettop(L);
    const char *data;
    MemoryFile *f = luaL_checkudata(L, 1, MEMORYFILE_MT_NAME);

    new_bytes = 0;
    for (i = 2; i <= num_args; ++i) {
        if (!lua_tolstring(L, i, &tmpsize))
            return luaL_argerror(L, i, "must be string or number");
        new_bytes += tmpsize;
    }

    if (new_bytes == 0)
        ;   /* nothing to write */
    else if (f->append || f->buf_pos == f->buf_size) {
        /* Append data to end of buffer */
        ensure_buf_size(f, L, f->buf_size + new_bytes);
        for (i = 2; i <= num_args; ++i) {
            data = lua_tolstring(L, i, &tmpsize);
            memcpy(f->buf + f->buf_size, data, tmpsize);
            f->buf_size += tmpsize;
        }
        if (!f->append)
            f->buf_pos = f->buf_size;
    }
    else {
        /* Write over the top of some existing data */
        end_pos = f->buf_pos + new_bytes;
        if (end_pos > f->buf_size) {
            ensure_buf_size(f, L, end_pos);
            f->buf_size = end_pos;
        }
        for (i = 2; i <= num_args; ++i) {
            data = lua_tolstring(L, i, &tmpsize);
            memcpy(f->buf + f->buf_pos, data, tmpsize);
            f->buf_pos += tmpsize;
        }
    }

    lua_pushboolean(L, 1);
    return 1;
}

static int
memfile_gc (lua_State *L) {
    MemoryFile *f = luaL_checkudata(L, 1, MEMORYFILE_MT_NAME);
    delete_memoryfile_buffer(f, L);
    return 0;
}

static int
memfile_tostring (lua_State *L) {
    MemoryFile *f = luaL_checkudata(L, 1, MEMORYFILE_MT_NAME);
    lua_pushlstring(L, f->buf, f->buf_size);
    return 1;
}

static int
memfile_size (lua_State *L) {
    lua_Integer new_size;
    MemoryFile *f = luaL_checkudata(L, 1, MEMORYFILE_MT_NAME);
    size_t old_size = f->buf_size;

    if (!lua_isnoneornil(L, 2)) {
        new_size = lua_tointeger(L, 2);
        if (new_size == 0) {
            luaL_argcheck(L, lua_isnumber(L, 2), 2, "new size must be integer");
            delete_memoryfile_buffer(f, L);
        }
        else if (new_size < 0)
            luaL_argerror(L, 2, "new size must be >= zero");
        else {
            if ((size_t) new_size > f->buf_size) {
                ensure_buf_size(f, L, new_size);
                memset(f->buf + f->buf_size, 0, new_size - f->buf_size);
            }
            f->buf_size = (size_t) new_size;
            if (f->buf_pos > f->buf_size)
                f->buf_pos = f->buf_size;
        }
    }

    lua_pushinteger(L, old_size);
    return 1;
}

static const luaL_Reg
memfile_lib[] = {
    /* Emulations of the methods on standard Lua file handle objects: */
    { "close", memfile_close },
    { "flush", memfile_noop },
    { "lines", memfile_lines },
    { "read", memfile_read },
    { "seek", memfile_seek },
    { "setvbuf", memfile_noop },
    { "write", memfile_write },
    { "__gc", memfile_gc },
    { "__tostring", memfile_tostring },
    /* Methods not provided by the standard Lua file handle objects: */
    { "size", memfile_size },
    { 0, 0 }
};


int
luaopen_memoryfile (lua_State *L) {
    const luaL_Reg *l;

#ifdef VALGRIND_LUA_MODULE_HACK
    /* Hack to allow Valgrind to access debugging info for the module. */
    luaL_getmetatable(L, "_LOADLIB");
    lua_pushnil(L);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
#endif

    /* Create the table to return from 'require' */
    lua_createtable(L, 0, 3);
    lua_pushliteral(L, "_NAME");
    lua_pushliteral(L, "memoryfile");
    lua_rawset(L, -3);
    lua_pushliteral(L, "_VERSION");
#define VERSION "1.2"
    lua_pushliteral(L, VERSION);
    lua_rawset(L, -3);
    lua_pushliteral(L, "open");
    lua_pushcfunction(L, memfile_open);
    lua_rawset(L, -3);

    /* Create the metatable for file handle objects returned from .open() */
    luaL_newmetatable(L, MEMORYFILE_MT_NAME);
    lua_pushliteral(L, "_NAME");
    lua_pushliteral(L, "memoryfile-object");
    lua_rawset(L, -3);

    for (l = memfile_lib; l->name; ++l) {
        lua_pushstring(L, l->name);
        lua_pushcfunction(L, l->func);
        lua_rawset(L, -3);
    }

    lua_pushliteral(L, "__index");
    lua_pushvalue(L, -2);
    lua_rawset(L, -3);
    lua_pop(L, 1);

    return 1;
}

/* vi:set ts=4 sw=4 expandtab: */
