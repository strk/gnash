## Process this fill with automake to generate Makefile.in
# 
# Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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
AS3TESTS = \
	hello.as \
	$(NULL)

EXTRA_DIST = $(AS3TESTS) dejagnu.as

abs_mediadir = `cd $(srcdir)/../media; pwd`

# AS3COMPILE set by configure.
AS3COMPILE_FLAGS = 
AS3COMPILE_CPP = $(CPP) -x c -P -I$(srcdir) -DMEDIADIR="$(abs_mediadir)"

AS3TESTS_OUT = $(AS3TESTS:.sc=.swf)

TEST_DRIVERS = ../simple.exp
TEST_CASES = \
	sanetests-runner \
	$(check_PROGRAMS)

sanetests-runner: $(srcdir)/../generic-testrunner.sh $(AS3TESTS_OUT) Makefile
	sh $< -c __END_OF_TEST__ -f 50 -r 50 $(top_builddir) $(AS3TESTS_OUT) > $@
	chmod 755 $@

$(AS3TESTS_OUT): dejagnu.as check.sc

SUFFIXES = .as .swf
.as.swf:
	$(AS3COMPILE_CPP) $< > $(@:%.swf=pp_%.as) 
	$(AS3COMPILE) $(AS3COMPILE_FLAGS) -o $@ $(@:%.swf=pp_%.as)

CLEANFILES =  \
	$(TEST_CASES) \
	gnash-dbg.log \
	site.exp.bak \
	alltests-runner \
	testrun.* \
	*.swf \
	*.pp \
	pp_*.as \
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

