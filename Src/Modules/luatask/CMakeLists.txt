INCLUDE(../../../LuaPlusBuildTypes.cmake)

SET(SRCS
		src/LuaTask/ltask.c
		src/LuaTask/queue.c
		src/LuaTask/queue.h
		src/LuaTask/syncos.c
		src/LuaTask/syncos.h
)

IF(WIN32)
ADD_DEFINITIONS(-DNATV_WIN32)
ENDIF(WIN32)

LUAPLUS_ADD_MODULE(task ${SRCS})

