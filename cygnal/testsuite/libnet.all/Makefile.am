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

AM_CPPFLAGS = # -Wall

AM_LDFLAGS = \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/cygnal/libamf/libgnashamf.la \
	$(top_builddir)/cygnal/libnet/libgnashnet.la \
	$(top_builddir)/libmedia/libgnashmedia.la \
	$(top_builddir)/libsound/libgnashsound.la \
	$(BOOST_LIBS) \
	$(DEJAGNU_CFLAGS) \
	$(NULL)

localedir = $(datadir)/locale

INCLUDES = \
        -I$(top_srcdir)/testsuite \
        -I$(top_srcdir)/cygnal/libnet \
        -I$(top_srcdir)/cygnal/libamf \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/libltdl \
        -I$(top_srcdir)/cygnal \
        -I$(top_srcdir)/libcore \
        -DLOCALEDIR=\"$(localedir)\" \
	$(BOOST_CFLAGS) \
	$(DEJAGNU_CFLAGS) \
	$(PTHREAD_CFLAGS)

test_progs = \
	test_cque \
	test_http \
	test_diskstream \
	test_cache \
	test_rtmp 
#	test_handler

# this is a utility program used to generate binary AMF files for testing protocols.
check_PROGRAMS = generate_amfbins $(test_progs)

generate_amfbins_SOURCES = generate_amfbins.cpp
generate_amfbins_LDADD = $(AM_LDFLAGS) 
generate_amfbins_DEPENDENCIES = site-update

test_http_SOURCES = test_http.cpp
test_http_LDADD = $(AM_LDFLAGS) 
test_http_DEPENDENCIES = site-update

test_cache_SOURCES = test_cache.cpp
test_cache_LDADD = $(AM_LDFLAGS) 
test_cache_DEPENDENCIES = site-update

test_diskstream_SOURCES = test_diskstream.cpp
test_diskstream_LDADD = $(AM_LDFLAGS) 
test_diskstream_DEPENDENCIES = site-update

test_rtmp_SOURCES = test_rtmp.cpp
test_rtmp_LDADD = $(AM_LDFLAGS) 
test_rtmp_DEPENDENCIES = site-update

test_cque_SOURCES = test_cque.cpp
test_cque_LDADD = $(AM_LDFLAGS) 
test_cque_DEPENDENCIES = site-update

# test_handler_SOURCES = test_handler.cpp
# test_handler_LDADD = $(AM_LDFLAGS) 
# test_handler_DEPENDENCIES = site-update

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) $(MUDFLAP_OPT)" LIBS="$(LIBS) $(MUDFLAP_LIB)"

TEST_DRIVERS = ../simple.exp

CLEANFILES =  \
	site.exp.bak \
	testrun.* \
	fooBar* \
	generate_amfbins \
	*.bin

check-DEJAGNU: site-update
	@runtest=$(RUNTEST); \
	if $(SHELL) -c "$$runtest --version" > /dev/null 2>&1; then \
	  $$runtest $(RUNTESTFLAGS) $(TEST_DRIVERS); true; \
	else \
	  echo "WARNING: could not find \`runtest'" 1>&2; \
	  for i in " $(test_progs)"; do \
	    $(SHELL) $$i; \
	  done; \
	fi

site-update: site.exp
	@rm -fr site.exp.bak
	@cp site.exp site.exp.bak
	@sed -e '/testcases/d' site.exp.bak > site.exp
	@echo "# This is a list of the pre-compiled testcases" >> site.exp
	@echo "set testcases \"$(check_PROGRAMS)\"" >> site.exp
