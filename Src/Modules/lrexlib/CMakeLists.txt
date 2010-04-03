include(../../../LuaPlusBuildTypes.cmake)

set(PCRE_SRCS
		src/pcre/pcre.h
		src/pcre/pcre_chartables.c
		src/pcre/pcre_compile.c
		src/pcre/pcre_config.c
		src/pcre/pcre_dfa_exec.c
		src/pcre/pcre_exec.c
		src/pcre/pcre_fullinfo.c
		src/pcre/pcre_get.c
		src/pcre/pcre_globals.c
		src/pcre/pcre_info.c
		src/pcre/pcre_maketables.c
		src/pcre/pcre_newline.c
		src/pcre/pcre_ord2utf8.c
		src/pcre/pcre_refcount.c
		src/pcre/pcre_study.c
		src/pcre/pcre_tables.c
		src/pcre/pcre_try_flipped.c
		src/pcre/pcre_ucp_searchfuncs.c
		src/pcre/pcre_valid_utf8.c
		src/pcre/pcre_version.c
		src/pcre/pcre_xclass.c
)

set(SRCS
		${PCRE_SRCS}
		src/algo.h
		src/common.c
		src/common.h
		src/lpcre.c
		src/lpcre_f.c
)

source_group(pcre FILES ${PCRE_SRCS})

add_definitions(-DHAVE_CONFIG_H -DPCRE_STATIC)
include_directories(src/pcre)

LUAPLUS_ADD_MODULE(lrexlib ${SRCS})
set_target_properties(lrexlib PROPERTIES OUTPUT_NAME rex_pcre)


