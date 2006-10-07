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
# Linking Gnash statically or dynamically with other modules is making a
# combined work based on Gnash. Thus, the terms and conditions of the GNU
# General Public License cover the whole combination.
#
# As a special exception, the copyright holders of Gnash give you
# permission to combine Gnash with free software programs or libraries
# that are released under the GNU LGPL and with code included in any
# release of Talkback distributed by the Mozilla Foundation. You may
# copy and distribute such a system following the terms of the GNU GPL
# for all but the LGPL-covered parts and Talkback, and following the
# LGPL for the LGPL-covered parts.
#
# Note that people who make modified versions of Gnash are not obligated
# to grant this special exception for their modified versions; it is their
# choice whether to do so. The GNU General Public License gives permission
# to release a modified version without this exception; this exception
# also makes it possible to release a modified version which carries
# forward this exception.

## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = 1.6.0
ACLOCAL_AMFLAGS = -I macros

STD_DIRS = \
	libbase \
	libgeometry \
	libamf \
	server \
	backend \
	gui \
	utilities \
	testsuite

if PLUGIN
PLUGIN_DIRS = plugin
endif

if DOCBOOK
DOC_DIRS = doc
endif

if KLASH
KLASH_DIRS = plugin/klash
endif

SUBDIRS      = $(STD_DIRS) $(DOC_DIRS) $(PLUGIN_DIRS) $(KLASH_DIRS)

# libamf isn't used yet, but we still want it in the distribution
DIST_SUBDIRS = $(STD_DIRS) doc plugin

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
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

dumpconfig:
	@uname -a
	@gcc -v
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
	@echo "SDL_MIXER_CFLAGS is $(SDL_MIXER_CFLAGS)"
	@echo "SDL_MIXER_LIBS is $(SDL_MIXER_LIBS)"
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
	@echo "KDE_INCLUDES is $(KDE_INCLUDES)"
	@echo "KDE_LDFLAGS is $(KDE_LDFLAGS)"
	@echo "CAIRO_CFLAGS is $(CAIRO_CFLAGS)"
	@echo "CAIRO_LIBS is $(CAIRO_LIBS)"
	@echo "BOOST_CFLAGS is $(BOOST_CFLAGS)"
	@echo "BOOST_LIBS is $(BOOST_LIBS)"
	@echo "AGG_CFLAGS is $(AGG_CFLAGS)"
	@echo "AGG_LIBS is $(AGG_LIBS)"
	@echo "A blank value for CFLAGS means the header is installed in the"
	@echo "default system header location. All the LIBS should have a"
	@echo "legit value"

