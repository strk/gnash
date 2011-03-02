## Process this file with automake to generate Makefile.in
# 
#  Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
#  2011 Free Software Foundation, Inc.
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
abs_builddir=$(shell cd $(top_builddir)/testsuite/misc-ming.all/loop; pwd)
abs_mediadir = $(shell cd $(srcdir)/../../media; pwd)

CLEANFILES =  \
	gnash-dbg.log \
	site.exp.bak \
	testrun.sum \
	testrun.log

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
	$(NULL)

check_PROGRAMS = \
	loop_test \
	loop_test2 \
	loop_test2runner \
	loop_test3 \
	loop_test4 \
	loop_test5 \
	loop_test6 \
	loop_test7 \
	loop_test8 \
	loop_test9 \
	loop_test10 \
	loop_test-Runner \
	simple_loop_test \
	simple_loop_testrunner \
	$(NULL)

check_SCRIPTS = \
	loop_test3runner \
	loop_test4runner \
	loop_test5runner \
	loop_test6runner \
	loop_test7runner \
	loop_test8runner \
	loop_test9runner \
	loop_test10runner \
	$(NULL)

loop_test_SOURCES = loop_test.c	
loop_test_LDADD = ../libgnashmingutils.la

loop_test.swf: loop_test
	./loop_test $(abs_mediadir)

loop_test_Runner_SOURCES = loop_test-Runner.cpp 
loop_test_Runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)

loop_test_Runner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)

loop_test_Runner_DEPENDENCIES = \
	loop_test.swf \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)

loop_test2_SOURCES = loop_test2.c	
loop_test2_LDADD = ../libgnashmingutils.la

loop_test2.swf: loop_test2
	./loop_test2 $(abs_mediadir)

loop_test2runner_SOURCES = \
	loop_test2runner.cpp \
	$(NULL)
loop_test2runner_CXXFLAGS = \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
loop_test2runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
loop_test2runner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	loop_test2.swf      \
	$(NULL)

loop_test3_SOURCES = loop_test3.c
loop_test3_LDADD = ../libgnashmingutils.la

loop_test3.swf: loop_test3
	./loop_test3 $(abs_mediadir)

loop_test3runner: $(top_srcdir)/testsuite/generic-testrunner.sh loop_test3.swf
	sh $(top_srcdir)/testsuite/generic-testrunner.sh -f20 $(top_builddir) loop_test3.swf > $@
	chmod 755 $@

loop_test4_SOURCES = loop_test4.c
loop_test4_LDADD = ../libgnashmingutils.la

loop_test4.swf: loop_test4
	./loop_test4 $(abs_mediadir)

loop_test4runner: $(top_srcdir)/testsuite/generic-testrunner.sh loop_test4.swf
	sh $(top_srcdir)/testsuite/generic-testrunner.sh -f20 $(top_builddir) loop_test4.swf > $@
	chmod 755 $@

loop_test5_SOURCES = loop_test5.c
loop_test5_LDADD = ../libgnashmingutils.la

loop_test5.swf: loop_test5
	./loop_test5 $(abs_mediadir)

loop_test5runner: $(top_srcdir)/testsuite/generic-testrunner.sh loop_test5.swf
	sh $(top_srcdir)/testsuite/generic-testrunner.sh -f20 $(top_builddir) loop_test5.swf > $@
	chmod 755 $@

loop_test6_SOURCES = loop_test6.c
loop_test6_LDADD = ../libgnashmingutils.la

loop_test6.swf: loop_test6
	./loop_test6 $(abs_mediadir)

loop_test6runner: $(top_srcdir)/testsuite/generic-testrunner.sh loop_test6.swf
	sh $(top_srcdir)/testsuite/generic-testrunner.sh -f20 $(top_builddir) loop_test6.swf > $@
	chmod 755 $@

loop_test7_SOURCES = loop_test7.c
loop_test7_LDADD = ../libgnashmingutils.la

loop_test7.swf: loop_test7
	./loop_test7 $(abs_mediadir)

loop_test7runner: $(top_srcdir)/testsuite/generic-testrunner.sh loop_test7.swf
	sh $(top_srcdir)/testsuite/generic-testrunner.sh -f20 $(top_builddir) loop_test7.swf > $@
	chmod 755 $@

loop_test8_SOURCES = loop_test8.c
loop_test8_LDADD = ../libgnashmingutils.la

loop_test8.swf: loop_test8
	./loop_test8 $(abs_mediadir)

loop_test8runner: $(top_srcdir)/testsuite/generic-testrunner.sh loop_test8.swf
	sh $(top_srcdir)/testsuite/generic-testrunner.sh -f20 $(top_builddir) loop_test8.swf > $@
	chmod 755 $@

loop_test9_SOURCES = loop_test9.c
loop_test9_LDADD = ../libgnashmingutils.la

loop_test9.swf: loop_test9
	./loop_test9 $(abs_mediadir)

loop_test9runner: $(top_srcdir)/testsuite/generic-testrunner.sh loop_test9.swf Makefile
	sh $(top_srcdir)/testsuite/generic-testrunner.sh -r10 -f20 -c "#total tests" $(top_builddir) loop_test9.swf > $@
	chmod 755 $@


loop_test10_SOURCES = loop_test10.c
loop_test10_LDADD = ../libgnashmingutils.la

loop_test10.swf: loop_test10
	./loop_test10 $(abs_mediadir)

loop_test10runner: $(top_srcdir)/testsuite/generic-testrunner.sh loop_test10.swf Makefile
	sh $(top_srcdir)/testsuite/generic-testrunner.sh  -f15 -c "#total tests" $(top_builddir) loop_test10.swf > $@
	chmod 755 $@

simple_loop_test_SOURCES =	\
	simple_loop_test.c	\
	$(NULL)
simple_loop_test_LDADD = ../libgnashmingutils.la

simple_loop_test.swf: simple_loop_test
	./simple_loop_test $(abs_mediadir)

simple_loop_testrunner_SOURCES = \
	simple_loop_testrunner.cpp \
	$(NULL)
simple_loop_testrunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
simple_loop_testrunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
simple_loop_testrunner_DEPENDENCIES = \
	simple_loop_test.swf \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)


clean-local: 
	-rm *.swf *.pp *unner

TEST_DRIVERS = ../../simple.exp
TEST_CASES = \
	loop_test-Runner \
	loop_test2runner \
	loop_test3runner \
	loop_test4runner \
	loop_test5runner \
	loop_test6runner \
	loop_test7runner \
	loop_test8runner \
	loop_test9runner \
	loop_test10runner \
	simple_loop_testrunner \
	$(NULL)

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

