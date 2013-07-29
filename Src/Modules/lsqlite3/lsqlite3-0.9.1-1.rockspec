package = "lsqlite3"
version = "0.9.1-1"
source = {
    url = "http://lua.sqlite.org/index.cgi/zip/lsqlite3_fsl09b.zip?uuid=fsl_9b",
    file = "lsqlite3_fsl09b.zip"
}
description = {
    summary = "A binding for Lua to the SQLite3 database library",
    detailed = [[
        lsqlite3 is a thin wrapper around the public domain SQLite3 database engine. 
        The lsqlite3 module supports the creation and manipulation of SQLite3 databases. 
        After a require('lsqlite3') the exported functions are called with prefix sqlite3. 
        However, most sqlite3 functions are called via an object-oriented interface to 
        either database or SQL statement objects.
    ]],
    license = "MIT/X11",
    homepage = "http://lua.sqlite.org/"
}
dependencies = {
    "lua >= 5.1"
}
external_dependencies = {
    SQLITE = {
        header = "sqlite3.h"
    }
}
build = {
    type = "builtin",
    modules = {
        lsqlite3 = {
            sources = { "lsqlite3.c" },
            libraries = { "sqlite3" },
            incdirs = { "$(SQLITE_INCDIR)" },
            libdirs = { "$(SQLITE_LIBDIR)" }
        }
    },
	copy_directories = { 'doc', 'examples' }
}
