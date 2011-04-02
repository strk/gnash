## Process this fill with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006,2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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

LDADD = $(top_builddir)/libbase/libgnashbase.la \
	$(LIBINTL)

# AM_CPPFLAGS isn't right here since per-program *_CPPFLAGS override global
# AM_CPPFLAGS
AM_CPPFLAGS = \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/libcore  \
        -I$(top_srcdir)/testsuite \
        $(MING_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(PTHREAD_CFLAGS) \
	$(DEJAGNU_CFLAGS) \
	$(NULL)

abs_builddir=`(cd $(top_builddir)/testsuite/libbase; pwd)`

CLEANFILES = \
	testrun.sum \
	testrun.log \
	gnash-dbg.log \
	site.exp.bak \
	NoSeekFileTestCache \
	$(NULL)

check_PROGRAMS = \
	NoSeekFileTest \
	URLTest \
	RcTest \
	IntTypesTest \
	Point2dTest \
	snappingrangetest \
	Range2dTest \
	string_tableTest \
	$(NULL)

#if CURL
## This test needs an http server running to be useful
#check_PROGRAMS += CurlStreamTest
#CurlStreamTest_SOURCES = CurlStreamTest.cpp
#CurlStreamTest_CPPFLAGS = '-DINPUT="$(srcdir)/CurlStreamTest.cpp"'
#endif

NoSeekFileTest_SOURCES = NoSeekFileTest.cpp
NoSeekFileTest_CPPFLAGS = $(AM_CPPFLAGS) \
	'-DINPUT="$(srcdir)/NoSeekFileTest.cpp"'

URLTest_SOURCES = URLTest.cpp
URLTest_CPPFLAGS =  $(AM_CPPFLAGS) \
	'-DBUILDDIR="$(abs_builddir)"'

RcTest_SOURCES = RcTest.cpp
RcTest_CPPFLAGS =  $(AM_CPPFLAGS) \
	-DSRCDIR="$(srcdir)"

IntTypesTest_SOURCES = IntTypesTest.cpp
IntTypesTest_CPPFLAGS =  $(AM_CPPFLAGS) \
	-DSRCDIR="$(srcdir)"

# Test the Memory class
#memtest_SOURCES = memtest.cpp

Point2dTest_SOURCES = Point2dTest.cpp
Range2dTest_SOURCES = Range2dTest.cpp
snappingrangetest_SOURCES = snappingrangetest.cpp

string_tableTest_SOURCES = string_tableTest.cpp
string_tableTest_CPPFLAGS =  $(AM_CPPFLAGS) \
	-DSRCDIR="$(srcdir)"
string_tableTest_LDFLAGS = $(BOOST_LIBS)

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
	    $(SHELL) $$i; \
	  done; \
	fi

site-update: site.exp
	@rm -fr site.exp.bak
	@cp site.exp site.exp.bak
	@sed -e '/testcases/d' site.exp.bak > site.exp
	@echo "# This is a list of the pre-compiled testcases" >> site.exp
	@echo "set testcases \"$(TEST_CASES)\"" >> site.exp
