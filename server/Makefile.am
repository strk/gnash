## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
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

# $Id: Makefile.am,v 1.86 2006/11/13 17:10:57 strk Exp $

AUTOMAKE_OPTIONS = 

AM_CPPFLAGS = 

SUBDIRS = asobj parser

# noinst_LTLIBRARIES = libserver.la 
lib_LTLIBRARIES = libgnashserver.la
# noinst_SCRIPTS = gen-files.sh

# Only enable if we're configured with --enable-mp3
INCLUDES = \
	-I$(top_srcdir)/backend \
	-I$(top_srcdir)/server/swf \
	-I$(top_srcdir)/server/asobj \
	-I$(top_srcdir)/server/parser \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)	\
	$(PTHREAD_CFLAGS) \
	$(DMALLOC_CFLAGS) \
        $(LIBXML_CFLAGS)

libgnashserver_la_SOURCES = \
	as_object.cpp	\
	as_value.cpp	 \
	as_environment.cpp	 \
	as_function.cpp		\
	swf_function.cpp	\
	character.cpp \
	generic_character.cpp \
        textformat.cpp \
        timers.cpp \
        xml.cpp \
        xmlattrs.cpp \
        xmlnode.cpp \
	movie_root.cpp \
        xmlsocket.cpp \
        action.cpp \
	ActionExec.cpp \
        button_character_instance.cpp \
        dlist.cpp \
	edit_text_character.cpp \
        font.cpp \
        fontlib.cpp \
	GetterSetter.cpp \
        impl.cpp \
	rect.cpp \
        render.cpp \
        shape.cpp \
	shm.cpp	\
        sound.cpp \
	sprite_instance.cpp \
	matrix.cpp	\
	movie_instance.cpp \
	PropertyList.cpp \
        stream.cpp \
        StreamProvider.cpp \
	URLAccessManager.cpp	\
        styles.cpp \
        tesselate.cpp \
        text.cpp \
        array.cpp \
	swf/ASHandlers.cpp \
	swf/TagLoadersTable.cpp	\
	swf/tag_loaders.cpp	\
        types.cpp 	\
	video_yuv.cpp	\
	video_stream_instance.cpp \
	$(NULL)

noinst_HEADERS = \
	action.h \
	ActionExec.h \
	array.h	\
	as_value.h \
	as_environment.h \
	as_function.h 		\
	builtin_function.h	\
	swf_function.h		\
	as_prop_flags.h	\
	as_object.h \
	button_character_instance.h \
	matrix.h \
	mouse_button_state.h \
	dlist.h	\
	character.h \
	generic_character.h \
	edit_text_character.h \
	event_id.h \
	execute_tag.h \
	font.h \
	fontlib.h \
	GetterSetter.h \
	Global.h \
	gnash.h	\
	impl.h \
	movie.h	\
	movie_interface.h \
	movie_root.h \
	Property.h \
	PropertyList.h \
	rect.h \
	resource.h \
	render.h \
	shape.h	\
	shm.h \
	sound.h	\
	Sprite.h \
	sprite_instance.h \
	StringPredicates.h \
	movie_instance.h \
	stream.h \
	StreamProvider.h	\
	URLAccessManager.h	\
	styles.h \
	swf.h \
	swf_event.h \
	tesselate.h \
	textformat.h \
	text.h \
	timers.h \
	types.h	\
        xmlattrs.h \
	xml.h \
        xmlnode.h \
	swf/ASHandlers.h \
	swf/TagLoadersTable.h \
	swf/tag_loaders.h \
	xmlsocket.h \
	with_stack_entry.h \
	fn_call.h

libgnashserver_la_LIBADD = \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/server/asobj/libgnashasobjs.la \
	$(top_builddir)/server/parser/libgnashparser.la \
	$(LIBXML_LIBS)

#	$(ZLIB_LIBS) \
#	$(OPENGL_LIBS)

#libserver_la_LDFLAGS = -module -avoid-version -no-undefined
#libserver_la_LIBDADD =  #@DL_LIB@  # $(LIBLTDL)
libgnashserver_la_LDFLAGS = -avoid-version

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

