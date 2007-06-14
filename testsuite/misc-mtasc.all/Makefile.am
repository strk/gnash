## Process this fill with automake to generate Makefile.in
# 
# Copyright (C) 2007 Free Software Foundation, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
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

if ENABLE_MTASC

# NOTE: 0.4.0.beta3 is required for prebuilt-clip to work (PREBUILT_CLIPS) !!
if ENABLE_MING
if MAKESWF_SUPPORTS_PREBUILT_CLIPS

DEJAGNU_SO_PATH = ../Dejagnu.swf
DEJAGNU_SO_FINI = $(srcdir)/../actionscript.all/dejagnu_so_fini.as
MTASC_FLAGS= -cp $(MTASC_CLASSPATH) -cp $(srcdir) -v -header 800:600:20

ASTESTS = \
	hello.as \
	inheritance.as \
	function.as \
	enum.as \
	$(NULL)

EXTRA_DIST = $(ASTESTS) TestClass.as 


ASTESTS_OUT = $(ASTESTS:.as=.swf)

TEST_DRIVERS = ../simple.exp
TEST_CASES = \
	alltests-runner

alltests-runner: $(srcdir)/../generic-testrunner.sh $(ASTESTS_OUT)
	sh $< $(top_builddir) $(ASTESTS_OUT) > $@
	chmod 755 $@

.as.swf:
	$(MTASC) $(MTASC_FLAGS) -swf mtasc-$@ -main $<
	$(MAKESWF) -o $@ $(DEJAGNU_SO_PATH) mtasc-$@  $(DEJAGNU_SO_FINI)

CLEANFILES =  \
	gnash-dbg.log \
	site.exp.bak \
	alltests-runner \
	testrun.* \
	*.swf \
	*.pp \
	*_testrunner \
	*_Runner

check-DEJAGNU: site-update $(TEST_CASES)
	@runtest=$(RUNTEST); \
	if $(SHELL) -c "$$runtest --version" > /dev/null 2>&1; then \
	    $$runtest $(RUNTESTFLAGS) $(TEST_DRIVERS); \
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

endif
endif
endif
