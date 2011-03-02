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

AUTOMAKE_OPTIONS = dejagnu -Wno-portability

abs_topbuilddir=$(shell cd $(top_builddir); pwd)
abs_builddir=$(shell cd $(top_builddir)/testsuite/misc-swfmill.all; pwd)

if ENABLE_AVM2
if SWFMILL_AS3_SUPPORT
DIR_TRACE_AS3 = trace-as3
endif
endif

if SWFMILL_FUNCTION2_FLAGS_ORDER_CORRECT
DIR_TRACE_AS2 = trace-as2
endif

SUBDIRS = $(DIR_TRACE_AS2) $(DIR_TRACE_AS3)

DIST_SUBDIRS = \
	trace-as2 \
	trace-as3 \
	$(NULL)

## Self-contained XML tests
SC_AS2_XMLTESTS = \
	mixed-bytecode-as2.xml \
	hello.xml \
	jump_after_end.xml \
	jump_to_prev_block.xml \
	dict_override.xml \
	initaction_in_definesprite.xml \
	zeroframe_definesprite.xml \
	background.xml \
	registers.xml \
	Try-without-jump.xml \
	$(NULL)

SC_AS3_XMLTESTS = \
	mixed-bytecode-as3.xml	\
	$(NULL)

# XML tests that rely on comparing trace output.

EXTRA_DIST = $(SC_AS2_XMLTESTS) \
	     $(SC_AS3_XMLTESTS) \
	     gen-swfmill-trace-runner.sh \
	     $(NULL)

AM_CPPFLAGS = \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libsound \
	-I$(top_srcdir)/librender \
	-I$(top_srcdir)/libcore \
	-I$(top_srcdir)/libcore/swf \
	-I$(top_srcdir)/libcore/parser  \
	-I$(top_srcdir)/libcore/vm \
	-I$(top_srcdir)/libcore/asobj \
	-I$(top_srcdir)/testsuite \
	$(NULL)

AM_LDFLAGS = \
	$(top_builddir)/librender/libgnashrender.la \
	$(NULL)

SC_XMLTESTS_OUT = $(SC_AS2_XMLTESTS:.xml=.swf)

# Dependencies for all self-contained SWF tests
$(SC_XMLTESTS_OUT) : 

check_SCRIPTS = \
	mixed-bytecode-as2-runner \
	jump_after_end-runner \
	jump_to_prev_block-runner \
	dict_override-runner \
	initaction_in_definesprite-runner \
	zeroframe_definesprite-runner \
	registers-runner \
	Try-without-jump-runner \
	$(NULL)

check_PROGRAMS = \
	backgroundTestRunner \
	$(NULL)

mixed-bytecode-as3-runner: $(srcdir)/../generic-testrunner.sh mixed-bytecode-as3.swf Makefile
	sh $(srcdir)/../generic-testrunner.sh -r 50 -c "AS3" $(top_builddir) mixed-bytecode-as3.swf > $@
	chmod 755 $@

mixed-bytecode-as2-runner: $(srcdir)/../generic-testrunner.sh mixed-bytecode-as2.swf Makefile
	sh $(srcdir)/../generic-testrunner.sh -r 50 -c "AS2" $(top_builddir) mixed-bytecode-as2.swf > $@
	chmod 755 $@

jump_after_end-runner: $(srcdir)/../generic-testrunner.sh jump_after_end.swf Makefile
	sh $(srcdir)/../generic-testrunner.sh -r 50 -c "END OF TEST" $(top_builddir) jump_after_end.swf > $@
	chmod 755 $@

jump_to_prev_block-runner: $(srcdir)/../generic-testrunner.sh jump_to_prev_block.swf Makefile
	sh $(srcdir)/../generic-testrunner.sh -r 50 -C "ABA" $(top_builddir) jump_to_prev_block.swf > $@
	chmod 755 $@

dict_override-runner: $(srcdir)/../generic-testrunner.sh dict_override.swf Makefile
	sh $(srcdir)/../generic-testrunner.sh -r 50 -c "END OF TEST" $(top_builddir) dict_override.swf > $@
	chmod 755 $@

initaction_in_definesprite-runner: $(srcdir)/../generic-testrunner.sh initaction_in_definesprite.swf Makefile
	sh $(srcdir)/../generic-testrunner.sh -r 50 -c "END OF TEST" $(top_builddir) initaction_in_definesprite.swf > $@
	chmod 755 $@

zeroframe_definesprite-runner: $(srcdir)/../generic-testrunner.sh zeroframe_definesprite.swf Makefile
	sh $(srcdir)/../generic-testrunner.sh -r 50 -c "END OF TEST" $(top_builddir) zeroframe_definesprite.swf > $@
	chmod 755 $@

registers-runner: $(srcdir)/../generic-testrunner.sh registers.swf Makefile
	sh $(srcdir)/../generic-testrunner.sh -r 50 -c "END OF TEST" $(top_builddir) registers.swf > $@
	chmod 755 $@

Try-without-jump-runner: $(srcdir)/../generic-testrunner.sh Try-without-jump.swf Makefile
	sh $(srcdir)/../generic-testrunner.sh -r 50 -c "END OF TEST" $(top_builddir) Try-without-jump.swf > $@
	chmod 755 $@

backgroundTestRunner_SOURCES = \
	backgroundTestRunner.cpp \
	$(NULL)
backgroundTestRunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
backgroundTestRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
backgroundTestRunner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	background.swf      \
	$(NULL)

.xml.swf: 
	$(SWFMILL) -v xml2swf $(@:%.swf=$(srcdir)/%.xml) $@

clean-local: 
	rm -f $(check_SCRIPTS) $(SC_XMLTESTS_OUT) $(TRACE_XMLTESTS_OUT) \
		*.log *.trace-gnash *.out \
		site.bak site.exp site.exp.bak testrun.sum \
		gnash.sum 


TEST_DRIVERS = ../simple.exp
TEST_CASES = ${check_SCRIPTS} ${check_PROGRAMS}

check-DEJAGNU: site-update $(check_PROGRAMS)
	@runtest=$(RUNTEST); \
	if $(SHELL) -c "$$runtest --version" > /dev/null 2>&1; then \
	    GNASH_GC_TRIGGER_THRESHOLD=0 GNASHRC=../gnashrc $$runtest $(RUNTESTFLAGS) $(TEST_DRIVERS); true; \
	else \
	  echo "WARNING: could not find \`runtest'" 1>&2; \
          for i in "$(TEST_CASES)"; do \
	    GNASH_GC_TRIGGER_THRESHOLD=0 GNASHRC=../gnashrc $(SHELL) $$i; \
	  done; \
	fi

site-update: site.exp
	@rm -fr site.exp.bak
	@cp site.exp site.exp.bak
	@sed -e '/testcases/d' site.exp.bak > site.exp
	@echo "# This is a list of the pre-compiled testcases" >> site.exp
	@echo "set testcases \"$(TEST_CASES)\"" >> site.exp

