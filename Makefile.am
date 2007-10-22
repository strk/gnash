# 
#   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
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

# $Id: Makefile.am,v 1.92 2007/10/22 15:58:40 rsavoye Exp $

## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = 1.6.0
ACLOCAL_AMFLAGS = -I macros
noinst_SCRIPT = autogen.sh

STD_DIRS = \
	libbase \
	libgeometry \
	libamf \
	libmedia \
	server \
	backend \
	utilities \
	gui \
	plugin \
	testsuite \
	po \
	$(NULL)

if CYGNAL
CYGNAL_DIRS = cygnal
endif

if DOCBOOK
DOC_DIRS = doc
endif

SUBDIRS      = $(STD_DIRS) $(CYGNAL_DIRS) $(DOC_DIRS)

if BUILD_EXTENSIONS
SUBDIRS += extensions
endif

man_MANS = doc/gnash.1

DIST_SUBDIRS = $(STD_DIRS) cygnal doc extensions

EXTRA_DIST = config.rpath $(man_MANS) macros/incllist macros/libslist ChangeLog-0.8.0 ChangeLog-0.8.1

noinst_HEADERS = compatibility_include.h

# # Build an RPM package targeted to the OLPC http://www.laptop.org.
# olpc: dist-gzip $(srcdir)/packaging/redhat/gnash.spec
# 	cp -f $(srcdir)/packaging/redhat/gnash.spec /usr/src/redhat/SPECS/
# 	cp -f gnash-$(VERSION).tar.gz /usr/src/redhat/SOURCES/
# 	rpmbuild -rcfile=$(srcdir)/packaging/redhat/rpmrc:/usr/lib/rpm/rpmrc \
# 	         -ba --target=geode-olpc /usr/src/redhat/SPECS/gnash.spec

# Build an RPM package
rpm redhat: dist-gzip $(srcdir)/packaging/redhat/gnash.spec
	cp -f $(srcdir)/packaging/redhat/gnash.spec /usr/src/redhat/SPECS/
	cp -f gnash-$(VERSION).tar.gz /usr/src/redhat/SOURCES/
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

# Set a few variables to what features we selected so we can dump it with the
# test of the config
dumpconfig:
	@echo ""
	@echo "Building in directories: $(SUBDIRS)"
	@echo "Some directories may be excluded from the build"
	@uname -a
	$(CXX) -v
	@echo "CXXFLAGS: $(CXXFLAGS)"
	@echo ""
	@echo "Image decoding support..."
	@echo "	JPEG_CFLAGS is $(JPEG_CFLAGS)"
	@echo "	JPEG_LIBS is $(JPEG_LIBS)"
	@echo "	PNG_CFLAGS is $(PNG_CFLAGS)"
	@echo "	PNG_LIBS is $(PNG_LIBS)"
	@echo "Audio decoding support..."
if USE_SOUND_GST
	@echo "	GSTREAMER_CFLAGS is $(GSTREAMER_CFLAGS)"
	@echo "	GSTREAMER_LIBS is $(GSTREAMER_LIBS)"
endif
if USE_FFMPEG_ENGINE
	@echo "	FFMPEG_CFLAGS is $(FFMPEG_CFLAGS)"
	@echo "	FFMPEG_LIBS is $(FFMPEG_LIBS)"
	@echo "	OGG_CFLAGS is $(OGG_CFLAGS)"
	@echo "	OGG_LIBS is $(OGG_LIBS)"
endif
	@echo "GUI Toolkit decoding support..."	
	@echo "	SDL_CFLAGS is $(SDL_CFLAGS)"
	@echo "	SDL_LIBS is $(SDL_LIBS)"
if HAVE_KDE
	@echo "	KDE_CFLAGS is $(KDE_CFLAGS)"
	@echo "	KDE_LIBS is $(KDE_LIBS)"
endif
if HAVE_QT
	@echo "	QT_CFLAGS is $(QT_CFLAGS)"
	@echo "	QT_LIBS is $(QT_LIBS)"
endif
if BUILD_FLTK_GUI
	@echo "	FLTK2_CFLAGS is $(FLTK2_CFLAGS)"
	@echo "	FLTK2_LIBS is $(FLTK2_LIBS)"
	@echo "	XFT_CFLAGS is $(XFT_CFLAGS)"
	@echo "	XFT_LIBS is $(XFT_LIBS)"
endif
if BUILD_GTK_GUI
	@echo "	GTK2_CFLAGS is $(GTK2_CFLAGS)"
	@echo "	GTK2_LIBS is $(GTK2_LIBS)"
	@echo "	PANGO_CFLAGS is $(PANGO_CFLAGS)"
	@echo "	PANGO_LIBS is $(PANGO_LIBS)"
	@echo "	ATK_CFLAGS is $(ATK_CFLAGS)"
	@echo "	ATK_LIBS is $(ATK_LIBS)"
endif
	@echo "	X11_CFLAGS is $(X11_CFLAGS)"
	@echo "	X11_LIBS is $(X11_LIBS)"
	@echo "Support libraries..."
	@echo "	INCLTDL $(INCLTDL) "
	@echo "	LIBLTDL $(LIBLTDL) "
	@echo "	GLIB_CFLAGS is $(GLIB_CFLAGS)"
	@echo "	GLIB_LIBS is $(GLIB_LIBS)"
	@echo "	Z_CFLAGS is $(Z_CFLAGS)"
	@echo "	Z_LIBS is $(Z_LIBS)"
	@echo "	LIBXML_CFLAGS is $(LIBXML_CFLAGS)"
	@echo "	LIBXML_LIBS is $(LIBXML_LIBS)"
	@echo "	FREETYPE_CFLAGS is $(FREETYPE2_CFLAGS)"
	@echo "	FREETYPE_LIBS is $(FREETYPE2_LIBS)"
	@echo "	FONTCONFIG_CFLAGS is $(FONTCONFIG_CFLAGS)"
	@echo "	FONTCONFIG_LIBS is $(FONTCONFIG_LIBS)"
if HAVE_DMALLOC
	@echo "	DMALLOC_CFLAGS is $(DMALLOC_CFLAGS)"
	@echo "	DMALLOC_LIBS is $(DMALLOC_LIBS)"
endif
if BUILD_MYSQL_EXT
	@echo "MYSQL_CFLAGS is $(MYSQL_CFLAGS)"
	@echo "MYSQL_LIBS is $(MYSQL_LIBS)"
endif
if USE_MAD_ENGINE
	@echo "	MAD_CFLAGS is $(MAD_CFLAGS)"
	@echo "	MAD_LIBS is $(MAD_LIBS)"
endif
	@echo "	PTHREAD_CFLAGS is $(PTHREAD_CFLAGS)"
	@echo "	PTHREAD_LIBS is $(PTHREAD_LIBS)"
	@echo "	CURL_CFLAGS is $(CURL_CFLAGS)"
	@echo "	CURL_LIBS is $(CURL_LIBS)"
	@echo "	BOOST_CFLAGS is $(BOOST_CFLAGS)"
	@echo "	BOOST_LIBS is $(BOOST_LIBS)"
	@if test -n $(BOOST_EXTRA_LIBS); then \
	  echo "	BOOST_EXTRA_LIBS is $(BOOST_EXTRA_LIBS)"; \
	fi
if ENABLE_MING
	@echo "	MING_VERSION_CODE $(MING_VERSION_CODE) "
	@echo "	MING_LIBS $(MING_LIBS) "
	@echo "	MING_CFLAGS $(MING_CFLAGS) "
	@echo "	MAKESWF $(MAKESWF) "
endif
if ENABLE_MTASC
	@echo "	MTASC $(MTASC)"
	@echo "	MTASC CLASSPATH $(MTASC_CLASSPATH)"
endif
if ENABLE_SWFMILL
	@echo "	SWFMILL $(SWFMILL) "
endif
if ENABLE_SWFC
	@echo "	SWFC $(SWFC) "
endif
if ENABLE_SWFDEC_TESTSUITE
	@echo "	SWFDEC_TESTSUITE $(SWFDEC_TESTSUITE)"
endif
	@echo "Graphics support..."	
if BUILD_CAIRO_RENDERER
	@echo "	CAIRO_CFLAGS is $(CAIRO_CFLAGS)"
	@echo "	CAIRO_LIBS is $(CAIRO_LIBS)"
endif
if BUILD_OGL_RENDERER
	@echo "	GLEXT_CFLAGS is $(GLEXT_CFLAGS)"
	@echo "	GLEXT_LIBS is $(GLEXT_LIBS)"
	@echo "	OPENGL_CFLAGS is $(OPENGL_CFLAGS)"
	@echo "	OPENGL_LIBS is $(OPENGL_LIBS)"
endif
if BUILD_AGG_RENDERER
	@echo "	AGG_CFLAGS is $(AGG_CFLAGS)"
	@echo "	AGG_LIBS is $(AGG_LIBS)"
endif
	@echo ""
	@echo "A blank value for CFLAGS means the header is installed in the"
	@echo "default system header location. All the LIBS should have a"
	@echo "legit value"
	@echo ""
	@echo "Configurable options are:"
	@echo "	Media handler: $(MEDIA_CONFIG)"
	@echo "	Renderer engine: $(RENDERER_CONFIG)"


if BUILD_GTK_GUI
	@echo "	Supported GUI: GTK"
endif
if BUILD_KDE_GUI
	@echo "	Supported GUI: KDE"
endif
if BUILD_SDL_GUI
	@echo "	Supported GUI: SDL"
endif
if BUILD_FLTK_GUI
	@echo "	Supported GUI: FLTK"
endif
if BUILD_AQUA_GUI
	@echo "	Supported GUI: AQUA"
endif
if BUILD_FB_GUI
	@echo "	Supported GUI: FB"
endif

if KPARTS
	@echo "	Konqueror (KPARTS) plugin enabled"
else
	@echo "	Konqueror (KPARTS) plugin disabled"
endif

if NSAPI
	@echo "	Firefox (NSAPI) plugin enabled"
else
	@echo "	Firefox (NSAPI) plugin disabled"
endif
	@if test -n "$(EXTENSIONS_LIST)" ; then \
	  echo "	Extensions added are: $(EXTENSIONS_LIST)"; \
	fi

check: check-recursive anal

anal testreport checksum:
	$(srcdir)/testsuite/anaylse-results.sh

.PHONY : dumpconfig anal testreport checksum check mudflap mudflap-check snapshot deb debian rmp redhat

