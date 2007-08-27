## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

# $Id: Makefile.am,v 1.42 2007/08/27 03:06:42 cmusick Exp $

AUTOMAKE_OPTIONS = 

noinst_LTLIBRARIES = libgnashasobjs.la 

AM_CPPFLAGS = \
	-I$(top_srcdir)/backend \
	-I$(top_srcdir)/server \
	-I$(top_srcdir)/server/swf \
	-I$(top_srcdir)/server/parser \
	-I$(top_srcdir)/server/vm \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/libgeometry \
	-I$(top_srcdir)/libamf \
	-I$(top_srcdir)/libltdl \
	-I$(top_srcdir)	\
	$(PTHREAD_CFLAGS) \
	$(DMALLOC_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(FFMPEG_CFLAGS) \
	$(GSTREAMER_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(LIBXML_CFLAGS) \
	$(NULL)


libgnashasobjs_la_SOURCES = \
        BevelFilter_as.cpp \
        BitmapFilter_as.cpp \
	Boolean.cpp	\
	Camera.cpp	\
	Color.cpp	\
	ContextMenu.cpp	\
	CustomActions.cpp\
	Date.cpp	\
	Error.cpp	\
	Global.cpp	\
	Key.cpp		\
	LoadVars.cpp	\
	LocalConnection.cpp\
	Math.cpp	\
	Microphone.cpp	\
	Mouse.cpp	\
	NetConnection.cpp\
	NetStream.cpp	\
	Number.cpp	\
	Object.cpp	\
	Selection.cpp	\
	SharedObject.cpp\
	Sound.cpp	\
	Stage.cpp	\
	System.cpp	\
	TextSnapshot.cpp\
	MovieClipLoader.cpp\
	string.cpp \
	xml.cpp \
	xmlattrs.cpp \
	xmlnode.cpp \
	xmlsocket.cpp \
	$(NULL)

noinst_HEADERS = \
        BevelFilter_as.h \
        BitmapFilter_as.h \
	Boolean.h	\
	Camera.h	\
	Color.h		\
	ContextMenu.h	\
	CustomActions.h	\
	Date.h		\
	Error.h		\
	GMath.h		\
	Global.h	\
	Key.h 		\
	LoadVars.h	\
	LocalConnection.h\
	Microphone.h	\
	Mouse.h		\
	MovieClipLoader.h \
	NetConnection.h	\
	NetStream.h	\
	NetStreamFfmpeg.h \
	NetStreamGst.h \
	Number.h	\
	Object.h	\
	Selection.h	\
	SharedObject.h	\
	Sound.h 	\
	SoundFfmpeg.h \
	SoundGst.h \
	SoundMad.h \
	Stage.h		\
	System.h	\
	TextSnapshot.h	\
	gstring.h	\
	xml.h		\
	xmlsocket.h \
	xmlattrs.h	\
	xmlnode.h	\
	$(NULL)

libgnashasobjs_la_LIBADD = \
	$(top_builddir)/libamf/libgnashamf.la \
	$(top_builddir)/libgeometry/libgnashgeo.la \
	$(top_builddir)/libbase/libgnashbase.la \
	$(LIBXML_LIBS)

if USE_FFMPEG_ENGINE
libgnashasobjs_la_SOURCES += NetStreamFfmpeg.cpp SoundFfmpeg.cpp
AM_CPPFLAGS += $(FFMPEG_CFLAGS)
# libgnashasobjs_la_LIBADD += $(FFMPEG_LIBS)
endif

if USE_SOUND_GST
libgnashasobjs_la_SOURCES += NetStreamGst.cpp SoundGst.cpp
AM_CPPFLAGS += $(GSTREAMER_CFLAGS)
libgnashasobjs_la_LIBADD += $(GSTREAMER_LIBS)
endif

if USE_MAD_ENGINE
libgnashasobjs_la_SOURCES += SoundMad.cpp
AM_CPPFLAGS += $(MAD_CFLAGS)
libgnashasobjs_la_LIBADD += $(MAD_LIBS)
endif

#libgnashasobjs_la_LDFLAGS = -release $(VERSION) -no-undefined -export-dynamic

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

