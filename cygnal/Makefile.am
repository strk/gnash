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
#

## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = dejagnu

if TESTSUITE
TEST_DIR = testsuite
endif

# if USE_CGI
CGI_DIR = cgi-bin
# endif

SUBDIRS = \
	$(CGI_DIR) \
	$(TEST_DIR) \
	.

# we don't want these anymore
#DIST_SUBDIRS  = ACT IO HTTP Net unit_tests 

AM_CPPFLAGS = # -Wall

AM_LDFLAGS = \
	../libbase/libgnashbase.la \
	../libcore/libgnashcore.la \
	../libnet/libgnashnet.la \
	../libamf/libgnashamf.la \
	$(LIBLTDL) \
	$(GLIB_LIBS) \
	$(LIBXML_LIBS) \
	$(CURL_LIBS) \
	$(BOOST_LIBS) \
	$(LIBINTL) \
	$(PTHREAD_LIBS)

localedir = $(datadir)/locale

INCLUDES = -I.. \
        -I$(top_srcdir)	\
        -I$(top_srcdir)/libnet \
        -I$(top_srcdir)/libamf \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/libmedia \
        -I$(top_srcdir)/libsound \
        -I$(top_srcdir)/backend \
        -I$(top_srcdir)/libcore \
        -I$(top_srcdir)/libcore/asobj \
        -I$(top_srcdir)/libcore/parser \
        -I$(top_srcdir)/libcore/vm \
        -DLOCALEDIR=\"$(localedir)\" \
        $(LIBXML_CFLAGS) \
	$(CURL_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(PTHREAD_CFLAGS)

noinst_HEADERS = \
	cygnal.h \
	rtmp_server.h \
	proc.h \
	crc.h

bin_PROGRAMS = cygnal
lib_LTLIBRARIES = libcygnal.la

cygnal_SOURCES = cygnal.cpp proc.cpp
cygnal_LDADD = $(AM_LDFLAGS) libcygnal.la

libcygnal_la_SOURCES = crc.cpp cvm.cpp rtmp_server.cpp 
libcygnal_la_LIBADD = # $(AM_LDFLAGS) 


# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) $(MUDFLAP_OPT)" LIBS="$(LIBS) $(MUDFLAP_LIB)"
