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
abs_builddir=$(shell cd $(top_builddir)/testsuite/misc-ming.all/displaylist_depths; pwd)
abs_mediadir = $(shell cd $(srcdir)/../../media; pwd)

CLEANFILES =  \
	gnash-dbg.log \
	site.exp.bak \
	testrun.sum \
	testrun.log

EXTRA_DIST = 

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
	displaylist_depths_test \
	displaylist_depths_test2 \
	displaylist_depths_test3 \
	displaylist_depths_test4 \
	displaylist_depths_test5 \
	displaylist_depths_test6 \
	displaylist_depths_test7 \
	displaylist_depths_test8 \
	displaylist_depths_test9 \
	displaylist_depths_test10 \
	displaylist_depths_test11 \
	$(NULL)

check_SCRIPTS = \
	displaylist_depths_testrunner \
	displaylist_depths_test2runner \
	displaylist_depths_test3runner \
	displaylist_depths_test4runner \
	displaylist_depths_test5runner \
	displaylist_depths_test6runner \
	displaylist_depths_test7runner \
	displaylist_depths_test8runner \
	displaylist_depths_test9runner \
	displaylist_depths_test10runner \
	displaylist_depths_test11runner \
	$(NULL)

displaylist_depths_test_SOURCES =	\
	displaylist_depths_test.c	\
	$(NULL)
displaylist_depths_test_LDADD = ../libgnashmingutils.la

displaylist_depths_test.swf: displaylist_depths_test
	./displaylist_depths_test $(abs_mediadir)

displaylist_depths_testrunner: $(srcdir)/../../generic-testrunner.sh displaylist_depths_test.swf
	sh $(srcdir)/../../generic-testrunner.sh -r5 $(top_builddir) displaylist_depths_test.swf > $@
	chmod 755 $@

displaylist_depths_test2_SOURCES =	\
	displaylist_depths_test2.c	\
	$(NULL)
displaylist_depths_test2_LDADD = ../libgnashmingutils.la

displaylist_depths_test2.swf: displaylist_depths_test2
	./displaylist_depths_test2 $(abs_mediadir)

displaylist_depths_test2runner: $(srcdir)/../../generic-testrunner.sh displaylist_depths_test2.swf
	sh $(srcdir)/../../generic-testrunner.sh -f20 $(top_builddir) displaylist_depths_test2.swf > $@
	chmod 755 $@

displaylist_depths_test3_SOURCES =	\
	displaylist_depths_test3.c	\
	$(NULL)
displaylist_depths_test3_LDADD = ../libgnashmingutils.la

displaylist_depths_test3.swf: displaylist_depths_test3
	./displaylist_depths_test3 $(abs_mediadir)

displaylist_depths_test3runner: $(srcdir)/../../generic-testrunner.sh displaylist_depths_test3.swf
	sh $(srcdir)/../../generic-testrunner.sh -f20 $(top_builddir) displaylist_depths_test3.swf > $@
	chmod 755 $@

displaylist_depths_test4_SOURCES =	\
	displaylist_depths_test4.c	\
	$(NULL)
displaylist_depths_test4_LDADD = ../libgnashmingutils.la

displaylist_depths_test4.swf: displaylist_depths_test4
	./displaylist_depths_test4 $(abs_mediadir)

displaylist_depths_test4runner: $(srcdir)/../../generic-testrunner.sh displaylist_depths_test4.swf
	sh $(srcdir)/../../generic-testrunner.sh -f20 $(top_builddir) displaylist_depths_test4.swf > $@
	chmod 755 $@

displaylist_depths_test5_SOURCES =	\
	displaylist_depths_test5.c	\
	$(NULL)
displaylist_depths_test5_LDADD = ../libgnashmingutils.la

displaylist_depths_test5.swf: displaylist_depths_test5
	./displaylist_depths_test5 $(abs_mediadir)

displaylist_depths_test5runner: $(srcdir)/../../generic-testrunner.sh displaylist_depths_test5.swf
	sh $(srcdir)/../../generic-testrunner.sh -f20 $(top_builddir) displaylist_depths_test5.swf > $@
	chmod 755 $@

displaylist_depths_test6_SOURCES =	\
	displaylist_depths_test6.c	\
	$(NULL)
displaylist_depths_test6_LDADD = ../libgnashmingutils.la

displaylist_depths_test6.swf: displaylist_depths_test6
	./displaylist_depths_test6 $(abs_mediadir)

displaylist_depths_test6runner: $(srcdir)/../../generic-testrunner.sh displaylist_depths_test6.swf
	sh $(srcdir)/../../generic-testrunner.sh -f20 $(top_builddir) displaylist_depths_test6.swf > $@
	chmod 755 $@

displaylist_depths_test7_SOURCES =	\
	displaylist_depths_test7.c	\
	$(NULL)
displaylist_depths_test7_LDADD = ../libgnashmingutils.la

displaylist_depths_test7.swf: displaylist_depths_test7
	./displaylist_depths_test7 $(abs_mediadir)

displaylist_depths_test7runner: $(srcdir)/../../generic-testrunner.sh displaylist_depths_test7.swf
	sh $(srcdir)/../../generic-testrunner.sh -f20 $(top_builddir) displaylist_depths_test7.swf > $@
	chmod 755 $@

displaylist_depths_test8_SOURCES =	\
	displaylist_depths_test8.c	\
	$(NULL)
displaylist_depths_test8_LDADD = ../libgnashmingutils.la

displaylist_depths_test8.swf: displaylist_depths_test8
	./displaylist_depths_test8 $(abs_mediadir)

displaylist_depths_test8runner: $(srcdir)/../../generic-testrunner.sh displaylist_depths_test8.swf
	sh $(srcdir)/../../generic-testrunner.sh -f20 $(top_builddir) displaylist_depths_test8.swf > $@
	chmod 755 $@


displaylist_depths_test9_SOURCES =	\
	displaylist_depths_test9.c	\
	$(NULL)
displaylist_depths_test9_LDADD = ../libgnashmingutils.la

displaylist_depths_test9.swf: displaylist_depths_test9
	./displaylist_depths_test9 $(abs_mediadir)

displaylist_depths_test9runner: $(srcdir)/../../generic-testrunner.sh displaylist_depths_test9.swf
	sh $(srcdir)/../../generic-testrunner.sh -f20 $(top_builddir) displaylist_depths_test9.swf > $@
	chmod 755 $@

displaylist_depths_test10_SOURCES =	\
	displaylist_depths_test10.c	\
	$(NULL)
displaylist_depths_test10_LDADD = ../libgnashmingutils.la

displaylist_depths_test10.swf: displaylist_depths_test10
	./displaylist_depths_test10 $(abs_mediadir)

displaylist_depths_test10runner: $(srcdir)/../../generic-testrunner.sh displaylist_depths_test10.swf
	sh $(srcdir)/../../generic-testrunner.sh -f20 $(top_builddir) displaylist_depths_test10.swf > $@
	chmod 755 $@

displaylist_depths_test11_SOURCES = displaylist_depths_test11.c	
displaylist_depths_test11_LDADD = ../libgnashmingutils.la

displaylist_depths_test11.swf: displaylist_depths_test11
	./displaylist_depths_test11 $(abs_mediadir)

displaylist_depths_test11runner: $(srcdir)/../../generic-testrunner.sh displaylist_depths_test11.swf
	sh $(srcdir)/../../generic-testrunner.sh -f20 $(top_builddir) displaylist_depths_test11.swf > $@
	chmod 755 $@


clean-local: 
	-rm *.swf media *.pp *unner

TEST_DRIVERS = ../../simple.exp
TEST_CASES = $(check_SCRIPTS)

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

