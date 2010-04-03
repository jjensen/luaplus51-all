
include $(top_srcdir)/build/autotroll.mk

noinst_PROGRAMS = qobject_iter qobject_owner

qobject_iter_SOURCES = qobject_iter.cc
qobject_iter_CXXFLAGS = $(QT_CXXFLAGS) $(AM_CXXFLAGS)
qobject_iter_CPPFLAGS = $(QT_CPPFLAGS) $(AM_CPPFLAGS) -I$(top_srcdir)/src
qobject_iter_LDFLAGS  = $(QT_LDFLAGS) $(LDFLAGS) $(libtool_flags)
qobject_iter_LDADD   = $(QT_LIBS) $(LDADD) $(top_builddir)/src/libqtlua.la

qobject_owner_SOURCES = qobject_owner.cc
qobject_owner_CXXFLAGS = $(QT_CXXFLAGS) $(AM_CXXFLAGS)
qobject_owner_CPPFLAGS = $(QT_CPPFLAGS) $(AM_CPPFLAGS) -I$(top_srcdir)/src
qobject_owner_LDFLAGS  = $(QT_LDFLAGS) $(LDFLAGS) $(libtool_flags)
qobject_owner_LDADD   = $(QT_LIBS) $(LDADD) $(top_builddir)/src/libqtlua.la

