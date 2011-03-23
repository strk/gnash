## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
#   2011 Free Software Foundation, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

AUTOMAKE_OPTIONS = dejagnu -Wno-portability

abs_top_builddir=$(shell cd $(top_builddir); pwd)
abs_builddir=$(shell cd $(top_builddir)/testsuite/misc-ming.all/loading; pwd)
abs_mediadir = $(shell cd $(srcdir)/../../media; pwd)

CLEANFILES =  \
	gnash-dbg.log \
	site.exp.bak \
	testrun.sum \
	testrun.log \
	*.swf *.pp *unner

EXTRA_DIST = \
	$(NULL)

AM_CPPFLAGS = \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libsound \
	-I$(top_srcdir)/librender \
	-I$(top_srcdir)/libcore  \
	-I$(top_srcdir)/libcore/swf  \
	-I$(top_srcdir)/libcore/parser  \
	-I$(top_srcdir)/libcore/vm \
	-I$(top_srcdir)/libcore/asobj \
	-I$(top_srcdir)/testsuite \
	-I$(top_srcdir)/testsuite/misc-ming.all \
	$(MING_CFLAGS) \
	$(OPENGL_CFLAGS) \
	$(BOOST_CFLAGS) \
	-DMING_VERSION_CODE=$(MING_VERSION_CODE) \
	$(NULL)

AM_LDFLAGS = \
	$(MING_LIBS) \
	$(OPENGL_LIBS) \
	$(GIF_LIBS) \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/librender/libgnashrender.la \
	$(top_builddir)/libcore/libgnashcore.la \
	$(NULL)

check_PROGRAMS = \
	moviecliploader_test \
	loadImageTest \
	loadMovieTest \
	loadMovieTestRunner \
	LoadVarsTest \
	$(NULL)

if MING_VERSION_0_4_4
check_PROGRAMS += \
	LoadBitmapTest \
	$(NULL)
endif


check_SCRIPTS = \
	LoadVarsTestRunner \
	$(NULL)

if MING_VERSION_0_4_4
check_SCRIPTS += \
	LoadBitmapTestRunner \
	$(NULL)
endif

moviecliploader_test_SOURCES =  moviecliploader_test.c
moviecliploader_test_LDADD = $(MING_LIBS)

#FIXME: make this rule consistent with the others 
#and write a test runner for the absolute *and* relative
#versions
moviecliploader_tests: moviecliploader_test
	: test with relative urls 
	ln -fs $(abs_mediadir) media
	./moviecliploader_test media
	mv moviecliploader_test.swf moviecliploader_test_relative.swf
	: test absolute url
	./moviecliploader_test $(abs_mediadir)

LoadBitmapTest_SOURCES = \
	LoadBitmapTest.c \
	$(NULL)

LoadBitmapTest_CFLAGS = \
	-DMEDIADIR='"$(abs_mediadir)"' \
	$(NULL)

LoadBitmapTest_LDADD = ../libgnashmingutils.la

LoadBitmapTest.swf: LoadBitmapTest
	./LoadBitmapTest $(abs_mediadir)

LoadBitmapTestRunner: $(srcdir)/../../generic-testrunner.sh LoadBitmapTest.swf
	sh $(srcdir)/../../generic-testrunner.sh $(top_builddir) LoadBitmapTest.swf > $@
	chmod 755 $@

loadImageTest_SOURCES =	\
	loadImageTest.c	\
	$(NULL)
loadImageTest_LDADD = ../libgnashmingutils.la
loadImageTest.swf: loadImageTest
	./loadImageTest $(abs_mediadir)

loadMovieTest_SOURCES =	\
	loadMovieTest.c	\
	$(NULL)
loadMovieTest_LDADD = ../libgnashmingutils.la
loadMovieTest.swf: loadMovieTest
	./loadMovieTest $(abs_mediadir)
loadMovieTestRunner_SOURCES = \
	loadMovieTestRunner.cpp \
	$(NULL)
loadMovieTestRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
loadMovieTestRunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	-DMEDIADIR='"$(abs_mediadir)"' \
	$(NULL)
loadMovieTestRunner_DEPENDENCIES = \
	loadMovieTest.swf \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)

LoadVarsTest_SOURCES = \
	LoadVarsTest.c \
	$(NULL)

LoadVarsTest_LDADD = ../libgnashmingutils.la

LoadVarsTest.swf: LoadVarsTest
	./LoadVarsTest $(abs_mediadir)

LoadVarsTestRunner: $(srcdir)/../../generic-testrunner.sh LoadVarsTest.swf
	sh $(srcdir)/../../generic-testrunner.sh $(top_builddir) LoadVarsTest.swf > $@
	chmod 755 $@



TEST_DRIVERS = ../../simple.exp
TEST_CASES = \
	loadMovieTestRunner \
	LoadVarsTestRunner \
	$(NULL)

if MING_VERSION_0_4_4
TEST_CASES += \
	LoadBitmapTestRunner \
	$(NULL)
endif


check-DEJAGNU: site-update $(check_PROGRAMS)
	@runtest=$(RUNTEST); \
	if $(SHELL) -c "$$runtest --version" > /dev/null 2>&1; then \
	    GNASH_GC_TRIGGER_THRESHOLD=0 GNASHRC=../../gnashrc $$runtest $(RUNTESTFLAGS) $(TEST_DRIVERS); true; \
	else \
	  echo "WARNING: could not find \`runtest'" 1>&2; \
          for i in "$(TEST_CASES)"; do \
	    GNASH_GC_TRIGGER_THRESHOLD=0 GNASHRC=../../gnashrc $(SHELL) $$i; \
	  done; \
	fi

site-update: site.exp
	@rm -fr site.exp.bak
	@cp site.exp site.exp.bak
	@sed -e '/testcases/d' site.exp.bak > site.exp
	@echo "# This is a list of the pre-compiled testcases" >> site.exp
	@echo "set testcases \"$(TEST_CASES)\"" >> site.exp

