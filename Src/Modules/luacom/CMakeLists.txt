include(../../../LuaPlusBuildTypes.cmake)

set(SRCS
		src/library/LuaAux.cpp
		src/library/LuaAux.h
		src/library/luabeans.cpp
		src/library/luabeans.cpp
		src/library/luacom.cpp
		include/luacom.h
		src/dll/luacom_dll.cpp
		src/dll/luacom_dll.h
		src/library/luacom_internal.h
		src/library/LuaCompat.c
		src/library/LuaCompat.h
		src/library/tCOMUtil.cpp
		src/library/tCOMUtil.h
		src/library/tLuaCOM.cpp
		src/library/tLuaCOM.h
		src/library/tLuaCOMClassFactory.cpp
		src/library/tLuaCOMClassFactory.h
		src/library/tLuaCOMConnPoints.cpp
		src/library/tLuaCOMConnPoints.h
		src/library/tLuaCOMEnumerator.cpp
		src/library/tLuaCOMEnumerator.h
		src/library/tLuaCOMException.cpp
		src/library/tLuaCOMException.h
		src/library/tLuaCOMTypeHandler.cpp
		src/library/tLuaCOMTypeHandler.h
		src/library/tLuaControl.cpp
		src/library/tLuaControl.h
		src/library/tLuaDispatch.cpp
		src/library/tLuaDispatch.h
		src/library/tLuaObject.cpp
		src/library/tLuaObject.h
		src/library/tLuaObjList.cpp
		src/library/tLuaObjList.h
		src/library/tLuaTLB.cpp
		src/library/tLuaTLB.h
		src/library/tLuaVector.cpp
		src/library/tLuaVector.h
		src/library/tStringBuffer.cpp
		src/library/tStringBuffer.h
		src/library/tUtil.cpp
		src/library/tUtil.h
)

COPY_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/library/luacom5.lua ${EXECUTABLE_OUTPUT_PATH}/lua/luacom5.lua SRCS)

add_definitions(-DLUA5 "-DLUACOM_DLL=\"luacom.dll\"")
include_directories(include src/library)

LUAPLUS_ADD_MODULE(luacom ${SRCS})
target_link_libraries(luacom shlwapi.lib)
