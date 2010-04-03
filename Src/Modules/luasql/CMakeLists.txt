include(../../../LuaPlusBuildTypes.cmake)

###############################################################################
###############################################################################
SET(SRCS )

COPY_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/ado/ado.lua ${EXECUTABLE_OUTPUT_PATH}/lua/luasql/ado.lua SRCS)

add_custom_target(luasql.ado DEPENDS ${SRCS})

###############################################################################
###############################################################################
SET(SRCS
		src/ls_mysql.c
		src/luasql.c
		src/luasql.h
)

if(MSVC)
	include_directories("C:/Program Files/MySQL/MySQL Server 5.0/include")
	if (CMAKE_CL_64)
		link_directories("C:/Program Files/MySQL/MySQL Server 5.0/lib_amd64/opt")
	else (CMAKE_CL_64)
		link_directories("C:/Program Files/MySQL/MySQL Server 5.0/lib/opt")
	endif (CMAKE_CL_64)
endif(MSVC)

LUAPLUS_ADD_MODULE(luasql.mysql ${SRCS})
if(MSVC)
	target_link_libraries(luasql.mysql libmysql.lib)
endif(MSVC)
get_target_property(MODULE_PREFIX luasql.mysql PREFIX)
set_target_properties(luasql.mysql PROPERTIES PREFIX ${MODULE_PREFIX}luasql/)
set_target_properties(luasql.mysql PROPERTIES OUTPUT_NAME mysql)

###############################################################################
###############################################################################
SET(SRCS
		src/ls_odbc.c
		src/luasql.c
		src/luasql.h
)

LUAPLUS_ADD_MODULE(luasql.odbc ${SRCS})
get_target_property(MODULE_PREFIX luasql.odbc PREFIX)
set_target_properties(luasql.odbc PROPERTIES PREFIX ${MODULE_PREFIX}luasql/)
set_target_properties(luasql.odbc PROPERTIES OUTPUT_NAME odbc)

IF (MSVC)
	target_link_libraries(luasql.odbc odbc32.lib)
ENDIF (MSVC)

###############################################################################
###############################################################################
IF (MSVC)
	SET(EXTRA_SRCS ../sqlite3/sqlite/os_win.c)
ELSE (MSVC)
	SET(EXTRA_SRCS ../sqlite3/sqlite/os_unix.c)
ENDIF (MSVC)

SET(SRCS
		src/ls_sqlite3.c
		src/luasql.c
		src/luasql.h
		../sqlite3/sqlite/alter.c
		../sqlite3/sqlite/analyze.c
		../sqlite3/sqlite/attach.c
		../sqlite3/sqlite/auth.c
		../sqlite3/sqlite/btree.c
		../sqlite3/sqlite/btree.h
		../sqlite3/sqlite/build.c
		../sqlite3/sqlite/callback.c
		../sqlite3/sqlite/complete.c
		../sqlite3/sqlite/date.c
		../sqlite3/sqlite/delete.c
		../sqlite3/sqlite/expr.c
#		../sqlite3/sqlite/fts1.c
#		../sqlite3/sqlite/fts1.h
#		../sqlite3/sqlite/fts1_hash.c
#		../sqlite3/sqlite/fts1_hash.h
#		../sqlite3/sqlite/fts1_porter.c
#		../sqlite3/sqlite/fts1_tokenizer.h
#		../sqlite3/sqlite/fts1_tokenizer1.c
		../sqlite3/sqlite/func.c
		../sqlite3/sqlite/hash.c
		../sqlite3/sqlite/hash.h
		../sqlite3/sqlite/insert.c
		../sqlite3/sqlite/keywordhash.h
		../sqlite3/sqlite/legacy.c
		../sqlite3/sqlite/loadext.c
		../sqlite3/sqlite/main.c
		../sqlite3/sqlite/opcodes.c
		../sqlite3/sqlite/opcodes.h
		../sqlite3/sqlite/os.c
		../sqlite3/sqlite/os.h
		../sqlite3/sqlite/os_common.h
		../sqlite3/sqlite/pager.c
		../sqlite3/sqlite/pager.h
		../sqlite3/sqlite/parse.c
		../sqlite3/sqlite/parse.h
		../sqlite3/sqlite/pragma.c
		../sqlite3/sqlite/prepare.c
		../sqlite3/sqlite/printf.c
		../sqlite3/sqlite/random.c
		../sqlite3/sqlite/select.c
		../sqlite3/sqlite/shell.c
		../sqlite3/sqlite/sqlite3.h
		../sqlite3/sqlite/sqlite3ext.h
		../sqlite3/sqlite/sqliteInt.h
		../sqlite3/sqlite/table.c
		../sqlite3/sqlite/tokenize.c
		../sqlite3/sqlite/trigger.c
		../sqlite3/sqlite/update.c
		../sqlite3/sqlite/utf.c
		../sqlite3/sqlite/util.c
		../sqlite3/sqlite/vacuum.c
		../sqlite3/sqlite/vdbe.c
		../sqlite3/sqlite/vdbe.h
		../sqlite3/sqlite/vdbeapi.c
		../sqlite3/sqlite/vdbeaux.c
		../sqlite3/sqlite/vdbefifo.c
		../sqlite3/sqlite/vdbeInt.h
		../sqlite3/sqlite/vdbemem.c
		../sqlite3/sqlite/vtab.c
		../sqlite3/sqlite/where.c
		${EXTRA_SRCS}
)

include_directories(../sqlite3/sqlite)

LUAPLUS_ADD_MODULE(luasql.sqlite3 ${SRCS})
get_target_property(MODULE_PREFIX luasql.sqlite3 PREFIX)
set_target_properties(luasql.sqlite3 PROPERTIES PREFIX ${MODULE_PREFIX}luasql/)
set_target_properties(luasql.sqlite3 PROPERTIES OUTPUT_NAME sqlite3)

#COPY_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/url.lua ${EXECUTABLE_OUTPUT_PATH}/lua/socket/url.lua SRCS)

