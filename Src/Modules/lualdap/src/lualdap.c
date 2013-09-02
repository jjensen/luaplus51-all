/*
** LuaLDAP
** See Copyright Notice in license.html
** $Id: lualdap.c,v 1.48 2007-12-14 15:11:22 carregal Exp $
*/

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef WIN32
#include <Winsock2.h>
#else
#include <sys/time.h>
#endif

#ifdef WINLDAP
#include "open2winldap.h"
#else
#include "ldap.h"
#endif

#include <lua.h>
#include <lauxlib.h>

#if LUA_VERSION_NUM < 502
/* lua_rawlen: Not entirely correct, but should work anyway */
#  define lua_rawlen lua_objlen
#  define luaL_newlib(L,l) (lua_newtable(L), luaL_register(L,NULL,l))
#  define luaL_setfuncs(L,l,n) (assert(n==0), luaL_register(L,NULL,l))
#endif

#ifdef WINLDAPAPI
#define timeval l_timeval
typedef ULONG ldap_int_t;
typedef PCHAR ldap_pchar_t;
#else
typedef int ldap_int_t;
typedef const char * ldap_pchar_t;
#endif

#define LUALDAP_PREFIX "LuaLDAP: "
#define LUALDAP_TABLENAME "lualdap"
#define LUALDAP_CONNECTION_METATABLE "LuaLDAP connection"
#define LUALDAP_SEARCH_METATABLE "LuaLDAP search"

#define LUALDAP_MOD_ADD (LDAP_MOD_ADD | LDAP_MOD_BVALUES)
#define LUALDAP_MOD_DEL (LDAP_MOD_DELETE | LDAP_MOD_BVALUES)
#define LUALDAP_MOD_REP (LDAP_MOD_REPLACE | LDAP_MOD_BVALUES)
#define LUALDAP_NO_OP   0

/* Maximum number of attributes manipulated in an operation */
#ifndef LUALDAP_MAX_ATTRS
#define LUALDAP_MAX_ATTRS 100
#endif

/* Size of buffer of NULL-terminated arrays of pointers to struct values */
#ifndef LUALDAP_ARRAY_VALUES_SIZE
#define LUALDAP_ARRAY_VALUES_SIZE (2 * LUALDAP_MAX_ATTRS)
#endif

/* Maximum number of values structures */
#ifndef LUALDAP_MAX_VALUES
#define LUALDAP_MAX_VALUES (LUALDAP_ARRAY_VALUES_SIZE / 2)
#endif


/* LDAP connection information */
typedef struct {
	int        version; /* LDAP version */
	LDAP      *ld;      /* LDAP connection */
} conn_data;


/* LDAP search context information */
typedef struct {
	int      conn;        /* conn_data reference */
	int      msgid;
} search_data;


/* LDAP attribute modification structure */
typedef struct {
	LDAPMod   *attrs[LUALDAP_MAX_ATTRS + 1];
	LDAPMod    mods[LUALDAP_MAX_ATTRS];
	int        ai;
	BerValue  *values[LUALDAP_ARRAY_VALUES_SIZE];
	int        vi;
	BerValue   bvals[LUALDAP_MAX_VALUES];
	int        bi;
} attrs_data;


int luaopen_lualdap (lua_State *L);


/*
** Typical error situation.
*/
static int faildirect (lua_State *L, const char *errmsg) {
    lua_pushnil (L);
    lua_pushstring (L, errmsg);
    return 2;
}


/*
** Get a connection object from the first stack position.
*/
static conn_data *getconnection (lua_State *L) {
	conn_data *conn = (conn_data *)luaL_checkudata (L, 1, LUALDAP_CONNECTION_METATABLE);
	luaL_argcheck(L, conn!=NULL, 1, LUALDAP_PREFIX"LDAP connection expected");
	luaL_argcheck(L, conn->ld, 1, LUALDAP_PREFIX"LDAP connection is closed");
	return conn;
}


/*
** Get a search object from the first upvalue position.
*/
static search_data *getsearch (lua_State *L) {
	/* don't need to check upvalue's integrity */
	search_data *search = (search_data *)lua_touserdata (L, lua_upvalueindex (1));
	luaL_argcheck (L,search->conn!=LUA_NOREF,1,LUALDAP_PREFIX"LDAP search is closed");
	return search;
}


/*
** Set metatable of userdata on top of the stack.
*/
static void lualdap_setmeta (lua_State *L, const char *name) {
	luaL_getmetatable (L, name);
	lua_setmetatable (L, -2);
}


/*
** Error on option.
*/
static int option_error (lua_State *L, const char *name, const char *type) {
	return luaL_error (L, LUALDAP_PREFIX"invalid value on option `%s': %s expected, got %s", name, type, lua_typename (L, lua_type (L, -1)));
}


/*
** Get the field called name of the table at position 2.
*/
static void strgettable (lua_State *L, int idx, const char *name) {
	lua_pushstring (L, name);
	lua_gettable (L, idx);
}


/*
** Get the field named name as a string.
** The table MUST be at position 2.
*/
static const char *strtabparam (lua_State *L, int idx, const char *name, char *def) {
	strgettable (L, idx, name);
	if (lua_isnil (L, -1))
		return def;
	else if (lua_isstring (L, -1))
		return lua_tostring (L, -1);
	else {
		option_error (L, name, "string");
		return NULL;
	}
}


/*
** Get the field named name as an integer.
** The table MUST be at position 2.
*/
static long longtabparam (lua_State *L, int idx, const char *name, int def) {
	strgettable (L, idx, name);
	if (lua_isnil (L, -1))
		return def;
	else if (lua_isnumber (L, -1))
		return (long)lua_tonumber (L, -1);
	else
		return option_error (L, name, "number");
}


/*
** Get the field named name as a double.
** The table MUST be at position 2.
*/
static double numbertabparam (lua_State *L, int idx, const char *name, double def) {
	strgettable (L, idx, name);
	if (lua_isnil (L, -1))
		return def;
	else if (lua_isnumber (L, -1))
		return lua_tonumber (L, -1);
	else
		return option_error (L, name, "number");
}


/*
** Get the field named name as a boolean.
** The table MUST be at position 2.
*/
static int booltabparam (lua_State *L, const char *name, int def) {
	strgettable (L, 2, name);
	if (lua_isnil (L, -1))
		return def;
	else if (lua_isboolean (L, -1))
		return lua_toboolean (L, -1);
	else
		return option_error (L, name, "boolean");
}


/*
** Error on attribute's value.
*/
static void value_error (lua_State *L, const char *name) {
	luaL_error (L, LUALDAP_PREFIX"invalid value of attribute `%s' (%s)",
		name, lua_typename (L, lua_type (L, -1)));
}


/*
** Initialize attributes structure.
*/
static void A_init (attrs_data *attrs) {
	attrs->ai = 0;
	attrs->attrs[0] = NULL;
	attrs->vi = 0;
	attrs->values[0] = NULL;
	attrs->bi = 0;
}


/*
** Store the string on top of the stack on the attributes structure.
** Increment the bvals counter.
*/
static BerValue *A_setbval (lua_State *L, attrs_data *a, const char *n) {
	BerValue *ret = &(a->bvals[a->bi]);
	if (a->bi >= LUALDAP_MAX_VALUES) {
		luaL_error (L, LUALDAP_PREFIX"too many values");
		return NULL;
	} else if (!lua_isstring (L, -1)) {
		value_error (L, n);
		return NULL;
	}
	a->bvals[a->bi].bv_len = lua_rawlen (L, -1);
	a->bvals[a->bi].bv_val = (char *)lua_tostring (L, -1);
	a->bi++;
	return ret;
}


/*
** Store a pointer to the value on top of the stack on the attributes structure.
*/
static BerValue **A_setval (lua_State *L, attrs_data *a, const char *n) {
	BerValue **ret = &(a->values[a->vi]);
	if (a->vi >= LUALDAP_ARRAY_VALUES_SIZE) {
		luaL_error (L, LUALDAP_PREFIX"too many values");
		return NULL;
	}
	a->values[a->vi] = A_setbval (L, a, n);
	a->vi++;
	return ret;
}


/*
** Store a NULL pointer on the attributes structure.
*/
static BerValue **A_nullval (lua_State *L, attrs_data *a) {
	BerValue **ret = &(a->values[a->vi]);
	if (a->vi >= LUALDAP_ARRAY_VALUES_SIZE) {
		luaL_error (L, LUALDAP_PREFIX"too many values");
		return NULL;
	}
	a->values[a->vi] = NULL;
	a->vi++;
	return ret;
}


/*
** Store the value of an attribute.
** Valid values are:
**	true => no values;
**	string => one value; or
**	table of strings => many values.
*/
static BerValue **A_tab2val (lua_State *L, attrs_data *a, const char *name) {
	int tab = lua_gettop (L);
	BerValue **ret = &(a->values[a->vi]);
	if (lua_isboolean (L, tab) && (lua_toboolean (L, tab) == 1)) /* true */
		return NULL;
	else if (lua_isstring (L, tab)) /* string */
		A_setval (L, a, name);
	else if (lua_istable (L, tab)) { /* list of strings */
		int i;
		int n = lua_rawlen (L, tab);
		for (i = 1; i <= n; i++) {
			lua_rawgeti (L, tab, i); /* push table element */
			A_setval (L, a, name);
		}
		lua_pop (L, n);
	} else {
		value_error (L, name);
		return NULL;
	}
	A_nullval (L, a);
	return ret;
}


/*
** Set a modification value (which MUST be on top of the stack).
*/
static void A_setmod (lua_State *L, attrs_data *a, int op, const char *name) {
	if (a->ai >= LUALDAP_MAX_ATTRS) {
		luaL_error (L, LUALDAP_PREFIX"too many attributes");
		return;
	}
	a->mods[a->ai].mod_op = op;
	a->mods[a->ai].mod_type = (char *)name;
	a->mods[a->ai].mod_bvalues = A_tab2val (L, a, name);
	a->attrs[a->ai] = &a->mods[a->ai];
	a->ai++;
}


/*
** Convert a Lua table into an array of modifications.
** An array of modifications is a NULL-terminated array of LDAPMod's.
*/
static void A_tab2mod (lua_State *L, attrs_data *a, int tab, int op) {
	lua_pushnil (L); /* first key for lua_next */
	while (lua_next (L, tab) != 0) {
		/* attribute must be a string and not a number */
		if ((!lua_isnumber (L, -2)) && (lua_isstring (L, -2)))
			A_setmod (L, a, op, lua_tostring (L, -2));
		/* pop value and leave last key on the stack as next key for lua_next */
		lua_pop (L, 1);
	}
}


/*
** Terminate the array of attributes.
*/
static void A_lastattr (lua_State *L, attrs_data *a) {
	if (a->ai >= LUALDAP_MAX_ATTRS) {
		luaL_error (L, LUALDAP_PREFIX"too many attributes");
		return;
	}
	a->attrs[a->ai] = NULL;
	a->ai++;
}


/*
** Copy a string or a table of strings from Lua to a NULL-terminated array
** of C-strings.
*/
static int table2strarray (lua_State *L, int tab, char *array[], int limit) {
	if (lua_isstring (L, tab)) {
		if (limit < 2)
			return luaL_error (L, LUALDAP_PREFIX"too many arguments");
		array[0] = (char *)lua_tostring (L, tab);
		array[1] = NULL;
	} else if (lua_istable (L, tab)) {
		int i;
		int n = lua_rawlen (L, tab);
		if (limit < (n+1))
			return luaL_error (L, LUALDAP_PREFIX"too many arguments");
		for (i = 0; i < n; i++) {
			lua_rawgeti (L, tab, i+1); /* push table element */
			if (lua_isstring (L, -1))
				array[i] = (char *)lua_tostring (L, -1);
			else {
				return luaL_error (L, LUALDAP_PREFIX"invalid value #%d", i+1);
			}
		}
		array[n] = NULL;
	} else 
		return luaL_error (L, LUALDAP_PREFIX"bad argument #%d (table or string expected, got %s)", tab, lua_typename (L, lua_type (L, tab)));
	return 0;
}


/*
** Fill in the struct timeval, according to the timeout parameter.
*/
static struct timeval *get_timeout_param (lua_State *L, int idx, struct timeval *st) {
	double t = numbertabparam (L, idx, "timeout", -1);
	if(t < 0)
		return NULL; /* No timeout, block */
	st->tv_sec = (long)t;
	st->tv_usec = (long)(1000000 * (t - st->tv_sec));
	return st;
}

/*
** Get the result message of an operation.
** #1 upvalue == connection
** #2 upvalue == msgid
** #3 upvalue == result code of the message (ADD, DEL etc.) to be received.
*/
static int result_message (lua_State *L) {
	struct timeval timeout;
	LDAPMessage *res;
	int rc;
	conn_data *conn = (conn_data *)lua_touserdata (L, lua_upvalueindex (1));
	int msgid = (int)lua_tonumber (L, lua_upvalueindex (2));
	/*int res_code = (int)lua_tonumber (L, lua_upvalueindex (3));*/

	luaL_argcheck (L, conn->ld, 1, LUALDAP_PREFIX"LDAP connection is closed");
	rc = ldap_result (conn->ld, msgid, LDAP_MSG_ONE, get_timeout_param (L, 1, &timeout), &res);
	if (rc == 0)
		return faildirect (L, LUALDAP_PREFIX"result timeout expired");
	else if (rc < 0) {
		ldap_msgfree (res);
		return faildirect (L, LUALDAP_PREFIX"result error");
	} else {
		int err, ret = 1;
		char *mdn, *msg;
		rc = ldap_parse_result (conn->ld, res, &err, &mdn, &msg, NULL, NULL, 1);
		if (rc != LDAP_SUCCESS)
			return faildirect (L, ldap_err2string (rc));
		switch (err) {
			case LDAP_SUCCESS:
			case LDAP_COMPARE_TRUE:
				lua_pushboolean (L, 1);
				break;
			case LDAP_COMPARE_FALSE:
				lua_pushboolean (L, 0);
				break;
			default:
				lua_pushnil (L);
				lua_pushliteral (L, LUALDAP_PREFIX);
				lua_pushstring (L, ldap_err2string(err));
				lua_concat (L, 2);
				if (msg != NULL) {
					lua_pushliteral (L, " (");
					lua_pushstring (L, msg);
					lua_pushliteral (L, ")");
					lua_concat (L, 4);
				}
				ret = 2;
		}
		ldap_memfree (mdn);
		ldap_memfree (msg);
		return ret;
	}
}


/*
** Push a function to process the LDAP result.
*/
static int create_future (lua_State *L, ldap_int_t rc, int conn, ldap_int_t msgid, int code) {
	if (rc != LDAP_SUCCESS)
		return faildirect (L, ldap_err2string (rc));
	lua_pushvalue (L, conn); /* push connection as #1 upvalue */
	lua_pushnumber (L, msgid); /* push msgid as #2 upvalue */
	lua_pushnumber (L, code); /* push code as #3 upvalue */
	lua_pushcclosure (L, result_message, 3);
	return 1;
}


/*
** Unbind from the directory.
** @param #1 LDAP connection.
** @return 1 in case of success; nothing when already closed.
*/
static int lualdap_close (lua_State *L) {
	conn_data *conn = (conn_data *)luaL_checkudata (L, 1, LUALDAP_CONNECTION_METATABLE);
	luaL_argcheck(L, conn!=NULL, 1, LUALDAP_PREFIX"LDAP connection expected");
	if (conn->ld == NULL) /* already closed */
		return 0;
#if defined(LDAP_API_FEATURE_X_OPENLDAP) && LDAP_API_FEATURE_X_OPENLDAP >= 20300
	ldap_unbind_ext (conn->ld, NULL, NULL);
#else
	ldap_unbind (conn->ld);
#endif
	conn->ld = NULL;
	lua_pushnumber (L, 1);
	return 1;
}


/*
** Add a new entry to the directory.
** @param #1 LDAP connection.
** @param #2 String with new entry's DN.
** @param #3 Table with new entry's attributes and values.
** @return Function to process the LDAP result.
*/
static int lualdap_add (lua_State *L) {
	conn_data *conn = getconnection (L);
	ldap_pchar_t dn = (ldap_pchar_t) luaL_checkstring (L, 2);
	attrs_data attrs;
	ldap_int_t rc, msgid;
	A_init (&attrs);
	if (lua_istable (L, 3))
		A_tab2mod (L, &attrs, 3, LUALDAP_MOD_ADD);
	A_lastattr (L, &attrs);
	rc = ldap_add_ext (conn->ld, dn, attrs.attrs, NULL, NULL, &msgid);
	return create_future (L, rc, 1, msgid, LDAP_RES_ADD);
}


/*
** Compare a value against an entry.
** @param #1 LDAP connection.
** @param #2 String with entry's DN.
** @param #3 String with attribute's name.
** @param #4 String with attribute's value.
** @return Function to process the LDAP result.
*/
static int lualdap_compare (lua_State *L) {
	conn_data *conn = getconnection (L);
	ldap_pchar_t dn = (ldap_pchar_t) luaL_checkstring (L, 2);
	ldap_pchar_t attr = (ldap_pchar_t) luaL_checkstring (L, 3);
	BerValue bvalue;
	ldap_int_t rc, msgid;
	bvalue.bv_val = (char *)luaL_checkstring (L, 4);
	bvalue.bv_len = lua_rawlen (L, 4);
	rc = ldap_compare_ext (conn->ld, dn, attr, &bvalue, NULL, NULL, &msgid);
	return create_future (L, rc, 1, msgid, LDAP_RES_COMPARE);
}


/*
** Delete an entry.
** @param #1 LDAP connection.
** @param #2 String with entry's DN.
** @return Boolean.
*/
static int lualdap_delete (lua_State *L) {
	conn_data *conn = getconnection (L);
	ldap_pchar_t dn = (ldap_pchar_t) luaL_checkstring (L, 2);
	ldap_int_t rc, msgid;
	rc = ldap_delete_ext (conn->ld, dn, NULL, NULL, &msgid);
	return create_future (L, rc, 1, msgid, LDAP_RES_DELETE);
}


/*
** Convert a string into an internal LDAP_MOD operation code.
*/
static int op2code (const char *s) {
	if (!s)
		return LUALDAP_NO_OP;
	switch (*s) {
		case '+':
			return LUALDAP_MOD_ADD;
		case '-':
			return LUALDAP_MOD_DEL;
		case '=':
			return LUALDAP_MOD_REP;
		default:
			return LUALDAP_NO_OP;
	}
}


/*
** Modify an entry.
** @param #1 LDAP connection.
** @param #2 String with entry's DN.
** @param #3, #4... Tables with modifications to apply.
** @return True on success or nil, error message otherwise.
*/
static int lualdap_modify (lua_State *L) {
	conn_data *conn = getconnection (L);
	ldap_pchar_t dn = (ldap_pchar_t) luaL_checkstring (L, 2);
	attrs_data attrs;
	ldap_int_t rc, msgid;
	int param = 3;
	A_init (&attrs);
	while (lua_istable (L, param)) {
		int op;
		/* get operation ('+','-','=' operations allowed) */
		lua_rawgeti (L, param, 1);
		op = op2code (lua_tostring (L, -1));
		if (op == LUALDAP_NO_OP)
			return luaL_error (L, LUALDAP_PREFIX"forgotten operation on argument #%d", param);
		/* get array of attributes and values */
		A_tab2mod (L, &attrs, param, op);
		param++;
	}
	A_lastattr (L, &attrs);
	rc = ldap_modify_ext (conn->ld, dn, attrs.attrs, NULL, NULL, &msgid);
	return create_future (L, rc, 1, msgid, LDAP_RES_MODIFY);
}


/*
** Change the distinguished name of an entry.
*/
static int lualdap_rename (lua_State *L) {
	conn_data *conn = getconnection (L);
	ldap_pchar_t dn = (ldap_pchar_t) luaL_checkstring (L, 2);
	ldap_pchar_t rdn = (ldap_pchar_t) luaL_checkstring (L, 3);
	ldap_pchar_t par = (ldap_pchar_t) luaL_optlstring (L, 4, NULL, NULL);
	const int del = luaL_optnumber (L, 5, 0);
	ldap_int_t msgid;
	ldap_int_t rc = ldap_rename (conn->ld, dn, rdn, par, del, NULL, NULL, &msgid);
	return create_future (L, rc, 1, msgid, LDAP_RES_MODDN);
}


/*
** Push an attribute value (or a table of values) on top of the stack.
** @param L lua_State.
** @param ld LDAP Connection.
** @param entry Current entry.
** @param attr Name of entry's attribute to get values from.
** @return 1 in case of success.
*/
static int push_values (lua_State *L, LDAP *ld, LDAPMessage *entry, char *attr) {
	int i, n;
	BerValue **vals = ldap_get_values_len (ld, entry, attr);
	n = ldap_count_values_len (vals);
	if (n == 0) /* no values */
		lua_pushboolean (L, 1);
	else if (n == 1) /* just one value */
		lua_pushlstring (L, vals[0]->bv_val, vals[0]->bv_len);
	else { /* Multiple values */
		lua_newtable (L);
		for (i = 0; i < n; i++) {
			lua_pushlstring (L, vals[i]->bv_val, vals[i]->bv_len);
			lua_rawseti (L, -2, i+1);
		}
	}
	ldap_value_free_len (vals);
	return 1;
}


/*
** Store entry's attributes and values at the given table.
** @param entry Current entry.
** @param tab Absolute stack index of the table.
*/
static void set_attribs (lua_State *L, LDAP *ld, LDAPMessage *entry, int tab) {
	char *attr;
	BerElement *ber = NULL;
	for (attr = ldap_first_attribute (ld, entry, &ber);
		attr != NULL;
		attr = ldap_next_attribute (ld, entry, ber))
	{
		lua_pushstring (L, attr);
		push_values (L, ld, entry, attr);
		lua_rawset (L, tab); /* tab[attr] = vals */
		ldap_memfree (attr);
	}
	ber_free (ber, 0); /* don't need to test if (ber == NULL) */
}


/*
** Get the distinguished name of the given entry and pushes it on the stack.
*/
static void push_dn (lua_State *L, LDAP *ld, LDAPMessage *entry) {
	char *dn = ldap_get_dn (ld, entry);
	lua_pushstring (L, dn);
	ldap_memfree (dn);
}


/*
** Release connection reference.
*/
static void search_close (lua_State *L, search_data *search) {
	luaL_unref (L, LUA_REGISTRYINDEX, search->conn);
	search->conn = LUA_NOREF;
}


/*
** Retrieve next message...
** @return #1 entry's distinguished name.
** @return #2 table with entry's attributes and values.
*/
static int next_message (lua_State *L) {
	search_data *search = getsearch (L);
	conn_data *conn;
	struct timeval timeout;
	LDAPMessage *res;
	int rc;
	int ret;

	luaL_checktype(L, 1, LUA_TTABLE);

	lua_rawgeti (L, LUA_REGISTRYINDEX, search->conn);
	conn = (conn_data *)lua_touserdata (L, -1); /* get connection */

	rc = ldap_result (conn->ld, search->msgid, LDAP_MSG_ONE, get_timeout_param (L, 1, &timeout), &res);
	if (rc == 0)
		return faildirect (L, LUALDAP_PREFIX"result timeout expired");
	else if (rc == -1)
		return faildirect (L, LUALDAP_PREFIX"result error");
	else if (rc == LDAP_RES_SEARCH_RESULT) { /* last message => nil */
		/* close search object to avoid reuse */
		search_close (L, search);
		ret = 0;
	} else {
		LDAPMessage *msg = ldap_first_message (conn->ld, res);
		switch (ldap_msgtype (msg)) {
			case LDAP_RES_SEARCH_ENTRY: {
				LDAPMessage *entry = ldap_first_entry (conn->ld, msg);
				push_dn (L, conn->ld, entry);
				lua_newtable (L);
				set_attribs (L, conn->ld, entry, lua_gettop (L));
				ret = 2; /* two return values */
				break;
			}
/*No reference to LDAP_RES_SEARCH_REFERENCE on MSDN. Maybe there is a replacement to it?*/
#ifdef LDAP_RES_SEARCH_REFERENCE
			case LDAP_RES_SEARCH_REFERENCE: {
				LDAPMessage *ref = ldap_first_reference (conn->ld, msg);
				push_dn (L, conn->ld, ref); /* is this supposed to work? */
				lua_pushnil (L);
				ret = 2; /* two return values */
				break;
			}
#endif
			case LDAP_RES_SEARCH_RESULT:
				/* close search object to avoid reuse */
				search_close (L, search);
				ret = 0;
				break;
			default:
				ldap_msgfree (res);
				return luaL_error (L, LUALDAP_PREFIX"error on search result chain");
		}
	}
	ldap_msgfree (res);
	return ret;
}


/*
** Convert a string to one of the possible scopes of the search.
*/
static int string2scope (lua_State *L, const char *s) {
	if ((s == NULL) || (*s == '\0'))
		return LDAP_SCOPE_DEFAULT;
	switch (*s) {
		case 'b':
			return LDAP_SCOPE_BASE;
		case 'o':
			return LDAP_SCOPE_ONELEVEL;
		case 's':
			return LDAP_SCOPE_SUBTREE;
		default:
			return luaL_error (L, LUALDAP_PREFIX"invalid search scope `%s'", s);
	}
}


/*
** Close the search object.
*/
static int lualdap_search_close (lua_State *L) {
	search_data *search = (search_data *)luaL_checkudata (L, 1, LUALDAP_SEARCH_METATABLE);
	luaL_argcheck (L, search!=NULL, 1, LUALDAP_PREFIX"LDAP search expected");
	if (search->conn == LUA_NOREF)
		return 0;
	search_close (L, search);
	lua_pushnumber (L, 1);
	return 1;
}


/*
** Create a search object and leaves it on top of the stack.
*/
static void create_search (lua_State *L, int conn_index, int msgid) {
	search_data *search = (search_data *)lua_newuserdata (L, sizeof (search_data));
	lualdap_setmeta (L, LUALDAP_SEARCH_METATABLE);
	search->conn = LUA_NOREF;
	search->msgid = msgid;
	lua_pushvalue (L, conn_index);
	search->conn = luaL_ref (L, LUA_REGISTRYINDEX);
}


/*
** Fill in the attrs array, according to the attrs parameter.
*/
static int get_attrs_param (lua_State *L, char *attrs[]) {
	lua_pushstring (L, "attrs");
	lua_gettable (L, 2);
	if (lua_isstring (L, -1)) {
		attrs[0] = (char *)lua_tostring (L, -1);
		attrs[1] = NULL;
	} else if (!lua_istable (L, -1))
		attrs[0] = NULL;
	else
		if (table2strarray (L, lua_gettop (L), attrs, LUALDAP_MAX_ATTRS))
			return 0;
	return 1;
}


/*
** Perform a search operation.
** @return #1 Function to iterate over the result entries.
** @return #2 nil.
** @return #3 nil as first entry.
** The search result is defined as an upvalue of the iterator.
*/
static int lualdap_search (lua_State *L) {
	conn_data *conn = getconnection (L);
	ldap_pchar_t base;
	ldap_pchar_t filter;
	char *attrs[LUALDAP_MAX_ATTRS];
	int scope, attrsonly, msgid, rc, sizelimit;
	struct timeval st, *timeout;

	if (!lua_istable (L, 2))
		return luaL_error (L, LUALDAP_PREFIX"no search specification");
	if (!get_attrs_param (L, attrs))
		return 2;
	/* get other parameters */
	attrsonly = booltabparam (L, "attrsonly", 0);
	base = (ldap_pchar_t) strtabparam (L, 2, "base", NULL);
	filter = (ldap_pchar_t) strtabparam (L, 2, "filter", NULL);
	scope = string2scope (L, strtabparam (L, 2, "scope", NULL));
	sizelimit = longtabparam (L, 2, "sizelimit", LDAP_NO_LIMIT);
	timeout = get_timeout_param (L, 2, &st);

	rc = ldap_search_ext (conn->ld, base, scope, filter, attrs, attrsonly,
		NULL, NULL, timeout, sizelimit, &msgid);
	if (rc != LDAP_SUCCESS)
		return luaL_error (L, LUALDAP_PREFIX"%s", ldap_err2string (rc));

	create_search (L, 1, msgid);
	lua_pushcclosure (L, next_message, 1);
	lua_pushvalue(L, 2);
	return 2;
}


/*
** Return the name of the object's metatable.
** This function is used by `tostring'.
*/
static int lualdap_conn_tostring (lua_State *L) {
	char buff[100];
	conn_data *conn = (conn_data *)lua_touserdata (L, 1);
	if (conn->ld == NULL)
		strcpy (buff, "closed");
	else
		sprintf (buff, "%p", conn);
	lua_pushfstring (L, "%s (%s)", LUALDAP_CONNECTION_METATABLE, buff);
	return 1;
}


/*
** Return the name of the object's metatable.
** This function is used by `tostring'.
*/
static int lualdap_search_tostring (lua_State *L) {
	char buff[100];
	search_data *search = (search_data *)lua_touserdata (L, 1);
	luaL_argcheck (L,search->conn!=LUA_NOREF,1,LUALDAP_PREFIX"LDAP search is closed");
	if (search->conn == LUA_NOREF)
		strcpy (buff, "closed");
	else
		sprintf (buff, "%p", search);
	lua_pushfstring (L, "%s (%s)", LUALDAP_SEARCH_METATABLE, buff);
	return 1;
}


/*
** Create a metatable.
*/
static int lualdap_createmeta (lua_State *L) {
	const luaL_Reg methods[] = {
		{"close", lualdap_close},
		{"add", lualdap_add},
		{"compare", lualdap_compare},
		{"delete", lualdap_delete},
		{"modify", lualdap_modify},
		{"rename", lualdap_rename},
		{"search", lualdap_search},
		{NULL, NULL}
	};

	if (!luaL_newmetatable (L, LUALDAP_CONNECTION_METATABLE))
		return 0;

	/* define methods */
	luaL_setfuncs(L, methods, 0);

	/* define metamethods */
	lua_pushliteral (L, "__gc");
	lua_pushcfunction (L, lualdap_close);
	lua_settable (L, -3);

	lua_pushliteral (L, "__index");
	lua_pushvalue (L, -2);
	lua_settable (L, -3);

	lua_pushliteral (L, "__tostring");
	lua_pushcfunction (L, lualdap_conn_tostring);
	lua_settable (L, -3);

	lua_pushliteral (L, "__metatable");
	lua_pushliteral(L,LUALDAP_PREFIX"you're not allowed to get this metatable");
	lua_settable (L, -3);

	if (!luaL_newmetatable (L, LUALDAP_SEARCH_METATABLE))
		return 0;

	lua_pushliteral (L, "__gc");
	lua_pushcfunction (L, lualdap_search_close);
	lua_settable (L, -3);

	lua_pushliteral (L, "__tostring");
	lua_pushcclosure (L, lualdap_search_tostring, 1);
	lua_settable (L, -3);

	lua_pushliteral (L, "__metatable");
	lua_pushliteral(L,LUALDAP_PREFIX"you're not allowed to get this metatable");
	lua_settable (L, -3);

	return 0;
}


/*
** Open and initialize a connection to a server.
** @param #1 String with hostname.
** @param #2 String with username.
** @param #3 String with password.
** @param #4 Boolean indicating if TLS must be used.
** @return #1 Userdata with connection structure.
*/
static int lualdap_open_simple (lua_State *L) {
	ldap_pchar_t host = (ldap_pchar_t) luaL_checkstring (L, 1);
	ldap_pchar_t who = (ldap_pchar_t) luaL_optstring (L, 2, NULL);
	const char *password = luaL_optstring (L, 3, NULL);
	int use_tls = lua_toboolean (L, 4);
	conn_data *conn = (conn_data *)lua_newuserdata (L, sizeof(conn_data));
#if defined(LDAP_API_FEATURE_X_OPENLDAP) && LDAP_API_FEATURE_X_OPENLDAP >= 20300
	struct berval cred = { 0, NULL };
	char *host_with_schema = NULL;
#endif
	int err;

	/* Initialize */
	lualdap_setmeta (L, LUALDAP_CONNECTION_METATABLE);
	conn->version = 0;
#if defined(LDAP_API_FEATURE_X_OPENLDAP) && LDAP_API_FEATURE_X_OPENLDAP >= 20300
	host_with_schema = malloc(strlen(host) + 8);
	strcpy(host_with_schema, "ldap://");
	strcat(host_with_schema, host);
	err = ldap_initialize(&conn->ld, host_with_schema);
	free(host_with_schema);
	host_with_schema = NULL;
	if (err != LDAP_SUCCESS)
#else
	conn->ld = ldap_init (host, LDAP_PORT);
	if (conn->ld == NULL)
#endif
		return faildirect(L,LUALDAP_PREFIX"Error connecting to server");
	/* Set protocol version */
	conn->version = LDAP_VERSION3;
	if (ldap_set_option (conn->ld, LDAP_OPT_PROTOCOL_VERSION, &conn->version)
		!= LDAP_OPT_SUCCESS)
		return faildirect(L, LUALDAP_PREFIX"Error setting LDAP version");
	/* Use TLS */
	if (use_tls) {
		int rc = ldap_start_tls_s (conn->ld, NULL, NULL);
		if (rc != LDAP_SUCCESS)
			return faildirect (L, ldap_err2string (rc));
	}
	/* Bind to a server */
#if defined(LDAP_API_FEATURE_X_OPENLDAP) && LDAP_API_FEATURE_X_OPENLDAP >= 20300
	cred.bv_len = strlen(password);
	cred.bv_val = malloc(cred.bv_len+1);
	strcpy(cred.bv_val, password);
	err = ldap_sasl_bind_s (conn->ld, who, LDAP_SASL_SIMPLE, &cred, NULL, NULL, NULL);
	free(cred.bv_val);
	memset(&cred, 0, sizeof(cred));
#else
	err = ldap_bind_s (conn->ld, who, password, LDAP_AUTH_SIMPLE);
#endif
	if (err != LDAP_SUCCESS)
		return faildirect (L, ldap_err2string (err));

	return 1;
}


/*
** Assumes the table is on top of the stack.
*/
static void set_info (lua_State *L) {
	lua_pushliteral (L, "_COPYRIGHT");
	lua_pushliteral (L, "Copyright (C) 2003-2007 Kepler Project");
	lua_settable (L, -3);
	lua_pushliteral (L, "_DESCRIPTION");
	lua_pushliteral (L, "LuaLDAP is a simple interface from Lua to an LDAP client");
	lua_settable (L, -3);
	lua_pushliteral (L, "_VERSION");
	lua_pushliteral (L, "LuaLDAP 1.1.1");
	lua_settable (L, -3);
}


/*
** Create ldap table and register the open method.
*/
int luaopen_lualdap (lua_State *L) {
	struct luaL_Reg lualdap[] = {
		{"open_simple", lualdap_open_simple},
		{NULL, NULL},
	};

	lualdap_createmeta (L);
	luaL_newlib(L, lualdap);
	lua_pushvalue(L, -1);
	lua_setglobal(L, LUALDAP_TABLENAME);
	set_info (L);

	return 1;
}
