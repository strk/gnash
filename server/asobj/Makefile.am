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

# $Id: Makefile.am,v 1.18 2006/11/21 20:13:26 strk Exp $

AUTOMAKE_OPTIONS = 

AM_CPPFLAGS = 

noinst_LTLIBRARIES = libgnashasobjs.la 

# Only enable if we're configured with --enable-mp3
INCLUDES = \
	-I$(top_srcdir)/backend \
	-I$(top_srcdir)/server \
	-I$(top_srcdir)/server/swf \
	-I$(top_srcdir)/server/parser \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/libamf \
        -I$(top_srcdir)	\
	$(PTHREAD_CFLAGS) \
	$(DMALLOC_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(LIBXML_CFLAGS) \
	$(FFMPEG_CFLAGS) \
	$(NULL)

#        $(ZLIB_CFLAGS) \
#        $(OGG_CFLAGS) \
#        $(JPEG_CFLAGS) \
#        $(SDL_CFLAGS) \
#        $(OPENGL_CFLAGS) \
#	$(GTK2_CFLAGS) \
#	$(PANGO_CFLAGS)	\
#	$(GLIB_CFLAGS) \
#	$(CAIRO_CFLAGS)	\
#	$(ATK_CFLAGS) \
#

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
	Number.cpp	\
	Object.cpp	\
	Selection.cpp	\
	SharedObject.cpp\
	ASSound.cpp	\
        string.cpp \
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
	gstring.h \
	Mouse.h		\
	MovieClip.h	\
	MovieClipLoader.h \
	NetConnection.h	\
	NetStream.h	\
	Number.h	\
	Object.h	\
	Selection.h	\
	SharedObject.h	\
	Stage.h		\
	System.h	\
	TextSnapshot.h	\
	Video.h

libgnashasobjs_la_LIBADD = \
	$(top_builddir)/libamf/libgnashamf.la \
	$(top_builddir)/libbase/libgnashbase.la \
	$(FFMPEG_LIBS)

libgnashasobjs_la_LDFLAGS = -avoid-version

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

