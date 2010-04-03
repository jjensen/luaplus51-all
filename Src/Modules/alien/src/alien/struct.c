/*
** Library for packing/unpacking structures.
**
** Valid formats:
** x - pading
** b/B - signed/unsigned byte
** h/H - signed/unsigned short
** l/L - signed/unsigned long
** i/In - signed/unsigned integer with size `n' (default is size of int)
** cn - sequence of `n' chars (from/to a string); when packing, n==0 means
        the whole string; when unpacking, n==0 means use the previous
        read number as the string length
** s - zero-terminated string
** f - float
** d - doulbe
*/


#include <ctype.h>
#include <string.h>


#include "lua.h"
#include "lauxlib.h"


/* dummy structure to get alignment requirements */
struct cD {
  char c;
  double d;
};


static int getmaxalign (void) {
  int ma = sizeof(int);
  int e = sizeof(struct cD) - sizeof(double);
  if (ma < e) ma = e;
  return ma;
}


static int getendianess (const char **s, int *native_out) {
  int endian;  /* 0 = little; 1 = big */
  int native = 1;
  if (*(char *)&native == 1)
    native = 0;
  if (**s == '>') {
    endian = 1;
    (*s)++;
  }
  else if (**s == '<') {
    endian = 0;
    (*s)++;
  }
  else
    endian = native;
  *native_out = native;
  return endian;
}


static int getnum (const char **fmt, int df) {
  if (!isdigit(**fmt))
    return df;  /* no number */
  else {
    int a = 0;
    do {
      a = a*10 + *((*fmt)++) - '0';
    } while (isdigit(**fmt));
    return a;
  }
}


static int optsize (char opt, const char **fmt) {
  switch (opt) {
    case 'B': case 'b': return 1;
    case 'H': case 'h': return 2;
    case 'L': case 'l': return 4;
    case 'f':  return sizeof(float);
    case 'd':  return sizeof(double);
    case 'x': return 1;
    case 'i': return getnum(fmt, sizeof(int));
    case 'I': return getnum(fmt, sizeof(int));
    case 'c': return getnum(fmt, 1);
    case 's': return 0;
    default: return 1;  /* invalid code */
  }
}


static int getalign (const char **fmt) {
  if (**fmt != '!') return 1;  /* no alignment */
  else {
    (*fmt)++;
    return getnum(fmt, getmaxalign());
  }
}


static int gettoalign (lua_State *L, int align, int opt, int size) {
  int toalign = (opt == 'c' || opt == 's') ? 1 : size;
  if (toalign > align) toalign = align;
  if (toalign == 0 || (toalign & (toalign - 1)) != 0)
    luaL_error(L, "alignment must be power of 2");
  return toalign;
}


static void putinteger (lua_State *L, luaL_Buffer *b, int arg, int endian,
                        int size) {
  unsigned char buff[sizeof(long) + sizeof(long)];
  lua_Number n = luaL_checknumber(L, arg);
  unsigned long value;
  unsigned char *s;
  int inc, i;
  if (n < 0) {
    value = (unsigned long)(-n);
    value = (~value) + 1;  /* 2's complement */
  }
  else
    value = (unsigned long)n;
  if (endian == 0) {
    inc = 1;
    s = buff;
  }
  else {
    inc = -1;
    s = buff+(size-1);
  }
  for (i=0; i<size; i++) {
    *s = (unsigned char)(value & 0xff);
    s += inc;
    value >>= 8;
  }
  luaL_addlstring(b, (char *)buff, size);
}


static void invertbytes (char *b, int size) {
  int i = 0;
  while (i < --size) {
    char temp = b[i];
    b[i++] = b[size];
    b[size] = temp;
  }
}


static void invalidformat (lua_State *L, char c) {
  const char *msg = lua_pushfstring(L, "invalid format option [%c]", c);
  luaL_argerror(L, 1, msg);
}


static int b_size (lua_State *L) {
  int native;
  const char *fmt = luaL_checkstring(L, 1);
  int align;
  int totalsize = 0;
  getendianess(&fmt, &native);
  align = getalign(&fmt);
  while (*fmt) {
    int opt = *fmt++;
    int size = optsize(opt, &fmt);
    int toalign = gettoalign(L, align, opt, size);
    if (size == 0)
      luaL_error(L, "options `c0' - `s' have undefined sizes");
    totalsize += toalign - 1;
    totalsize -= totalsize&(toalign-1);
    totalsize += size;
  }
  lua_pushnumber(L, totalsize);
  return 1;
}


static int b_pack (lua_State *L) {
  luaL_Buffer b;
  int native;
  const char *fmt = luaL_checkstring(L, 1);
  int endian = getendianess(&fmt, &native);
  int align = getalign(&fmt);
  int arg = 2;
  int totalsize = 0;
  lua_pushnil(L);  /* mark to separate arguments from string buffer */
  luaL_buffinit(L, &b);
  for (; *fmt; arg++) {
    int opt = *fmt++;
    int size = optsize(opt, &fmt);
    int toalign = gettoalign(L, align, opt, size);
    while ((totalsize&(toalign-1)) != 0) {
       luaL_putchar(&b, '\0');
       totalsize++;
    }
    switch (opt) {
      case ' ': break;  /* ignore white spaces */
      case 'b': case 'B': case 'h': case 'H':
      case 'l': case 'L': case 'i': case 'I': {  /* integer types */
        putinteger(L, &b, arg, endian, size);
        break;
      }
      case 'x': {
        arg--;  /* undo increment */
        luaL_putchar(&b, '\0');
        break;
      }
      case 'f': {
        float f = (float)luaL_checknumber(L, arg);
        if (endian != native) invertbytes((char *)&f, size);
        luaL_addlstring(&b, (char *)&f, size);
        break;
      }
      case 'd': {
        double d = luaL_checknumber(L, arg);
        if (endian != native) invertbytes((char *)&d, size);
        luaL_addlstring(&b, (char *)&d, size);
        break;
      }
      case 'c': case 's': {
        size_t l;
        const char *s = luaL_checklstring(L, arg, &l);
        if (size == 0) size = l;
        luaL_argcheck(L, l >= (size_t)size, arg, "string too short");
        luaL_addlstring(&b, s, size);
        if (opt == 's') {
          luaL_putchar(&b, '\0');  /* add zero at the end */
          size++;
        }
        break;
      }
      default: invalidformat(L, opt);
    }
    totalsize += size;
  }
  luaL_pushresult(&b);
  return 1;
}


static void getinteger (lua_State *L, const char *buff, int endian,
                        int withsign, int size) {
  unsigned long l = 0;
  int i, inc;
  if (endian == 1)
    inc = 1;
  else {
    inc = -1;
    buff += size-1;
  }
  for (i=0; i<size; i++) {
    l = (l<<8) + (unsigned char)(*buff);
    buff += inc;
  }
  if (withsign) {  /* signed format? */
    unsigned long mask = ~(0UL) << (size*8 - 1);
    if (l & mask) {  /* negative value? */
      l = (l^~(mask<<1)) + 1;
      lua_pushnumber(L, -(lua_Number)l);
      return;
    }
  }
  lua_pushnumber(L, l);
}


static int b_unpack (lua_State *L) {
  int native;
  const char *fmt = luaL_checkstring(L, 1);
  size_t ld;
  int pos, align, endian;
  const char *data;
  if(lua_isuserdata(L, 2)) {
    data = (const char*)lua_touserdata(L, 2);
    ld = (size_t)luaL_checkinteger(L, 3);
    pos = luaL_optint(L, 4, 1) - 1;
  } else {
    data = luaL_checklstring(L, 2, &ld);
    pos = luaL_optint(L, 3, 1) - 1;
  }
  endian = getendianess(&fmt, &native);
  align = getalign(&fmt);
  lua_settop(L, 2);
  while (*fmt) {
    int opt = *fmt++;
    int size = optsize(opt, &fmt);
    int toalign = gettoalign(L, align, opt, size);
    pos += toalign - 1;
    pos -= pos&(toalign-1);
    luaL_argcheck(L, pos+size <= (int)ld, 2, "data string too short");
    switch (opt) {
      case ' ': break;  /* ignore white spaces */
      case 'b': case 'B': case 'h': case 'H':
      case 'l': case 'L': case 'i':  case 'I': {  /* integer types */
        int withsign = islower(opt);
        getinteger(L, data+pos, endian, withsign, size);
        break;
      }
      case 'x': {
        break;
      }
      case 'f': {
        float f;
        memcpy(&f, data+pos, size);
        if (endian != native) invertbytes((char *)&f, sizeof(f));
        lua_pushnumber(L, f);
        break;
      }
      case 'd': {
        double d;
        memcpy(&d, data+pos, size);
        if (endian != native) invertbytes((char *)&d, sizeof(d));
        lua_pushnumber(L, d);
        break;
      }
      case 'c': {
        if (size == 0) {
          if (!lua_isnumber(L, -1))
            luaL_error(L, "format `c0' needs a previous size");
          size = lua_tonumber(L, -1);
          lua_pop(L, 1);
          luaL_argcheck(L, pos+size <= (int)ld, 2, "data string too short");
        }
        lua_pushlstring(L, data+pos, size);
        break;
      }
      case 's': {
        const char *e = (const char *)memchr(data+pos, '\0', ld - pos);
        if (e == NULL)
          luaL_error(L, "unfinished string in data");
        size = (e - (data+pos)) + 1;
        lua_pushlstring(L, data+pos, size - 1);
        break;
      }
      default: invalidformat(L, opt);
    }
    pos += size;
  }
  lua_pushnumber(L, pos + 1);
  return lua_gettop(L) - 2;
}



static const struct luaL_reg thislib[] = {
  {"pack", b_pack},
  {"unpack", b_unpack},
  {"size", b_size},
  {NULL, NULL}
};


LUAMODULE_API int luaopen_alien_struct (lua_State *L) {
  lua_getglobal(L, "alien");
  if(lua_isnil(L, -1)) {
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "alien");
  }
  lua_newtable(L);
  lua_pushvalue(L, -1);
  lua_setfield(L, -3, "struct");
  luaL_register(L, NULL, thislib);
  return 1;
}
