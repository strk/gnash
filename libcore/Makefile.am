## Process this file with automake to generate Makefile.in
# 
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

AUTOMAKE_OPTIONS = 

SUBDIRS = asobj parser vm

# noinst_LTLIBRARIES = libserver.la 
pkglib_LTLIBRARIES = libgnashcore.la
# noinst_SCRIPTS = gen-files.sh

# TODO: use a conditional to optionally link ?
FREETYPE_SOURCES = FreetypeGlyphsProvider.h FreetypeGlyphsProvider.cpp

# Only enable if we're configured with --enable-mp3
AM_CPPFLAGS = \
	-I$(top_srcdir)/gui \
	-I$(top_srcdir)/libamf \
	-I$(top_srcdir)/libnet \
	-I$(top_srcdir)/backend \
	-I$(top_srcdir)/libcore/swf \
	-I$(top_srcdir)/libcore/asobj \
	-I$(top_srcdir)/libcore/parser \
	-I$(top_srcdir)/libltdl \
	-I$(top_srcdir)/libcore/vm \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libamf \
	$(PTHREAD_CFLAGS) \
	$(DMALLOC_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(LIBXML_CFLAGS) \
	$(FREETYPE2_CFLAGS) \
	$(FONTCONFIG_CFLAGS) \
	$(NULL)

libgnashcore_la_SOURCES = \
	BitmapMovieInstance.cpp \
	Property.cpp \
	PropertyList.cpp \
	URLAccessManager.cpp \
	SystemClock.cpp \
	as_environment.cpp \
	as_function.cpp	\
	as_object.cpp \
	as_value.cpp \
	asClass.cpp \
	character.cpp \
	CharacterProxy.cpp \
	cxform.cpp \
	DynamicShape.cpp	\
	edit_text_character.cpp \
        BlurFilter.cpp \
        GlowFilter.cpp \
        DropShadowFilter.cpp \
        ConvolutionFilter.cpp \
        ColorMatrixFilter.cpp \
        GradientGlowFilter.cpp \
        BevelFilter.cpp \
        GradientBevelFilter.cpp \
        parser/filter_factory.cpp \
	generic_character.cpp \
	matrix.cpp \
	movie_instance.cpp \
	movie_root.cpp \
	namedStrings.cpp \
	rect.cpp \
	sprite_instance.cpp \
	swf/TagLoadersTable.cpp	\
	swf/tag_loaders.cpp \
	swf/DefineFontAlignZonesTag.cpp \
	swf/CSMTextSettingsTag.cpp \
	swf/PlaceObject2Tag.cpp \
	swf/RemoveObjectTag.cpp \
	swf/StartSoundTag.cpp \
	swf/StreamSoundBlockTag.cpp \
	swf_function.cpp \
	video_stream_instance.cpp \
	StreamProvider.cpp \
	array.cpp \
	Button.cpp \
	DisplayList.cpp \
	fill_style.cpp \
	font.cpp \
	fontlib.cpp \
	impl.cpp \
	LoadVariablesThread.cpp \
	render.cpp \
	shape.cpp \
	SWFStream.cpp \
	styles.cpp \
	text.cpp \
	timers.cpp \
	types.cpp 	\
	$(FREETYPE_SOURCES) \
	$(NULL)

noinst_HEADERS = \
	Property.h \
	PropertyList.h \
	Sprite.h \
	StreamProvider.h \
	StringPredicates.h \
	URLAccessManager.h \
	VirtualClock.h \
	SystemClock.h \
	ManualClock.h \
	array.h	\
	as_environment.h \
	as_function.h \
	as_object.h \
	as_prop_flags.h	\
	as_value.h \
	asClass.h \
	bitmap_info.h \
	BitmapMovieInstance.h \
	builtin_function.h \
	Button.h \
	character.h \
	CharacterProxy.h \
	cxform.h \
	debugger.h \
	DynamicShape.h	\
	DisplayList.h	\
	drag_state.h \
	edit_text_character.h \
	event_id.h \
	fill_style.h \
        BitmapFilter.h \
        BlurFilter.h \
        BevelFilter.h \
        GradientBevelFilter.h \
        GlowFilter.h \
        GradientGlowFilter.h \
        DropShadowFilter.h \
        ConvolutionFilter.h \
        ColorMatrixFilter.h \
        parser/filter_factory.h \
	font.h \
	fontlib.h \
	generic_character.h \
	gnash.h	\
	GnashKey.h \
	impl.h \
	LoadVariablesThread.h \
	matrix.h \
	mouse_button_state.h \
	movie_instance.h \
	movie_root.h \
	namedStrings.h \
	rect.h \
	render.h \
	resource.h \
	shape.h	\
	sprite_instance.h \
	SWFStream.h \
	styles.h \
	swf.h \
	swf/TagLoadersTable.h \
	swf/tag_loaders.h \
	swf/ControlTag.h \
	swf/DefineFontAlignZonesTag.h \
	swf/CSMTextSettingsTag.h \
	swf/DefineButtonCxformTag.h \
	swf/PlaceObject2Tag.h \
	swf/RemoveObjectTag.h \
	swf/DisplayListTag.h \
	swf/DoActionTag.h \
	swf/DoInitActionTag.h \
	swf/SetBackgroundColorTag.h \
	swf/StartSoundTag.h \
	swf/StreamSoundBlockTag.h \
	swf/ScriptLimitsTag.h \
	swf_event.h \
	swf_function.h \
	text.h \
	timers.h \
	types.h	\
	video_stream_instance.h \
	$(NULL)

libgnashcore_la_LIBADD = \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/libamf/libgnashamf.la \
	$(top_builddir)/libnet/libgnashnet.la \
	$(top_builddir)/libcore/asobj/libgnashasobjs.la \
	$(top_builddir)/libcore/parser/libgnashparser.la \
	$(top_builddir)/libcore/vm/libgnashvm.la \
	$(LIBXML_LIBS) \
	$(BOOST_LIBS) \
	$(PTHREAD_LIBS) \
	$(FREETYPE2_LIBS) \
	$(FONTCONFIG_LIBS) \
	$(NULL)

if USE_GST_ENGINE
AM_CPPFLAGS += $(GSTREAMER_CFLAGS) \
		-I$(top_srcdir)/libmedia/gst
libgnashcore_la_LIBADD += $(GSTREAMER_LIBS)
endif

if USE_FFMPEG_ENGINE
AM_CPPFLAGS += $(FFMPEG_CFLAGS) \
		-I$(top_srcdir)/libmedia/ffmpeg
endif

# Enable building the Flash debugger
if DEBUGGER
libgnashcore_la_SOURCES += debugger.cpp
endif

#libserver_la_LIBDADD =  #@DL_LIB@  # $(LIBLTDL)
libgnashcore_la_LDFLAGS = -release $(VERSION) -export-dynamic

if WIN32
  libgnashcore_la_LDFLAGS += -no-undefined
  libgnashcore_la_LIBADD += \
	$(top_builddir)/libmedia/libgnashmedia_la-MediaParser.lo \
	$(top_builddir)/libmedia/libgnashmedia_la-MediaHandler.lo \
	$(top_builddir)/libmedia/libgnashmedia_la-FLVParser.lo
else
  libgnashcore_la_LIBADD += $(top_builddir)/libmedia/libgnashmedia.la
endif

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

