include $(top_srcdir)/build/autotroll.mk

noinst_PROGRAMS = plugin_load
lib_LTLIBRARIES = plugin.la

plugin_la_SOURCES = plugin.cc plugin.moc.cc plugin.hh

QT_CPPFLAGS += -I$(top_srcdir)/src

plugin_la_CXXFLAGS = $(QT_CXXFLAGS) $(AM_CXXFLAGS)
plugin_la_CPPFLAGS = $(QT_CPPFLAGS) $(AM_CPPFLAGS)
plugin_la_LDFLAGS  = $(QT_LDFLAGS) $(LDFLAGS) $(libtool_flags) -module
plugin_la_LIBADD   = $(QT_LIBS) $(LIBADD) $(top_builddir)/src/libqtlua.la

plugin_load_SOURCES = plugin_load.cc
plugin_load_CXXFLAGS = $(QT_CXXFLAGS) $(AM_CXXFLAGS)
plugin_load_CPPFLAGS = $(QT_CPPFLAGS) $(AM_CPPFLAGS) -I$(top_srcdir)/src
plugin_load_LDFLAGS  = $(QT_LDFLAGS) $(LDFLAGS) $(libtool_flags)
plugin_load_LDADD   = $(QT_LIBS) $(LDADD) $(top_builddir)/src/libqtlua.la

BUILT_SOURCES = plugin.moc.cc

