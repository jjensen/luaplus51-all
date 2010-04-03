include(../../../LuaPlusBuildTypes.cmake)

set(SRCS
		luainpython.c
		pythoninlua.c
)

include_directories(c:/python25/include)
link_directories(c:/python25/libs)

LUAPLUS_ADD_MODULE(python ${SRCS})


