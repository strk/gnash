## Process this fill with automake to generate Makefile.in
# 
# Copyright (C) 2007 Free Software Foundation, Inc.
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
SCTESTS = \
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
	$(NULL)

# These will get compiled to SWFs just as above, but will not be executed as a test
# They are for secondary movies to be loaded by the above tests.
AUXMOVIES = \
	    $(NULL)


EXTRA_DIST = $(SCTESTS) $(AUXMOVIES) check.sc Dejagnu.sc

if ENABLE_SWFC

abs_mediadir = $(shell cd $(srcdir)/../media; pwd)

#SWFC = swfc # this is set by ./configure
SWFC_FLAGS = 
SWFC_CPP = cpp -P -I$(srcdir) -DMEDIADIR='$(abs_mediadir)'

SCTESTS_OUT = $(SCTESTS:.sc=.swf)

AUXMOVIES_OUT = $(AUXMOVIES:.sc=.swf)

TEST_DRIVERS = ../simple.exp
TEST_CASES = \
	alltests-runner

alltests-runner: $(srcdir)/../generic-testrunner.sh $(SCTESTS_OUT) $(AUXMOVIES_OUT)
	sh $< -r 50 $(top_builddir) $(SCTESTS_OUT) > $@
	chmod 755 $@

.sc.swf:
	$(SWFC_CPP) $< > $(@:%.swf=pp_%.sc) 
	$(SWFC) $(SWFC_FLAGS) -o $@ $(@:%.swf=pp_%.sc)

CLEANFILES =  \
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
	    $$runtest $(RUNTESTFLAGS) $(TEST_DRIVERS); true; \
	else \
	  echo "WARNING: could not find \`runtest'" 1>&2; \
          for i in "$(TEST_CASES)"; do \
	    $(SHELL) $$i; \
	  done; \
	fi

site-update: site.exp
	@rm -fr site.exp.bak
	@cp site.exp site.exp.bak
	@sed -e '/testcases/d' site.exp.bak > site.exp
	@echo "# This is a list of the pre-compiled testcases" >> site.exp
	@echo "set testcases \"$(TEST_CASES)\"" >> site.exp

endif # ENABLE_SWFC
