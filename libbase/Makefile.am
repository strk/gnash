#   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

# this is where Gnash plugins get installed
pluginsdir = $(prefix)/lib/gnash/plugins

if INSTALL_LTDL
if !LIBLTDL2
include_HEADERS =  $(top_srcdir)/libltdl/ltdl.h
LIBLTDLLIB = libltdl.la
LIBLTDLHEAD =  $(top_srcdir)/libltdl/ltdl.h
endif
endif

sysconf_DATA = gnashrc gnashpluginrc

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

# If we are using an installable libltdl, then it needs to be built
# before libgnash, as libgnash depends on it for extensions.
pkglib_LTLIBRARIES =  $(LIBLTDLLIB) libgnashbase.la

libgnashbase_la_CPPFLAGS = -DPLUGINSDIR=\"$(pluginsdir)\" \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-I$(top_srcdir)/libnet	\
	-I$(top_srcdir)/libamf	\
	-I$(top_srcdir)/libcore	\
	-I$(top_srcdir)/libcore/swf	\
	$(PTHREAD_CFLAGS) \
	$(LIBXML_CFLAGS) \
	$(LIBPNG_CFLAGS) \
	$(GIF_CFLAG) \
	$(GLIB_CFLAGS) \
	$(GSTREAMER_CFLAGS) \
	$(CURL_CFLAGS) \
	$(Z_CFLAGS) \
	$(JPEG_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(INCLTDL) \
	$(NULL)

# These headers get installed
# include_HEADERS = log.h err.h proc.h serial.h xantrex.h outback.h

libgnashbase_la_LIBADD = \
	$(JPEG_LIBS) \
	$(LIBPNG_LIBS) \
	$(GIF_LIBS) \
	$(Z_LIBS) \
	$(CURL_LIBS) \
	$(OPENGL_LIBS) \
	$(LIBINTL) \
	$(LIBLTDL) \
	$(BOOST_LIBS) \
	$(PTHREAD_LIBS) \
	$(NULL)

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

libgnashbase_la_SOURCES = \
	extension.cpp \
	image.cpp \
	$(MALLOC) \
	GnashImageJpeg.cpp \
	log.cpp	\
	memory.cpp \
	rc.cpp \
	shm.cpp \
	sharedlib.cpp \
	string_table.cpp \
	tu_file.cpp \
	IOChannel.cpp \
	ClockTime.cpp \
	WallClockTimer.cpp \
	utf8.cpp \
	curl_adapter.cpp \
	noseek_fd_adapter.cpp \
	zlib_adapter.cpp \
	URL.cpp \
	LoadThread.cpp \
	GC.cpp \
	BitsReader.cpp \
	arg_parser.cpp \
	$(NULL)

if USE_PNG
libgnashbase_la_SOURCES += GnashImagePng.cpp
endif
if USE_GIF
libgnashbase_la_SOURCES += GnashImageGif.cpp
endif

noinst_HEADERS = \
	$(LIBLTDLHEAD) \
	SimpleBuffer.h \
	extension.h \
	GnashException.h \
	gettext.h \
	image.h	\
	jemtree.h \
	GnashImage.h \
	GnashImageJpeg.h \
	GnashSleep.h \
	gmemory.h \
	log.h \
	ogl.h \
	rc.h \
	ref_counted.h \
	shm.h \
	smart_ptr.h \
	sharedlib.h \
	string_table.h \
	tree.hh \
	dsodefs.h \
	tu_file.h \
	IOChannel.h \
	tu_opengl_includes.h \
	ClockTime.h \
	WallClockTimer.h \
	utf8.h \
	utility.h \
	NetworkAdapter.h \
	noseek_fd_adapter.h \
	zlib_adapter.h \
	URL.h \
	LoadThread.h \
	GC.h \
	BitsReader.h \
	arg_parser.h \
	getclocktime.hpp \
	Point2d.h \
	Range2d.h \
	snappingrange.h \
	$(NULL)

if USE_PNG
noinst_HEADERS += GnashImagePng.h
endif
if USE_GIF
noinst_HEADERS += GnashImageGif.h
endif


libgnashbase_la_LDFLAGS = -release $(VERSION)

if WIN32
  libgnashbase_la_LDFLAGS += -no-undefined
  libgnashbase_la_LIBADD += -lws2_32 -lwinmm
endif

if !LIBLTDL2
libltdlc_la_CPPFLAGS = $(LTDLINCL) -I$(top_srcdir)/libltdl
if CONVENIENCE_LTDL
noinst_LTLIBRARIES = libltdlc.la
libltdlc_la_SOURCES = $(top_srcdir)/libltdl/ltdl.c
endif

libltdl_la_SOURCES = $(top_srcdir)/libltdl/ltdl.c
libltdl_la_LDFLAGS = -no-undefined -version-info 4:4:1
libltdl_la_LIBADD = $(LIBADD_DL)

if WIN32
  libltdl_la_LDFLAGS += -static
endif
endif
CLEANFILES = libltdl.la libltdlc.la gnashrc gnashpluginrc

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"
