## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

# $Id: Makefile.am,v 1.32 2008/01/21 22:55:26 rsavoye Exp $

AUTOMAKE_OPTIONS = 

pkglib_LTLIBRARIES = libgnashamf.la
libgnashamf_la_SOURCES = \
	amf.cpp \
	element.cpp \
	lcshm.cpp \
	sol.cpp
#	rtmp.cpp

libgnashamf_la_LIBADD = \
	$(top_builddir)/libbase/libgnashbase.la

libgnashamf_la_LDFLAGS =  -release $(VERSION) ${AM_LDFLAGS} 
# noinst_HEADERS = amf.h amfutf8.h

AM_CPPFLAGS = -I.. -I$(srcdir) \
	-I$(top_srcdir)/server \
	-I$(top_srcdir)/libbase \
	$(BOOST_CFLAGS) \
        $(LIBXML_CFLAGS)

AM_LDFLAGS = $(BOOST_LIBS)

noinst_HEADERS = \
	amfutf8.h \
	amf.h \
	element.h \
	lcshm.h \
	rtmp.h \
	protocol.h \
	sol.h

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

