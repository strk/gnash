## Process this fill with automake to generate Makefile.in
# 
# Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

if ENABLE_SWFDEC_TESTSUITE

check_SCRIPTS = \
	run_swfdec_testsuite

clean-local: 
	rm -f $(check_SCRIPTS) \
		*.log *.trace-gnash \
		site.bak site.exp site.exp.bak testrun.sum \
		gnash.sum 

run_swfdec_testsuite: $(srcdir)/gen_run_swfdec_testsuite.sh 
	sh $< $(abs_topbuilddir) $(srcdir) $(SWFDEC_TESTSUITE)/trace > $@
	chmod 755 $@

TEST_DRIVERS = ../simple.exp
TEST_CASES = ${check_SCRIPTS}

check-DEJAGNU: site-update $(check_PROGRAMS)
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
