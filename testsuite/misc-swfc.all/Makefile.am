## Process this fill with automake to generate Makefile.in
# 
# Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

AUTOMAKE_OPTIONS = dejagnu

# These will get executed and parsed for test output
SANE_SCTESTS = \
	hello.sc \
	sound.sc \
	soft_reference_test1.sc \
	movieclip_destruction_test1.sc \
	movieclip_destruction_test2.sc \
	movieclip_destruction_test3.sc \
	movieclip_destruction_test4.sc \
	action_execution_order_test10.sc \
	action_execution_order_test12.sc \
	registerclass_test3.sc \
	edittext_test1.sc \
	opcode_guard_test2.sc \
	swf4opcode.sc \
	mouse_drag_test.sc \
	gotoFrameFromInterval.sc \
	gotoFrameFromInterval2.sc \
	gotoFrameLabelAsFunction.sc \
	matrix_accuracy_test1.sc \
	stackscope.sc \
	$(NULL)

# These ones are the ones for which consistency check fail 
BOGO_SCTESTS = \
	$(NULL)

# These ones are the ones for which a MovieTester-based runner exists
INTERACTIVE_SCTESTS = \
	button_test1.sc \
	button_test2.sc  \
	$(NULL)

AM_CPPFLAGS = -I.. \
        -I$(top_srcdir)		\
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/libmedia \
        -I$(top_srcdir)/libsound \
        -I$(top_srcdir)/librender \
        -I$(top_srcdir)/libcore  \
        -I$(top_srcdir)/libcore/asobj  \
        -I$(top_srcdir)/libcore/swf  \
        -I$(top_srcdir)/libcore/vm  \
        -I$(top_srcdir)/libcore/parser  \
        -I$(top_srcdir)/testsuite \
	$(BOOST_CFLAGS) \
	$(NULL)

GNASH_LIBS = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(top_builddir)/libcore/libgnashcore.la \
	$(top_builddir)/libbase/libgnashbase.la \
	$(NULL)

if LIBLTDL2
GNASH_LIBS += $(top_builddir)/libltdl/libltdlc.la
endif
if LIBLTDL1
GNASH_LIBS += $(top_builddir)/libbase/libltdlc.la
endif

# These will get compiled to SWFs just as above, but will not be executed as a test
# They are for secondary movies to be loaded by the above tests.
AUXMOVIES = \
	    $(NULL)

EXTRA_DIST = $(SANE_SCTESTS) $(BOGO_SCTESTS) $(AUXMOVIES) $(INTERACTIVE_SCTESTS) check.sc Dejagnu.sc hello.sc

check_PROGRAMS = \
	button_test1runner \
	$(NULL)

abs_mediadir = $(abs_top_srcdir)/testsuite/media 

#SWFC = swfc # this is set by ./configure
SWFC_FLAGS = 
SWFC_CPP = $(CPP) -x c -P -undef -I$(srcdir) -DMEDIADIR="$(abs_mediadir)"

SANE_SCTESTS_OUT = $(SANE_SCTESTS:.sc=.swf)
BOGO_SCTESTS_OUT = $(BOGO_SCTESTS:.sc=.swf)
SCTESTS_OUT = $(SANE_SCTESTS_OUT) $(BOGO_SCTESTS_OUT)

AUXMOVIES_OUT = $(AUXMOVIES:.sc=.swf)

TEST_DRIVERS = ../simple.exp
TEST_CASES = \
	sanetests-runner \
	bogustests-runner \
	$(check_PROGRAMS)

sanetests-runner: $(srcdir)/../generic-testrunner.sh $(SANE_SCTESTS_OUT) $(AUXMOVIES_OUT) Makefile
	sh $(srcdir)/../generic-testrunner.sh -c __END_OF_TEST__ -f 50 -r 50 $(top_builddir) $(SANE_SCTESTS_OUT) > $@
	chmod 755 $@

bogustests-runner: $(srcdir)/../generic-testrunner.sh $(BOGO_SCTESTS_OUT) $(AUXMOVIES_OUT) Makefile
	sh $(srcdir)/../generic-testrunner.sh -C __END_OF_TEST__ -f 50 -r 50 $(top_builddir) $(BOGO_SCTESTS_OUT) > $@
	chmod 755 $@

$(SANE_SCTESTS_OUT): Dejagnu.sc check.sc

SUFFIXES = .sc .swf
.sc.swf:
	$(SWFC_CPP) $< > $(@:%.swf=pp_%.sc) 
	$(SWFC) $(SWFC_FLAGS) -o $@ $(@:%.swf=pp_%.sc)

button_test1runner_SOURCES = \
	button_test1runner.cpp \
	$(NULL)
button_test1runner_LDADD = \
	$(GNASH_LIBS) \
	$(NULL)
button_test1runner_CXXFLAGS = \
	-DBUILDDIR='"$(top_builddir)/testsuite/misc-swfc.all"' \
	$(NULL)
button_test1runner_DEPENDENCIES = \
	button_test1.swf \
	$(GNASH_LIBS) \
	$(NULL)

CLEANFILES =  \
	$(TEST_CASES) \
	gnash-dbg.log \
	site.exp.bak \
	alltests-runner \
	testrun.* \
	*.swf \
	*.pp \
	pp_*.sc \
	*_testrunner \
	*_Runner

check-DEJAGNU: site-update $(TEST_CASES)
	@runtest=$(RUNTEST); \
	if $(SHELL) -c "$$runtest --version" > /dev/null 2>&1; then \
	    GNASH_GC_TRIGGER_THRESHOLD=0 GNASHRC=../gnashrc $$runtest $(RUNTESTFLAGS) $(TEST_DRIVERS); true; \
	else \
	  echo "WARNING: could not find \`runtest'" 1>&2; \
          for i in "$(TEST_CASES)"; do \
	    GNASH_GC_TRIGGER_THRESHOLD=0  GNASHRC=../gnashrc $(SHELL) $$i; \
	  done; \
	fi

site-update: site.exp
	@rm -fr site.exp.bak
	@cp site.exp site.exp.bak
	@sed -e '/testcases/d' site.exp.bak > site.exp
	@echo "# This is a list of the pre-compiled testcases" >> site.exp
	@echo "set testcases \"$(TEST_CASES)\"" >> site.exp

