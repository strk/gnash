## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#

AUTOMAKE_OPTIONS = 

AM_CXXFLAGS = $(CFLAGS) $(SDL_CFLAGS) $(SDL_MIXER_CFLAGS) $(INCLUDES)
AM_CPPFLAGS = -Wall

noinst_LTLIBRARIES = libserver.la

# Only enable if we're configured with --enable-mp3
INCLUDES = -I.. \
        -I$(top_srcdir)		\
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)		\
        $(ENGINE_INCLUDE)	\
        $(ZLIB_CFLAGS)		\
        $(SDL_MIXER_CFLAGS)	\
	$(DMALLOC_CFLAGS)	\
        $(LIBXML_CFLAGS)

if MP3
MP3_CFLAGS = $(MAD_CFLAGS)
MP3_LIBS = $(MAD_LIBS)
MP3_HANDLER = sound_handler_mp3.cpp
endif

libserver_la_SOURCES = \
        textformat.cpp   \
        timers.cpp       \
        xml.cpp          \
        movie.cpp        \
        xmlsocket.cpp    \
        string.cpp       \
        action.cpp       \
        button.cpp       \
        dlist.cpp        \
        font.cpp         \
        fontlib.cpp      \
        impl.cpp         \
        log.cpp          \
        morph2.cpp       \
        render.cpp       \
        shape.cpp        \
        sound.cpp        \
        stream.cpp       \
        styles.cpp       \
        tesselate.cpp    \
        text.cpp         \
        array.cpp        \
        types.cpp $(MP3_HANDLER)

noinst_HEADERS = \
	action.h	\
	array.h		\
	button.h	\
	dlist.h		\
	font.h		\
	fontlib.h	\
	gnash.h		\
	impl.h		\
	log.h		\
	morph2.h	\
	morph.h		\
	movie.h		\
	render.h	\
	shape.h		\
	sound.h		\
	stream.h	\
	gstring.h	\
	styles.h	\
	tesselate.h	\
	textformat.h	\
	thread.h	\
	timers.h	\
	types.h		\
	xml.h		\
	xmlsocket.h

#libserver_la_LDFLAGS = -module -avoid-version -no-undefined
#libserver_la_LIBDADD =  #@DL_LIB@  # $(LIBLTDL)

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

