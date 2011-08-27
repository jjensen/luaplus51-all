/*
** $Id: lobject.h,v 2.20.1.2 2008/08/06 13:29:48 roberto Exp $
** Type definitions for Lua objects
** See Copyright Notice in lua.h
*/


#ifndef lobject_h
#define lobject_h


#include <stdarg.h>


#include "llimits.h"
#include "lua.h"


/* tags for values visible from Lua */
#if !LUA_WIDESTRING
#if LNUM_PATCH
#if defined(LUA_TINT) && (LUA_TINT > LUA_TTHREAD)
# define LAST_TAG   LUA_TINT
#else
# define LAST_TAG	LUA_TTHREAD
#endif
#else /* LNUM_PATCH */
#define LAST_TAG	LUA_TTHREAD
#endif /* LNUM_PATCH */
#else /* LUA_WIDESTRING */
#if LNUM_PATCH
#if defined(LUA_TINT) && (LUA_TINT > LUA_TTHREAD)
# define LAST_TAG   LUA_TINT
#else
#define LAST_TAG	LUA_TWSTRING
#endif
#else /* LNUM_PATCH */
#define LAST_TAG	LUA_TWSTRING
#endif /* LNUM_PATCH */
#endif /* LUA_WIDESTRING */

#define NUM_TAGS	(LAST_TAG+1)


/*
** Extra tags for non-values
*/
#define LUA_TPROTO	(LAST_TAG+1)
#define LUA_TUPVAL	(LAST_TAG+2)
#define LUA_TDEADKEY	(LAST_TAG+3)


/*
** Union of all collectable objects
*/
typedef union GCObject GCObject;


/*
** Common Header for all collectable objects (in macro form, to be
** included in other objects)
*/
#if LUA_REFCOUNT

#define CommonHeader	GCObject *next; GCObject* prev; lu_byte tt; lu_byte marked; unsigned short ref

#define luarc_addref(o) { TValue* i_o2 = (o); if(iscollectable(i_o2))	\
        { \
            gcvalue(i_o2)->gch.ref++; \
            lua_assert(gcvalue(i_o2)->gch.ref != 0); \
        }  }

#define luarc_addreftvalue(o) { gcvalue(o)->gch.ref++; lua_assert(gcvalue(o)->gch.ref != 0); }
#define luarc_addreftable(o) { o->ref++; lua_assert(o->ref != 0); }
#define luarc_addrefproto(o) { o->ref++; lua_assert(o->ref != 0); }
#define luarc_addrefupval(o) { o->ref++; lua_assert(o->ref != 0); }
#define luarc_addrefstring(o) { o->tsv.ref++; lua_assert(o->tsv.ref != 0); }

#define luarc_release(L,o) { TValue* i_o2 = (o); if(iscollectable(i_o2) && ((--gcvalue(i_o2)->gch.ref)<=0))	\
		{	\
			luarc_releaseobject(L,gcvalue(i_o2));	\
		}	}

#define luarc_releasetable(L,o) { Table* i_o2 = (o); if((--i_o2->ref)<=0) luarc_releaseobject(L,(GCObject*)i_o2); }
#define luarc_releaseproto(L,o) { Proto* i_o2 = (o); if((--i_o2->ref)<=0) luarc_releaseobject(L,(GCObject*)i_o2); }
#define luarc_releaseupval(L,o) { UpVal* i_o2 = (o); if((--i_o2->ref)<=0) luarc_releaseobject(L,(GCObject*)i_o2); }
#define luarc_releasestring(L,o) { TString* i_o2 = (o); if((--i_o2->tsv.ref)<=0) luarc_releaseobject(L,(GCObject*)i_o2); }

#define luarc_makevaluebackup(v) TValue bak = *v;

extern void luarc_releaseobject(lua_State *L, GCObject* header);

#define luarc_newvalue(o) { setnilvalue2n(L, (o)); }
#define luarc_newarray(from,to) { TValue* i_from = (from); TValue* i_to = (to); while (i_from < i_to) { luarc_newvalue(i_from++); } }
#define luarc_cleanvalue(o) { setnilvalue((o)); }
#define luarc_cleanarray(from,to) { TValue* i_from = (from); TValue* i_to = (to); while (i_from < i_to) { luarc_cleanvalue(i_from++); } }
#define luarc_cleanarrayreverse(to,from) { TValue* i_from = (from); TValue* i_to = (to); while (i_from >= i_to) { luarc_cleanvalue(i_from--); } }

#else /* !LUA_REFCOUNT */

#define CommonHeader	GCObject *next; lu_byte tt; lu_byte marked

#endif /* LUA_REFCOUNT */

/*
** Common header in struct form
*/
typedef struct GCheader {
  CommonHeader;
} GCheader;




/*
** Union of all Lua values
*/
typedef union {
  GCObject *gc;
  void *p;
#if LNUM_PATCH
#ifdef LNUM_COMPLEX
  lua_Complex n;
#else
  lua_Number n;
#endif
#ifdef LUA_TINT
  lua_Integer i;
#endif
#else
  lua_Number n;
#endif /* LNUM_PATCH */
  int b;
} Value;

/*
** Tagged Values
*/

#if LUA_PACK_VALUE == 0

#define TValuefields	Value value; int tt
#define LUA_TVALUE_NIL  {NULL}, LUA_TNIL

typedef struct lua_TValue {
  TValuefields;
} TValue;

#else

#define TValuefields	union { \
  struct { \
    int _pad0; \
    int tt_sig; \
  } _ts; \
  struct { \
    int _pad; \
    short tt; \
    short sig; \
  } _t; \
  Value value; \
}
#define LUA_NOTNUMBER_SIG (-1)
#define add_sig(tt) ( 0xffff0000 | (tt) )
#define LUA_TVALUE_NIL {0, add_sig(LUA_TNIL)}

typedef TValuefields TValue;

#endif

/* Macros to test type */
#if LUA_PACK_VALUE == 0

#define ttisnil(o)	(ttype(o) == LUA_TNIL)
#if LNUM_PATCH
#ifdef LUA_TINT
# if (LUA_TINT & 0x0f) == LUA_TNUMBER
#  define ttisnumber(o) ((ttype(o) & 0x0f) == LUA_TNUMBER)
# else
#  define ttisnumber(o) ((ttype(o) == LUA_TINT) || (ttype(o) == LUA_TNUMBER))
# endif
# define ttisint(o) (ttype(o) == LUA_TINT)
#else
# define ttisnumber(o) (ttype(o) == LUA_TNUMBER)
#endif

#ifdef LNUM_COMPLEX
# define ttiscomplex(o) ((ttype(o) == LUA_TNUMBER) && (nvalue_img_fast(o)!=0))
#endif
#else
#define ttisnumber(o)	(ttype(o) == LUA_TNUMBER)
#endif /* LNUM_PATCH */
#define ttisstring(o)	(ttype(o) == LUA_TSTRING)
#define ttistable(o)	(ttype(o) == LUA_TTABLE)
#define ttisfunction(o)	(ttype(o) == LUA_TFUNCTION)
#define ttisboolean(o)	(ttype(o) == LUA_TBOOLEAN)
#define ttisuserdata(o)	(ttype(o) == LUA_TUSERDATA)
#define ttisthread(o)	(ttype(o) == LUA_TTHREAD)
#define ttislightuserdata(o)	(ttype(o) == LUA_TLIGHTUSERDATA)
#if LUA_WIDESTRING
#define ttiswstring(o)	(ttype(o) == LUA_TWSTRING)
#endif /* LUA_WIDESTRING */

#else

#define ttisnil(o)	(ttype_sig(o) == add_sig(LUA_TNIL))
#define ttisnumber(o)	((o)->_t.sig != LUA_NOTNUMBER_SIG)
#define ttisstring(o)	(ttype_sig(o) == add_sig(LUA_TSTRING))
#define ttistable(o)	(ttype_sig(o) == add_sig(LUA_TTABLE))
#define ttisfunction(o)	(ttype_sig(o) == add_sig(LUA_TFUNCTION))
#define ttisboolean(o)	(ttype_sig(o) == add_sig(LUA_TBOOLEAN))
#define ttisuserdata(o)	(ttype_sig(o) == add_sig(LUA_TUSERDATA))
#define ttisthread(o)	(ttype_sig(o) == add_sig(LUA_TTHREAD))
#define ttislightuserdata(o)	(ttype_sig(o) == add_sig(LUA_TLIGHTUSERDATA))
#if LUA_WIDESTRING
#define ttiswstring(o)	(ttype_sig(o) == add_sig(LUA_TWSTRING))
#endif /* LUA_WIDESTRING */

#endif


/* Macros to access values */
#if LUA_PACK_VALUE == 0
#define ttype(o)	((o)->tt)
#else
#define ttype(o)	((o)->_t.sig == LUA_NOTNUMBER_SIG ? (o)->_t.tt : LUA_TNUMBER)
#define ttype_sig(o)	((o)->_ts.tt_sig)
#endif
#define gcvalue(o)	check_exp(iscollectable(o), (o)->value.gc)
#define pvalue(o)	check_exp(ttislightuserdata(o), (o)->value.p)
#if LNUM_PATCH
#if defined(LUA_TINT) && (LUA_TINT&0x0f == LUA_TNUMBER)
/* expects never to be called on <0 (LUA_TNONE) */
# define ttype_ext(o) ( ttype(o) & 0x0f )
#elif defined(LUA_TINT)
# define ttype_ext(o) ( ttype(o) == LUA_TINT ? LUA_TNUMBER : ttype(o) )
#else
# define ttype_ext(o) ttype(o)
#endif

/* '_fast' variants are for cases where 'ttype(o)' is known to be LUA_TNUMBER
 */
#ifdef LNUM_COMPLEX
# ifndef LUA_TINT
#  error "LNUM_COMPLEX only allowed with LNUM_INTxx defined (can be changed)"
# endif
# define nvalue_complex_fast(o) check_exp( ttype(o)==LUA_TNUMBER, (o)->value.n )   
# define nvalue_fast(o)     ( _LF(creal) ( nvalue_complex_fast(o) ) )
# define nvalue_img_fast(o) ( _LF(cimag) ( nvalue_complex_fast(o) ) )
# define nvalue_complex(o) check_exp( ttisnumber(o), (ttype(o)==LUA_TINT) ? (o)->value.i : (o)->value.n )
# define nvalue_img(o) check_exp( ttisnumber(o), (ttype(o)==LUA_TINT) ? 0 : _LF(cimag)( (o)->value.n ) ) 
# define nvalue(o) check_exp( ttisnumber(o), (ttype(o)==LUA_TINT) ? cast_num((o)->value.i) : _LF(creal)((o)->value.n) ) 
/* */
#elif defined(LUA_TINT)
# define nvalue(o)	check_exp( ttisnumber(o), (ttype(o)==LUA_TINT) ? cast_num((o)->value.i) : (o)->value.n )
# define nvalue_fast(o) check_exp( ttype(o)==LUA_TNUMBER, (o)->value.n )   
#else
# define nvalue(o)	check_exp( ttisnumber(o), (o)->value.n )
# define nvalue_fast(o) nvalue(o)
#endif

#ifdef LUA_TINT
# define ivalue(o)	check_exp( ttype(o)==LUA_TINT, (o)->value.i )
#endif
#else
#define nvalue(o)	check_exp(ttisnumber(o), (o)->value.n)
#endif /* LNUM_PATCH */
#define rawtsvalue(o)	check_exp(ttisstring(o), &(o)->value.gc->ts)
#define tsvalue(o)	(&rawtsvalue(o)->tsv)
#define rawuvalue(o)	check_exp(ttisuserdata(o), &(o)->value.gc->u)
#define uvalue(o)	(&rawuvalue(o)->uv)
#define clvalue(o)	check_exp(ttisfunction(o), &(o)->value.gc->cl)
#define hvalue(o)	check_exp(ttistable(o), &(o)->value.gc->h)
#define bvalue(o)	check_exp(ttisboolean(o), (o)->value.b)
#define thvalue(o)	check_exp(ttisthread(o), &(o)->value.gc->th)
#if LUA_WIDESTRING
#define rawtwsvalue(o)	check_exp(ttiswstring(o), &(o)->value.gc->ts)
#define twsvalue(o)	(&rawtwsvalue(o)->tsv)
#endif /* LUA_WIDESTRING */

#define l_isfalse(o)	(ttisnil(o) || (ttisboolean(o) && bvalue(o) == 0))

/*
** for internal debug only
*/
#if LUA_PACK_VALUE == 0
#define checkconsistency(obj) \
  lua_assert(!iscollectable(obj) || (ttype(obj) == (obj)->value.gc->gch.tt))

#define checkliveness(g,obj) \
  lua_assert(!iscollectable(obj) || \
  ((ttype(obj) == (obj)->value.gc->gch.tt) && !isdead(g, (obj)->value.gc)))
#else
#define checkconsistency(obj) \
  lua_assert(!iscollectable(obj) || (ttype(obj) == (obj)->value.gc->gch.tt))

#define checkliveness(g,obj) \
  lua_assert(!iscollectable(obj) || \
  ((ttype(obj) == (obj)->value.gc->gch.tt) && !isdead(g, (obj)->value.gc)))
#endif

/* Macros to set values */
#if LUA_REFCOUNT
#define setnilvalue(obj) { TValue *i_o=(obj); luarc_release(L, i_o); i_o->tt=LUA_TNIL; }

#define setnvalue(obj,x) \
  { TValue *i_o=(obj); luarc_release(L, i_o); i_o->value.n=(x); i_o->tt=LUA_TNUMBER; }

#define setpvalue(obj,x) \
  { TValue *i_o=(obj); luarc_release(L, i_o); i_o->value.p=(x); i_o->tt=LUA_TLIGHTUSERDATA; }

#define setbvalue(obj,x) \
  { TValue *i_o=(obj); luarc_release(L, i_o); i_o->value.b=(x); i_o->tt=LUA_TBOOLEAN; }

#define setsvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    luarc_makevaluebackup(i_o); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TSTRING; \
    luarc_addreftvalue(i_o); luarc_release(L, &bak); \
    checkliveness(G(L),i_o); }

#define setuvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    luarc_makevaluebackup(i_o); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TUSERDATA; \
    luarc_addreftvalue(i_o); luarc_release(L, &bak); \
    checkliveness(G(L),i_o); }

#define setthvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    luarc_makevaluebackup(i_o); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TTHREAD; \
    luarc_addreftvalue(i_o); luarc_release(L, &bak); \
    checkliveness(G(L),i_o); }

#define setclvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    luarc_makevaluebackup(i_o); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TFUNCTION; \
    luarc_addreftvalue(i_o); luarc_release(L, &bak); \
    checkliveness(G(L),i_o); }

#define sethvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    luarc_makevaluebackup(i_o); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TTABLE; \
    luarc_addreftvalue(i_o); luarc_release(L, &bak); \
    checkliveness(G(L),i_o); }

#define setptvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    luarc_makevaluebackup(i_o); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TPROTO; \
    luarc_addreftvalue(i_o); luarc_release(L, &bak); \
    checkliveness(G(L),i_o); }

#define setobj(L,obj1,obj2) \
  { TValue *o2=(TValue *)(obj2); TValue *o1=(obj1); \
    luarc_addref(o2); luarc_release(L, o1); \
    o1->value = o2->value; o1->tt=o2->tt; \
    checkliveness(G(L),o1); }

#define setobj2n(L,obj1,obj2) \
  { TValue *o2=(TValue *)(obj2); TValue *o1=(obj1); \
    luarc_addref(o2); \
    o1->value = o2->value; o1->tt=o2->tt; \
    checkliveness(G(L),o1); }

#if LUA_WIDESTRING

#define setwsvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    luarc_makevaluebackup(i_o); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TWSTRING; \
    luarc_addreftvalue(i_o); luarc_release(L, &bak); \
    checkliveness(G(L),i_o); }

#endif /* LUA_WIDESTRING */

#define lua_addreftobject(obj)

#define setnvalue2n(obj,x) \
  { TValue *i_o=(obj); i_o->value.n=(x); i_o->tt=LUA_TNUMBER; }

#define setpvalue2n(obj,x) \
  { TValue *i_o=(obj); i_o->value.p=(x); i_o->tt=LUA_TLIGHTUSERDATA; }

#define setbvalue2n(obj,x) \
  { TValue *i_o=(obj); i_o->value.b=(x); i_o->tt=LUA_TBOOLEAN; }

#define setsvalue2n(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TSTRING; \
    luarc_addreftvalue(i_o); \
    checkliveness(G(L),i_o); }

#define setuvalue2n(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TUSERDATA; \
    luarc_addreftvalue(i_o); \
    checkliveness(G(L),i_o); }

#define setthvalue2n(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TTHREAD; \
    luarc_addreftvalue(i_o); \
    checkliveness(G(L),i_o); }

#define setclvalue2n(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TFUNCTION; \
    luarc_addreftvalue(i_o); \
    checkliveness(G(L),i_o); }

#define sethvalue2n(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TTABLE; \
    luarc_addreftvalue(i_o); \
    checkliveness(G(L),i_o); }

#define setptvalue2n(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TPROTO; \
    luarc_addreftvalue(i_o); \
    checkliveness(G(L),i_o); }

#if LUA_WIDESTRING
#define setwsvalue2n(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TWSTRING; \
    luarc_addreftvalue(i_o); \
    checkliveness(G(L),i_o); }
#endif /* LUA_WIDESTRING */

#else /* !LUA_REFCOUNT */

#if LUA_PACK_VALUE == 0

#define setnilvalue(obj) ((obj)->tt=LUA_TNIL)

#if LNUM_PATCH
/* Must not have side effects, 'x' may be expression.
*/
#define setnvalue(obj,x) { TValue *i_o=(obj); i_o->value.n= (x); i_o->tt=LUA_TNUMBER; }

#ifdef LUA_TINT
# define setivalue(obj,x) { TValue *i_o=(obj); i_o->value.i=(x); i_o->tt=LUA_TINT; }
#else
# define setivalue(obj,x) setnvalue(obj,cast_num(x))
#endif

/* Note: Complex always has "inline", both are C99.
*/
#ifdef LNUM_COMPLEX
  static inline void setnvalue_complex_fast( TValue *obj, lua_Complex x ) {
    lua_assert( _LF(cimag)(x) != 0 );
    obj->value.n= x; obj->tt= LUA_TNUMBER;
  }
  static inline void setnvalue_complex( TValue *obj, lua_Complex x ) {
    if (_LF(cimag)(x) == 0) { setnvalue(obj, _LF(creal)(x)); }
    else { obj->value.n= x; obj->tt= LUA_TNUMBER; }
  }
#endif
#else
#define setnvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.n=(x); i_o->tt=LUA_TNUMBER; }
#endif /* LNUM_PATCH */

#define setpvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.p=(x); i_o->tt=LUA_TLIGHTUSERDATA; }

#define setbvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.b=(x); i_o->tt=LUA_TBOOLEAN; }

#define setsvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TSTRING; \
    checkliveness(G(L),i_o); }

#define setuvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TUSERDATA; \
    checkliveness(G(L),i_o); }

#define setthvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TTHREAD; \
    checkliveness(G(L),i_o); }

#define setclvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TFUNCTION; \
    checkliveness(G(L),i_o); }

#define sethvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TTABLE; \
    checkliveness(G(L),i_o); }

#define setptvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TPROTO; \
    checkliveness(G(L),i_o); }




#define setobj(L,obj1,obj2) \
  { const TValue *o2=(obj2); TValue *o1=(obj1); \
    o1->value = o2->value; o1->tt=o2->tt; \
    checkliveness(G(L),o1); }

#if LUA_WIDESTRING
#define setwsvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TWSTRING; \
    checkliveness(G(L),i_o); }
#endif /* LUA_WIDESTRING */

#else /* LUA_PACK_VALUE != 0 */

#define setnilvalue(obj) ( ttype_sig(obj) = add_sig(LUA_TNIL) )

#define setnvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.n=(x); }

#define setpvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.p=(x); i_o->_ts.tt_sig=add_sig(LUA_TLIGHTUSERDATA);}

#define setbvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.b=(x); i_o->_ts.tt_sig=add_sig(LUA_TBOOLEAN);}

#define setsvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->_ts.tt_sig=add_sig(LUA_TSTRING); \
    checkliveness(G(L),i_o); }

#define setuvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->_ts.tt_sig=add_sig(LUA_TUSERDATA); \
    checkliveness(G(L),i_o); }

#define setthvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->_ts.tt_sig=add_sig(LUA_TTHREAD); \
    checkliveness(G(L),i_o); }

#define setclvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->_ts.tt_sig=add_sig(LUA_TFUNCTION); \
    checkliveness(G(L),i_o); }

#define sethvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->_ts.tt_sig=add_sig(LUA_TTABLE); \
    checkliveness(G(L),i_o); }

#define setptvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->_ts.tt_sig=add_sig(LUA_TPROTO); \
    checkliveness(G(L),i_o); }




#define setobj(L,obj1,obj2) \
  { const TValue *o2=(obj2); TValue *o1=(obj1); \
    o1->value = o2->value; \
    checkliveness(G(L),o1); }

#if LUA_WIDESTRING
#define setwsvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->_ts.tt_sig=add_sig(LUA_TWSTRING); \
    checkliveness(G(L),i_o); }
#endif /* LUA_WIDESTRING */

#endif

#endif /* LUA_REFCOUNT */

/*
** different types of sets, according to destination
*/

/* from stack to (same) stack */
#define setobjs2s	setobj
/* to stack (not from same stack) */
#define setobj2s	setobj
#define setsvalue2s	setsvalue
#if LUA_WIDESTRING
#define setwsvalue2s	setwsvalue
#endif /* LUA_WIDESTRING */
#define sethvalue2s	sethvalue
#define setptvalue2s	setptvalue
/* from table to same table */
#define setobjt2t	setobj
/* to table */
#define setobj2t	setobj
/* to new object */
#if LUA_REFCOUNT
#define setnilvalue2n(L,obj) ((obj)->tt=LUA_TNIL)
#else
#define setobj2n	setobj
#define setsvalue2n	setsvalue
#if LUA_WIDESTRING
#define setwsvalue2n	setwsvalue
#endif /* LUA_WIDESTRING */
#endif /* LUA_REFCOUNT */

#if LUA_PACK_VALUE == 0
#define setttype(obj, tt) (ttype(obj) = (tt))
#else
/* considering it used only in lgc to set LUA_TDEADKEY */
/* we could define it this way */
#define setttype(obj, _tt) ( ttype_sig(obj) = add_sig(_tt) )
#endif


#if LNUM_PATCH
#if defined(LUA_TINT) && (LUA_TINT >= LUA_TSTRING)
# define iscollectable(o)	((ttype(o) >= LUA_TSTRING) && (ttype(o) != LUA_TINT))
#else
# define iscollectable(o)	(ttype(o) >= LUA_TSTRING)
#endif
#else
#define iscollectable(o)	(ttype(o) >= LUA_TSTRING)
#endif /* LNUM_PATCH */



typedef TValue *StkId;  /* index to stack elements */


/*
** String headers for string table
*/
typedef union TString {
  L_Umaxalign dummy;  /* ensures maximum alignment for strings */
  struct {
    CommonHeader;
    lu_byte reserved;
    unsigned int hash;
    size_t len;
  } tsv;
} TString;


#define getstr(ts)	cast(const char *, (ts) + 1)
#define svalue(o)       getstr(rawtsvalue(o))
#if LUA_WIDESTRING
#define getwstr(ts)	cast(const lua_WChar *, (ts) + 1)
#define wsvalue(o)      getwstr(rawtsvalue(o))
#endif /* LUA_WIDESTRING */



typedef union Udata {
  L_Umaxalign dummy;  /* ensures maximum alignment for `local' udata */
  struct {
    CommonHeader;
    struct Table *metatable;
    struct Table *env;
    size_t len;
  } uv;
} Udata;




/*
** Function Prototypes
*/
typedef struct Proto {
  CommonHeader;
  TValue *k;  /* constants used by the function */
  Instruction *code;
  struct Proto **p;  /* functions defined inside the function */
  int *lineinfo;  /* map from opcodes to source lines */
  struct LocVar *locvars;  /* information about local variables */
  TString **upvalues;  /* upvalue names */
  TString  *source;
  int sizeupvalues;
  int sizek;  /* size of `k' */
  int sizecode;
  int sizelineinfo;
  int sizep;  /* size of `p' */
  int sizelocvars;
  int linedefined;
  int lastlinedefined;
  GCObject *gclist;
  lu_byte nups;  /* number of upvalues */
  lu_byte numparams;
  lu_byte is_vararg;
  lu_byte maxstacksize;
} Proto;


/* masks for new-style vararg */
#define VARARG_HASARG		1
#define VARARG_ISVARARG		2
#define VARARG_NEEDSARG		4


typedef struct LocVar {
  TString *varname;
  int startpc;  /* first point where variable is active */
  int endpc;    /* first point where variable is dead */
} LocVar;



/*
** Upvalues
*/

typedef struct UpVal {
  CommonHeader;
  TValue *v;  /* points to stack or to its own value */
  union {
    TValue value;  /* the value (when closed) */
    struct {  /* double linked list (when open) */
      struct UpVal *prev;
      struct UpVal *next;
    } l;
  } u;
} UpVal;


/*
** Closures
*/

#define ClosureHeader \
	CommonHeader; lu_byte isC; lu_byte nupvalues; GCObject *gclist; \
	struct Table *env

typedef struct CClosure {
  ClosureHeader;
  lua_CFunction f;
  TValue upvalue[1];
} CClosure;


typedef struct LClosure {
  ClosureHeader;
  struct Proto *p;
  UpVal *upvals[1];
} LClosure;


typedef union Closure {
  CClosure c;
  LClosure l;
} Closure;


#define iscfunction(o)	(ttype(o) == LUA_TFUNCTION && clvalue(o)->c.isC)
#define isLfunction(o)	(ttype(o) == LUA_TFUNCTION && !clvalue(o)->c.isC)


/*
** Tables
*/

#if LUA_PACK_VALUE == 0 

typedef union TKey {
  struct {
    TValuefields;
    struct Node *next;  /* for chaining */
  } nk;
  TValue tvk;
} TKey;

#define LUA_TKEY_NIL {LUA_TVALUE_NIL, NULL}

#else

typedef struct TKey {
  TValue tvk;
  struct {
     struct Node *next; /* for chaining */
  } nk;
} TKey;

#define LUA_TKEY_NIL {LUA_TVALUE_NIL}, {NULL}

#endif

typedef struct Node {
  TValue i_val;
  TKey i_key;
} Node;


typedef struct Table {
  CommonHeader;
  lu_byte flags;  /* 1<<p means tagmethod(p) is not present */ 
  lu_byte lsizenode;  /* log2 of size of `node' array */
  struct Table *metatable;
  TValue *array;  /* array part */
  Node *node;
  Node *lastfree;  /* any free position is before this position */
  GCObject *gclist;
  int sizearray;  /* size of `array' array */
} Table;



/*
** `module' operation for hashing (size is always a power of 2)
*/
#define lmod(s,size) \
	(check_exp((size&(size-1))==0, (cast(int, (s) & ((size)-1)))))


#define twoto(x)	(1<<(x))
#define sizenode(t)	(twoto((t)->lsizenode))


#define luaO_nilobject		(&luaO_nilobject_)

LUAI_DATA const TValue luaO_nilobject_;

#define ceillog2(x)	(luaO_log2((x)-1) + 1)

LUAI_FUNC int luaO_log2 (unsigned int x);
LUAI_FUNC int luaO_int2fb (unsigned int x);
LUAI_FUNC int luaO_fb2int (int x);
LUAI_FUNC int luaO_rawequalObj (const TValue *t1, const TValue *t2);
#if !LNUM_PATCH
LUAI_FUNC int luaO_str2d (const char *s, lua_Number *result);
#endif /* !LNUM_PATCH */
#if LUA_WIDESTRING
LUAI_FUNC int luaO_wstr2d (const lua_WChar *s, lua_Number *result);
#endif /* LUA_WIDESTRING */
LUAI_FUNC const char *luaO_pushvfstring (lua_State *L, const char *fmt,
                                                       va_list argp);
LUAI_FUNC const char *luaO_pushfstring (lua_State *L, const char *fmt, ...);
LUAI_FUNC void luaO_chunkid (char *out, const char *source, size_t len);


#endif

