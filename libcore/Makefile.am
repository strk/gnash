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

SUBDIRS = parser vm

# noinst_LTLIBRARIES = libserver.la 
pkglib_LTLIBRARIES = libgnashcore.la
# noinst_SCRIPTS = gen-files.sh

# TODO: use a conditional to optionally link ?
FREETYPE_SOURCES = FreetypeGlyphsProvider.h FreetypeGlyphsProvider.cpp

# Only enable if we're configured with --enable-mp3
AM_CPPFLAGS = \
	-I$(top_srcdir)/libamf \
	-I$(top_srcdir)/libnet \
	-I$(top_srcdir)/backend \
	-I$(top_srcdir)/libcore/swf \
	-I$(top_srcdir)/libcore/asobj \
	-I$(top_srcdir)/libcore/asobj/flash \
	-I$(top_srcdir)/libcore/parser \
	-I$(top_srcdir)/libltdl \
	-I$(top_srcdir)/libcore/vm \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libsound \
	-I$(top_srcdir)/libamf \
	$(PTHREAD_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(FREETYPE2_CFLAGS) \
	$(FONTCONFIG_CFLAGS) \
	$(NULL)


libgnashcore_la_SOURCES = \
	BitmapMovie.cpp \
	Property.cpp \
	PropertyList.cpp \
	URLAccessManager.cpp \
	SystemClock.cpp \
	ClassHierarchy.cpp \
	as_environment.cpp \
	as_function.cpp	\
	as_object.cpp \
	as_value.cpp \
	DisplayObjectContainer.cpp \
	DisplayObject.cpp \
	CharacterProxy.cpp \
	cxform.cpp \
	Geometry.cpp \
	DynamicShape.cpp	\
	Bitmap.cpp \
	Shape.cpp \
	MorphShape.cpp \
	StaticText.cpp \
	TextField.cpp \
        BlurFilter.cpp \
        GlowFilter.cpp \
        DropShadowFilter.cpp \
        ConvolutionFilter.cpp \
        ColorMatrixFilter.cpp \
        GradientGlowFilter.cpp \
        BevelFilter.cpp \
        GradientBevelFilter.cpp \
        parser/filter_factory.cpp \
	InteractiveObject.cpp \
	SWFMatrix.cpp \
	SWFMovie.cpp \
	movie_root.cpp \
	namedStrings.cpp \
	rect.cpp \
	MovieClip.cpp \
	swf/SWF.cpp \
	swf/TagLoadersTable.cpp	\
	swf/DefaultTagLoaders.cpp \
	swf/DefineVideoStreamTag.cpp \
	swf/DefineTextTag.cpp \
	swf/DefineButtonSoundTag.cpp \
	swf/DefineButtonCxformTag.cpp \
	swf/DefineButtonTag.cpp \
	swf/DefineFontTag.cpp \
	swf/VideoFrameTag.cpp \
	swf/ShapeRecord.cpp \
	swf/SoundInfoRecord.cpp \
	swf/TextRecord.cpp \
	swf/tag_loaders.cpp \
	swf/DefineFontAlignZonesTag.cpp \
	swf/DefineShapeTag.cpp \
	swf/DefineScalingGridTag.cpp \
	swf/DefineEditTextTag.cpp \
	swf/DefineMorphShapeTag.cpp \
	swf/CSMTextSettingsTag.cpp \
	swf/PlaceObject2Tag.cpp \
	swf/RemoveObjectTag.cpp \
	swf/StartSoundTag.cpp \
	swf/SetTabIndexTag.cpp \
	swf/StreamSoundBlockTag.cpp \
	swf_function.cpp \
	Video.cpp \
	StreamProvider.cpp \
	Button.cpp \
	DisplayList.cpp \
	fill_style.cpp \
	Font.cpp \
	fontlib.cpp \
	impl.cpp \
	LoadVariablesThread.cpp \
	SWFStream.cpp \
	styles.cpp \
	Timers.cpp \
	RGBA.cpp 	\
	asClass.cpp \
	asNamespace.cpp \
	$(FREETYPE_SOURCES) \
	$(NULL)

if ENABLE_AVM2
libgnashcore_la_SOURCES += \
	abc_function.cpp \
	asMethod.cpp \
	$(NULL)
endif

noinst_HEADERS = \
	URLAccessManager.h \
	VirtualClock.h \
	SystemClock.h \
	ClassHierarchy.h \
	ManualClock.h \
	Bitmap.h \
	BitmapMovie.h \
	Button.h \
	debugger.h \
	TextField.h \
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
	Font.h \
	fontlib.h \
	Shape.h \
	MorphShape.h \
	StaticText.h \
	LoadVariablesThread.h \
	SWFMovie.h \
	SWFStream.h \
	MovieLibrary.h \
	swf/tag_loaders.h \
	swf/DefaultTagLoaders.h \
	swf/VideoFrameTag.h \
	swf/DefineVideoStreamTag.h \
	swf/DefineFontAlignZonesTag.h \
	swf/SymbolClassTag.h \
	swf/DefineSceneAndFrameLabelDataTag.h \
	swf/CSMTextSettingsTag.h \
	swf/DefineShapeTag.h \
	swf/DefineScalingGridTag.h \
	swf/DefineMorphShapeTag.h \
	swf/SoundInfoRecord.h \
	swf/TextRecord.h \
	swf/DefineButtonSoundTag.h \
	swf/DefineTextTag.h \
	swf/DefineButtonTag.h \
	swf/DefineEditTextTag.h \
	swf/DefineButtonCxformTag.h \
	swf/PlaceObject2Tag.h \
	swf/DefineFontTag.h \
	swf/DefineFontNameTag.h \
	swf/RemoveObjectTag.h \
	swf/DisplayListTag.h \
	swf/DoActionTag.h \
	swf/DoInitActionTag.h \
	swf/SetBackgroundColorTag.h \
	swf/SetTabIndexTag.h \
	swf/StartSoundTag.h \
	swf/StreamSoundBlockTag.h \
	swf/ScriptLimitsTag.h \
	swf_event.h \
	swf_function.h \
	Timers.h \
	Video.h \
	$(NULL)

if ENABLE_AVM2
noinst_HEADERS += \
	swf/DoABCTag.h \
	abc_function.h \
	asMethod.h \
	asException.h \
	asBoundValues.h \
	$(NULL)
endif

EXTENSIONS_API = \
	as_object.h \
	Property.h \
	PropertyList.h \
	as_value.h \
	PropFlags.h	\
	CharacterProxy.h \
	StringPredicates.h \
	builtin_function.h \
	as_function.h \
	namedStrings.h \
	as_environment.h \
	movie_root.h \
	MouseButtonState.h \
	drag_state.h \
	InteractiveObject.h \
	rect.h \
	GnashKey.h \
	Movie.h \
	RunResources.h \
	gnash.h	\
	DisplayObjectContainer.h \
	DisplayObject.h \
	MovieClip.h \
	StreamProvider.h \
	event_id.h \
	SWFMatrix.h \
	cxform.h \
	DisplayList.h	\
	DynamicShape.h	\
	swf/ControlTag.h \
	swf/DefinitionTag.h \
	swf/ShapeRecord.h \
	swf/TagLoadersTable.h \
	swf/SWF.h \
	MovieFactory.h \
	fill_style.h \
	styles.h \
	ExportableResource.h \
	BitmapInfo.h \
	RGBA.h	\
	Geometry.h	\
	Video.h \
	asClass.h \
	asNamespace.h \
	$(NULL)

# These makefile fragments build the ActionScript library for
# Gnash, and supports both AS2 as well as AS3, so we always
# want to build.
include $(srcdir)/asobj/flash.am
libgnashcore_la_SOURCES += $(libgnashasobjs_la_SOURCES)

if SDKINSTALL
noinst_HEADERS += $(EXTENSIONS_API)
else
instdir = $(includedir)/gnash
inst_HEADERS = $(EXTENSIONS_API)
endif

libgnashcore_la_LIBADD = \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/libamf/libgnashamf.la \
	$(top_builddir)/libnet/libgnashnet.la \
	$(top_builddir)/libcore/parser/libgnashparser.la \
	$(top_builddir)/libcore/vm/libgnashvm.la \
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
	$(top_builddir)/libmedia/libgnashmedia_la-FLVParser.lo \
	$(top_builddir)/libmedia/libgnashmedia_la-AudioDecoderNellymoser.lo \
	$(top_builddir)/libmedia/libgnashmedia_la-AudioDecoderSimple.lo \
	$(top_builddir)/libmedia/libgnashmedia_la-AudioResampler.lo 
if HAVE_SPEEX
  libgnashcore_la_LIBADD += \
	$(top_builddir)/libmedia/libgnashmedia_la-AudioDecoderSpeex.lo
endif
else
  libgnashcore_la_LIBADD += $(top_builddir)/libmedia/libgnashmedia.la \
	$(top_builddir)/libsound/libgnashsound.la
endif

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

