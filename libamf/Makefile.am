## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

# $Id: Makefile.am,v 1.17 2006/10/29 18:34:18 rsavoye Exp $

AUTOMAKE_OPTIONS = 

AM_CXXFLAGS =  $(CXXFLAGS) $(SDL_CFLAGS) $(INCLUDES)
AM_CPPFLAGS = 
AM_LDFLAGS = \
	$(top_builddir)/libbase/libgnashbase.la \
        $(OPENGL_LIBS)      \
        $(SDL_LIBS)         \
	$(X_LIBS)	    \
	$(JPEG_LIBS)	    \
	$(PNG_LIBS)	    \
        $(LIBXML_LIBS)      \
	$(DMALLOC_LIBS)	    \
	$(MAD_LIBS)	    \
	$(OGG_LIBS)


# AM_LDFLAGS  = $(OPENGL_LIBNS) $(SDL_LIBS)

lib_LTLIBRARIES = libgnashamf.la
libgnashamf_la_SOURCES = amf.cpp rtmp.cpp
# noinst_HEADERS = amf.h amfutf8.h

INCLUDES = -I.. -I$(srcdir) \
	-I$(top_srcdir)/server \
	-I$(top_srcdir)/libbase \
        $(LIBXML_CFLAGS)

noinst_HEADERS = amfutf8.h amf.h rtmp.h protocol.h

EXTRA_PROGRAMS = amftest
amftest_SOURCES = amftest.cpp
amftest_LDADD = $(AM_LDFLAGS) libgnashamf.la

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

