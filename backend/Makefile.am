# 
#   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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

AUTOMAKE_OPTIONS =

#AM_CPPFLAGS = # -Wall

## WARNING: make sure GLIB_LIBS appears first
## See: http://lists.gnu.org/archive/html/gnash-dev/2006-07/msg00076.html
libgnashbackend_LIBS =  \
	$(GLIB_LIBS) \
	$(JPEG_LIBS) \
	$(PNG_LIBS) \
        $(OGG_LIBS) \
	$(Z_LIBS) \
        $(SDL_LIBS) \
	$(PANGO_LIBS) \
	$(ATK_LIBS) \
	$(X_LIBS) \
	$(DMALLOC_LIBS)	\
	$(PTHREAD_LIBS)	\
	$(NULL)

AM_CPPFLAGS = -I.. \
        -I$(srcdir) \
        -I$(top_srcdir) \
        -I$(top_srcdir)/server \
        -I$(top_srcdir)/server/parser \
        -I$(top_srcdir)/libbase \
	-I$(top_srcdir)/backend \
        -I$(top_srcdir)/libgeometry \
	$(PTHREAD_CFLAGS) \
        $(SDL_CFLAGS) \
	$(PANGO_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(ATK_CFLAGS) \
	$(DMALLOC_CFLAGS) \
	$(PNG_CFLAGS) \
	$(JPEG_CFLAGS) \
	$(OGG_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(NULL)

noinst_HEADERS = \
	render_handler.h \
	render_handler_agg.h \
	render_handler_agg_bitmap.h \
	render_handler_agg_compat.h \
	render_handler_agg_style.h \
	render_handler_cairo.h \
	render_handler_tri.h \
	sound_handler.h \
	sound_handler_gst.h \
	sound_handler_sdl.h

# bin_PROGRAMS = gnash

#noinst_LTLIBRARIES = libgbackend.la
lib_LTLIBRARIES = libgnashbackend.la

RENDER_SOURCES = render_handler_tri.cpp
SOUND_SOURCES = sound_handler.cpp

if USE_RENDERER_AGG
RENDER_SOURCES += render_handler_agg.cpp
libgnashbackend_LIBS += $(AGG_LIBS)
AM_CPPFLAGS += $(AGG_CFLAGS)
endif

if USE_RENDERER_CAIRO
RENDER_SOURCES += render_handler_cairo.cpp
libgnashbackend_LIBS += $(CAIRO_LIBS)
AM_CPPFLAGS += $(CAIRO_CFLAGS)
endif

if USE_RENDERER_OPENGL
RENDER_SOURCES += render_handler_ogl.cpp
libgnashbackend_LIBS += $(OPENGL_LIBS) # $(GLEXT_LIBS)		
AM_CPPFLAGS += $(OPENGL_CFLAGS) # $(GLEXT_CFLAGS)	
endif

if USE_SOUND_GST
SOUND_SOURCES += sound_handler_gst.cpp sound_handler_gst.h 
libgnashbackend_LIBS += $(GSTREAMER_LIBS) 
libgnashbackend_LIBS += $(LIBXML_LIBS) 
AM_CPPFLAGS += $(GSTREAMER_CFLAGS)
AM_CPPFLAGS += $(LIBXML_CFLAGS)
endif

if USE_SOUND_SDL
SOUND_SOURCES += sound_handler_sdl.cpp sound_handler_sdl.h
if USE_FFMPEG_ENGINE
libgnashbackend_LIBS += $(FFMPEG_LIBS)
AM_CPPFLAGS += $(FFMPEG_CFLAGS)
endif
if USE_MAD_ENGINE
libgnashbackend_LIBS += $(MAD_LIBS)
AM_CPPFLAGS += $(MAD_CFLAGS)
endif
endif

libgnashbackend_la_LIBADD = \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/server/libgnashserver.la  \
	$(libgnashbackend_LIBS)

libgnashbackend_la_SOURCES = \
	$(RENDER_SOURCES) \
	$(SOUND_SOURCES)

libgnashbackend_la_LDFLAGS = -release $(VERSION)

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*
