
include $(top_srcdir)/build/autotroll.mk

noinst_PROGRAMS = function ref

function_SOURCES = function.cc
function_CXXFLAGS = $(QT_CXXFLAGS) $(AM_CXXFLAGS)
function_CPPFLAGS = $(QT_CPPFLAGS) $(AM_CPPFLAGS) -I$(top_srcdir)/src
function_LDFLAGS  = $(QT_LDFLAGS) $(LDFLAGS) $(libtool_flags)
function_LDADD   = $(QT_LIBS) $(LDADD) $(top_builddir)/src/libqtlua.la

ref_SOURCES = ref.cc
ref_CXXFLAGS = $(QT_CXXFLAGS) $(AM_CXXFLAGS)
ref_CPPFLAGS = $(QT_CPPFLAGS) $(AM_CPPFLAGS) -I$(top_srcdir)/src
ref_LDFLAGS  = $(QT_LDFLAGS) $(LDFLAGS) $(libtool_flags)
ref_LDADD   = $(QT_LIBS) $(LDADD) $(top_builddir)/src/libqtlua.la

#BUILT_SOURCES = 

