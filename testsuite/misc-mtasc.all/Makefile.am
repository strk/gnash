## Process this fill with automake to generate Makefile.in
# 
#   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
#   Free Software Foundation, Inc.
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
SANE_ASTESTS = \
	hello.as \
	inheritance.as \
	function_test.as \
	enum.as \
	exception.as \
	levels.as \
	implementsOpTest.as \
	TextFieldTest.as \
	super_test1.as \
	$(NULL)
# These ones are the ones for which consistency check fail 
BOGUS_ASTESTS = \
	$(NULL)

ASTESTS = $(SANE_ASTESTS) $(BOGUS_ASTESTS)


# These will get compiled to SWFs just as above, but will not be executed as a test
# They are for secondary movies to be loaded by the above tests.
AUXMOVIES = \
	    level5.as \
	    level99.as \
	    level87.as \
	    $(NULL)

EXTRA_DIST = $(ASTESTS) $(AUXMOVIES) TestClass.as Dejagnu.as check.as Derived1.as Base1.as  implementsOp/BExtendingImplementation.as  implementsOp/ImplementationA.as  implementsOp/ImplementationB.as implementsOp/SimpleInterface.as

MTASC_FLAGS = -version 6 -cp $(MTASC_CLASSPATH) -cp $(srcdir) -v -header 800:600:20
MTASC_CPP = $(CPP) -x c -P -I$(srcdir)

SANE_ASTESTS_OUT = $(SANE_ASTESTS:.as=.swf)
BOGUS_ASTESTS_OUT = $(BOGUS_ASTESTS:.as=.swf)
ASTESTS_OUT = $(SANE_ASTESTS_OUT) $(BOGUS_ASTESTS_OUT)

AUXMOVIES_OUT = $(AUXMOVIES:.as=.swf)

$(ASTESTS_OUT) $(AUXMOVIES_OUT): Dejagnu.as check.as


TEST_DRIVERS = ../simple.exp
TEST_CASES = \
	sanetests-runner \
	bogustests-runner \
	$(NULL)

sanetests-runner: $(srcdir)/../generic-testrunner.sh $(SANE_ASTESTS_OUT) $(AUXMOVIES_OUT) Makefile
	sh $(srcdir)/../generic-testrunner.sh -c __END_OF_TEST__ -r 50 -d 30 $(top_builddir) $(SANE_ASTESTS_OUT) > $@
	chmod 755 $@

bogustests-runner: $(srcdir)/../generic-testrunner.sh $(BOGUS_ASTESTS_OUT) $(AUXMOVIES_OUT) Makefile
	sh $(srcdir)/../generic-testrunner.sh -C __END_OF_TEST__ -r 50 $(top_builddir) $(BOGUS_ASTESTS_OUT) > $@
	chmod 755 $@

.as.swf: 
	$(MTASC_CPP) $< > $(@:%.swf=pp_%.as) 
	$(MTASC) $(MTASC_FLAGS) -swf $@ -main $(@:%.swf=pp_%.as)

CLEANFILES =  \
	gnash-dbg.log \
	site.exp.bak \
	*-runner \
	testrun.* \
	*.swf \
	*.pp \
	pp_*.as \
	*_testrunner \
	*_Runner

check-DEJAGNU: site-update $(TEST_CASES)
	@runtest=$(RUNTEST); \
	if $(SHELL) -c "$$runtest --version" > /dev/null 2>&1; then \
	    GNASH_GC_TRIGGER_THRESHOLD=0  GNASHRC=../gnashrc $$runtest $(RUNTESTFLAGS) $(TEST_DRIVERS); true; \
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

