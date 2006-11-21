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
# 

# $Id: Makefile.am,v 1.49 2006/11/21 19:23:57 strk Exp $

## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = 1.6.0
ACLOCAL_AMFLAGS = -I macros
noinst_SCRIPT = autogen.sh

STD_DIRS = \
	libltdl \
	libbase \
	libgeometry \
	libamf \
	server \
	backend \
	utilities \
	gui \
	testsuite

if PLUGIN
if HAVE_GTK2
PLUGIN_DIRS = plugin
endif
endif

if DOCBOOK
DOC_DIRS = doc
endif

if PLUGIN
if KLASH
KLASH_DIRS = plugin/klash
endif
endif

SUBDIRS      = $(STD_DIRS) $(DOC_DIRS) $(PLUGIN_DIRS) $(KLASH_DIRS)

man_MANS = doc/gnash.1

# libamf isn't used yet, but we still want it in the distribution
DIST_SUBDIRS = $(STD_DIRS) doc plugin

EXTRA_DIST= $(man_MANS)
#EXTRA_DIST = macros
# if HTTP
# SUBDIRS += net $(STD_DIRS)
# else
# SUBDIRS += $(STD_DIRS)
# endif

noinst_HEADERS = compatibility_include.h

# Build an RPM package
rpm redhat: dist-gzip $(srcdir)/packaging/redhat/gnash.spec
	cp -f $(srcdir)/packaging/redhat/gnash.spec /usr/src/redhat/SPECS/
	cp -f gnash-${VERSION}.tar.gz /usr/src/redhat/SOURCES/
	rpmbuild -ba /usr/src/redhat/SPECS/gnash.spec

# Build a Debian GNU/Linux package
deb debian: dist-bzip2 $(srcdir)/packaging/debian/rules
	rm -rf gnash-${VERSION}
	tar jxf gnash-${VERSION}.tar.bz2
	cp -fr $(srcdir)/packaging/debian gnash-${VERSION}/
	(cd gnash-${VERSION}; dpkg-buildpackage)

snapshot: distdir
	now=`date -u "+%Y%m%d"`; \
	mv gnash-${VERSION} gnash-$${now}; \
	tardir=gnash-$${now} && $(am__tar) | bzip2 -9 -c > gnash-$${now}.tar.bz2; \
	$(RM) -fr gnash-$${now}

mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflapth" LDFLAGS="$(LDFLAGS) -lmudflapth"

mudflap-check:
	@echo "Rechecking with GCC Mudflap support"
	$(MAKE) check CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

# Set a few variables to what features we selected so we cab dump it with the
# test of the config
if USE_SOUND_SDL
  SOUND_OPT=sdl
else
  SOUND_OPT=none
endif
if USE_SOUND_GST
  SOUND_OPT=gst
endif
if USE_MAD_ENGINE
  ENGINE_OPT=mad
else
  ENGINE_OPT=none
endif
if USE_FFMPEG_ENGINE
  ENGINE_OPT=ffmpeg
endif

if USE_GUI_GTK
  GUI_OPT=gtk2
endif
if USE_GUI_SDL
  GUI_OPT=sdl
endif
if USE_GUI_KDE
  GUI_OPT=kde
endif
if USE_GUI_FLTK
  GUI_OPT=fltk2
endif
if USE_RENDERER_OPENGL
  RENDER_OPT=opengl
endif
if USE_RENDERER_CAIRO
  RENDER_OPT=cairo
endif
if USE_RENDERER_AGG
  RENDER_OPT=agg
endif

dumpconfig:
	@uname -a
	$(CC) -v
	@echo "JPEG_CFLAGS is $(JPEG_CFLAGS)"
	@echo "JPEG_LIBS is $(JPEG_LIBS)"
	@echo "PNG_CFLAGS is $(PNG_CFLAGS)"
	@echo "PNG_LIBS is $(PNG_LIBS)"
	@echo "OGG_CFLAGS is $(OGG_CFLAGS)"
	@echo "OGG_LIBS is $(OGG_LIBS)"
	@echo "ZLIB_CFLAGS is $(ZLIB_CFLAGS)"
	@echo "ZLIB_LIBS is $(ZLIB_LIBS)"
	@echo "LIBXML_CFLAGS is $(LIBXML_CFLAGS)"
	@echo "LIBXML_LIBS is $(LIBXML_LIBS)"
	@echo "SDL_CFLAGS is $(SDL_CFLAGS)"
	@echo "SDL_LIBS is $(SDL_LIBS)"
	@echo "OPENGL_CFLAGS is $(OPENGL_CFLAGS)"
	@echo "OPENGL_LIBS is $(OPENGL_LIBS)"
	@echo "GLEXT_CFLAGS is $(GLEXT_CFLAGS)"
	@echo "GLEXT_LIBS is $(GLEXT_LIBS)"
	@echo "GTK2_CFLAGS is $(GTK2_CFLAGS)"
	@echo "GTK2_LIBS is $(GTK2_LIBS)"
	@echo "FLTK_CFLAGS is $(FLTK_CFLAGS)"
	@echo "FLTK_LIBS is $(FLTK_LIBS)"
	@echo "XFT_CFLAGS is $(XFT_CFLAGS)"
	@echo "XFT_LIBS is $(XFT_LIBS)"
	@echo "PANGO_CFLAGS is $(PANGO_CFLAGS)"
	@echo "PANGO_LIBS is $(PANGO_LIBS)"
	@echo "ATK_CFLAGS is $(ATK_CFLAGS)"
	@echo "ATK_LIBS is $(ATK_LIBS)"
	@echo "GLIB_CFLAGS is $(GLIB_CFLAGS)"
	@echo "GLIB_LIBS is $(GLIB_LIBS)"
	@echo "X_CFLAGS is $(X_CFLAGS)"
	@echo "X_LIBS is $(X_LIBS)"
	@echo "DMALLOC_CFLAGS is $(DMALLOC_CFLAGS)"
	@echo "DMALLOC_LIBS is $(DMALLOC_LIBS)"
	@echo "MAD_CFLAGS is $(MAD_CFLAGS)"
	@echo "MAD_LIBS is $(MAD_LIBS)"
	@echo "PTHREAD_CFLAGS is $(PTHREAD_CFLAGS)"
	@echo "PTHREAD_LIBS is $(PTHREAD_LIBS)"
	@echo "CURL_CFLAGS is $(CURL_CFLAGS)"
	@echo "CURL_LIBS is $(CURL_LIBS)"
	@echo "GSTREAMER_CFLAGS is $(GSTREAMER_CFLAGS)"
	@echo "GSTREAMER_LIBS is $(GSTREAMER_LIBS)"
	@echo "FFMPEG_CFLAGS is $(FFMPEG_CFLAGS)"
	@echo "FFMPEG_LIBS is $(FFMPEG_LIBS)"
	@echo "KDE_CFLAGS is $(KDE_CFLAGS)"
	@echo "KDE_LIBS is $(KDE_LIBS)"
	@echo "CAIRO_CFLAGS is $(CAIRO_CFLAGS)"
	@echo "CAIRO_LIBS is $(CAIRO_LIBS)"
	@echo "BOOST_CFLAGS is $(BOOST_CFLAGS)"
	@echo "BOOST_LIBS is $(BOOST_LIBS)"
	@echo "AGG_CFLAGS is $(AGG_CFLAGS)"
	@echo "AGG_LIBS is $(AGG_LIBS)"
	@echo "INCLTDL $(INCLTDL) "
	@echo "LIBLTDL $(LIBLTDL) "
	@echo "A blank value for CFLAGS means the header is installed in the"
	@echo "default system header location. All the LIBS should have a"
	@echo "legit value"
	@echo ""
	@echo "Configurable options are:"
	@echo "Sound handler: $(SOUND_OPT)"
	@echo "MP3 decoder: $(ENGINE_OPT)"
	@echo "Renderer engine: $(RENDER_OPT)"
	@echo "GUI: $(GUI_OPT)"
