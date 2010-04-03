if (NOT CMAKE_CL_64)

include(../../../LuaPlusBuildTypes.cmake)

if(WIN32)
include_directories(libffi/win32)
endif(WIN32)

if (WIN32)
	set(EXTRA_SRCS
			libffi/win32/ffi.c
			libffi/win32/ffi.h
			libffi/win32/ffi_common.h
			libffi/win32/fficonfig.h
			libffi/win32/ffitarget.h
			libffi/win32/prep_cif.c
			libffi/win32/types.c
			libffi/win32/win32.c
	)
else (WIN32)
	set(EXTRA_SRCS )
endif (WIN32)

SET(SRCS
	src/alien/core.c
	${EXTRA_SRCS}
)

COPY_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/alien.lua ${EXECUTABLE_OUTPUT_PATH}/lua/alien.lua SRCS)

LUAPLUS_ADD_MODULE(alien.core ${SRCS})
get_target_property(MODULE_PREFIX alien.core PREFIX)
set_target_properties(alien.core PROPERTIES PREFIX ${MODULE_PREFIX}alien/)
set_target_properties(alien.core PROPERTIES OUTPUT_NAME core)

LUAPLUS_ADD_MODULE(alien.struct src/alien/struct.c)
get_target_property(MODULE_PREFIX alien.struct PREFIX)
set_target_properties(alien.struct PROPERTIES PREFIX ${MODULE_PREFIX}alien/)
set_target_properties(alien.struct PROPERTIES OUTPUT_NAME struct)

endif(NOT CMAKE_CL_64)
