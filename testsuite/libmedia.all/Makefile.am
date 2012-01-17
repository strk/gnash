# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
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
#

## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = dejagnu

AM_CPPFLAGS = # -Wall

AM_LDFLAGS = \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/libmedia/libgnashmedia.la \
	$(GSTREAMER_PLUGINS_BASE_LIBS) \
	$(GSTREAMER_APP_LIBS) \
	$(GSTREAMER_LIBS) \
	$(BOOST_LIBS) \
	$(NULL)

localedir = $(datadir)/locale

INCLUDES = \
	-I$(top_srcdir)/testsuite \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libmedia/gst \
	-I$(top_srcdir)/cygnal \
	-I$(top_srcdir)/libcore \
	-DLOCALEDIR=\"$(localedir)\" \
	$(BOOST_CFLAGS) \
	$(DEJAGNU_CFLAGS) \
	$(PTHREAD_CFLAGS) \
	$(GSTREAMER_CFLAGS) \
	$(GSTAPP_CFLAGS) \
	$(GSTINTERFACES_CFLAGS) 

check_PROGRAMS = 

if USE_GST_ENGINE

# check_PROGRAMS += \
#	test_videoinput \
#	test_audioinput
#
# test_videoinput_SOURCES = test_videoinput.cpp
# test_videoinput_LDADD = $(AM_LDFLAGS) 
# test_videoinput_DEPENDENCIES = site-update
#
# test_audioinput_SOURCES = test_audioinput.cpp
# test_audioinput_LDADD = $(AM_LDFLAGS)
# test_audioinput_DEPENDENCIES = site-update

endif

TEST_DRIVERS = ../simple.exp

CLEANFILES =  \
	site.exp.bak \
	testrun.* \
	fooBar* \
	*.bin

check-DEJAGNU: site-update
	@runtest=$(RUNTEST); \
	if $(SHELL) -c "$$runtest --version" > /dev/null 2>&1; then \
	  $$runtest $(RUNTESTFLAGS) $(TEST_DRIVERS); true; \
	else \
	  echo "WARNING: could not find \`runtest'" 1>&2; \
	  for i in "$(check_PROGRAMS)"; do \
	    $(SHELL) $$i; \
	  done; \
	fi

site-update: site.exp
	@rm -fr site.exp.bak
	@cp site.exp site.exp.bak
	@sed -e '/testcases/d' site.exp.bak > site.exp
	@echo "# This is a list of the pre-compiled testcases" >> site.exp
	@echo "set testcases \"$(check_PROGRAMS)\"" >> site.exp
