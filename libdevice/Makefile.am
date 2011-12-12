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

# this is where Gnash plugins get installed
pluginsdir = $(libdir)/gnash/plugins

## WARNING: make sure GLIB_LIBS appears first
## See: http://lists.gnu.org/archive/html/gnash-dev/2006-07/msg00076.html
AM_CPPFLAGS = -I.. \
	-I$(srcdir) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/librender \
	-I$(top_srcdir)/libcore \
	-I$(top_srcdir)/libcore/vm \
	-I$(top_srcdir)/libcore/parser \
	-I$(top_srcdir)/libcore/swf \
	-I$(top_srcdir)/gui \
	$(PTHREAD_CFLAGS) \
	$(SDL_CFLAGS) \
	$(PANGO_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GTK2_CFLAGS) \
	$(ATK_CFLAGS) \
	$(DMALLOC_CFLAGS) \
	$(PNG_CFLAGS) \
	$(JPEG_CFLAGS) \
	$(OGG_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(OPENGL_CFLAGS) \
	$(AGG_CFLAGS) \
	$(DIRECTFB_CFLAGS) \
	$(CAIRO_CFLAGS) \
	$(NULL)

instdir = $(includedir)/gnash

noinst_HEADERS = \
	GnashDevice.h \
	DeviceGlue.h \
	$(NULL)

pkglib_LTLIBRARIES =  libgnashdevice.la

libgnashdevice_la_CPPFLAGS = $(AM_CPPFLAGS)
libgnashdevice_la_LIBADD = \
	$(CURL_LIBS) \
	$(LIBVA_LIBS) \
	$(LIBVA_X11_LIBS) \
	$(LIBVA_GLX_LIBS) \
	$(TS_LIBS) \
	$(LTDL_LIBS) \
	$(GNASH_LIBS)
libgnashdevice_la_LDFLAGS =  -release $(VERSION) 
libgnashdevice_la_SOURCES = \
	GnashDevice.h \
	DeviceGlue.h \
	DeviceGlue.cpp \
	$(NULL)       

if BUILD_X11_DEVICE
libgnashdevice_la_CPPFLAGS += $(X11_CFLAGS)
libgnashdevice_la_SOURCES += \
	x11/X11Device.cpp \
	x11/X11Device.h
libgnashdevice_la_LIBADD += \
	$(TS_LIBS) \
	$(X11_LIBS)
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

bin_PROGRAMS =
check_PROGRAMS =

if BUILD_RAWFB_DEVICE
include rawfb/rawfb.am
endif

# This is for Linux Input Event support
if ENABLE_INPUT_EVENTS
include events/events.am
endif

if BUILD_EGL_DEVICE
include egl/egl.am
endif

if BUILD_DIRECTFB_DEVICE
include directfb/directfb.am
endif

if ENABLE_DEVELOPER_TESTS
if BUILD_X11_DEVICE

check_PROGRAMS += test_x11
test_x11_SOURCES = x11/test_x11.cpp
test_x11_CPPFLAGS = $(AM_CPPFLAGS) $(X11_CFLAGS)
test_x11_LDADD = \
	libgnashdevice.la \
	$(X11_LIBS) \
	$(GNASH_LIBS)
endif
endif

if BUILD_VAAPI_DEVICE
include vaapi/vaapi.am
endif
