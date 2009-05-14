## Process this fill with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

AM_CPPFLAGS = \
        -I$(top_srcdir)/testsuite  \
        -I$(top_srcdir)/backend  \
        -I$(top_srcdir)/libamf  \
        -I$(top_srcdir)/libnet  \
        -I$(top_srcdir)/libbase  \
        -I$(top_srcdir)/libcore  \
        -I$(top_srcdir)/libcore/asobj \
        -I$(top_srcdir)/libcore/swf \
        -I$(top_srcdir)/libcore/parser  \
        -I$(top_srcdir)/libcore/vm  \
	$(FFMPEG_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(NULL)

check_PROGRAMS = \
	StreamTest \
	BitsReaderTest \
	MatrixTest \
	PointTest \
	EdgeTest \
	PropertyListTest \
	as_prop_flagsTest \
	DisplayListTest \
	AsValueTest \
	ClassSizes \
	SafeStackTest \
	$(NULL)

if ENABLE_AVM2
check_PROGRAMS += \
	CodeStreamTest
endif

CLEANFILES = \
	testrun.sum \
	testrun.log \
	gnash-dbg.log \
	site.exp.bak \
	gnash-dbg.log \
	$(NULL)

GNASH_LIBS = \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/libamf/libgnashamf.la \
	$(top_builddir)/libcore/libgnashcore.la \
	$(NULL)

if USE_FFMPEG_ENGINE
	GNASH_LIBS += $(FFMPEG_LIBS)
endif

BitsReaderTest_SOURCES = BitsReaderTest.cpp
BitsReaderTest_LDADD = \
	$(GNASH_LIBS) \
	$(NULL)

StreamTest_SOURCES = StreamTest.cpp
StreamTest_LDADD = \
	$(GNASH_LIBS) \
	$(NULL)

MatrixTest_SOURCES = MatrixTest.cpp
MatrixTest_LDADD = \
	$(GNASH_LIBS) \
	$(NULL)

PointTest_SOURCES = PointTest.cpp
PointTest_LDADD = \
	$(GNASH_LIBS) \
	$(NULL)

EdgeTest_SOURCES = EdgeTest.cpp
EdgeTest_LDADD = \
	$(GNASH_LIBS) \
	$(NULL)

PropertyListTest_SOURCES = PropertyListTest.cpp
PropertyListTest_LDADD = \
	$(GNASH_LIBS) \
	$(NULL)

as_prop_flagsTest_SOURCES = as_prop_flagsTest.cpp
as_prop_flagsTest_LDADD = \
	$(GNASH_LIBS) \
	$(NULL)

DisplayListTest_SOURCES = DisplayListTest.cpp
DisplayListTest_LDADD = \
	$(GNASH_LIBS) \
	$(NULL)

AsValueTest_SOURCES = AsValueTest.cpp
AsValueTest_LDADD = \
	$(GNASH_LIBS) \
	$(NULL)

ClassSizes_SOURCES = ClassSizes.cpp
ClassSizes_LDADD = \
	$(GNASH_LIBS) \
	$(NULL)

SafeStackTest_SOURCES = SafeStackTest.cpp
SafeStackTest_LDADD = \
	$(GNASH_LIBS) \
	$(NULL)

CodeStreamTest_SOURCES = CodeStreamTest.cpp
CodeStreamTest_LDADD = \
	$(GNASH_LIBS) \
	$(NULL)

TEST_DRIVERS = ../simple.exp
TEST_CASES = \
	$(check_PROGRAMS) \
	$(NULL)

check-DEJAGNU: site-update $(TEST_CASES)
	@runtest=$(RUNTEST); \
	if $(SHELL) -c "$$runtest --version" > /dev/null 2>&1; then \
	    $$runtest $(RUNTESTFLAGS) $(TEST_DRIVERS); true; \
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

