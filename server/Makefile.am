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

# Linking Gnash statically or dynamically with other modules is making a
# combined work based on Gnash. Thus, the terms and conditions of the GNU
# General Public License cover the whole combination.
#
# As a special exception, the copyright holders of Gnash give you
# permission to combine Gnash with free software programs or libraries
# that are released under the GNU LGPL and with code included in any
# release of Talkback distributed by the Mozilla Foundation. You may
# copy and distribute such a system following the terms of the GNU GPL
# for all but the LGPL-covered parts and Talkback, and following the
# LGPL for the LGPL-covered parts.
#
# Note that people who make modified versions of Gnash are not obligated
# to grant this special exception for their modified versions; it is their
# choice whether to do so. The GNU General Public License gives permission
# to release a modified version without this exception; this exception
# also makes it possible to release a modified version which carries
# forward this exception.
# 
#

AUTOMAKE_OPTIONS = 

AM_CPPFLAGS = 

SUBDIRS = asobj

# noinst_LTLIBRARIES = libserver.la 
lib_LTLIBRARIES = libgnashserver.la
# noinst_SCRIPTS = gen-files.sh

# Only enable if we're configured with --enable-mp3
INCLUDES = \
	-I$(top_srcdir)/server/swf \
	-I$(top_srcdir)/server/asobj \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)	\
	$(PTHREAD_CFLAGS) \
        $(ENGINE_INCLUDE) \
        $(ZLIB_CFLAGS) \
        $(OGG_CFLAGS) \
        $(JPEG_CFLAGS) \
        $(SDL_CFLAGS) \
        $(SDL_MIXER_CFLAGS) \
        $(OPENGL_CFLAGS) \
	$(GTK2_CFLAGS) \
	$(PANGO_CFLAGS)	\
	$(GLIB_CFLAGS) \
	$(CAIRO_CFLAGS)	\
	$(ATK_CFLAGS) \
	$(DMALLOC_CFLAGS) \
        $(LIBXML_CFLAGS)

libgnashserver_la_SOURCES = \
	as_object.cpp	\
	as_value.cpp	 \
	as_environment.cpp	 \
	as_function.cpp		\
	swf_function.cpp	\
	character.cpp \
	character_def.cpp \
        textformat.cpp \
        timers.cpp \
        xml.cpp \
        xmlattrs.cpp \
        xmlnode.cpp \
	movie_root.cpp \
	movie_def_impl.cpp \
        xmlsocket.cpp \
        string.cpp \
        action.cpp \
	action_buffer.cpp \
	ActionExec.cpp \
        button.cpp \
        dlist.cpp \
	edit_text_character.cpp \
	edit_text_character_def.cpp \
        font.cpp \
        fontlib.cpp \
        impl.cpp \
        morph2.cpp \
	MovieClip.cpp \
        render.cpp \
        shape.cpp \
	shape_character_def.cpp	\
	shm.cpp	\
        sound.cpp \
	sprite_definition.cpp \
	sprite_instance.cpp \
        stream.cpp \
        StreamProvider.cpp \
        styles.cpp \
        tesselate.cpp \
        text.cpp \
        array.cpp \
	swf/ASHandlers.cpp \
	swf/TagLoadersTable.cpp	\
	swf/tag_loaders.cpp	\
        types.cpp 

noinst_HEADERS = \
	action.h \
	action_buffer.h \
	ActionExec.h \
	array.h	\
	as_value.h \
	as_environment.h \
	as_function.h 		\
	builtin_function.h	\
	swf_function.h		\
	as_member.h \
	as_prop_flags.h	\
	as_object.h \
	button.h \
	dlist.h	\
	character.h \
	character_def.h	\
	generic_character.h \
	bitmap_character_def.h \
	edit_text_character.h \
	edit_text_character_def.h \
	execute_tag.h \
	font.h \
	fontlib.h \
	Global.h \
	gnash.h	\
	gstring.h \
	impl.h \
	morph2.h \
	morph.h	\
	MovieClip.h \
	movie.h	\
	movie_definition.h \
	movie_def_impl.h \
	movie_interface.h \
	movie_root.h \
	Object.h \
	resource.h \
	ref_counted.h \
	render.h \
	shape.h	\
	shape_character_def.h \
	shm.h \
	sound.h	\
	Sprite.h \
	sprite_definition.h \
	sprite_instance.h \
	stream.h \
	StreamProvider.h	\
	styles.h \
	swf.h \
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
	$(ZLIB_LIBS) \
	$(LIBXML_LIBS) \
	$(LIBXML_LIBS) \
	$(OPENGL_LIBS)

#libserver_la_LDFLAGS = -module -avoid-version -no-undefined
#libserver_la_LIBDADD =  #@DL_LIB@  # $(LIBLTDL)

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

