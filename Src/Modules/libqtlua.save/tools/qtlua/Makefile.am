
include $(top_srcdir)/build/autotroll.mk

bin_PROGRAMS = qtlua

qtlua_SOURCES = qtlua.cc
qtlua_CXXFLAGS = $(QT_CXXFLAGS) $(AM_CXXFLAGS)
qtlua_CPPFLAGS = $(QT_CPPFLAGS) $(AM_CPPFLAGS) -I$(top_srcdir)/src
qtlua_LDFLAGS  = $(QT_LDFLAGS) $(LDFLAGS) $(libtool_flags)
qtlua_LDADD   = $(QT_LIBS) $(LDADD) $(top_builddir)/src/libqtlua.la

