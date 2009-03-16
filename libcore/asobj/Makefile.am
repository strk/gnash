## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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


AUTOMAKE_OPTIONS = 

# if the compiler has broken flaot support when optimized
if BROKEN_FLOAT
AM_CXXFLAGS = -ffloat-store
endif

noinst_LTLIBRARIES = libgnashasobjs.la 

AM_CPPFLAGS = \
	-I$(top_srcdir)/libnet \
	-I$(top_srcdir)/backend \
	-I$(top_srcdir)/libcore \
	-I$(top_srcdir)/libcore/swf \
	-I$(top_srcdir)/libcore/parser \
	-I$(top_srcdir)/libcore/vm \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/libamf \
	-I$(top_srcdir)/libltdl \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libsound \
	$(PTHREAD_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(FREETYPE2_CFLAGS) \
	$(NULL)


libgnashasobjs_la_SOURCES = \
	AsBroadcaster.cpp \
	Accessibility_as.cpp \
	Array_as.cpp \
	Boolean_as.cpp \
	Camera.cpp \
	ClassHierarchy.cpp \
	Color_as.cpp \
	ContextMenu.cpp	\
	CustomActions.cpp\
	Date_as.cpp \
	Error_as.cpp \
	Global.cpp \
	Key_as.cpp	\
	LoadVars_as.cpp \
	LocalConnection_as.cpp\
	Math_as.cpp \
	Microphone.cpp	\
	Mouse_as.cpp \
	NetConnection_as.cpp \
	NetStream_as.cpp \
	Number_as.cpp \
	Object.cpp \
	PlayHead.cpp \
	Selection_as.cpp \
	SharedObject_as.cpp\
	Sound_as.cpp \
	Stage_as.cpp \
	System_as.cpp \
	TextFormat_as.cpp \
	TextSnapshot_as.cpp \
	MovieClipLoader.cpp\
	String_as.cpp \
	XML_as.cpp \
	XMLNode_as.cpp \
	XMLSocket_as.cpp \
	flash/display/BitmapData_as.cpp \
	flash/display_pkg.cpp \
	flash/external/ExternalInterface_as.cpp \
	flash/external_pkg.cpp \
        flash/filters/BevelFilter_as.cpp \
        flash/filters/BitmapFilter_as.cpp \
        flash/filters/DisplacementMapFilter_as.cpp \
        flash/filters/BlurFilter_as.cpp \
	flash/filters/ColorMatrixFilter_as.cpp \
	flash/filters/ConvolutionFilter_as.cpp \
	flash/filters/DropShadowFilter_as.cpp \
	flash/filters/GlowFilter_as.cpp \
	flash/filters/GradientBevelFilter_as.cpp \
	flash/filters/GradientGlowFilter_as.cpp \
	flash/filters_pkg.cpp \
	flash/geom/ColorTransform_as.cpp \
	flash/geom/Matrix_as.cpp \
	flash/geom_pkg.cpp \
	flash/geom/Point_as.cpp \
	flash/geom/Rectangle_as.cpp \
	flash/geom/Transform_as.cpp \
	flash/net/FileReference_as.cpp \
	flash/net/FileReferenceList_as.cpp \
	flash/net_pkg.cpp \
	flash_pkg.cpp \
	flash/text_pkg.cpp \
	flash/text/TextRenderer_as.cpp \
	LoadableObject.cpp \
	$(NULL)

noinst_HEADERS = \
	AsBroadcaster.h \
	Accessibility_as.h \
	Array_as.h \
	Boolean_as.h \
	Camera.h \
	ClassHierarchy.h \
	Color_as.h	\
	ContextMenu.h \
	CustomActions.h	\
	Date_as.h \
	Error_as.h	\
	Math_as.h	\
	Global.h\
	Key_as.h \
	LoadVars_as.h \
	LocalConnection_as.h\
	Microphone.h \
	Mouse_as.h	\
	MovieClipLoader.h \
	NetConnection_as.h	\
	NetStream_as.h \
	Number_as.h \
	Object.h \
	PlayHead.h \
	prophelper.h \
	Selection_as.h \
	SharedObject_as.h \
	Sound_as.h \
	Stage_as.h	\
	System_as.h \
	TextFormat_as.h \
	TextSnapshot_as.h \
	String_as.h \
	XML_as.h \
	XMLSocket_as.h \
	XMLNode_as.h \
	flash/display/BitmapData_as.h \
	flash/display_pkg.h \
	flash/external/ExternalInterface_as.h \
	flash/external_pkg.h \
        flash/filters/BevelFilter_as.h \
        flash/filters/BitmapFilter_as.h \
        flash/filters/DisplacementMapFilter_as.h \
        flash/filters/BlurFilter_as.h \
	flash/filters/ColorMatrixFilter_as.h \
	flash/filters/ConvolutionFilter_as.h \
	flash/filters/DropShadowFilter_as.h \
	flash/filters/GlowFilter_as.h \
	flash/filters/GradientBevelFilter_as.h \
	flash/filters/GradientGlowFilter_as.h \
	flash/filters_pkg.h \
	flash/geom/ColorTransform_as.h \
	flash/geom/Matrix_as.h \
	flash/geom_pkg.h \
	flash/geom/Point_as.h \
	flash/geom/Rectangle_as.h \
	flash/geom/Transform_as.h \
	flash/net/FileReference_as.h \
	flash/net/FileReferenceList_as.h \
	flash/net_pkg.h \
	flash_pkg.h \
	flash/text_pkg.h \
	flash/text/TextRenderer_as.h \
	LoadableObject.h \
	$(NULL)

libgnashasobjs_la_LIBADD = \
	$(top_builddir)/libamf/libgnashamf.la \
	$(top_builddir)/libbase/libgnashbase.la

if USE_FFMPEG_ENGINE
AM_CPPFLAGS += $(FFMPEG_CFLAGS) \
		$(SDL_CFLAGS) \
		-I$(top_srcdir)/libmedia/ffmpeg
libgnashasobjs_la_LIBADD += $(FFMPEG_LIBS) \
				$(SDL_LIBS)
endif

if USE_GST_ENGINE
AM_CPPFLAGS += $(GSTREAMER_CFLAGS) \
		-I$(top_srcdir)/libmedia/gst
libgnashasobjs_la_LIBADD += $(GSTREAMER_LIBS) \
				$(SDL_LIBS)
endif

#libgnashasobjs_la_LDFLAGS = -release $(VERSION) -no-undefined -export-dynamic

if WIN32
  libgnashasobjs_la_LDFLAGS = -no-undefined
endif

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

