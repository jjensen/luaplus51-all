include(../../../LuaPlusBuildTypes.cmake)

add_definitions(-DLUASOCKET_EXPORTS)

if (MSVC)
	add_definitions("-DLUASOCKET_API=__declspec(dllexport)")
	set(EXTRA_SRCS src/wsocket.c src/wsocket.h)
else (MSVC)
	set(EXTRA_SRCS src/usocket.c src/usocket.h)
endif (MSVC)

SET(SRCS
	src/auxiliar.c
	src/auxiliar.h
	src/buffer.c
	src/buffer.h
	src/except.c
	src/except.h
	src/inet.c
	src/inet.h
	src/io.c
	src/io.h
	src/luasocket.c
	src/luasocket.h
	src/options.c
	src/options.h
	src/select.c
	src/select.h
	src/socket.h
	src/tcp.c
	src/tcp.h
	src/timeout.c
	src/timeout.h
	src/udp.c
	src/udp.h
	${EXTRA_SRCS}
)

COPY_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/ftp.lua ${EXECUTABLE_OUTPUT_PATH}/lua/socket/ftp.lua SRCS)
COPY_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/http.lua ${EXECUTABLE_OUTPUT_PATH}/lua/socket/http.lua SRCS)
COPY_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/ltn12.lua ${EXECUTABLE_OUTPUT_PATH}/lua/ltn12.lua SRCS)
COPY_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/mime.lua ${EXECUTABLE_OUTPUT_PATH}/lua/mime.lua SRCS)
COPY_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/smtp.lua ${EXECUTABLE_OUTPUT_PATH}/lua/socket/smtp.lua SRCS)
COPY_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/socket.lua ${EXECUTABLE_OUTPUT_PATH}/lua/socket.lua SRCS)
COPY_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/tp.lua ${EXECUTABLE_OUTPUT_PATH}/lua/socket/tp.lua SRCS)
COPY_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/url.lua ${EXECUTABLE_OUTPUT_PATH}/lua/socket/url.lua SRCS)

LUAPLUS_ADD_MODULE(socket ${SRCS})
get_target_property(MODULE_PREFIX socket PREFIX)
set_target_properties(socket PROPERTIES PREFIX ${MODULE_PREFIX}socket/)
set_target_properties(socket PROPERTIES OUTPUT_NAME core)

IF (MSVC)
	target_link_libraries(socket wsock32.lib)
ENDIF (MSVC)

LUAPLUS_ADD_MODULE(mime src/mime.c)
get_target_property(MODULE_PREFIX mime PREFIX)
set_target_properties(mime PROPERTIES PREFIX ${MODULE_PREFIX}mime/)
set_target_properties(mime PROPERTIES OUTPUT_NAME core)
