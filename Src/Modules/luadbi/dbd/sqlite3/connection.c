#include "dbd_sqlite3.h"

#if LUA_VERSION_NUM >= 502
#define luaL_register(a, b, c) luaL_setfuncs(a, c, 0)
#endif

int dbd_sqlite3_statement_create(lua_State *L, connection_t *conn, const char *sql_query);

static int run(connection_t *conn, const char *command) {
    int res = sqlite3_exec(conn->sqlite, command, NULL, NULL, NULL);

    return res != SQLITE_OK;
}

static int commit(connection_t *conn) {
    return run(conn, "COMMIT TRANSACTION");
}


static int begin(connection_t *conn) {
    int err = 0;

    if (sqlite3_get_autocommit(conn->sqlite)) {
        err = run(conn, "BEGIN TRANSACTION");
    } else {
        err = 0;
    }

    return err;
}

static int rollback(connection_t *conn) {
    return run(conn, "ROLLBACK TRANSACTION");
}

int try_begin_transaction(connection_t *conn) {
    if (conn->autocommit) {
        return 1;
    }

    return begin(conn) == 0;
}

/* 
 * connection,err = DBD.SQLite3.New(dbfile)
 */
static int connection_new(lua_State *L) {
    int n = lua_gettop(L);

    const char *db = NULL;
    connection_t *conn = NULL;

    /* db */
    switch(n) {
    default:
	/*
	 * db is the only mandatory parameter
	 */
	db = luaL_checkstring(L, 1);
    }

    conn = (connection_t *)lua_newuserdata(L, sizeof(connection_t));

    if (sqlite3_open(db, &conn->sqlite) != SQLITE_OK) {
	lua_pushnil(L);
	lua_pushfstring(L, DBI_ERR_CONNECTION_FAILED, sqlite3_errmsg(conn->sqlite));
	return 2;
    }

    conn->autocommit = 0;

    luaL_getmetatable(L, DBD_SQLITE_CONNECTION);
    lua_setmetatable(L, -2);

    return 1;
}

/*
 * success = connection:autocommit(on)
 */
static int connection_autocommit(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_SQLITE_CONNECTION);
    int on = lua_toboolean(L, 2); 
    int err = 1;

    if (conn->sqlite) {
	if (on) {
	    err = rollback(conn);
        }

	conn->autocommit = on;	
    }

    lua_pushboolean(L, !err);
    return 1;
}


/*
 * success = connection:close()
 */
static int connection_close(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_SQLITE_CONNECTION);
    int disconnect = 0;   

    if (conn->sqlite) {
        rollback(conn);
	sqlite3_close(conn->sqlite);
	disconnect = 1;
	conn->sqlite = NULL;
    }

    lua_pushboolean(L, disconnect);
    return 1;
}

/*
 * success = connection:commit()
 */
static int connection_commit(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_SQLITE_CONNECTION);
    int err = 1;

    if (conn->sqlite) {
	err = commit(conn);
    }

    lua_pushboolean(L, !err);
    return 1;
}

/*
 * ok = connection:ping()
 */
static int connection_ping(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_SQLITE_CONNECTION);
    int ok = 0;   

    if (conn->sqlite) {
	ok = 1;
    }

    lua_pushboolean(L, ok);
    return 1;
}

/*
 * statement,err = connection:prepare(sql_str)
 */
static int connection_prepare(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_SQLITE_CONNECTION);

    if (conn->sqlite) {
	return dbd_sqlite3_statement_create(L, conn, luaL_checkstring(L, 2));
    }

    lua_pushnil(L);    
    lua_pushstring(L, DBI_ERR_DB_UNAVAILABLE);
    return 2;
}

/*
 * quoted = connection:quote(str)
 */
static int connection_quote(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_SQLITE_CONNECTION);
    size_t len;
    const char *from = luaL_checklstring(L, 2, &len);
    char *to;

    if (!conn->sqlite) {
        luaL_error(L, DBI_ERR_DB_UNAVAILABLE);
    }

    to = sqlite3_mprintf("%q", from);

    lua_pushstring(L, to);
    sqlite3_free(to);

    return 1;
}

/*
 * success = connection:rollback()
 */
static int connection_rollback(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_SQLITE_CONNECTION);
    int err = 1;

    if (conn->sqlite) {
	err =rollback(conn);
    }

    lua_pushboolean(L, !err);
    return 1;
}

/*
 * __gc 
 */
static int connection_gc(lua_State *L) {
    /* always close the connection */
    connection_close(L);

    return 0;
}

/*
 * __tostring
 */
static int connection_tostring(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_SQLITE_CONNECTION);

    lua_pushfstring(L, "%s: %p", DBD_SQLITE_CONNECTION, conn);

    return 1;
}

int dbd_sqlite3_connection(lua_State *L) {
    /*
     * instance methods
     */
    static const luaL_Reg connection_methods[] = {
	{"autocommit", connection_autocommit},
	{"close", connection_close},
	{"commit", connection_commit},
	{"ping", connection_ping},
	{"prepare", connection_prepare},
	{"quote", connection_quote},
	{"rollback", connection_rollback},
	{NULL, NULL}
    };

    /*
     * class methods
     */
    static const luaL_Reg connection_class_methods[] = {
	{"New", connection_new},
	{NULL, NULL}
    };

    luaL_newmetatable(L, DBD_SQLITE_CONNECTION);
    luaL_register(L, 0, connection_methods);
    lua_pushvalue(L,-1);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, connection_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, connection_tostring);
    lua_setfield(L, -2, "__tostring");

    luaL_register(L, DBD_SQLITE_CONNECTION, connection_class_methods);

    return 1;    
}
