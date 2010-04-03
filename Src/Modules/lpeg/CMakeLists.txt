INCLUDE(../../../LuaPlusBuildTypes.cmake)

SET(SRCS
		lpeg.c
)

COPY_FILE(${CMAKE_CURRENT_SOURCE_DIR}/re.lua ${EXECUTABLE_OUTPUT_PATH}/lua/re.lua SRCS)

LUAPLUS_ADD_MODULE(lpeg ${SRCS})

