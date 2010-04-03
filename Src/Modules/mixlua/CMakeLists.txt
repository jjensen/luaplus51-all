INCLUDE(../../../LuaPlusBuildTypes.cmake)

SET(SRCS
		mixlua.c
)

LUAPLUS_ADD_MODULE(mixlua ${SRCS})

