# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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
        -I$(top_srcdir)/libamf \
        -I$(top_srcdir)/libnet \
        -I$(top_srcdir)/libcore \
        -I$(top_srcdir)/libcore/parser \
        -I$(top_srcdir)/libcore/swf \
        -I$(top_srcdir)/libbase \
	-I$(top_srcdir)/backend \
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
	Renderer.h \
	Renderer_agg.h \
	Renderer_agg_bitmap.h \
	Renderer_agg_style.h \
	Renderer_cairo.h \
	Renderer_ogl.h \
        PathParser.h

# bin_PROGRAMS = gnash

noinst_LTLIBRARIES =		# set later on by conditionals

if  BUILD_OGL_RENDERER
if HAVE_OPENGL
noinst_LTLIBRARIES += libgnashogl.la
#plugins_LTLIBRARIES += libgnashogl.la
libgnashogl_la_SOURCES = Renderer_ogl.cpp PathParser.cpp
#libgnashogl_la_LDFLAGS = -module -avoid-version -no-undefined
libgnashogl_la_LDFLAGS =
libgnashogl_la_LIBADD = $(OPENGL_LIBS)
endif
endif

if  BUILD_AGG_RENDERER
if HAVE_AGG
noinst_LTLIBRARIES += libgnashagg.la
#plugins_LTLIBRARIES += libgnashagg.la
libgnashagg_la_SOURCES = Renderer_agg.cpp 
#libgnashagg_la_LDFLAGS = -module -avoid-version -no-undefined
libgnashagg_la_LIBADD = $(AGG_LIBS)
endif
endif

if  BUILD_CAIRO_RENDERER
if HAVE_CAIRO
noinst_LTLIBRARIES += libgnashcairo.la
# plugins_LTLIBRARIES += libgnashcairo.la
libgnashcairo_la_SOURCES = Renderer_cairo.cpp PathParser.cpp
#libgnashcairo_la_LDFLAGS = -module -avoid-version -no-undefined
libgnashcairo_la_LIBADD = $(CAIRO_LIBS)
endif
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
