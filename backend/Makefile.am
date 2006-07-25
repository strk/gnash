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

AM_CPPFLAGS = # -Wall

AM_LDFLAGS =  \
	$(JPEG_LIBS)		\
	$(PNG_LIBS)		\
        $(OGG_LIBS)		\
	$(ZLIB_LIBS)		\
	$(CURL_LIBS)		\
        $(LIBXML_LIBS)		\
        $(SDL_LIBS)		\
        $(SDL_MIXER_LIBS)	\
        $(OPENGL_LIBS)		\
	$(GLEXT_LIBS)		\
	$(GTK2_LIBS)		\
	$(PANGO_LIBS)		\
	$(CAIRO_LIBS)		\
	$(ATK_LIBS)		\
	$(GLIB_LIBS)		\
	$(X_LIBS)		\
	$(DMALLOC_LIBS)		\
	$(MP3_LIBS)		\
	$(PTHREAD_LIBS)		\
	$(GSTREAMER_LIBS)

INCLUDES = -I.. \
        -I$(top_srcdir) 	\
        -I$(top_srcdir)/server  \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/backend \
        -I$(top_srcdir)/libgeometry \
	$(PTHREAD_CFLAGS)	\
        $(SDL_CFLAGS)		\
        $(SDL_MIXER_CFLAGS)	\
	$(LIBXML_CFLAGS)        \
	$(OPENGL_CFLAGS)	\
	$(GLEXT_CFLAGS)		\
	$(GTK2_CFLAGS)		\
	$(PANGO_CFLAGS)		\
	$(GLIB_CFLAGS)		\
	$(CAIRO_CFLAGS)		\
	$(ATK_CFLAGS)		\
	$(DMALLOC_CFLAGS)	\
	$(PNG_CFLAGS)		\
	$(JPEG_CFLAGS)		\
	$(MP3_CFLAGS)		\
	$(OGG_CFLAGS)		\
	$(GSTREAMER_CFLAGS)

# bin_PROGRAMS = gnash

#noinst_LTLIBRARIES = libgbackend.la
lib_LTLIBRARIES = libgnashbackend.la

if USE_RENDERER_CAIRO
RENDER_SOURCES = render_handler_cairo.cpp
endif
if USE_RENDERER_OPENGL
RENDER_SOURCES = render_handler_ogl.cpp
endif

if USE_SOUND_GST
SOUND_SOURCES = sound_handler_gst.cpp 
endif
if USE_SOUND_SDL
SOUND_SOURCES = sound_handler_sdl.cpp 
endif

libgnashbackend_la_SOURCES = \
	$(RENDER_SOURCES) \
	$(SOUND_SOURCES)

# gnash_SOURCES = gnash.cpp
# gnash_LDFLAGS = -module -avoid-version -no-undefined #-Wl,-z,defs
# gnash_LDADD =  $(AM_LDFLAGS) \
#	gtksup.cpp gtksup.h
# 	libgnashbackend.la \
# 	../server/libgnashserver.la \
# 	../server/asobj/libgnashasobjs.la \
# 	../libgeometry/libgnashgeo.la \
# 	 ../libbase/libgnashbase.la 

MUDFLAP_OPT = -fmudflap
MUDFLAP_LIB =  /usr/local/lib/libmudflap.so

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) $(MUDFLAP_OPT)" LIBS="$(LIBS) $(MUDFLAP_LIB)"

