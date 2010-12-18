## Process this fill with automake to generate Makefile.in
# 
# Copyright (C) 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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

# XML tests that rely on comparing trace output.

TRACE_TESTS = \
	scope1.xml \
	stack1.xml \
	earlyshowframe.xml \
	noshowframe.xml \
	earlysymbolclass.xml \
	scriptorder.xml \
	$(NULL)	

TRACE_TRACES = \
	scope1.trace \
	stack1.trace \
	earlyshowframe.trace \
	noshowframe.trace \
	earlysymbolclass.trace \
	scriptorder.trace \
	$(NULL)	

EXTRA_DIST = \
	$(TRACE_TESTS) \
	$(TRACE_TRACES) \
	$(NULL)

AM_CPPFLAGS = \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libsound \
	-I$(top_srcdir)/librender \
	-I$(top_srcdir)/libcore  \
	-I$(top_srcdir)/libcore/swf \
	-I$(top_srcdir)/libcore/parser  \
	-I$(top_srcdir)/libcore/vm \
	-I$(top_srcdir)/libcore/asobj \
	-I$(top_srcdir)/testsuite \
	$(NULL)

TRACE_XMLTESTS_OUT = $(TRACE_TESTS:.xml=.swf)

check_SCRIPTS = \
	trace-test-runner \
	$(NULL)

trace-test-runner: $(srcdir)/../gen-swfmill-trace-runner.sh $(TRACE_TESTS) $(TRACE_XMLTESTS_OUT)
	sh $(srcdir)/../gen-swfmill-trace-runner.sh $(srcdir) $(builddir) > $@
	chmod 755 $@

.xml.swf: 
	$(SWFMILL) -v xml2swf $(@:%.swf=$(srcdir)/%.xml) $@

clean-local: 
	rm -f $(check_SCRIPTS) $(SC_XMLTESTS_OUT) $(TRACE_XMLTESTS_OUT) \
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

