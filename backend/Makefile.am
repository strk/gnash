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

# this is where Gnash plugins get installed
pluginsdir = $(libdir)/gnash/plugins

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
	$(OPENGL_CFLAGS) \
	$(AGG_CFLAGS) \
	$(CAIRO_CFLAGS) \
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

pkglib_LTLIBRARIES = libgnashbackend.la

# RENDER_SOURCES = render_handler_tri.cpp
SOUND_SOURCES = 

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

libgnashbackend_la_SOURCES = $(SOUND_SOURCES)

libgnashbackend_la_LDFLAGS = -release $(VERSION)

noinst_LTLIBRARIES =		# set later on by conditionals
if HAVE_OPENGL
noinst_LTLIBRARIES += libgnashogl.la
#plugins_LTLIBRARIES += libgnashogl.la
libgnashogl_la_SOURCES = render_handler_tri.cpp render_handler_ogl.cpp 
#libgnashogl_la_LDFLAGS = -module -avoid-version -no-undefined
libgnashogl_la_LDFLAGS =
endif

if HAVE_AGG
noinst_LTLIBRARIES += libgnashagg.la
#plugins_LTLIBRARIES += libgnashagg.la
libgnashagg_la_SOURCES = render_handler_tri.cpp render_handler_agg.cpp 
#libgnashagg_la_LDFLAGS = -module -avoid-version -no-undefined
libgnashagg_la_LDFLAGS = 
libgnashagg_la_LIBADD = $(AGG_LIBS)
endif

if HAVE_CAIRO
noinst_LTLIBRARIES += libgnashcairo.la
# plugins_LTLIBRARIES += libgnashcairo.la
libgnashcairo_la_SOURCES = render_handler_tri.cpp  render_handler_cairo.cpp
#libgnashcairo_la_LDFLAGS = -module -avoid-version -no-undefined
libgnashcairo_la_LDFLAGS =
endif

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

# install-pluginsLTLIBRARIES: $(plugins_LTLIBRARIES)
# 	test -d "$(DESTDIR)$(pluginsdir)" || $(mkinstalldirs) "$(DESTDIR)$(pluginsdir)"
# 	for i in $(plugins_LTLIBRARIES); do \
# 	  $(LIBTOOL) --mode=install $(INSTALL) $(INSTALL_STRIP_FLAG) $$i "$(DESTDIR)$(pluginsdir)/$$i" ; \
# 	done
