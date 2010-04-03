
include $(top_srcdir)/build/autotroll.mk

noinst_PROGRAMS = treeview

treeview_SOURCES = treeview.cc treeview.moc.cc treeview.hh
treeview_CXXFLAGS = $(QT_CXXFLAGS) $(AM_CXXFLAGS)
treeview_CPPFLAGS = $(QT_CPPFLAGS) $(AM_CPPFLAGS) -I$(top_srcdir)/src
treeview_LDFLAGS  = $(QT_LDFLAGS) $(LDFLAGS) $(libtool_flags)
treeview_LDADD   = $(QT_LIBS) $(LDADD) $(top_builddir)/src/libqtlua.la

BUILT_SOURCES = treeview.moc.cc

