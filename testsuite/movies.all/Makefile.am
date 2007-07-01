## Process this fill with automake to generate Makefile.in
# 
# Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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


#

AUTOMAKE_OPTIONS = dejagnu

check_PROGRAMS = \
	gravity_embedded-TestRunner \
	$(NULL)

TEST_DRIVERS = ../simple.exp
TEST_CASES = \
	gravity_embedded-TestRunner

CLEANFILES =  \
      gnash-dbg.log \
      site.exp.bak \
      testrun.sum \
      testrun.log


AM_CPPFLAGS = -I.. \
        -I$(top_srcdir)		\
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/backend \
        -I$(top_srcdir)/server  \
        -I$(top_srcdir)/server/parser  \
        -I$(top_srcdir)/libgeometry \
        -I$(top_srcdir)/testsuite \
	$(BOOST_CFLAGS) \
	$(NULL)

gravity_embedded_TestRunner_SOURCES = \
	gravity_embedded-TestRunner.cpp \
	$(NULL)
gravity_embedded_TestRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)
gravity_embedded_TestRunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	$(NULL)
gravity_embedded_TestRunner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	gravity-embedded.swf \
	gravity.swf \
	$(NULL)

clean-local: 

EXTRA_DIST = gravity-embedded.swf gravity.swf


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

