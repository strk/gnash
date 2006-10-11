AUTOMAKE_OPTIONS = 

RUNTESTDEFAULTFLAGS = 

# movies.all
SUBDIRS = \
	actionscript.all	\
	libbase			\
	server			\
	misc-ming.all		\
	$(NULL)

EXTRA_DIST = check.h

# could not find a way to only build this on make check
noinst_LTLIBRARIES = libtestsuite.la 

libtestsuite_la_SOURCES = \
	MovieTester.h \
	MovieTester.cpp \
	$(NULL)
libtestsuite_la_LIBADD = \
	../server/libgnashserver.la \
	../libbase/libgnashbase.la \
	$(NULL)
libtestsuite_la_CXXFLAGS = \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/server  \
        -I$(top_srcdir)/server/parser  \
	$(NULL)
