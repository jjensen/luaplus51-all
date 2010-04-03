
ACLOCAL_AMFLAGS = -I build

SUBDIRS = src tools test doc

if EXAMPLES
SUBDIRS += examples
endif

EXTRA_DIST = bootstrap

doc: doc/manual doc/internals

doc/manual:
	mkdoc -I src qt_4.5.mkdoclib --conf-file doc/manual.conf src/QtLua/*.hh src/internal/*.hh

doc/internals:
	mkdoc -I src qt_4.5.mkdoclib --conf-file doc/internals.conf src/QtLua/*.hh src/internal/*.hh
