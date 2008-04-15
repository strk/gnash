## Process this fill with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

AUTOMAKE_OPTIONS = dejagnu

AM_LDFLAGS =  \
	$(NULL)

# AM_CPPFLAGS isn't right here since per-program *_CPPFLAGS override global
# AM_CPPFLAGS
INCLUDES = -I.. \
        -I$(top_srcdir)	 \
        -I$(top_srcdir)/libgeometry \
        -I$(top_srcdir)/testsuite \
	$(NULL)

check_PROGRAMS = \
	Range2dTest	\
	Point2dTest	\
	snappingrangetest	\
	$(NULL)

CLEANFILES = \
      testrun.sum \
      testrun.log \
      gnash-dbg.log \
      site.exp.bak \
      Range2dTest \
      Point2dTest \
      snappingrangetest \
      $(NULL)

Point2dTest_SOURCES = Point2dTest.cpp
Range2dTest_SOURCES = Range2dTest.cpp
snappingrangetest_SOURCES = snappingrangetest.cpp

TEST_DRIVERS = ../simple.exp
TEST_CASES = \
	$(check_PROGRAMS) \
	$(NULL)

check-DEJAGNU: site-update $(TEST_CASES)
	@runtest=$(RUNTEST); \
	if $(SHELL) -c "$$runtest --version" > /dev/null 2>&1; then \
	    $$runtest $(RUNTESTFLAGS) $(TEST_DRIVERS); \
	else \
	  echo "WARNING: could not find \`runtest'" 1>&2; \
          for i in "$(TEST_CASES)"; do \
	    ./$$i; \
	  done; \
	fi

site-update: site.exp
	@rm -fr site.exp.bak
	@cp site.exp site.exp.bak
	@sed -e '/testcases/d' site.exp.bak > site.exp
	@echo "# This is a list of the pre-compiled testcases" >> site.exp
	@echo "set testcases \"$(TEST_CASES)\"" >> site.exp
