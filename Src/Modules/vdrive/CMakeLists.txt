INCLUDE(../../../LuaPlusBuildTypes.cmake)
INCLUDE_DIRECTORIES(..)
LUAPLUS_ADD_MODULE(vdrive vdrive.cpp)
TARGET_LINK_LIBRARIES(vdrive Misc)

