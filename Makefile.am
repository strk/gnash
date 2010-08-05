# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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

## Process this file with automake to produce Makefile.in
# no-portability allows us to use GNU make variable assignments
# without automake complaining.
AUTOMAKE_OPTIONS = 1.6.0 -Wno-portability

ACLOCAL_AMFLAGS = -I macros -I cygnal
noinst_SCRIPT = autogen.sh

##
## Any custom ./configure switch we want to be
## given at 'make distcheck' time should go here
##
## We set plugins install dirs to a user-writable 
## and likely unused directory for now to avoid
## permission problems and unexpected unistalls
## from home dir. Ideally we should put them somewhere
## under ${prefix} to make the uninstall check effective
## (and I believe it should actually be the default when
## --prefix is given, but that's another story --strk)
##
## We also want to build everything possible, since we're
## testing builds.
DISTCHECK_CONFIGURE_FLAGS = \
	--with-plugins-install=prefix \
	--enable-extensions=all \
	--enable-gui=all \
	--disable-testsuite
#--enable-cygnal

if CYGNAL
CYGNAL_DIR = cygnal
endif

STD_DIRS = \
	libbase \
	libmedia \
	libsound \
	libcore \
	librender \
	gui \
	plugin \
	$(CYGNAL_DIR) \
	utilities \
	doc \
	po \
	$(NULL)

if LIBLTDL2
LIBLTDLDIR = libltdl
endif

if HAVE_VAAPI
VAAPI_DIR = libvaapi
endif

SUBDIRS = $(LIBLTDLDIR) $(VAAPI_DIR) $(STD_DIRS)
DIST_SUBDIRS = $(STD_DIRS) cygnal extensions testsuite libltdl libvaapi

if TESTSUITE
SUBDIRS += testsuite
endif

if BUILD_EXTENSIONS
SUBDIRS += extensions
endif

# man_MANS = doc/gnash.1 doc/gprocessor.1 doc/dumpshm.1 soldumper.1

EXTRA_DIST =  \
	autogen.sh \
	config.rpath \
	macros/incllist macros/libslist \
	ChangeLog-0.8.0 \
	ChangeLog-0.8.1 \
	ChangeLog-0.8.2 \
	ChangeLog-0.8.3 \
	ChangeLog-0.8.4 \
	packaging/deb.am \
	packaging/snapshot.am \
	packaging/alp.am \
	packaging/ipkg.am \
	packaging/bsd.am \
	packaging/rpm.am \
	packaging/gnash.xpm \
	packaging/klash.xpm \
	revno.h


# dist-hook:
# 	@test -d "$(distdir)/packaging" || $(mkinstalldirs) "$(distdir)/packaging"
# 	cp -p $(srcdir)/*.am $(top_distdir)/packaging/

CLEANFILES =

#
# Extract info from the repository to include in the build. If
# bazar (bzr) isn't installed, just print stubs, as this is only
# used for display purposes. It's primarily used by the testsuites
# and when building binary packages from multiple branches.
# Note that this is only rebuilt after "make clean", if the Makefile
# has changed, or the source tree gets reconfigured. So unless you do
# a make clean, or nuke the revno.h file, this can get slightly
# behind the current version. Currently Gnash doesn't have this info
# so it'll be an improvement anyway. Bzr is amazing slow producing
# the info, which is why we don't want to do it all the time.
#
CLEANFILES += revno.h
BUILT_SOURCES = revno.h
revno.h: Makefile.in
	@echo "Generating revno.h"
	-@if test -d $(srcdir)/.bzr; then \
	  status=`which bzr 2>&1 | grep -c " no bzr"`; \
	  if test x"$${status}" = x"0"; then \
	    echo "Generating revno.h"; \
	    $(RM) -f revno.h; \
	    revno=`(cd $(srcdir) && bzr revno)`; \
	    nick=`basename $(top_srcdir)`; \
	  else \
	    revno="${NOW}"; \
	  fi; \
	else \
	  if test -d $(srcdir)/.git; then \
            status=`which git 2>&1 | grep -c " no git"`; \
            if test x"$${status}" = x"0"; then \
              echo "Generating revno.h"; \
              $(RM) -f revno.h; \
              revno=`(cd $(srcdir) && git describe --tags | cut -d '-' -f 2-3)`; \
              nick=`basename $(top_srcdir)`; \
	    fi; \
          else \
            revno="${NOW}"; \
            nick="none"; \
	  fi; \
	fi; \
	echo "static const char *BRANCH_REVNO  = \"$${revno}\";" > revno.h; \
	echo "static const char *BRANCH_NICK = \"$${nick}\";" >> revno.h;

# The branch nickname and revision number must be set before including
# the other Makefile fragments used for package building, as they use
# these values.
BRANCH_REVNO  := $(shell grep "REVNO" revno.h | cut -d '"' -f 2)
BRANCH_NICK   := $(shell grep "NICK" revno.h | cut -d '"' -f 2)


# Extract info from the repository to include in the build. If
# GIT (git) isn't installed, just print stubs, as this is only
# used for display purposes. It's primarily used by the testsuites
# and when building binary packages from multiple branches.
# Note that this is only rebuilt after "make clean", if the Makefile
# has changed, or the source tree gets reconfigured. So unless you do
# a make clean, or nuke the revno.h file, this can get slightly
# behind the current version. Currently Gnash doesn't have this info
# so it'll be an improvement anyway. 

revno.h: Makefile.in
        @status=`which git 2>&1 | grep -c " no bzr"`; \
        if test x"$${status}" = x"0"; then \
           echo "Generating revno.h"; \
           $(RM) -f revno.h; \
           revno=`(cd $(srcdir) && git describe --tags | cut -d '-' -f 2-3)`; \
           nick=`basename $(top_srcdir)`; \
        else \
           revno="${NOW}"; \
           nick="none"; \
        fi; \
        echo "Generating revno.h"; \
        echo "static const char *BRANCH_REVNO  = \"$${revno}\";" > revno.h; \
        echo "static const char *BRANCH_NICK = \"$${nick}\";" >> revno.h;

#
# Precompiled header support
#
include $(srcdir)/pch.am

#
# Binary tarball packaging
#
include $(srcdir)/packaging/snapshot.am

#
# Build an RPM package
#
include $(srcdir)/packaging/rpm.am

# Build a Debian/Ubuntu .deb GNU/Linux package
include $(srcdir)/packaging/deb.am

#
# Build a Debian Familiar .ipk GNU/Linux package. These are always
# cross compiled, typically for the ARM or XScale.
#
include $(srcdir)/packaging/ipkg.am

#
# Access Linux Platform support.
#
include $(srcdir)/packaging/alp.am

#
# BSD package building
#
include $(srcdir)/packaging/bsd.am

#
# Mozille/Firefox XPI support
#
include $(srcdir)/packaging/xpi.am

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
	@echo "	GIF_CFLAGS is $(GIF_CFLAGS)"
	@echo "	GIF_LIBS is $(GIF_LIBS)"
	@echo "Audio decoding support..."
if USE_GST_ENGINE
	@echo "	GSTREAMER_CFLAGS is $(GSTREAMER_CFLAGS)"
	@echo "	GSTREAMER_LIBS is $(GSTREAMER_LIBS)"
endif
if USE_FFMPEG_ENGINE
	@echo "	FFMPEG_CFLAGS is $(FFMPEG_CFLAGS)"
	@echo "	FFMPEG_LIBS is $(FFMPEG_LIBS)"
	@echo "	OGG_CFLAGS is $(OGG_CFLAGS)"
	@echo "	OGG_LIBS is $(OGG_LIBS)"
endif
if HAVE_SPEEX
	@echo " SPEEX_CFLAGS is $(SPEEX_CFLAGS)"
	@echo " SPEEXDSP_CFLAGS is $(SPEEXDSP_CFLAGS)"
	@echo " SPEEX_LIBS is $(SPEEX_LIBS)"
	@echo " SPEEXDSP_LIBS is $(SPEEXDSP_LIBS)"
endif
	@echo "GUI Toolkit decoding support..."	
	@echo "	SDL_CFLAGS is $(SDL_CFLAGS)"
	@echo "	SDL_LIBS is $(SDL_LIBS)"
if BUILD_KDE3_GUI
	@echo "	KDE3_CFLAGS is $(KDE3_CFLAGS)"
	@echo "	KDE3_LIBS is $(KDE3_LIBS)"
endif
if BUILD_KDE4_GUI
	@echo "	KDE4_CFLAGS is $(KDE4_CFLAGS)"
	@echo "	KDE4_LIBS is $(KDE4_LIBS)"
endif
if BUILD_QT3_GUI
	@echo "	QT3_CFLAGS is $(QT3_CFLAGS)"
	@echo "	QT3_LIBS is $(QT3_LIBS)"
endif
if BUILD_QT4_GUI
	@echo "	QT4_CFLAGS is $(QT4_CFLAGS)"
	@echo "	QT4_LIBS is $(QT4_LIBS)"
endif
if BUILD_QTOPIA3_GUI
	@echo "	QTOPIA3_CFLAGS is $(QTOPIA3_CFLAGS)"
	@echo "	QTOPIA3_LIBS is $(QTOPIA3_LIBS)"
endif
if BUILD_QTOPIA4_GUI
	@echo "	QTOPIA4_CFLAGS is $(QTOPIA4_CFLAGS)"
	@echo "	QTOPIA4_LIBS is $(QTOPIA4_LIBS)"
endif
if BUILD_FLTK_GUI
	@echo "	FLTK2_CFLAGS is $(FLTK2_CFLAGS)"
	@echo "	FLTK2_LIBS is $(FLTK2_LIBS)"
	@echo "	XFT_CFLAGS is $(XFT_CFLAGS)"
	@echo "	XFT_LIBS is $(XFT_LIBS)"
endif
if HAIKU
	@echo "	HAIKU_LIBS is $(HAIKU_LIBS)"
endif
if BUILD_GTK_GUI
	@echo "	GTK2_CFLAGS is $(GTK2_CFLAGS)"
	@echo "	GTK2_LIBS is $(GTK2_LIBS)"
	@echo "	PANGO_CFLAGS is $(PANGO_CFLAGS)"
	@echo "	PANGO_LIBS is $(PANGO_LIBS)"
	@echo "	ATK_CFLAGS is $(ATK_CFLAGS)"
	@echo "	ATK_LIBS is $(ATK_LIBS)"
endif
if BUILD_HILDON_GUI
	@echo "	HILDON_CFLAGS is $(HILDON_CFLAGS)"
	@echo "	HILDON_LIBS is $(HILDON_LIBS)"
endif
if BUILD_ALP_GUI
	@echo " ALP_CFLAGS is $(ALP_CFLAGS)"
	@echo "	ALP_LIBS is $(ALP_LIBS)"
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
	@echo "	FREETYPE_CFLAGS is $(FREETYPE2_CFLAGS)"
	@echo "	FREETYPE_LIBS is $(FREETYPE2_LIBS)"
	@echo "	FONTCONFIG_CFLAGS is $(FONTCONFIG_CFLAGS)"
	@echo "	FONTCONFIG_LIBS is $(FONTCONFIG_LIBS)"
	@echo "	LIBINTL is $(LIBINTL)"
if HAVE_DMALLOC
	@echo "	DMALLOC_CFLAGS is $(DMALLOC_CFLAGS)"
	@echo "	DMALLOC_LIBS is $(DMALLOC_LIBS)"
endif
if BUILD_MYSQL_EXT
	@echo "MYSQL_CFLAGS is $(MYSQL_CFLAGS)"
	@echo "MYSQL_LIBS is $(MYSQL_LIBS)"
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
if ENABLE_HAXE
	@echo "	HAXE $(HAXE)"
	@echo "	HAXE CLASSPATH $(HAXE_CLASSPATH)"
endif
if ENABLE_SWFMILL
	@echo "	SWFMILL $(SWFMILL) "
	@echo "	SWFMILL_VERSION_CODE $(SWFMILL_VERSION) "
endif
if ENABLE_SWFC
	@echo "	SWFC $(SWFC) "
endif
if ENABLE_SWFDEC_TESTSUITE
	@echo "	SWFDEC_TESTSUITE $(SWFDEC_TESTSUITE)"
endif
if ENABLE_HTTP_TESTSUITE
	@echo "	HTTP_TESTSUITE $(HTTP_TESTSUITE)"
endif
if ENABLE_RED5_TESTING
	@echo "	RED5_HOST $(RED5_HOST)"
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
	@echo "	Media handlers: $(MEDIA_CONFIG)"
	@echo "	Renderer engines: $(RENDERER_CONFIG)"
	@echo "	Hardware Acceleration: $(HWACCEL_CONFIG)"

if BUILD_SSL
	@echo "	SSL_CFLAGS is $(SSL_CFLAGS)"
	@echo "	SSL_LIBS is $(SSL_LIBS)"
endif
if BUILD_SSH
	@echo "	SSH_CFLAGS is $(SSH_CFLAGS)"
	@echo "	SSH_LIBS is $(SSH_LIBS)"
endif
if BUILD_GTK_GUI
if HAVE_XV
	@echo "	Supported GUI: GTK (+XVideo)"
else
	@echo "	Supported GUI: GTK"
endif
endif
if BUILD_HILDON_GUI
	@echo "	Supported GUI: HILDON"
endif
if BUILD_DUMP_GUI
	@echo "	Supported GUI: Dump"
endif
if BUILD_ALP_GUI
	@echo "	Supported GUI: ALP/Hiker"
endif
if BUILD_KDE3_GUI
	@echo "	Supported GUI: KDE3"
endif
if BUILD_KDE4_GUI
	@echo "	Supported GUI: KDE4"
endif
if BUILD_QTOPIA3_GUI
	@echo "	Supported GUI: QTOPIA3"
endif
if BUILD_QTOPIA4_GUI
	@echo "	Supported GUI: QTOPIA4"
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
if BUILD_AMIGAOS4_GUI
       @echo " Supported GUI: AMIGAOS4"
endif
if BUILD_HAIKU_GUI
	@echo " Supported GUI: Haiku"
endif
if JEMALLOC
	@echo "	Using jemalloc memory manager"
endif

	@if test -n "$(EXTENSIONS_LIST)" ; then \
	  echo "	Extensions added are: $(EXTENSIONS_LIST)"; \
	fi
	@if test -n "$(SECURITY_LIST)" ; then \
	  echo "	Security features enabled are: $(SECURITY_LIST)"; \
	fi
	@if test -n "$(STATISTICS_LIST)" ; then \
	  echo "	Statistics features enabled are: $(STATISTICS_LIST)"; \
	fi
if KPARTS3
	@echo "	KPARTS 3.x (KDE) plugin enabled"
	@echo "	  KPARTS 3.x plugin install dir: $(KDE_PLUGINDIR)"
	@echo "	  KPARTS 3.x service install dir: $(KDE_SERVICESDIR)"
	@echo "	  KPARTS 3.x config dir: ${KDE_CONFIGDIR}"
	@echo "	  KPARTS 3.x appsdata dir: ${KDE_APPSDATADIR}"
else
	@echo "	KPARTS 3.x (KDE) plugin disabled"
endif
if KPARTS4
	@echo "	KPARTS 4.x (KDE) plugin enabled"
	@echo "	  KPARTS 4.x plugin install dir: $(KDE4_PLUGINDIR)"
	@echo "	  KPARTS 4.x service install dir: $(KDE4_SERVICESDIR)"
	@echo "	  KPARTS 4.x config dir: ${KDE4_CONFIGDIR}"
	@echo "	  KPARTS 4.x appsdata dir: ${KDE4_APPSDATADIR}"
else
	@echo "	KPARTS 4.x (KDE) plugin disabled"
endif

if NPAPI
	@echo "	NPAPI (Mozilla) plugin enabled"
	@echo "	  NPAPI plugin install dir: $(FIREFOX_PLUGINS)"
else
	@echo "	NPAPI (Mozilla) plugin disabled"
endif
if USE_PYTHON
	@echo "	PYTHON support enabled"
else
	@echo "	PYTHON support disabled"
endif

if TESTSUITE
check-local:
	$(MAKE) anal

anal testreport checksum:
	$(srcdir)/testsuite/anaylse-results.sh
endif

.PHONY : dumpconfig \
	anal \
	testreport \
	checksum \
	check \
	mudflap \
	mudflap-check \
	install-plugins \
	uninstall-plugins \
	install-plugin \
	apidoc

install-plugins install-plugin uninstall-plugins install-pkglib:
	$(MAKE) -C plugin $@

apidoc:
	$(MAKE) -C doc $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = \
	gnash.pc \
	$(NULL)

