#   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
#   Foundation, Inc.
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

AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = gnashrc.in gnashpluginrc.in

sysconf_DATA = gnashrc gnashpluginrc

# this is where Gnash plugins get installed
pluginsdir = $(prefix)/lib/gnash/plugins

pkglib_LTLIBRARIES = libgnashbase.la

libgnashbase_la_CPPFLAGS = -DPLUGINSDIR=\"$(pluginsdir)\" \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	$(PTHREAD_CFLAGS) \
	$(PNG_CFLAGS) \
	$(GIF_CFLAGS) \
	$(CURL_CFLAGS) \
	$(Z_CFLAGS) \
	$(JPEG_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(OPENGL_CFLAGS) \
	$(LTDL_CFLAGS) \
	$(NULL)

if ANDROID
libgnashbase_la_CPPFLAGS += $(ANDROID_NDK)/usr/include
endif

# These headers get installed

libgnashbase_la_LIBADD = \
	$(JPEG_LIBS) \
	$(PNG_LIBS) \
	$(GIF_LIBS) \
	$(Z_LIBS) \
	$(CURL_LIBS) \
	$(LIBINTL) \
	$(BOOST_LIBS) \
	$(OPENGL_LIBS) \
	$(PTHREAD_LIBS) \
	$(NULL)

libgnashbase_la_SOURCES = \
	$(MALLOC) \
	GnashImage.cpp \
	SWFCtype.cpp \
	GnashImageJpeg.cpp \
	GnashFileUtilities.cpp \
	AMF.cpp \
	RTMP.cpp \
	log.cpp	\
	memory.cpp \
	rc.cpp \
	string_table.cpp \
	tu_file.cpp \
	IOChannel.cpp \
	Socket.cpp \
	ClockTime.cpp \
	NamingPolicy.cpp \
	WallClockTimer.cpp \
	utf8.cpp \
	curl_adapter.cpp \
	noseek_fd_adapter.cpp \
	zlib_adapter.cpp \
	URL.cpp \
	GC.cpp \
	BitsReader.cpp \
	arg_parser.cpp \
	URLAccessManager.cpp \
	StreamProvider.cpp \
	$(NULL)


if ANDROID
libgnashbase_la_SOURCES += SharedMemHaiku.cpp
else
if HAIKU
libgnashbase_la_SOURCES += SharedMemHaiku.cpp
else
if WIN32
libgnashbase_la_SOURCES += SharedMemW32.cpp
else
libgnashbase_la_SOURCES += SharedMem.cpp
endif
endif
endif

if ENABLE_EXTENSIONS
libgnashbase_la_SOURCES += extension.cpp sharedlib.cpp
endif

edit = sed \
	-e 's|@DEFAULT_FLASH_PLATFORM_ID[@]|$(DEFAULT_FLASH_PLATFORM_ID)|g' \
	-e 's|@DEFAULT_FLASH_MAJOR_VERSION[@]|$(DEFAULT_FLASH_MAJOR_VERSION)|g' \
	-e 's|@DEFAULT_FLASH_MINOR_VERSION[@]|$(DEFAULT_FLASH_MINOR_VERSION)|g' \
	-e 's|@DEFAULT_FLASH_REV_NUMBER[@]|$(DEFAULT_FLASH_REV_NUMBER)|g' \
	-e 's|@DEFAULT_STREAMS_TIMEOUT[@]|$(DEFAULT_STREAMS_TIMEOUT)|g' \
	-e 's|@DEFAULT_SOL_SAFEDIR[@]|$(DEFAULT_SOL_SAFEDIR)|g' 

gnashrc: gnashrc.in Makefile
	$(edit) '$(srcdir)/$@.in' >$@

gnashpluginrc: gnashpluginrc.in
	$(edit) '$(srcdir)/$@.in' >$@

if USE_PNG
libgnashbase_la_SOURCES += GnashImagePng.cpp
endif

if USE_GIF
libgnashbase_la_SOURCES += GnashImageGif.cpp
endif

libgnashbase_la_LIBADD += $(LTDL_LIBS)

if HAIKU
  libgnashbase_la_LIBADD += $(HAIKU_LIBS)
endif

if WIN32
  libgnashbase_la_LIBADD += -lws2_32 -lwinmm
endif

# Maybe use jemalloc, which handles memory fragmentation for
# ECAMscript languages better than the regular system malloc.
# This is controlled by the --enable-jemalloc (disabled by default)
# configure option.
if JEMALLOC
MALLOC = jemalloc.c
else
MALLOC = 
endif

noinst_HEADERS =

inst_HEADERS = \
	accumulator.h \
	SimpleBuffer.h \
	GnashNumeric.h \
	jemalloc_gnash.h \
	jemalloc_types.h \
	jemalloc_rb.h \
	jemalloc.h \
	GnashSleep.h \
	gmemory.h \
	SharedMem.h \
	tree.hh \
	tu_file.h \
	IOChannel.h \
	Socket.h \
	GnashSystemFDHeaders.h \
	GnashSystemNetHeaders.h \
	GnashSystemIOHeaders.h \
	GnashFileUtilities.h \
	ClockTime.h \
	WallClockTimer.h \
	utf8.h \
	noseek_fd_adapter.h \
	zlib_adapter.h \
	BitsReader.h \
	arg_parser.h \
	getclocktime.hpp \
	GnashAlgorithm.h \
	GnashFactory.h \
	URLAccessManager.h \
	StreamProvider.h \
	$(NULL)

if USE_PNG
noinst_HEADERS += GnashImagePng.h
endif
if USE_GIF
noinst_HEADERS += GnashImageGif.h
endif

if ENABLE_EXTENSIONS
inst_HEADERS += extension.h sharedlib.h
endif

EXTENSIONS_API = \
	StringPredicates.h \
	Stats.h \
	GnashEnums.h \
	string_table.h \
	ref_counted.h \
	GC.h \
	GnashException.h \
	AMF.h \
	RTMP.h \
	dsodefs.h \
	utility.h \
	log.h \
	rc.h \
	gettext.h \
	URL.h \
	Point2d.h \
	Range2d.h \
	snappingrange.h \
	NetworkAdapter.h \
	NamingPolicy.h \
	GnashImageJpeg.h \
	CachedBitmap.h \
	GnashImage.h \
	ImageIterators.h \
	SWFCtype.h \
	$(NULL)

instdir = $(includedir)/gnash
inst_HEADERS += $(EXTENSIONS_API)

libgnashbase_la_LDFLAGS = -release $(VERSION)
libgnashbase_la_DEPENDENCIES =

if BUILD_OGL_RENDERER
   libgnashbase_la_SOURCES += \
	GnashTexture.cpp \
	$(NULL)

   noinst_HEADERS += \
	GnashTexture.h \
	$(NULL)
endif

if HAVE_VAAPI
   libgnashbase_la_SOURCES += \
	GnashVaapiImage.cpp \
	$(NULL)

   noinst_HEADERS += \
	GnashVaapiImage.h \
	GnashVaapiImageProxy.h \
	$(NULL)

if HAVE_VAAPI_GLX
   libgnashbase_la_SOURCES += \
	GnashVaapiTexture.cpp \
	$(NULL)

   noinst_HEADERS += \
	GnashVaapiTexture.h \
	$(NULL)
endif

if HAVE_VAAPI
   libgnashbase_la_CPPFLAGS += \
	-I$(top_srcdir)/libvaapi \
	$(NULL)

   libgnashbase_la_LIBADD += \
	$(top_builddir)/libvaapi/libgnashvaapi.la \
	$(NULL)

   libgnashbase_la_DEPENDENCIES += \
	$(top_builddir)/libvaapi/libgnashvaapi.la \
	$(NULL)
endif
endif

if WIN32
  libgnashbase_la_LDFLAGS += -no-undefined
  libgnashbase_la_LIBADD += -lws2_32 -lwinmm
endif

if ENABLE_PCH
AM_CXXFLAGS = $(PCH_FLAGS)
endif

CLEANFILES = gnashrc gnashpluginrc

# Remove libtool .la files
install-exec-hook:
	$(RM) $(DESTDIR)$(libdir)/gnash/libgnashbase.la

uninstall-local:
	$(RM) $(DESTDIR)$(libdir)/gnash/libgnashbase-*.so
