/* ltre.c - Lua binding of TRE regular expressions library */
/* See Copyright Notice in the file LICENSE */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lua.h"
#include "lauxlib.h"
#include "../common.h"

#include <tre/tre.h>

void bufferZ_putrepstringW (TBuffer *BufRep, int reppos, int nsub);

/* These 2 settings may be redefined from the command-line or the makefile.
 * They should be kept in sync between themselves and with the target name.
 */
#ifndef REX_LIBNAME
#  define REX_LIBNAME "rex_tre"
#endif
#ifndef REX_OPENLIB
#  define REX_OPENLIB luaopen_rex_tre
#endif

#define REX_TYPENAME REX_LIBNAME"_regex"

#define ALG_CFLAGS_DFLT REG_EXTENDED
#define ALG_EFLAGS_DFLT 0
#define ALG_CHARSIZE 2
#define BUFFERZ_PUTREPSTRING bufferZ_putrepstringW

#define ALG_NOMATCH(res)   ((res) == REG_NOMATCH)
#define ALG_ISMATCH(res)   ((res) == 0)
#define ALG_SUBBEG(ud,n)   (ALG_CHARSIZE * ud->match[n].rm_so)
#define ALG_SUBEND(ud,n)   (ALG_CHARSIZE * ud->match[n].rm_eo)
#define ALG_SUBLEN(ud,n)   (ALG_SUBEND(ud,n) - ALG_SUBBEG(ud,n))
#define ALG_SUBVALID(ud,n) (ALG_SUBBEG(ud,n) >= 0)
#define ALG_NSUB(ud)       ((int)ud->r.re_nsub)

#define ALG_PUSHSUB(L,ud,text,n) \
  lua_pushlstring (L, (text) + ALG_SUBBEG(ud,n), ALG_SUBLEN(ud,n))

#define ALG_PUSHSUB_OR_FALSE(L,ud,text,n) \
  (ALG_SUBVALID(ud,n) ? ALG_PUSHSUB (L,ud,text,n) : lua_pushboolean (L,0))

#define ALG_PUSHSTART(L,ud,offs,n)   lua_pushinteger(L, ((offs) + ALG_SUBBEG(ud,n))/ALG_CHARSIZE + 1)
#define ALG_PUSHEND(L,ud,offs,n)     lua_pushinteger(L, ((offs) + ALG_SUBEND(ud,n))/ALG_CHARSIZE)
#define ALG_PUSHOFFSETS(L,ud,offs,n) \
  (ALG_PUSHSTART(L,ud,offs,n), ALG_PUSHEND(L,ud,offs,n))

#define ALG_BASE(st)                  (st)
#define ALG_GETCFLAGS(L,pos)          luaL_optint(L, pos, ALG_CFLAGS_DFLT)

typedef struct {
  regex_t      r;
  regmatch_t * match;
  int          freed;
} TPosix;

#define TUserdata TPosix

#include "../algo.h"

/*  Functions
 ******************************************************************************
 */

static void checkarg_regaparams (lua_State *L, int stackpos,  regaparams_t *argP) {
  if (lua_type (L, stackpos) != LUA_TTABLE) /* allow for userdata? */
    luaL_argerror (L, stackpos, "table expected");
  lua_pushvalue (L, stackpos);
  argP->cost_ins   = get_int_field (L, "cost_ins");
  argP->cost_del   = get_int_field (L, "cost_del");
  argP->cost_subst = get_int_field (L, "cost_subst");
  argP->max_cost   = get_int_field (L, "max_cost");
  argP->max_ins    = get_int_field (L, "max_ins");
  argP->max_del    = get_int_field (L, "max_del");
  argP->max_subst  = get_int_field (L, "max_subst");
  argP->max_err    = get_int_field (L, "max_err");
  lua_pop (L, 1);
}

/* method r:atfind (s, params, [st], [ef]) */
/* method r:aexec  (s, params, [st], [ef]) */
static void checkarg_atfind (lua_State *L, TArgExec *argE, TPosix **ud,
                             regaparams_t *argP) {
  *ud = check_ud (L);
  argE->text = luaL_checklstring (L, 2, &argE->textlen);
  checkarg_regaparams (L, 3, argP);
  argE->startoffset = get_startoffset (L, 4, argE->textlen);
  argE->eflags = luaL_optint (L, 5, ALG_EFLAGS_DFLT);
}

static int generate_error (lua_State *L, const TPosix *ud, int errcode) {
  char errbuf[80];
  tre_regerror (errcode, &ud->r, errbuf, sizeof (errbuf));
  return luaL_error (L, "%s", errbuf);
}

static int compile_regex (lua_State *L, const TArgComp *argC, TPosix **pud) {
  int res;
  TPosix *ud;

  ud = (TPosix *)lua_newuserdata (L, sizeof (TPosix));
  memset (ud, 0, sizeof (TPosix));          /* initialize all members to 0 */

  res = tre_regwncomp (&ud->r, (const wchar_t*)argC->pattern, argC->patlen/ALG_CHARSIZE, argC->cflags);
  if (res != 0)
    return generate_error (L, ud, res);

  if (argC->cflags & REG_NOSUB)
    ud->r.re_nsub = 0;
  ud->match = (regmatch_t *) Lmalloc (L, (ALG_NSUB(ud) + 1) * sizeof (regmatch_t));
  if (!ud->match)
    luaL_error (L, "malloc failed");
  lua_pushvalue (L, ALG_ENVIRONINDEX);
  lua_setmetatable (L, -2);

  if (pud) *pud = ud;
  return 1;
}

static int generic_atfind (lua_State *L, int tfind) {
  int res;
  TArgExec argE;
  TPosix *ud;
  regaparams_t argP;
  regamatch_t res_match;

  checkarg_atfind (L, &argE, &ud, &argP);
  if (argE.startoffset > (int)argE.textlen)
    return lua_pushnil(L), 1;

  argE.text += argE.startoffset;
  res_match.nmatch = ALG_NSUB(ud) + 1;
  res_match.pmatch = ud->match;

  /* execute the search */
  res = tre_regawnexec (&ud->r, (const wchar_t*)argE.text,
    (argE.textlen - argE.startoffset)/ALG_CHARSIZE, &res_match, argP, argE.eflags);
  if (ALG_ISMATCH (res)) {
    ALG_PUSHOFFSETS (L, ud, argE.startoffset, 0);
    if (tfind)
      push_substring_table (L, ud, argE.text);
    else
      push_offset_table (L, ud, argE.startoffset);
    /* set values in the dictionary part of the table */
    set_int_field (L, "cost", res_match.cost);
    set_int_field (L, "num_ins", res_match.num_ins);
    set_int_field (L, "num_del", res_match.num_del);
    set_int_field (L, "num_subst", res_match.num_subst);
    return 3;
  }
  else if (ALG_NOMATCH (res))
    return lua_pushnil (L), 1;
  else
    return generate_error (L, ud, res);
}

static int Ltre_atfind (lua_State *L) {
  return generic_atfind (L, 1);
}

static int Ltre_aexec (lua_State *L) {
  return generic_atfind (L, 0);
}

static int gmatch_exec (TUserdata *ud, TArgExec *argE) {
  if (argE->startoffset > 0)
    argE->eflags |= REG_NOTBOL;
  argE->text += argE->startoffset;
  return tre_regwnexec (&ud->r, (const wchar_t*)argE->text, (argE->textlen - argE->startoffset)/ALG_CHARSIZE,
                   ALG_NSUB(ud) + 1, ud->match, argE->eflags);
}

static void gmatch_pushsubject (lua_State *L, TArgExec *argE) {
  lua_pushlstring (L, argE->text, argE->textlen);
}

static int findmatch_exec (TPosix *ud, TArgExec *argE) {
  argE->text += argE->startoffset;
  return tre_regwnexec (&ud->r, (const wchar_t*)argE->text, (argE->textlen - argE->startoffset)/ALG_CHARSIZE,
                   ALG_NSUB(ud) + 1, ud->match, argE->eflags);
}

static int gsub_exec (TPosix *ud, TArgExec *argE, int st) {
  if (st > 0)
    argE->eflags |= REG_NOTBOL;
  return tre_regwnexec (&ud->r, (const wchar_t*)(argE->text+st), (argE->textlen-st)/ALG_CHARSIZE, ALG_NSUB(ud)+1,
                    ud->match, argE->eflags);
}

static int split_exec (TPosix *ud, TArgExec *argE, int offset) {
  if (offset > 0)
    argE->eflags |= REG_NOTBOL;
  return tre_regwnexec (&ud->r, (const wchar_t*)(argE->text + offset), (argE->textlen - offset)/ALG_CHARSIZE,
                   ALG_NSUB(ud) + 1, ud->match, argE->eflags);
}

static const luaL_Reg r_methods[] = {
  { "wexec",         algm_exec },
  { "wfind",         algm_find },
  { "wmatch",        algm_match },
  { "wtfind",        algm_tfind },
  { "waexec",        Ltre_aexec },
  { "watfind",       Ltre_atfind },
  { NULL, NULL}
};

static const luaL_Reg r_functions[] = {
  { "wnew",          algf_new },
  { "wfind",         algf_find },
  { "wgmatch",       algf_gmatch },
  { "wgsub",         algf_gsub },
  { "wmatch",        algf_match },
  { "wsplit",        algf_split },
  { NULL, NULL }
};

/* Add the library */
void add_wide_lib (lua_State *L)
{
  (void)alg_register;
  lua_pushvalue(L, -2);
#if LUA_VERSION_NUM == 501
  luaL_register(L, NULL, r_methods);
  lua_pop(L, 1);
  luaL_register(L, NULL, r_functions);
#else
  lua_pushvalue(L, -1);
  luaL_setfuncs(L, r_methods, 1);
  luaL_setfuncs(L, r_functions, 1);
#endif
}

/* 1. When called repeatedly on the same TBuffer, its existing data
      is discarded and overwritten by the new data.
   2. The TBuffer's array is never shrunk by this function.
*/
void bufferZ_putrepstringW (TBuffer *BufRep, int reppos, int nsub) {
  wchar_t dbuf[] = { 0, 0 };
  size_t replen;
  const wchar_t *p = (const wchar_t*) lua_tolstring (BufRep->L, reppos, &replen);
  replen /= sizeof(wchar_t);
  const wchar_t *end = p + replen;
  BufRep->top = 0;
  while (p < end) {
    const wchar_t *q;
    for (q = p; q < end && *q != L'%'; ++q)
      {}
    if (q != p)
      bufferZ_addlstring (BufRep, p, (q - p) * sizeof(wchar_t));
    if (q < end) {
      if (++q < end) {  /* skip % */
        if (iswdigit (*q)) {
          int num;
          *dbuf = *q;
          num = wcstol (dbuf, NULL, 10);
          if (num == 1 && nsub == 0)
            num = 0;
          else if (num > nsub) {
            freelist_free (BufRep->freelist);
            luaL_error (BufRep->L, "invalid capture index");
          }
          bufferZ_addnum (BufRep, num);
        }
        else bufferZ_addlstring (BufRep, q, 1 * sizeof(wchar_t));
      }
      p = q + 1;
    }
    else break;
  }
}
