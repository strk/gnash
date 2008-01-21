## Process this fill with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006,2007, 2008 Free Software Foundation, Inc.
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


#

AUTOMAKE_OPTIONS = dejagnu

LDADD = $(top_builddir)/libbase/libgnashbase.la 

# AM_CPPFLAGS isn't right here since per-program *_CPPFLAGS override global
# AM_CPPFLAGS
INCLUDES = -I.. \
        -I$(top_srcdir)	 \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/server  \
        -I$(top_srcdir)/libgeometry \
        -I$(top_srcdir)/testsuite \
        $(MING_CFLAGS) \
	$(LIBXML_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(DEJAGNU_CFLAGS) \
	$(NULL)

abs_builddir=`(cd $(top_builddir)/testsuite/libbase; pwd)`

check_PROGRAMS = \
	NoSeekFileTest \
	URLTest \
	RcTest \
	IntTypesTest \
	$(NULL)

if CURL
check_PROGRAMS += CurlStreamTest
endif

CLEANFILES = \
      testrun.sum \
      testrun.log \
      gnash-dbg.log \
      site.exp.bak \
      NoSeekFileTestCache

CurlStreamTest_SOURCES = CurlStreamTest.cpp
CurlStreamTest_CPPFLAGS = '-DINPUT="$(srcdir)/CurlStreamTest.cpp"'

NoSeekFileTest_SOURCES = NoSeekFileTest.cpp
NoSeekFileTest_CPPFLAGS = '-DINPUT="$(srcdir)/NoSeekFileTest.cpp"'

URLTest_SOURCES = URLTest.cpp
URLTest_CPPFLAGS = '-DBUILDDIR="$(abs_builddir)"'

RcTest_SOURCES = TCXXRc.cpp
RcTest_CPPFLAGS = -DSRCDIR="$(srcdir)"

IntTypesTest_SOURCES = IntTypesTest.cpp
IntTypesTest_CPPFLAGS = -DSRCDIR="$(srcdir)"

# LogTest_SOURCES = TCXXLog.cpp
# LogTest_CPPFLAGS = '-DBUILDDIR="$(abs_builddir)"'

TEST_DRIVERS = ../simple.exp
TEST_CASES = \
	NoSeekFileTest \
	URLTest \
	RcTest \
	$(NULL)

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
