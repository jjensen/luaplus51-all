INCLUDE(../../../LuaPlusBuildTypes.cmake)

IF (MSVC)
	SET(EXTRA_SRCS sqlite/os_win.c)
ELSE (MSVC)
	SET(EXTRA_SRCS sqlite/os_unix.c)
ENDIF (MSVC)

SET(SRCS
		sqlite/alter.c
		sqlite/analyze.c
		sqlite/attach.c
		sqlite/auth.c
		sqlite/btree.c
		sqlite/btree.h
		sqlite/build.c
		sqlite/callback.c
		sqlite/complete.c
		sqlite/date.c
		sqlite/delete.c
		sqlite/expr.c
#		sqlite/fts1.c
#		sqlite/fts1.h
#		sqlite/fts1_hash.c
#		sqlite/fts1_hash.h
#		sqlite/fts1_porter.c
#		sqlite/fts1_tokenizer.h
#		sqlite/fts1_tokenizer1.c
		sqlite/func.c
		sqlite/hash.c
		sqlite/hash.h
		sqlite/insert.c
		sqlite/keywordhash.h
		sqlite/legacy.c
		sqlite/loadext.c
		sqlite/main.c
		sqlite/opcodes.c
		sqlite/opcodes.h
		sqlite/os.c
		sqlite/os.h
		sqlite/os_common.h
		sqlite/pager.c
		sqlite/pager.h
		sqlite/parse.c
		sqlite/parse.h
		sqlite/pragma.c
		sqlite/prepare.c
		sqlite/printf.c
		sqlite/random.c
		sqlite/select.c
		sqlite/shell.c
		sqlite/sqlite3.h
		sqlite/sqlite3ext.h
		sqlite/sqliteInt.h
		sqlite/table.c
		sqlite/tokenize.c
		sqlite/trigger.c
		sqlite/update.c
		sqlite/utf.c
		sqlite/util.c
		sqlite/vacuum.c
		sqlite/vdbe.c
		sqlite/vdbe.h
		sqlite/vdbeapi.c
		sqlite/vdbeaux.c
		sqlite/vdbefifo.c
		sqlite/vdbeInt.h
		sqlite/vdbemem.c
		sqlite/vtab.c
		sqlite/where.c
		libluasqlite3.c
		${EXTRA_SRCS}
)

COPY_FILE(${CMAKE_CURRENT_SOURCE_DIR}/libluasqlite3-loader.lua ${EXECUTABLE_OUTPUT_PATH}/lua/libluasqlite3-loader.lua SRCS)
COPY_FILE(${CMAKE_CURRENT_SOURCE_DIR}/luasql-sqlite3.lua ${EXECUTABLE_OUTPUT_PATH}/lua/luasql-sqlite3.lua SRCS)
COPY_FILE(${CMAKE_CURRENT_SOURCE_DIR}/lunit.lua ${EXECUTABLE_OUTPUT_PATH}/lua/lunit.lua SRCS)
COPY_FILE(${CMAKE_CURRENT_SOURCE_DIR}/sqlite3.lua ${EXECUTABLE_OUTPUT_PATH}/lua/sqlite3.lua SRCS)

INCLUDE_DIRECTORIES(sqlite)
LUAPLUS_ADD_MODULE(sqlite3 ${SRCS})

