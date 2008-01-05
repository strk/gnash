AUTOMAKE_OPTIONS = 

# movies.all
SUBDIRS = \
	.	\
	swfdec \
	libbase	\
	libamf.all \
	libgeometry \
	server \
	samples	\
	misc-ming.all \
	misc-mtasc.all \
	misc-swfmill.all \
	misc-swfc.all \
	movies.all \
	actionscript.all \
	$(NULL)

EXTRA_DIST = check.h DummyMovieDefinition.h DummyCharacter.h media gnashrc.in

dist_noinst_SCRIPTS = anaylse-results.sh generic-testrunner.sh

abs_mediadir = `cd $(srcdir)/media; pwd`
abs_tmpsodir = `cd tmpSharedObject; pwd`

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
	$(top_builddir)/libmedia/libgnashmedia.la \
	$(top_builddir)/server/libgnashserver.la \
	$(top_builddir)/libbase/libgnashbase.la \
	$(NULL)
libtestsuite_la_CXXFLAGS = \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/backend \
	-I$(top_srcdir)/libgeometry \
	-I$(top_srcdir)/server  \
	-I$(top_srcdir)/server/parser  \
	-I$(top_srcdir)/server/vm  \
	$(BOOST_CFLAGS) \
	$(CAIRO_CFLAGS) \
	$(NULL)

tmpSharedObject: 
	mkdir tmpSharedObject

gnashrc: gnashrc.in Makefile tmpSharedObject
	sed -e 's#@@MEDIADIR@@#'$(abs_mediadir)'#g' -e 's#@@TMPSODIR@@#'$(abs_tmpsodir)'#g' $< > $@

if ENABLE_MING

.PHONY: Dejagnu.swf
Dejagnu.swf:
	$(MAKE) -C misc-ming.all $@
	cp misc-ming.all/Dejagnu.swf $@

check-local: Dejagnu.swf gnashrc

endif

clean-local:
	rm -f Dejagnu.swf gnashrc

