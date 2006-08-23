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

noinst_LTLIBRARIES = libgnashasobjs.la 

# Only enable if we're configured with --enable-mp3
INCLUDES = \
	-I$(top_srcdir)/server \
	-I$(top_srcdir)/server/swf \
	-I$(top_srcdir)/server/parser \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/libamf \
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

libgnashasobjs_la_SOURCES = \
	Boolean.cpp	\
	Camera.cpp	\
	Color.cpp	\
	ContextMenu.cpp	\
	CustomActions.cpp\
	Date.cpp	\
	Error.cpp	\
	Math.cpp	\
	Global.cpp	\
	Key.cpp		\
	LoadVars.cpp	\
	LocalConnection.cpp\
	Microphone.cpp	\
	Mouse.cpp	\
	MovieClip.cpp   \
        MovieClipLoader.cpp\
	NetConnection.cpp\
	NetStream.cpp	\
	Selection.cpp	\
	SharedObject.cpp\
	ASSound.cpp	\
	Stage.cpp	\
	System.cpp	\
	TextSnapshot.cpp\
	Video.cpp

noinst_HEADERS = \
	ASSound.h 	\
	Boolean.h	\
	Camera.h	\
	Color.h		\
	ContextMenu.h	\
	CustomActions.h	\
	Date.h		\
	Error.h		\
	Key.h \
	LoadVars.h	\
	LocalConnection.h\
	Microphone.h	\
	GMath.h		\
	Mouse.h		\
	MovieClip.h	\
	MovieClipLoader.h \
	NetConnection.h	\
	NetStream.h	\
	Selection.h	\
	SharedObject.h	\
	Stage.h		\
	System.h	\
	TextSnapshot.h	\
	Video.h

libgnashasobjs_la_LIBADD = \
	../../libamf/libgnashamf.la \
	../../libbase/libgnashbase.la

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

