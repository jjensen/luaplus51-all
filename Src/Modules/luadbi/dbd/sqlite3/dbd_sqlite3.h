#include <sqlite3.h>
#include <dbd/common.h>

#define DBD_SQLITE_CONNECTION	"DBD.SQLite3.Connection"
#define DBD_SQLITE_STATEMENT	"DBD.SQLite3.Statement"

/*
 * connection object
 */
typedef struct _connection {
    sqlite3 *sqlite;
    int autocommit;
//    int txn_in_progress;
} connection_t;

/*
 * statement object
 */
typedef struct _statement {
    connection_t *conn;
    sqlite3_stmt *stmt;
    int more_data;
    int affected;
} statement_t;

