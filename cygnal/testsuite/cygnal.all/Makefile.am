# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
#   2011 Free Software Foundation, Inc.
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

noinst_LTLIBRARIES = libcygnal.la
libcygnal_la_SOURCES = \
	$(top_builddir)/cygnal/crc.cpp

libcygnal_la_LDFLAGS = \
	$(top_builddir)/libamf/libgnashamf.la

AM_CPPFLAGS = # -Wall

AM_LDFLAGS = \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/libamf/libgnashamf.la \
	$(top_builddir)/libnet/libgnashnet.la \
	$(top_builddir)/cygnal/libcygnal.la \
	$(BOOST_LIBS) \
	$(PTHREAD_LIBS)

localedir = $(datadir)/locale

INCLUDES = \
        -I$(top_srcdir)/libamf \
        -I$(top_srcdir)/libnet \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/cygnal \
        -I$(top_srcdir)/libcore \
        -DLOCALEDIR=\"$(localedir)\" \
		$(BOOST_CFLAGS) \
		$(DEJAGNU_CFLAGS) \
		$(PTHREAD_CFLAGS)

check_PROGRAMS = \
	test_crc

test_crc_SOURCES = test_crc.cpp
test_crc_LDADD = $(AM_LDFLAGS) 
test_crc_DEPENDENCIES = site-update

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) $(MUDFLAP_OPT)" LIBS="$(LIBS) $(MUDFLAP_LIB)"

TEST_DRIVERS = ../cygnal.exp

CLEANFILES =  \
	site.exp.bak \
	testrun.* \
	foo* \
	*.bin* \
	wget-log* \
	gateway*

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
