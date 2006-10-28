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

# $Id: Makefile.am,v 1.11 2006/10/28 11:41:11 udog Exp $

AUTOMAKE_OPTIONS = 

AM_CPPFLAGS = 

noinst_LTLIBRARIES = libgnashparser.la 

# Only enable if we're configured with --enable-mp3
#
# The 'backend' dir should not be required if we finally drop
# rendering calls from the parser (see https://savannah.gnu.org/task/?5837)
#
INCLUDES = \
	-I$(top_srcdir)/backend \
	-I$(top_srcdir)/server \
	-I$(top_srcdir)/server/swf \
	-I$(top_srcdir)/server/parser \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/libamf \
        -I$(top_srcdir)	\
	$(PTHREAD_CFLAGS) \
        $(ZLIB_CFLAGS) \
        $(OGG_CFLAGS) \
        $(JPEG_CFLAGS) \
        $(SDL_CFLAGS) \
        $(OPENGL_CFLAGS) \
	$(GTK2_CFLAGS) \
	$(PANGO_CFLAGS)	\
	$(GLIB_CFLAGS) \
	$(CAIRO_CFLAGS)	\
	$(ATK_CFLAGS) \
	$(DMALLOC_CFLAGS) \
        $(LIBXML_CFLAGS)

libgnashparser_la_SOURCES = \
	action_buffer.cpp \
	bitmap_character_def.cpp \
	button_character_def.cpp \
	character_def.cpp \
	edit_text_character_def.cpp \
	text_character_def.cpp \
	movie_def_impl.cpp \
        morph2_character_def.cpp \
	shape_character_def.cpp	\
	sprite_definition.cpp \
	video_stream_def.cpp

noinst_HEADERS = \
	action_buffer.h \
	button_character_def.h \
	character_def.h	\
	bitmap_character_def.h \
	edit_text_character_def.h \
	text_character_def.h \
	morph2_character_def.h \
	movie_definition.h \
	movie_def_impl.h \
	shape_character_def.h \
	sprite_definition.h 

libgnashparser_la_LIBADD = \
	$(top_builddir)/libbase/libgnashbase.la 

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

