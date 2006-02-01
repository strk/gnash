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

if MP3
MP3_CFLAGS = $(MAD_CFLAGS)
MP3_LIBS = $(MAD_LIBS)
endif

AM_CPPFLAGS = -Wall
AM_LDFLAGS = \
	../server/libserver.la \
	../server/libasobjs.la \
	../libgeometry/libgeometry.la \
	 ../libbase/libbase.la \
        $(OPENGL_LIBS)      \
        $(SDL_LIBS)         \
        $(SDL_MIXER_LIBS)   \
	$(JPEG_LIBS)	    \
	$(PNG_LIBS)	    \
        $(LIBXML_LIBS)      \
	$(DMALLOC_LIBS)	    \
	$(MP3_LIBS)	    \
	$(OGG_LIBS)

INCLUDES = -I.. \
        -I$(top_srcdir)		\
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/server  \
        -I$(top_srcdir)/libgeometry \
	$(LIBXML_CFLAGS)        \
	$(OPENGL_CFLAGS)	\
	$(DMALLOC_CFLAGS)	\
	$(MP3_CFLAGS)		\
	$(OGG_CFLAGS)		\
        $(SDL_CFLAGS)		\
        $(SDL_MIXER_CFLAGS)

bin_PROGRAMS = gparser gprocessor

gparser_SOURCES = \
	parser.cpp

gprocessor_SOURCES = \
	processor.cpp

gparser_LDADD = $(AM_LDFLAGS)
gprocessor_LDADD = $(AM_LDFLAGS)
# gnash_DEPENDENCIES = 

MUDFLAP_OPT = -fmudflap
MUDFLAP_LIB =  /usr/local/lib/libmudflap.so

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) $(MUDFLAP_OPT)" LIBS="$(LIBS) $(MUDFLAP_LIB)"

