include(../../../LuaPlusBuildTypes.cmake)

set(SRCS
		dotnet.cpp
		luastdcall.cpp
		luastdcall.h
)

LUAPLUS_ADD_MODULE(luanet ${SRCS})
target_link_libraries(luanet mscoree.lib)

