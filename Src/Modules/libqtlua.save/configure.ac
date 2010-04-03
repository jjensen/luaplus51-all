
AC_INIT(libqtlua, 1.0.0b)

AC_CONFIG_AUX_DIR([build])

AM_INIT_AUTOMAKE(libqtlua, 1.0.0b)
AC_CONFIG_HEADERS(config.hh)

AC_PROG_CXX
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

#AC_LANG(C++)

AT_WITH_QT([core gui], [uitools])

AC_CONFIG_FILES([
  Makefile
    src/Makefile
    src/QtLua/Makefile
    src/internal/Makefile
    tools/Makefile
    tools/qtlua/Makefile
    examples/Makefile
    examples/lua/Makefile
    examples/cpp/Makefile
    examples/cpp/console/Makefile
    examples/cpp/proxy/Makefile
    examples/cpp/qobject/Makefile
    examples/cpp/userdata/Makefile
    examples/cpp/mvc/Makefile
    examples/cpp/value/Makefile
    examples/cpp/plugin/Makefile
    test/Makefile
    doc/Makefile
])

AC_ARG_WITH(lua-inc-dir,
 AC_HELP_STRING(--with-lua-inc-dir, [Specify lua include directory (default /usr/include)]),
 [CPPFLAGS+=" -I$withval"])

AC_ARG_WITH(lua-lib,
 AC_HELP_STRING(--with-lua-lib, [Specify lua library name (default lua)]),
 luaname=$withval, luaname=lua)

AC_ARG_WITH(lualib-lib,
 AC_HELP_STRING(--with-lualib-lib, [Specify lua extra library name (default lualib); not needed for all lua versions]),
 lualibname=$withval, lualibname=lualib)

AC_CHECK_HEADER(lua.h, , AC_MSG_ERROR([can not find lua.h; try --with-lua* arguments]))
AC_CHECK_HEADER(lualib.h, , AC_MSG_ERROR([can not find lualib.h; try --with-lua* arguments]))
AC_CHECK_HEADER(lauxlib.h, , AC_MSG_ERROR([can not find lauxlib.h; try --with-lua* arguments]))

AC_CHECK_LIB($luaname, lua_pushnil, , AC_MSG_ERROR([can not find $luaname library; try --with-lua* arguments]))
AC_CHECK_LIB($luaname, lua_gc, AC_DEFINE(HAVE_LUA_GC, 1, [lua_gc function is available]))

AC_CHECK_LIB($luaname, luaL_loadbuffer, lualibname=$luaname, AC_CHECK_LIB($lualibname, luaL_loadbuffer))

AC_CHECK_LIB($lualibname, luaopen_package, AC_DEFINE(HAVE_LUA_PACKAGELIB, 1, [lua package library is available]))
AC_CHECK_LIB($lualibname, luaopen_os, AC_DEFINE(HAVE_LUA_OSLIB, 1, [lua os library is available]))
AC_CHECK_LIB($lualibname, luaL_openlibs, AC_DEFINE(HAVE_LUA_OPENLIBS, 1, [lua os library is available]))

CXXFLAGS+=" -Wall -Wno-unused-parameter -DQTLUAREF_DEBUG "

eval shared_ext=\"$shrext_cmds\"
AC_DEFINE_UNQUOTED(SHLIBEXT, $shared_ext, [suffix for dynamic libraries is $shared_ext])

AC_ARG_ENABLE(examples, AC_HELP_STRING(--disable-examples, [Disable compilation of examples]), disable_examples=yes)
AM_CONDITIONAL(EXAMPLES, test x$disable_examples != xyes)

AC_OUTPUT

