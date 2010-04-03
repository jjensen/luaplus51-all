INCLUDE(../../../LuaPlusBuildTypes.cmake)

SET(SRCS pipe.cpp)

COPY_FILE(${CMAKE_CURRENT_SOURCE_DIR}/pipe.lua ${EXECUTABLE_OUTPUT_PATH}/lua/pipe.lua SRCS)

LUAPLUS_ADD_MODULE(pipe ${SRCS})

