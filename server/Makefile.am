## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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

# $Id: Makefile.am,v 1.124 2007/08/28 08:45:11 strk Exp $

AUTOMAKE_OPTIONS = 

SUBDIRS = asobj parser vm

# noinst_LTLIBRARIES = libserver.la 
pkglib_LTLIBRARIES = libgnashserver.la
# noinst_SCRIPTS = gen-files.sh

# TODO: use a conditional to optionally link ?
FREETYPE_SOURCES = FreetypeGlyphsProvider.h FreetypeGlyphsProvider.cpp

# Only enable if we're configured with --enable-mp3
AM_CPPFLAGS = \
	-I$(top_srcdir)/backend \
	-I$(top_srcdir)/server/swf \
	-I$(top_srcdir)/server/asobj \
	-I$(top_srcdir)/server/parser \
	-I$(top_srcdir)/libltdl \
	-I$(top_srcdir)/server/vm \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/libgeometry \
	-I$(top_srcdir)	\
	$(PTHREAD_CFLAGS) \
	$(DMALLOC_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GSTREAMER_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(LIBXML_CFLAGS) \
	$(FREETYPE2_CFLAGS) \
	$(FONTCONFIG_CFLAGS) \
	$(NULL)

libgnashserver_la_SOURCES = \
	BitmapMovieInstance.cpp \
	BitsReader.cpp \
	GetterSetter.cpp \
	PropertyList.cpp \
	URLAccessManager.cpp \
	as_environment.cpp \
	as_function.cpp	\
	as_object.cpp \
	as_value.cpp \
	character.cpp \
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
	rect.cpp \
	shm.cpp	\
	sprite_instance.cpp \
	swf/TagLoadersTable.cpp	\
	swf/tag_loaders.cpp \
	swf/DefineFontAlignZonesTag.cpp \
	swf/PlaceObject2Tag.cpp \
	swf/RemoveObjectTag.cpp \
	swf_function.cpp \
	video_stream_instance.cpp \
	StreamProvider.cpp \
	array.cpp \
	button_character_instance.cpp \
	dlist.cpp \
	fill_style.cpp \
	font.cpp \
	fontlib.cpp \
	impl.cpp \
	LoadVariablesThread.cpp \
	render.cpp \
	shape.cpp \
	stream.cpp \
	styles.cpp \
	tesselate.cpp \
	text.cpp \
	textformat.cpp \
	timers.cpp \
	types.cpp 	\
	$(FREETYPE_SOURCES) \
	$(NULL)

noinst_HEADERS = \
	BitsReader.h \
	GetterSetter.h \
	Property.h \
	PropertyList.h \
	Sprite.h \
	StreamProvider.h \
	StringPredicates.h \
	URLAccessManager.h \
	array.h	\
	as_environment.h \
	as_function.h \
	as_object.h \
	as_prop_flags.h	\
	as_value.h \
	bitmap_info.h \
	BitmapMovieInstance.h \
	builtin_function.h \
	button_character_instance.h \
	character.h \
	cxform.h \
	debugger.h \
	DynamicShape.h	\
	dlist.h	\
	drag_state.h \
	edit_text_character.h \
	event_id.h \
	execute_tag.h \
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
	impl.h \
	LoadVariablesThread.h \
	matrix.h \
	mouse_button_state.h \
	movie_instance.h \
	movie_root.h \
	rect.h \
	render.h \
	resource.h \
	shape.h	\
	shm.h \
	sprite_instance.h \
	stream.h \
	styles.h \
	swf.h \
	swf/TagLoadersTable.h \
	swf/tag_loaders.h \
	swf/DefineFontAlignZonesTag.h \
	swf/PlaceObject2Tag.h \
	swf/RemoveObjectTag.h \
	swf/DisplayListTag.h \
	swf/DoActionTag.h \
	swf_event.h \
	swf_function.h \
	tesselate.h \
	text.h \
	textformat.h \
	timers.h \
	types.h	\
	video_stream_instance.h \
	$(NULL)

libgnashserver_la_LIBADD = \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/server/asobj/libgnashasobjs.la \
	$(top_builddir)/server/parser/libgnashparser.la \
	$(top_builddir)/server/vm/libgnashvm.la \
	$(LIBXML_LIBS) \
	$(BOOST_LIBS) \
	$(PTHREAD_LIBS) \
	$(FREETYPE2_LIBS) \
	$(FONTCONFIG_LIBS) \
	$(NULL)

if USE_SOUND_GST
AM_CPPFLAGS += $(GSTREAMER_CFLAGS)
endif


# Enable building the Flash debugger
if DEBUGGER
libgnashserver_la_SOURCES += debugger.cpp
endif

#libserver_la_LIBDADD =  #@DL_LIB@  # $(LIBLTDL)
libgnashserver_la_LDFLAGS = -release $(VERSION) -export-dynamic #-no-undefined 

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

