## Process this file with automake to generate Makefile.in
# 
# Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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

if LIBLTDL2
AM_LDFLAGS  += $(top_builddir)/libltdl/libltdlc.la
endif
if LIBLTDL1
AM_LDFLAGS += $(top_builddir)/libbase/libltdlc.la
endif

check_PROGRAMS = \
	action_execution_order_test \
	action_execution_order_extend_test \
	action_execution_order_test1 \
	action_execution_order_test2 \
	action_execution_order_test3 \
	action_execution_order_test4 \
	action_execution_order_test5 \
	action_execution_order_test6 \
	action_execution_order_test7 \
	action_execution_order_test8 \
	action_execution_order_test9 \
	action_execution_order_test11 \
	$(NULL)

if MING_SUPPORTS_INIT_ACTIONS
check_PROGRAMS += \
	ActionOrderTest3 \
	ActionOrderTest4 \
	ActionOrderTest5 \
	$(NULL)
endif
	
check_SCRIPTS = \
	action_execution_order_testrunner \
	action_execution_order_extend_testrunner \
	action_execution_order_test1runner \
	action_execution_order_test2runner \
	action_execution_order_test3runner \
	action_execution_order_test4runner \
	action_execution_order_test5runner \
	action_execution_order_test6runner \
	action_execution_order_test7runner \
	action_execution_order_test8runner \
	action_execution_order_test9runner \
	action_execution_order_test11runner \
	$(NULL)

if MING_SUPPORTS_INIT_ACTIONS
check_SCRIPTS += \
	ActionOrderTest3Runner \
	ActionOrderTest4Runner \
	ActionOrderTest5Runner \
	$(NULL)
endif
	
action_execution_order_test_SOURCES =	\
	action_execution_order_test.c	\
	$(NULL)
action_execution_order_test_LDADD = ../libgnashmingutils.la

action_execution_order_test.swf: action_execution_order_test
	./action_execution_order_test $(abs_mediadir)

action_execution_order_testrunner: $(srcdir)/../../generic-testrunner.sh action_execution_order_test.swf
	sh $(srcdir)/../../generic-testrunner.sh $(top_builddir) action_execution_order_test.swf > $@
	chmod 755 $@

action_execution_order_test1_SOURCES =	\
	action_execution_order_test1.c	\
	$(NULL)
action_execution_order_test1_LDADD = ../libgnashmingutils.la

action_execution_order_test1.swf: action_execution_order_test1
	./action_execution_order_test1 $(abs_mediadir)

action_execution_order_test1runner: $(srcdir)/../../generic-testrunner.sh action_execution_order_test1.swf
	sh $(srcdir)/../../generic-testrunner.sh $(top_builddir) action_execution_order_test1.swf > $@
	chmod 755 $@

action_execution_order_test2_SOURCES =	\
	action_execution_order_test2.c	\
	$(NULL)
action_execution_order_test2_LDADD = ../libgnashmingutils.la

action_execution_order_test2.swf: action_execution_order_test2
	./action_execution_order_test2 $(abs_mediadir)

action_execution_order_test2runner: $(srcdir)/../../generic-testrunner.sh action_execution_order_test2.swf
	sh $(srcdir)/../../generic-testrunner.sh $(top_builddir) action_execution_order_test2.swf > $@
	chmod 755 $@

action_execution_order_test3_SOURCES =	\
	action_execution_order_test3.c	\
	$(NULL)
action_execution_order_test3_LDADD = ../libgnashmingutils.la

action_execution_order_test3.swf: action_execution_order_test3
	./action_execution_order_test3 $(abs_mediadir)

action_execution_order_test3runner: $(srcdir)/../../generic-testrunner.sh action_execution_order_test3.swf
	sh $(srcdir)/../../generic-testrunner.sh $(top_builddir) action_execution_order_test3.swf > $@
	chmod 755 $@

action_execution_order_test4_SOURCES =	\
	action_execution_order_test4.c	\
	$(NULL)
action_execution_order_test4_LDADD = ../libgnashmingutils.la

action_execution_order_test4.swf: action_execution_order_test4
	./action_execution_order_test4 $(abs_mediadir)

action_execution_order_test4runner: $(srcdir)/../../generic-testrunner.sh action_execution_order_test4.swf
	sh $(srcdir)/../../generic-testrunner.sh $(top_builddir) action_execution_order_test4.swf > $@
	chmod 755 $@

action_execution_order_test5_SOURCES =	\
	action_execution_order_test5.c	\
	$(NULL)
action_execution_order_test5_LDADD = ../libgnashmingutils.la

action_execution_order_test5.swf: action_execution_order_test5
	./action_execution_order_test5 $(abs_mediadir)

action_execution_order_test5runner: $(srcdir)/../../generic-testrunner.sh action_execution_order_test5.swf
	sh $(srcdir)/../../generic-testrunner.sh $(top_builddir) action_execution_order_test5.swf > $@
	chmod 755 $@

action_execution_order_test6_SOURCES =	\
	action_execution_order_test6.c	\
	$(NULL)
action_execution_order_test6_LDADD = ../libgnashmingutils.la

action_execution_order_test6.swf: action_execution_order_test6
	./action_execution_order_test6 $(abs_mediadir)

action_execution_order_test6runner: $(srcdir)/../../generic-testrunner.sh action_execution_order_test6.swf
	sh $(srcdir)/../../generic-testrunner.sh $(top_builddir) action_execution_order_test6.swf > $@
	chmod 755 $@

action_execution_order_test7_SOURCES =	\
	action_execution_order_test7.c	\
	$(NULL)
action_execution_order_test7_LDADD = ../libgnashmingutils.la

action_execution_order_test7.swf: action_execution_order_test7
	./action_execution_order_test7 $(abs_mediadir)

action_execution_order_test7runner: $(srcdir)/../../generic-testrunner.sh action_execution_order_test7.swf
	sh $(srcdir)/../../generic-testrunner.sh $(top_builddir) action_execution_order_test7.swf > $@
	chmod 755 $@

action_execution_order_test8_SOURCES =	\
	action_execution_order_test8.c	\
	$(NULL)
action_execution_order_test8_LDADD = ../libgnashmingutils.la

action_execution_order_test8-v6.swf: action_execution_order_test8
	./action_execution_order_test8 $(abs_mediadir) 6

action_execution_order_test8-v5.swf: action_execution_order_test8
	./action_execution_order_test8 $(abs_mediadir) 5

action_execution_order_test8runner: $(srcdir)/../../generic-testrunner.sh action_execution_order_test8-v5.swf action_execution_order_test8-v6.swf
	sh $(srcdir)/../../generic-testrunner.sh -f8 $(top_builddir) action_execution_order_test8-v5.swf action_execution_order_test8-v6.swf > $@
	chmod 755 $@

action_execution_order_test9_SOURCES =	\
	action_execution_order_test9.c	\
	$(NULL)
action_execution_order_test9_LDADD = ../libgnashmingutils.la

action_execution_order_test9.swf: action_execution_order_test9
	./action_execution_order_test9 $(abs_mediadir)

action_execution_order_test9runner: $(srcdir)/../../generic-testrunner.sh action_execution_order_test9.swf
	sh $(srcdir)/../../generic-testrunner.sh -f8 $(top_builddir) action_execution_order_test9.swf > $@
	chmod 755 $@

action_execution_order_test11_SOURCES =	\
	action_execution_order_test11.c	\
	$(NULL)
action_execution_order_test11_LDADD = ../libgnashmingutils.la

action_execution_order_test11.swf: action_execution_order_test11
	./action_execution_order_test11 $(abs_mediadir)

action_execution_order_test11runner: $(srcdir)/../../generic-testrunner.sh action_execution_order_test11.swf
	sh $(srcdir)/../../generic-testrunner.sh -f10 $(top_builddir) action_execution_order_test11.swf > $@
	chmod 755 $@

ActionOrderTest3_SOURCES =	\
	ActionOrderTest3.c	\
	$(NULL)
ActionOrderTest3_LDADD = ../libgnashmingutils.la

ActionOrderTest3.swf: ActionOrderTest3
	./ActionOrderTest3 $(abs_mediadir)

ActionOrderTest3Runner: $(srcdir)/../../generic-testrunner.sh ActionOrderTest3.swf
	sh $(srcdir)/../../generic-testrunner.sh $(top_builddir) ActionOrderTest3.swf > $@
	chmod 755 $@

ActionOrderTest4_SOURCES =	\
	ActionOrderTest4.c	\
	$(NULL)
ActionOrderTest4_LDADD = ../libgnashmingutils.la

ActionOrderTest4.swf: ActionOrderTest4
	./ActionOrderTest4 $(abs_mediadir)

ActionOrderTest4Runner: $(srcdir)/../../generic-testrunner.sh ActionOrderTest4.swf
	sh $(srcdir)/../../generic-testrunner.sh $(top_builddir) ActionOrderTest4.swf > $@
	chmod 755 $@

ActionOrderTest5_SOURCES =	\
	ActionOrderTest5.c	\
	$(NULL)
ActionOrderTest5_LDADD = ../libgnashmingutils.la

ActionOrderTest5.swf: ActionOrderTest5
	./ActionOrderTest5 $(abs_mediadir)

ActionOrderTest5Runner: $(srcdir)/../../generic-testrunner.sh ActionOrderTest5.swf
	sh $(srcdir)/../../generic-testrunner.sh $(top_builddir) ActionOrderTest5.swf > $@
	chmod 755 $@

action_execution_order_extend_test_SOURCES =	\
	action_execution_order_extend_test.c	\
	$(NULL)
action_execution_order_extend_test_LDADD = ../libgnashmingutils.la

action_execution_order_extend_test.swf: action_execution_order_extend_test
	./action_execution_order_extend_test $(abs_mediadir)

action_execution_order_extend_testrunner: $(srcdir)/../../generic-testrunner.sh action_execution_order_extend_test.swf
	sh $(srcdir)/../../generic-testrunner.sh $(top_builddir) action_execution_order_extend_test.swf > $@
	chmod 755 $@

clean-local: 
	-rm *.swf *.pp *unner

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

