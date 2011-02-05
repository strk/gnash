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

# XML tests that rely on comparing trace output.

TRACE_TESTS = \
	super.xml \
	this.xml \
	arguments.xml \
	$(NULL)	

TRACE_TRACES = \
	super.trace \
	this.trace \
	arguments.trace \
	$(NULL)	

EXTRA_DIST = \
	$(TRACE_TESTS) \
	$(TRACE_TRACES) \
	$(NULL)

TRACE_XMLTESTS_OUT = $(TRACE_TESTS:.xml=.swf)

check_SCRIPTS = \
	trace-test-runner \
	$(NULL)

trace-test-runner: $(srcdir)/../gen-swfmill-trace-runner.sh $(TRACE_TESTS) $(TRACE_XMLTESTS_OUT) Makefile
	sh $(srcdir)/../gen-swfmill-trace-runner.sh $(srcdir) $(builddir) > $@
	chmod 755 $@

.xml.swf: 
	$(SWFMILL) -v xml2swf $(@:%.swf=$(srcdir)/%.xml) $@

clean-local: 
	rm -f $(check_SCRIPTS) $(TRACE_XMLTESTS_OUT) \
		*.log *.trace-gnash *.out \
		site.bak site.exp site.exp.bak testrun.sum \
		gnash.sum 


TEST_DRIVERS = ../../simple.exp

TEST_CASES = ${check_SCRIPTS} 

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

