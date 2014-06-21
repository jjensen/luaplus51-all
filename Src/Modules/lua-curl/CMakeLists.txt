project(Lua-cURL C)

cmake_minimum_required(VERSION 2.8)

option(USE_LUA "Use Lua (also called 'C' Lua) version 5.1 includes (default)" ON)
option(USE_LUA52 "Use Lua (also called 'C' Lua) version 5.2 includes (currelty unsupported)")
option(USE_LUAJIT "Use LuaJIT includes instead of 'C' Lua ones (recommended, if you're using LuaJIT, but disabled by default)")


if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(SRC
        src/Lua-cURL.c
        src/Lua-cURL-share.c
        src/Lua-cURL-multi.c
        src/Lua-cURL-callback.c
        src/Lua-cURL-getinfo.c
        src/Lua-cURL-post.c
        src/Lua-cURL-setopt.c
        src/Lua-utility.c
)


if(USE_LUAJIT)
	find_package(LuaJIT REQUIRED)
	set(USE_LUA52 OFF)
	set(USE_LUA OFF)
elseif(USE_LUA52)
	find_package(Lua52 REQUIRED)
	set(USE_LUA OFF)
	set(USE_LUAJIT OFF)
elseif(USE_LUA)
	find_package(Lua51 REQUIRED)
	set(USE_LUA52 OFF)
	set(USE_LUAJIT OFF)
endif()


find_package(CURL REQUIRED)

include_directories(
	${LUA_INCLUDE_DIR}
	${CURL_INCLUDE_DIRS}
)

set(_MODULE_LINK "${CURL_LIBRARY}")
get_filename_component(_lua_lib_dir ${LUA_LIBRARY} PATH)

if(APPLE)
    set(CMAKE_SHARED_MODULE_CREATE_C_FLAGS
        "${CMAKE_SHARED_MODULE_CREATE_C_FLAGS} -undefined dynamic_lookup")
endif()

if(WIN32)
    # Win32 modules need to be linked to the Lua library.
    set(_MODULE_LINK ${LUA_LIBRARY} ${_MODULE_LINK})
    set(_lua_module_dir "${_lua_lib_dir}")
    # Windows sprintf()/strtod() handle NaN/inf differently. Not supported.
    add_definitions(-DDISABLE_INVALID_NUMBERS)
else()
    if (USE_LUA52)
        set(_lua_module_dir "${_lua_lib_dir}/lua/5.2")
    else ()
	set(_lua_module_dir "${_lua_lib_dir}/lua/5.1")
    endif ()
endif()


add_library(cURL MODULE ${SRC})
set_target_properties(cURL PROPERTIES PREFIX "")
target_link_libraries(cURL ${_MODULE_LINK})
install(TARGETS cURL DESTINATION "${_lua_module_dir}")
