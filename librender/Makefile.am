# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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

AM_CPPFLAGS = -I.. \
        -I$(srcdir) \
        -I$(top_srcdir) \
        -I$(top_srcdir)/libcore \
        -I$(top_srcdir)/libcore/parser \
        -I$(top_srcdir)/libcore/swf \
        -I$(top_srcdir)/libbase \
	-I$(top_srcdir)/librender \
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

GNASH_LIBS = \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/libcore/libgnashcore.la

noinst_HEADERS = \
	Renderer.h \
	Renderer_agg.h \
	Renderer_agg_bitmap.h \
	Renderer_agg_style.h \
	Renderer_cairo.h \
	Renderer_ogl.h \
        PathParser.h

pkglib_LTLIBRARIES = libgnashrender.la
libgnashrender_la_LIBADD = $(LIBVA_LIBS) $(LIBVA_X11_LIBS) $(LIBVA_GLX_LIBS)
libgnashrender_la_SOURCES = PathParser.cpp

if BUILD_OGL_RENDERER
libgnashrender_la_SOURCES += Renderer_ogl.cpp
libgnashrender_la_LIBADD += $(OPENGL_LIBS)
endif

if  BUILD_AGG_RENDERER
libgnashrender_la_SOURCES += Renderer_agg.cpp 
libgnashrender_la_LIBADD += $(AGG_LIBS) $(LIBVA)
endif

if  BUILD_CAIRO_RENDERER
libgnashrender_la_SOURCES += Renderer_cairo.cpp
libgnashrender_la_LIBADD += $(CAIRO_LIBS)
endif

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="`$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*


if ENABLE_PCH
AM_CXXFLAGS = $(PCH_FLAGS)
endif
