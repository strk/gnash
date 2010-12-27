## Process this fill with automake to generate Makefile.in
# 
# Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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

CLEANFILES =  \
      gnash-dbg.log \
      site.exp.bak \
      testrun.* \
      GotoAndPlayTestRunner

AM_CPPFLAGS = \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/libmedia \
        -I$(top_srcdir)/libsound \
        -I$(top_srcdir)/librender \
        -I$(top_srcdir)/libcore  \
        -I$(top_srcdir)/libcore/swf \
        -I$(top_srcdir)/libcore/parser  \
        -I$(top_srcdir)/libcore/vm  \
        -I$(top_srcdir)/testsuite \
        $(OPENGL_CFLAGS) \
        $(BOOST_CFLAGS) \
        $(FFMPEG_CFLAGS) \
        $(NULL)

AM_LDFLAGS = \
	$(OPENGL_LIBS) \
	$(NULL)

GNASH_LIBS = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(top_builddir)/libcore/libgnashcore.la \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/librender/libgnashrender.la \
	$(NULL)

if LIBLTDL2
GNASH_LIBS += $(top_builddir)/libltdl/libltdlc.la
endif
if LIBLTDL1
GNASH_LIBS += $(top_builddir)/libbase/libltdlc.la
endif

check_PROGRAMS = \
	clip_as_button2-TestRunner \
	subshapes-TestRunner \
	gotoFrameOnKeyEvent-TestRunner \
	$(NULL)

check_SCRIPTS = \
	GotoAndPlayTestRunner \
	$(NULL)

clip_as_button2_TestRunner_SOURCES = \
	clip_as_button2-TestRunner.cpp \
	$(NULL)
clip_as_button2_TestRunner_LDADD = \
	$(GNASH_LIBS) \
	$(NULL)
clip_as_button2_TestRunner_CXXFLAGS = \
	-DSRCDIR=\"$(abs_srcdir)\" \
	$(NULL)

subshapes_TestRunner_SOURCES = \
	subshapes-TestRunner.cpp \
	$(NULL)
subshapes_TestRunner_LDADD = \
	$(GNASH_LIBS) \
	$(NULL)
subshapes_TestRunner_CXXFLAGS = \
	-DSRCDIR=\"$(abs_srcdir)\" \
	$(NULL)

gotoFrameOnKeyEvent_TestRunner_SOURCES = \
	gotoFrameOnKeyEvent-TestRunner.cpp \
	$(NULL)
gotoFrameOnKeyEvent_TestRunner_LDADD = \
	$(GNASH_LIBS) \
	$(NULL)
gotoFrameOnKeyEvent_TestRunner_CXXFLAGS = \
	-DSRCDIR=\"$(abs_srcdir)\" \
	$(NULL)

GotoAndPlayTestRunner: $(srcdir)/../generic-testrunner.sh $(srcdir)/GotoAndPlayTest.swf
	sh $(srcdir)/../generic-testrunner.sh $(top_builddir) $(srcdir)/GotoAndPlayTest.swf > $@
	chmod 755 $@

clean-local: 

EXTRA_DIST = \
	clip_as_button2.swf \
	GotoAndPlayTest.swf \
	gotoFrameOnKeyEvent.swf \
	subshapes.swf \
	README \
	$(NULL)

TEST_DRIVERS = ../simple.exp
TEST_CASES = \
	$(check_PROGRAMS) \
	$(check_SCRIPTS)

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

