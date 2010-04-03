
include $(top_srcdir)/build/autotroll.mk

noinst_PROGRAMS = console console_fcn

console_SOURCES = console.cc console.moc.cc console.hh
console_CXXFLAGS = $(QT_CXXFLAGS) $(AM_CXXFLAGS)
console_CPPFLAGS = $(QT_CPPFLAGS) $(AM_CPPFLAGS) -I$(top_srcdir)/src
console_LDFLAGS  = $(QT_LDFLAGS) $(LDFLAGS) $(libtool_flags)
console_LDADD   = $(QT_LIBS) $(LDADD) $(top_builddir)/src/libqtlua.la

console_fcn_SOURCES = console_fcn.cc console.moc.cc console.hh
console_fcn_CXXFLAGS = $(QT_CXXFLAGS) $(AM_CXXFLAGS)
console_fcn_CPPFLAGS = $(QT_CPPFLAGS) $(AM_CPPFLAGS) -I$(top_srcdir)/src
console_fcn_LDFLAGS  = $(QT_LDFLAGS) $(LDFLAGS) $(libtool_flags)
console_fcn_LDADD   = $(QT_LIBS) $(LDADD) $(top_builddir)/src/libqtlua.la

BUILT_SOURCES = console.moc.cc

