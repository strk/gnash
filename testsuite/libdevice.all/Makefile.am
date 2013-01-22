## Process this fill with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006,2007, 2008, 2009, 2010, 2011, 2012
#   Free Software Foundation, Inc.
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

LDADD = \
	$(top_builddir)/libbase/libgnashbase.la \
	$(LIBINTL) \
	$(NULL)

if ANDROID
LDADD +=  -lui -llog
endif	# ANDROID

AM_LDFLAGS = $(CROSS_LDFLAGS)
AM_CXXFLAGS = $(CROSS_CXXFLAGS)

# AM_CPPFLAGS isn't right here since per-program *_CPPFLAGS override global
# AM_CPPFLAGS
AM_CPPFLAGS = \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/libdevice \
        -I$(top_srcdir)/testsuite \
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

check_PROGRAMS = 

if BUILD_EGL_DEVICE
# this tests the low level EGL interface, used by OpenVG, OpenGLES1, and OpenGLES2.
check_PROGRAMS += test_egl 

test_egl_SOURCES = test_egl.cpp
test_egl_CPPFLAGS = $(AM_CPPFLAGS) $(EGL_CFLAGS) -I$(top_srcdir)/libdevice/egl
test_egl_LDADD = \
	../../libdevice/libgnashdevice.la \
	../../libbase/libgnashbase.la \
	$(EXTRA_EGL_LIBS) \
	$(EGL_LIBS) \
	$(BOOST_LIBS) \
	$(NULL)
endif

# if BUILD_X11_DEVICE
# check_PROGRAMS += test_x11
# endif

if BUILD_DIRECTFB_DEVICE
check_PROGRAMS += test_directfb
endif

if ENABLE_INPUT_EVENTS
check_PROGRAMS += test_events

test_events_SOURCES = test_events.cpp
test_events_CPPFLAGS = $(AM_CPPFLAGS) \
	-I$(top_srcdir)/libdevice/events \
	-I$(top_srcdir)/libcore
test_events_LDADD = \
	../../libdevice/libgnashdevice.la \
	../../libbase/libgnashbase.la \
        $(EXTRA_EVENTS_LIBS) \
        $(TS_LIBS) \
        $(CURL_LIBS) \
        $(EVENTS_LIBS) \
        $(NULL)
endif

if BUILD_RAWFB_DEVICE
check_PROGRAMS += test_rawfb

# this tests the low level EGL interface, used by OpenVG, OpenGLES1,
# and OpenGLES2.
test_rawfb_SOURCES = test_rawfb.cpp
test_rawfb_CPPFLAGS = $(AM_CPPFLAGS) \
	 -I$(top_srcdir)/libbase \
	 -I$(top_srcdir)/libdevice/rawfb
test_rawfb_LDADD = \
	../../libdevice/libgnashdevice.la \
	../../libbase/libgnashbase.la \
	$(NULL)
endif

if BUILD_VAAPI_DEVICE
check_PROGRAMS += test_vaapi
endif

#
# Testing stuff
#
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
