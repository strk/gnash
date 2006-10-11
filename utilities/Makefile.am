# 
#   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
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

AUTOMAKE_OPTIONS = # dejagnu

# @@ shouldn't we drop package_LIBS and leave deps
#    on the gnash libraries side instead ?

GNASH_LIBS = \
	$(top_builddir)/backend/libgnashbackend.la \
	$(top_builddir)/server/libgnashserver.la \
	$(top_builddir)/server/asobj/libgnashasobjs.la \
	$(top_builddir)/libgeometry/libgnashgeo.la \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/libamf/libgnashamf.la 

AM_CPPFLAGS = # -Wall

## WARNING: make sure GLIB_LIBS appears first
## See: http://lists.gnu.org/archive/html/gnash-dev/2006-07/msg00076.html
AM_LDFLAGS = \
	$(GLIB_LIBS) \
	$(BOOST_LIBS) \
	$(PTHREAD_LIBS)

INCLUDES = -I.. \
        -I$(top_srcdir)	\
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/server \
        -I$(top_srcdir)/server/parser \
        -I$(top_srcdir)/libgeometry \
        $(LIBXML_CFLAGS) \
        $(SDL_CFLAGS) \
	$(BOOST_CFLAGS)

if USE_FFMPEG_ENGINE
 INCLUDES += $(FFMPEG_INCLS) 
 AM_LDFLAGS += $(FFMPEG_LIBS)
endif

if USE_MAD_ENGINE
 INCLUDES += $(MAD_INCLS) 
 AM_LDFLAGS += $(MAD_LIBS)
endif

bin_PROGRAMS = gparser gprocessor

gparser_SOURCES = \
	parser.cpp
gparser_LDADD = $(GNASH_LIBS)
#gparser_LDFLAGS = $(AM_LDFLAGS)

gprocessor_SOURCES = \
	processor.cpp
gprocessor_LDADD = $(GNASH_LIBS)
#gprocessor_LDFLAGS = $(AM_LDFLAGS)

MUDFLAP_OPT = -fmudflap
MUDFLAP_LIB =  /usr/local/lib/libmudflap.so

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) $(MUDFLAP_OPT)" LIBS="$(LIBS) $(MUDFLAP_LIB)"

