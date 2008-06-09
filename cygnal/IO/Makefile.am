# 
#   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

AUTOMAKE_OPTIONS = #dejagnu

AM_CPPFLAGS = # -Wall

## WARNING: make sure GLIB_LIBS appears first
## See: http://lists.gnu.org/archive/html/gnash-dev/2006-07/msg00076.html
AM_LDFLAGS = \
	../../libbase/libgnashbase.la \
	../../libamf/libgnashamf.la \
	$(LIBLTDL) \
	$(GLIB_LIBS) \
	$(LIBXML_LIBS) \
	$(CURL_LIBS) \
	$(BOOST_LIBS) \
	$(PTHREAD_LIBS)

localedir = $(datadir)/locale

INCLUDES = \
        -I$(top_srcdir)/cygnal	\
        -I$(top_srcdir)/libamf \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/server \
        -DLOCALEDIR=\"$(localedir)\" \
        $(LIBXML_CFLAGS) \
	$(DMALLOC_CFLAGS) \
	$(CURL_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(PTHREAD_CFLAGS)

lib_LTLIBRARIES = libcygnalio.la
libcygnalio_la_SOURCES= \
	Buffer.cpp \
	IO_Device.cpp \
	IO_Generator.cpp \
	Stream_Consumer.cpp

noinst_HEADERS = \
	Buffer.hpp \
	IO_Device.hpp \
	IO_Generator.hpp \
	Stream_Consumer.hpp

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) $(MUDFLAP_OPT)" LIBS="$(LIBS) $(MUDFLAP_LIB)"
