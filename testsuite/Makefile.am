AUTOMAKE_OPTIONS = 

# movies.all
SUBDIRS = \
	.	\
	swfdec \
	libbase	\
	libgeometry \
	server \
	samples	\
	libamf.all \
	misc-ming.all \
	misc-mtasc.all \
	misc-swfmill.all \
	movies.all \
	actionscript.all \
	$(NULL)

EXTRA_DIST = check.h DummyMovieDefinition.h DummyCharacter.h media

dist_noinst_SCRIPTS = anaylse-results.sh generic-testrunner.sh

# could not find a way to only build this on make check
check_LTLIBRARIES = libtestsuite.la 

libtestsuite_la_SOURCES = \
	MovieTester.h \
	MovieTester.cpp \
	FuzzyPixel.cpp \
	FuzzyPixel.h \
	$(NULL)
libtestsuite_la_LIBADD = \
	$(RENDERER_LIBS) \
	$(top_builddir)/backend/libgnashbackend.la \
	$(top_builddir)/server/libgnashserver.la \
	$(top_builddir)/libbase/libgnashbase.la \
	$(NULL)
libtestsuite_la_CXXFLAGS = \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/backend \
	-I$(top_srcdir)/libgeometry \
	-I$(top_srcdir)/server  \
	-I$(top_srcdir)/server/parser  \
	-I$(top_srcdir)/server/vm  \
	$(BOOST_CFLAGS) \
	$(NULL)

if ENABLE_MING

.PHONY: Dejagnu.swf
Dejagnu.swf:
	$(MAKE) -C misc-ming.all $@
	cp misc-ming.all/Dejagnu.swf $@

check-local: Dejagnu.swf

clean-local:
	rm -f Dejagnu.swf

endif
