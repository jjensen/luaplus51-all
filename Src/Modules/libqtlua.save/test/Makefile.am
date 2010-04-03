include $(top_srcdir)/build/autotroll.mk

noinst_PROGRAMS = test_value test_table test_qobject_arg test_item

test_value_SOURCES = test_value.cc test.hh
test_value_CXXFLAGS = $(QT_CXXFLAGS) $(AM_CXXFLAGS)
test_value_CPPFLAGS = $(QT_CPPFLAGS) $(AM_CPPFLAGS) -I$(top_srcdir)/src
test_value_LDFLAGS  = $(QT_LDFLAGS) $(LDFLAGS) $(libtool_flags)
test_value_LDADD   = $(QT_LIBS) $(LDADD) $(top_builddir)/src/libqtlua.la

test_table_SOURCES = test_table.cc test.hh
test_table_CXXFLAGS = $(QT_CXXFLAGS) $(AM_CXXFLAGS)
test_table_CPPFLAGS = $(QT_CPPFLAGS) $(AM_CPPFLAGS) -I$(top_srcdir)/src
test_table_LDFLAGS  = $(QT_LDFLAGS) $(LDFLAGS) $(libtool_flags)
test_table_LDADD   = $(QT_LIBS) $(LDADD) $(top_builddir)/src/libqtlua.la

test_item_SOURCES = test_item.cc test.hh
test_item_CXXFLAGS = $(QT_CXXFLAGS) $(AM_CXXFLAGS)
test_item_CPPFLAGS = $(QT_CPPFLAGS) $(AM_CPPFLAGS) -I$(top_srcdir)/src
test_item_LDFLAGS  = $(QT_LDFLAGS) $(LDFLAGS) $(libtool_flags)
test_item_LDADD   = $(QT_LIBS) $(LDADD) $(top_builddir)/src/libqtlua.la

test_qobject_arg_SOURCES = test_qobject_arg.cc test_qobject_arg.hh test_qobject_arg.moc.cc test.hh
test_qobject_arg_CXXFLAGS = $(QT_CXXFLAGS) $(AM_CXXFLAGS)
test_qobject_arg_CPPFLAGS = $(QT_CPPFLAGS) $(AM_CPPFLAGS) -I$(top_srcdir)/src
test_qobject_arg_LDFLAGS  = $(QT_LDFLAGS) $(LDFLAGS) $(libtool_flags)
test_qobject_arg_LDADD   = $(QT_LIBS) $(LDADD) $(top_builddir)/src/libqtlua.la

BUILT_SOURCES = test_qobject_arg.moc.cc

TESTS=test_value test_table test_qobject_arg test_item

