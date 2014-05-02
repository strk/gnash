## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
#   2011 Free Software Foundation, Inc.
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

abs_top_builddir=$(shell cd $(top_builddir); pwd)
abs_builddir=$(shell cd $(top_builddir)/testsuite/misc-ming.all/sound; pwd)
abs_mediadir=$(shell cd $(srcdir)/../../media; pwd)

CLEANFILES =  \
	gnash-dbg.log \
	site.exp.bak \
	testrun.sum \
	testrun.log

EXTRA_DIST = \
	$(NULL)

AM_CPPFLAGS = \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libsound \
	-I$(top_srcdir)/librender \
	-I$(top_srcdir)/libcore  \
	-I$(top_srcdir)/libcore/swf  \
	-I$(top_srcdir)/libcore/parser  \
	-I$(top_srcdir)/libcore/vm \
	-I$(top_srcdir)/libcore/asobj \
	-I$(top_srcdir)/testsuite \
	-I$(top_srcdir)/testsuite/misc-ming.all \
	$(MING_CFLAGS) \
	$(OPENGL_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(PTHREAD_CFLAGS) \
	-DMING_VERSION_CODE=$(MING_VERSION_CODE) \
	$(NULL)

AM_LDFLAGS = \
	$(MING_LIBS) \
	$(OPENGL_LIBS) \
	$(GIF_LIBS) \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/librender/libgnashrender.la \
	$(top_builddir)/libcore/libgnashcore.la \
	$(NULL)

check_PROGRAMS = \
	$(NULL)

if MING_VERSION_0_4_3
check_PROGRAMS += \
	StreamSoundTest \
	StreamSoundTestRunner \
	EmbeddedSoundTest \
	EmbeddedSoundTest-Runner \
	eventSoundTest1 \
	eventSoundTest1-Runner \
	streamingSoundTest1 \
	streamingSoundTest1-Runner \
	streamingSoundTest2 \
	streamingSoundTest2-Runner \
	$(NULL)
endif

# This is so check.as finds revno.h
MAKESWF_FLAGS = -I$(top_builddir)

eventSoundTest1_SOURCES = eventSoundTest1.c
eventSoundTest1_LDADD = ../libgnashmingutils.la

eventSoundTest1.swf: eventSoundTest1
	./eventSoundTest1 $(srcdir)/../../media/brokenchord.wav \
	$(srcdir)/../../media/

eventSoundTest1_Runner_SOURCES = \
	eventSoundTest1-Runner.cpp \
	$(NULL)
eventSoundTest1_Runner_CXXFLAGS = \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
eventSoundTest1_Runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
eventSoundTest1_Runner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	eventSoundTest1.swf      \
	$(NULL)

streamingSoundTest1_SOURCES = streamingSoundTest1.c

streamingSoundTest1_LDADD = ../libgnashmingutils.la

streamingSoundTest1_CFLAGS= \
	-DMEDIADIR='"$(abs_mediadir)"' \
	$(NULL)

streamingSoundTest1.swf: streamingSoundTest1
	./streamingSoundTest1

streamingSoundTest1_Runner_SOURCES = \
	streamingSoundTest1-Runner.cpp \
	$(NULL)

streamingSoundTest1_Runner_CXXFLAGS = \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)

streamingSoundTest1_Runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)

streamingSoundTest1_Runner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	streamingSoundTest1.swf      \
	$(NULL)

streamingSoundTest2_SOURCES = streamingSoundTest2.c

streamingSoundTest2_LDADD = ../libgnashmingutils.la

streamingSoundTest2_CFLAGS= \
	-DMEDIADIR='"$(abs_mediadir)"' \
	$(NULL)

streamingSoundTest2.swf: streamingSoundTest2
	./streamingSoundTest2 $(abs_mediadir)

streamingSoundTest2_Runner_SOURCES = \
	streamingSoundTest2-Runner.cpp \
	$(NULL)

streamingSoundTest2_Runner_CXXFLAGS = \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)

streamingSoundTest2_Runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)

streamingSoundTest2_Runner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	streamingSoundTest2.swf      \
	$(NULL)

StreamSoundTest_SOURCES = \
	StreamSoundTest.c \
	$(NULL)

StreamSoundTest_CFLAGS = \
	-DMEDIADIR='"$(abs_mediadir)"' \
	$(NULL)

StreamSoundTest_LDADD = ../libgnashmingutils.la

StreamSoundTest.swf: StreamSoundTest
	./StreamSoundTest $(abs_mediadir)

StreamSoundTestRunner_SOURCES = \
	StreamSoundTestRunner.cpp \
	$(NULL)

StreamSoundTestRunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)

StreamSoundTestRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)

StreamSoundTestRunner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	StreamSoundTest.swf	\
	$(NULL)

EmbeddedSoundTest_SOURCES = \
	EmbeddedSoundTest.c \
	$(NULL)

EmbeddedSoundTest_CFLAGS = \
	-DMEDIADIR='"$(abs_mediadir)"' \
	$(NULL)

EmbeddedSoundTest_LDADD = ../libgnashmingutils.la

EmbeddedSoundTest.swf: EmbeddedSoundTest
	./EmbeddedSoundTest $(abs_mediadir)

EmbeddedSoundTest_Runner_SOURCES = \
	EmbeddedSoundTest-Runner.cpp \
	$(NULL)

EmbeddedSoundTest_Runner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)

EmbeddedSoundTest_Runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)

EmbeddedSoundTest_Runner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	EmbeddedSoundTest.swf	\
	$(NULL)


TEST_DRIVERS = ../../simple.exp
TEST_CASES = \
	$(NULL)

if MING_VERSION_0_4_3
TEST_CASES += \
	StreamSoundTestRunner \
	EmbeddedSoundTest-Runner \
	eventSoundTest1-Runner \
	streamingSoundTest1-Runner \
	streamingSoundTest2-Runner \
	$(NULL)
endif

TEST_ENV = GNASH_GC_TRIGGER_THRESHOLD=0
TEST_ENV += GNASHRC=../gnashrc
TEST_ENV += GST_INSTALL_PLUGINS_HELPER=$(top_srcdir)/testsuite/gst_install_plugins_helper.sh

clean-local: 
	-rm *.swf

check-DEJAGNU: site-update $(check_PROGRAMS)
	runtest=$(RUNTEST); \
	if $(SHELL) -c "$$runtest --version" > /dev/null 2>&1; then \
	    $(TEST_ENV) $$runtest $(RUNTESTFLAGS) $(TEST_DRIVERS); true; \
	else \
	  echo "WARNING: could not find \`runtest'" 1>&2; \
          for i in "$(TEST_CASES)"; do \
	    $(TEST_ENV) $(SHELL) $$i; \
	  done; \
	fi

site-update: site.exp
	@rm -fr site.exp.bak
	@cp site.exp site.exp.bak
	@sed -e '/testcases/d' site.exp.bak > site.exp
	@echo "# This is a list of the pre-compiled testcases" >> site.exp
	@echo "set testcases \"$(TEST_CASES)\"" >> site.exp

