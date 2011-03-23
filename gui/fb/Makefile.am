## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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

#
# Build the FB gui
#

AUTOMAKE_OPTIONS = -Wno-portability

# top_builddir and top_srcdir are for revno.h; its location depends on
# whether we're building from a snapshot or a checkout.
AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/gui \
	-I$(top_srcdir)/libnet \
	-I$(top_srcdir)/libcore \
	-I$(top_srcdir)/libcore/swf \
	-I$(top_srcdir)/libcore/parser \
	-I$(top_srcdir)/libcore/vm \
	-I$(top_srcdir)/libcore/asobj \
	-I$(top_srcdir)/libltdl \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/librender \
	-I$(top_srcdir)/librender/agg \
	-I$(top_srcdir)/librender/cairo \
	-I$(top_srcdir)/librender/opengl \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libsound \
	-DLOCALEDIR=\"$(localedir)\" \
	-DPLUGINSDIR=\"$(pluginsdir)\" \
	-DLIBAVCODEC_IDENT=\"$(LIBAVCODEC_IDENT)\" \
	$(GLIB_CFLAGS) \
	$(LIRC_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(PYTHON_LIBS) \
	$(PTHREAD_CFLAGS) \
	$(NULL)

AM_CPPFLAGS += -DPKGDATADIR="\"$(pkgdatadir)\""
AM_CPPFLAGS += -DRENDERER_CONFIG="\"$(RENDERER_CONFIG)\""
AM_CPPFLAGS += -DHWACCEL_CONFIG="\"$(HWACCEL_CONFIG)\""
AM_CPPFLAGS += -DMEDIA_CONFIG="\"$(MEDIA_CONFIG)\""
AM_CPPFLAGS += -DCONFIG_CONFIG="\"$(shell cat ../../.configline)\""
AM_CPPFLAGS += -DCXXFLAGS="\"$(CXXFLAGS)\"" 

## WARNING: make sure GLIB_LIBS appears first
## See: http://lists.gnu.org/archive/html/gnash-dev/2006-07/msg00076.html
AM_LDFLAGS =  \
	$(GLIB_LIBS) \
	$(CURL_LIBS) \
	$(JPEG_LIBS) \
	$(PNG_LIBS) \
	$(INTLLIBS) \
	$(TS_LIBS) \
	$(BOOST_LIBS) \
	$(PTHREAD_LIBS) \
	$(NULL)

if USE_FFMPEG_ENGINE
AM_LDFLAGS += $(FFMPEG_LIBS) $(LIBVA_LIBS) $(LIBVA_X11_LIBS) 
AM_CPPFLAGS += $(FFMPEG_CFLAGS) 
endif

if USE_GST_ENGINE
GST_STUFF = $(GSTREAMER_PBUTILS_LIBS) $(GSTREAMER_PLUGINS_BASE_LIBS) $(GSTREAMER_LIBS)
AM_CPPFLAGS += $(GSTREAMER_CFLAGS)
AM_LDFLAGS += $(GST_STUFF)
endif

GNASH_LIBS = \
	$(top_builddir)/libcore/libgnashcore.la \
	$(top_builddir)/librender/libgnashrender.la \
	$(top_builddir)/libsound/libgnashsound.la \
	$(top_builddir)/libmedia/libgnashmedia.la \
	$(top_builddir)/libbase/libgnashbase.la


# These are used to handle input events from a keyboard, mouse, or
# touchscreen. These are only used by the Framebuffer GUI, as without
# X11, it has no event handling. We do it this way so the optionally
# built source files still get included in the source tarball.
DEVICES =
if ENABLE_INPUT_DEVICES
DEVICES += InputDevice.cpp InputDevice.h

# Touchscreen
if ENABLE_TSLIB
DEVICES += TouchDevice.cpp
endif
# PS/2 Mouse
if ENABLE_MOUSE
DEVICES += MouseDevice.cpp
endif
# Linux input events
if ENABLE_INPUT_EVENTS
DEVICES += EventDevice.cpp
endif
endif

if BUILD_FB_GUI
bin_PROGRAMS = fb-gnash

# TODO: stop using these 
GUI_SRCS = ../gnash.cpp \
	../gui.cpp ../gui.h \
	../Player.cpp ../Player.h \
	../NullGui.cpp ../NullGui.h \
	../ScreenShotter.cpp ../ScreenShotter.h \
	$(NULL)

fb_gnash_SOURCES = $(GUI_SRCS) $(DEVICES) gui_fb.cpp fb.cpp fbsup.h
fb_gnash_CPPFLAGS = -DGUI_FB -DGUI_CONFIG=\"FB\" -DFAKEFB=\"$(FAKEFB)\" \
	$(AM_CPPFLAGS) 
fb_gnash_LDFLAGS = -export-dynamic $(AM_LDFLAGS)
#fb_gnash_DEPENDENCIES = .configline
fb_gnash_LDADD = \
	$(top_builddir)/librender/libgnashrender.la \
	$(GNASH_LIBS) 

if BUILD_AGG_RENDERER
fb_gnash_CPPFLAGS += $(AGG_CFLAGS)
fb_gnash_LDADD += $(AGG_LIBS)
endif				# BUILD_AGG_RENDERER

endif				# BUILD_FB_GUI

