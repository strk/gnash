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
# Linking Gnash statically or dynamically with other modules is making
# a combined work based on Gnash. Thus, the terms and conditions of
# the GNU General Public License cover the whole combination.
# 
# In addition, as a special exception, the copyright holders of Gnash give
# you permission to combine Gnash with free software programs or
# libraries that are released under the GNU LGPL and/or with Mozilla, 
# so long as the linking with Mozilla, or any variant of Mozilla, is
# through its standard plug-in interface. You may copy and distribute
# such a system following the terms of the GNU GPL for Gnash and the
# licenses of the other code concerned, provided that you include the
# source code of that other code when and as the GNU GPL requires
# distribution of source code. 
# 
# Note that people who make modified versions of Gnash are not obligated
# to grant this special exception for their modified versions; it is
# their choice whether to do so.  The GNU General Public License gives
# permission to release a modified version without this exception; this
# exception also makes it possible to release a modified version which
# carries forward this exception.
#

AUTOMAKE_OPTIONS = 

AM_CPPFLAGS = 

# noinst_LTLIBRARIES = libserver.la libasobjs.la
lib_LTLIBRARIES = libgnashserver.la libgnashasobjs.la
# noinst_SCRIPTS = gen-files.sh

# Only enable if we're configured with --enable-mp3
INCLUDES = -I.. \
        -I$(top_srcdir)		\
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)		\
	$(PTHREAD_CFLAGS)	\
        $(ENGINE_INCLUDE)	\
        $(ZLIB_CFLAGS)		\
        $(OGG_CFLAGS)		\
        $(JPEG_CFLAGS)		\
        $(SDL_CFLAGS)		\
        $(SDL_MIXER_CFLAGS)	\
        $(OPENGL_CFLAGS)	\
	$(DMALLOC_CFLAGS)	\
        $(LIBXML_CFLAGS)

if MP3
MP3_CFLAGS = $(MAD_CFLAGS)
MP3_LIBS = $(MAD_LIBS)
MP3_HANDLER = sound_handler_mp3.cpp
endif

libgnashasobjs_la_SOURCES = \
	Boolean.cpp	\
	Camera.cpp	\
	Color.cpp	\
	ContextMenu.cpp	\
	CustomActions.cpp\
	Date.cpp	\
	Error.cpp	\
	Function.cpp	\
	LoadVars.cpp	\
	LocalConnection.cpp\
	Microphone.cpp	\
	Mouse.cpp	\
	NetConnection.cpp\
	NetStream.cpp	\
	Object.cpp	\
	Selection.cpp	\
	SharedObject.cpp\
	Stage.cpp	\
	System.cpp	\
	TextSnapshot.cpp\
	Video.cpp

as_incls = \
	Boolean.h	\
	Camera.h	\
	Color.h		\
	ContextMenu.h	\
	CustomActions.h	\
	Date.h		\
	Error.h		\
	Function.h	\
	LoadVars.h	\
	LocalConnection.h\
	Microphone.h	\
	Mouse.h		\
	NetConnection.h	\
	NetStream.h	\
	Selection.h	\
	SharedObject.h	\
	Stage.h		\
	System.h	\
	TextSnapshot.h	\
	Video.h

libgnashserver_la_SOURCES = \
        textformat.cpp   \
        timers.cpp       \
        xml.cpp          \
        xmlattrs.cpp     \
        xmlnode.cpp      \
        MovieClipLoader.cpp\
        xmlsocket.cpp    \
        string.cpp       \
        action.cpp       \
        button.cpp       \
        dlist.cpp        \
        font.cpp         \
        fontlib.cpp      \
        impl.cpp         \
        morph2.cpp       \
	network.cpp	 \
	Movie.cpp	 \
        render.cpp       \
        shape.cpp        \
	shm.cpp		 \
        sound.cpp        \
	Sprite.cpp	 \
        stream.cpp       \
        styles.cpp       \
        tesselate.cpp    \
        text.cpp         \
        array.cpp        \
        types.cpp $(MP3_HANDLER)

noinst_HEADERS = $(as_incls) \
	action.h	\
	array.h		\
	button.h	\
	dlist.h		\
	font.h		\
	fontlib.h	\
	gnash.h		\
	impl.h		\
	morph2.h	\
	morph.h		\
	network.h	\
	Movie.h		\
	MovieClipLoader.h	\
	Object.h	\
	render.h	\
	shape.h		\
	shm.h		\
	sound.h		\
	stream.h	\
	gstring.h	\
	Sprite.h	\
	styles.h	\
	swf.h		\
	tesselate.h	\
	text.h		\
	textformat.h	\
	thread.h	\
	timers.h	\
	types.h		\
	xml.h		\
        xmlattrs.h      \
        xmlnode.h       \
	xmlsocket.h

#libserver_la_LDFLAGS = -module -avoid-version -no-undefined
#libserver_la_LIBDADD =  #@DL_LIB@  # $(LIBLTDL)

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

