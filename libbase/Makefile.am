#   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
#   Free Software Foundation, Inc.
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

pkglib_LTLIBRARIES = libgnashbase.la
sysconf_DATA = gnashrc gnashpluginrc
EXTRA_DIST = gnashrc.in gnashpluginrc.in

libgnashbase_la_SOURCES = \
	AMF.cpp \
	AMF.h \
	arg_parser.cpp \
	arg_parser.h \
	BitsReader.cpp \
	BitsReader.h \
	ClockTime.cpp \
	ClockTime.h \
	curl_adapter.cpp \
	dsodefs.h \
	GC.cpp \
	GC.h \
	getclocktime.hpp \
	gettext.h \
	gmemory.h \
	GnashAlgorithm.h \
	GnashEnums.h \
	GnashException.h \
	GnashFileUtilities.cpp \
	GnashFileUtilities.h \
	GnashImage.cpp \
	GnashImage.h \
	GnashImageJpeg.cpp \
	GnashImageJpeg.h \
	GnashNumeric.h \
	GnashSleep.h \
	GnashSystemFDHeaders.h \
	GnashSystemIOHeaders.h \
	GnashSystemNetHeaders.h \
	IOChannel.cpp \
	IOChannel.h \
	log.cpp \
	log.h \
	memory.cpp \
	NamingPolicy.cpp \
	NamingPolicy.h \
	NetworkAdapter.h \
	noseek_fd_adapter.cpp \
	noseek_fd_adapter.h \
	rc.cpp \
	rc.h \
	RTMP.cpp \
	RTMP.h \
	SharedMem.h \
	SimpleBuffer.h \
	Socket.cpp \
	Socket.h \
	Stats.h \
	StreamProvider.cpp \
	StreamProvider.h \
	StringPredicates.h \
	string_table.cpp \
	string_table.h \
	SWFCtype.cpp \
	SWFCtype.h \
	tu_file.cpp \
	tu_file.h \
	URLAccessManager.cpp \
	URLAccessManager.h \
	URL.cpp \
	URL.h \
	utf8.cpp \
	utf8.h \
	utility.h \
	WallClockTimer.cpp \
	WallClockTimer.h \
	zlib_adapter.cpp \
	zlib_adapter.h \
	$(NULL)

if JEMALLOC
libgnashbase_la_SOURCES += \
	jemalloc_gnash.c \
	$(NULL)
endif

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
libgnashbase_la_SOURCES += \
	extension.cpp \
	extension.h \
	sharedlib.cpp \
	sharedlib.h \
	$(NULL)
endif

if USE_PNG
libgnashbase_la_SOURCES += \
	GnashImagePng.cpp \
	GnashImagePng.h \
	$(NULL)
endif

if USE_GIF
libgnashbase_la_SOURCES += \
	GnashImageGif.cpp \
	GnashImageGif.h \
	$(NULL)
endif

if HAVE_VAAPI
libgnashbase_la_SOURCES += \
	GnashVaapiImage.cpp \
	GnashVaapiImage.h \
	GnashVaapiImageProxy.h \
	GnashVaapiTexture.cpp \
	GnashVaapiTexture.h \
	$(NULL)

endif


# this is where Gnash plugins get installed
pluginsdir = $(prefix)/lib/gnash/plugins

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
	$(JEMALLOC_CFLAGS) \
	-I$(top_srcdir)/librender \
	$(NULL)

libgnashbase_la_LIBADD = \
	$(JPEG_LIBS) \
	$(PNG_LIBS) \
	$(GIF_LIBS) \
	$(Z_LIBS) \
	$(CURL_LIBS) \
	$(LIBINTL) \
	$(BOOST_LIBS) \
	$(OPENGL_LIBS) \
	$(OPENVG_LIBS) \
	$(EGL_LIBS) \
	$(PTHREAD_LIBS) \
	$(LTDL_LIBS) \
	$(JEMALLOC_LIBS) \
	$(NULL)

if ANDROID
libgnashbase_la_LIBADD += -lui -llog
endif	# ANDROID

if HAIKU
  libgnashbase_la_LIBADD += $(HAIKU_LIBS)
endif

inst_HEADERS = \
	accumulator.h \
	SimpleBuffer.h \
	GnashNumeric.h \
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

if HAVE_VAAPI
   libgnashbase_la_CPPFLAGS += \
	-I$(top_srcdir)/libdevice/vaapi \
	$(NULL)

   libgnashbase_la_LIBADD += \
	$(top_builddir)/libdevice/libgnashvaapi.la \
	$(NULL)

   libgnashbase_la_DEPENDENCIES += \
	$(top_builddir)/libdevice/libgnashvaapi.la \
	$(NULL)
endif

if WIN32
  libgnashbase_la_LDFLAGS += -no-undefined
  libgnashbase_la_LIBADD += -lws2_32 -lwinmm
endif

AM_CXXFLAGS = $(CROSS_CXXFLAGS)
AM_LDFLAGS = $(CROSS_LDFLAGS)

if ENABLE_PCH
AM_CXXFLAGS += $(PCH_FLAGS)
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

CLEANFILES = gnashrc gnashpluginrc

# Remove libtool .la files
install-exec-hook:
	$(RM) $(DESTDIR)$(libdir)/gnash/libgnashbase.la

uninstall-local:
	$(RM) $(DESTDIR)$(libdir)/gnash/libgnashbase-*.so
