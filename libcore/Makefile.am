## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 
#   2011 Free Software Foundation, Inc.
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

if ENABLE_PCH
AM_CXXFLAGS = $(PCH_FLAGS)
endif

pkglib_LTLIBRARIES = libgnashcore.la

# TODO: use a conditional to optionally link ?
FREETYPE_SOURCES = FreetypeGlyphsProvider.h FreetypeGlyphsProvider.cpp

# Only enable if we're configured with --enable-mp3
AM_CPPFLAGS = \
	-I$(top_srcdir)/librender \
	-I$(top_srcdir)/libdevice \
	-I$(top_srcdir)/libcore/swf \
	-I$(top_srcdir)/libcore/abc \
	-I$(top_srcdir)/libcore/asobj \
	-I$(top_srcdir)/libcore/asobj/flash \
	-I$(top_srcdir)/libcore/parser \
	-I$(top_srcdir)/libcore/vm \
	-I$(top_srcdir)/libltdl \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libsound \
	$(PTHREAD_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(FREETYPE2_CFLAGS) \
	$(FONTCONFIG_CFLAGS) \
	$(NULL)

if BUILD_EGL_DEVICE
AM_CPPFLAGS += $(EGL_CFLAGS)
endif
if BUILD_DIRECTFB_DEVICE
AM_CPPFLAGS += $(DIRECTFB_CFLAGS)
endif

libgnashcore_la_SOURCES = \
	BitmapMovie.cpp \
	Property.cpp \
	PropertyList.cpp \
	SystemClock.cpp \
	ClassHierarchy.cpp \
	as_environment.cpp \
	as_function.cpp	\
	event_id.cpp \
	Relay.cpp \
	as_object.cpp \
	AMFConverter.cpp \
	as_value.cpp \
	DisplayObjectContainer.cpp \
	DisplayObject.cpp \
	CharacterProxy.cpp \
	SWFCxForm.cpp \
	Geometry.cpp \
	DynamicShape.cpp	\
	Bitmap.cpp \
	Shape.cpp \
	MorphShape.cpp \
	StaticText.cpp \
	TextField.cpp \
	parser/filter_factory.cpp \
	InteractiveObject.cpp \
	ExternalInterface.cpp \
	HostInterface.cpp \
	SWFMatrix.cpp \
	SWFMovie.cpp \
	movie_root.cpp \
	namedStrings.cpp \
	SWFRect.cpp \
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
	swf/DefinitionTag.cpp \
	swf/ShapeRecord.cpp \
	swf/SoundInfoRecord.cpp \
	swf/TextRecord.cpp \
	swf/tag_loaders.cpp \
	swf/DefineBitsTag.cpp \
	swf/DefineFontAlignZonesTag.cpp \
	swf/DefineShapeTag.cpp \
	swf/DefineScalingGridTag.cpp \
	swf/DefineEditTextTag.cpp \
	swf/DefineMorphShapeTag.cpp \
	swf/CSMTextSettingsTag.cpp \
	swf/PlaceObject2Tag.cpp \
	swf/RemoveObjectTag.cpp \
	swf/SoundStreamHeadTag.cpp	\
	swf/StartSoundTag.cpp \
	swf/SetTabIndexTag.cpp \
	swf/StreamSoundBlockTag.cpp \
	Function.cpp \
	Function2.cpp \
	Video.cpp \
	Button.cpp \
	DisplayList.cpp \
	FillStyle.cpp \
	Font.cpp \
	fontlib.cpp \
	LoadVariablesThread.cpp \
	SWFStream.cpp \
	LineStyle.cpp \
	Timers.cpp \
	RGBA.cpp 	\
	MovieFactory.cpp \
	MovieLoader.cpp \
	$(FREETYPE_SOURCES) \
	$(NULL)

if ENABLE_AVM2
libgnashcore_la_SOURCES += \
	abc/CodeStream.cpp \
	abc/Class.cpp \
	abc/Namespace.cpp \
	abc/as_class.cpp \
	abc/abc_function.cpp \
	abc/Method.cpp \
	abc/AbcBlock.cpp \
	$(NULL)
endif

noinst_HEADERS = \
	VirtualClock.h \
	SystemClock.h \
	ClassHierarchy.h \
	ManualClock.h \
	Bitmap.h \
	BitmapMovie.h \
	Transform.h \
	Button.h \
	TextField.h \
	Filters.h \
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
	HostInterface.h \
	ExternalInterface.h \
	swf/tag_loaders.h \
	swf/DefineBitsTag.h \
	swf/DefaultTagLoaders.h \
	swf/ImportAssetsTag.h \
	swf/ExportAssetsTag.h \
	swf/VideoFrameTag.h \
	swf/DefineVideoStreamTag.h \
	swf/DefineFontAlignZonesTag.h \
	swf/SymbolClassTag.h \
	swf/DefineSceneAndFrameLabelDataTag.h \
	swf/CSMTextSettingsTag.h \
	swf/DefineShapeTag.h \
	swf/DefineScalingGridTag.h \
	swf/DefineMorphShapeTag.h \
	swf/SoundStreamHeadTag.h \
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
	Function.h \
	Function2.h \
	Timers.h \
	Video.h \
	MovieLoader.h \
	$(NULL)

if ENABLE_AVM2
noinst_HEADERS += \
	abc/CodeStream.h \
	abc/Class.h \
	abc/Namespace.h \
	swf/DoABCTag.h \
	abc/as_class.h \
	abc/abc_function.h \
	abc/Method.h \
	abc/asException.h \
	abc/BoundValues.h \
	abc/AbcBlock.h \
	abc/MultiName.h \
	$(NULL)
endif

EXTENSIONS_API = \
	Relay.h \
	as_object.h \
	ObjectURI.h \
	Property.h \
	PropertyList.h \
	AMFConverter.h \
	as_value.h \
	PropFlags.h	\
	CharacterProxy.h \
	builtin_function.h \
	UserFunction.h \
	NativeFunction.h \
	as_function.h \
	namedStrings.h \
	as_environment.h \
	movie_root.h \
	MouseButtonState.h \
	DragState.h \
	InteractiveObject.h \
	SWFRect.h \
	GnashKey.h \
	Movie.h \
	RunResources.h \
	DisplayObjectContainer.h \
	DisplayObject.h \
	MovieClip.h \
	event_id.h \
	SWFMatrix.h \
	SWFCxForm.h \
	DisplayList.h	\
	DynamicShape.h	\
	swf/ControlTag.h \
	swf/DefinitionTag.h \
	swf/ShapeRecord.h \
	swf/TagLoadersTable.h \
	swf/SWF.h \
	MovieFactory.h \
	FillStyle.h \
	LineStyle.h \
	RGBA.h	\
	Geometry.h	\
	Video.h \
	$(NULL)

# These makefile fragments build the ActionScript library for
# Gnash, and supports both AS2 as well as AS3, so we always
# want to build.
include $(srcdir)/asobj/flash.am
libgnashcore_la_SOURCES += $(libgnashasobjs_la_SOURCES)

instdir = $(includedir)/gnash/asobj
inst_HEADERS += $(EXTENSIONS_API)

libgnashcore_la_LIBADD = \
	$(top_builddir)/libcore/parser/libgnashparser.la \
	$(top_builddir)/libcore/vm/libgnashvm.la \
	$(top_builddir)/libmedia/libgnashmedia.la \
	$(top_builddir)/libsound/libgnashsound.la \
	$(top_builddir)/libbase/libgnashbase.la \
	$(BOOST_LIBS) \
	$(FREETYPE2_LIBS) \
	$(FONTCONFIG_LIBS) \
	$(NULL)

libgnashcore_la_LDFLAGS = -release $(VERSION) -export-dynamic

if HAIKU
  libgnashcore_la_LIBADD += $(HAIKU_LIBS)
endif

# Remove libtool .la files
install-exec-hook:
	$(RM) $(DESTDIR)$(libdir)/gnash/libgnashcore.la

uninstall-local:
	$(RM) $(DESTDIR)$(libdir)/gnash/libgnashcore-*.so
