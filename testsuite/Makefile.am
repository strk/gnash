AUTOMAKE_OPTIONS = 

# movies.all
SUBDIRS = \
	.			\
	libbase			\
	server			\
	samples			\
	misc-ming.all		\
	movies.all		\
	actionscript.all	\
	$(NULL)

EXTRA_DIST = check.h media

dist_noinst_SCRIPTS = anaylse-results.sh

# could not find a way to only build this on make check
check_LTLIBRARIES = libtestsuite.la 

libtestsuite_la_SOURCES = \
	MovieTester.h \
	MovieTester.cpp \
	$(NULL)
libtestsuite_la_LIBADD = \
	$(top_builddir)/server/libgnashserver.la \
	$(top_builddir)/libbase/libgnashbase.la \
	$(NULL)
libtestsuite_la_CXXFLAGS = \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/server  \
        -I$(top_srcdir)/server/parser  \
	$(NULL)

check: check-recursive
	$(srcdir)/anaylse-results.sh
