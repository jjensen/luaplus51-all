
include $(top_srcdir)/build/autotroll.mk

noinst_PROGRAMS = iterate global

iterate_SOURCES = iterate.cc
iterate_CXXFLAGS = $(QT_CXXFLAGS) $(AM_CXXFLAGS)
iterate_CPPFLAGS = $(QT_CPPFLAGS) $(AM_CPPFLAGS) -I$(top_srcdir)/src
iterate_LDFLAGS  = $(QT_LDFLAGS) $(LDFLAGS) $(libtool_flags)
iterate_LDADD   = $(QT_LIBS) $(LDADD) $(top_builddir)/src/libqtlua.la

global_SOURCES = global.cc
global_CXXFLAGS = $(QT_CXXFLAGS) $(AM_CXXFLAGS)
global_CPPFLAGS = $(QT_CPPFLAGS) $(AM_CPPFLAGS) -I$(top_srcdir)/src
global_LDFLAGS  = $(QT_LDFLAGS) $(LDFLAGS) $(libtool_flags)
global_LDADD   = $(QT_LIBS) $(LDADD) $(top_builddir)/src/libqtlua.la

#BUILT_SOURCES = 

