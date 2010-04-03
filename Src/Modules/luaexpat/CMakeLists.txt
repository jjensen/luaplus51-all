INCLUDE(../../../LuaPlusBuildTypes.cmake)
INCLUDE(CheckSymbolExists)
CHECK_SYMBOL_EXISTS(memmove string.h HAVE_MEMMOVE)
INCLUDE_DIRECTORIES(../expat/lib)
ADD_DEFINITIONS(-DXML_STATIC -DHAVE_MEMMOVE)
IF (MSVC)
	ADD_DEFINITIONS(-DCOMPILED_FROM_DSP)
ENDIF (MSVC)

SET(SRCS
		src/lxplib.c
		src/lxplib.h
		../expat/lib/xmlparse.c
		../expat/lib/xmlrole.c
		../expat/lib/xmltok.c
		../expat/lib/ascii.h
		../expat/lib/asciitab.h
		../expat/lib/expat.h
		../expat/lib/expat_external.h
		../expat/lib/iasciitab.h
		../expat/lib/internal.h
		../expat/lib/latin1tab.h
		../expat/lib/nametab.h
		../expat/lib/utf8tab.h
		../expat/lib/xmlrole.h
		../expat/lib/xmltok.h
		../expat/lib/xmltok_impl.h
)

COPY_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/lxp/lom.lua ${EXECUTABLE_OUTPUT_PATH}/lua/lxp/lom.lua SRCS)

LUAPLUS_ADD_MODULE(luaexpat ${SRCS})
SET_TARGET_PROPERTIES(luaexpat PROPERTIES OUTPUT_NAME lxp)

