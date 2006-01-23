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

AM_LDFLAGS =  ../libbase/libbase.la \
	../libgeometry/libgeometry.la \
	../server/libserver.la \
	$(JPEG_LIBS)		\
	$(PNG_LIBS)		\
        $(LIBXML_LIBS)		\
        $(SDL_LIBS)		\
        $(SDL_CFLAGS)		\
        $(SDL_MIXER_LIBS)	\
        $(OPENGL_LIBS)		\
	$(DMALLOC_LIBS)		\
	$(MP3_LIBS)		\
        $(OGG_LIBS)		\
        $(VORBIS_LIBS)

INCLUDES = -I.. \
        -I$(top_srcdir) 	\
        -I$(top_srcdir)/server  \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/libgeometry \
        $(SDL_MIXER_CFLAGS)	\
	$(LIBXML_CFLAGS)        \
	$(OPENGL_CFLAGS)	\
	$(DMALLOC_CFLAGS)		\
	$(MP3_CFLAGS)		\
	$(OGG_CFLAGS)		\
	$(VORBIS_CFLAGS)

bin_PROGRAMS = gnash

lib_LTLIBRARIES = libgbackend.la

libgbackend_la_SOURCES = \
	render_handler_ogl.cpp	\
	sound_handler_sdl.cpp

gnash_SOURCES = gnash.cpp

gnash_LDADD = $(AM_LDFLAGS) libgbackend.la
# gnash_DEPENDENCIES = 

MUDFLAP_OPT = -fmudflap
MUDFLAP_LIB =  /usr/local/lib/libmudflap.so

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) $(MUDFLAP_OPT)" LIBS="$(LIBS) $(MUDFLAP_LIB)"

