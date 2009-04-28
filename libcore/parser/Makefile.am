## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

noinst_LTLIBRARIES = libgnashparser.la 

# Only enable if we're configured with --enable-mp3
#
# The 'backend' dir should not be required if we finally drop
# rendering calls from the parser (see https://savannah.gnu.org/task/?5837)
#
AM_CPPFLAGS = \
	-I$(top_srcdir)/libnet \
	-I$(top_srcdir)/backend \
	-I$(top_srcdir)/libcore \
	-I$(top_srcdir)/libcore/swf \
	-I$(top_srcdir)/libcore/parser \
	-I$(top_srcdir)/libcore/vm \
	-I$(top_srcdir)/libcore/asobj \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libsound \
	-I$(top_srcdir)/libamf \
	$(GLIB_CFLAGS) \
	$(PTHREAD_CFLAGS) \
        $(BOOST_CFLAGS) \
	$(FREETYPE2_CFLAGS) \
	$(NULL)

libgnashparser_la_SOURCES = \
	abc_block.cpp \
	action_buffer.cpp \
	BitmapMovieDefinition.cpp \
	SWFMovieDefinition.cpp \
	sound_definition.cpp \
	sprite_definition.cpp

noinst_HEADERS = \
	abc_block.h \
	action_buffer.h \
	BitmapMovieDefinition.h \
	movie_definition.h \
	SWFMovieDefinition.h \
	Namespace.h \
	sound_definition.h \
	sprite_definition.h

EXTENSIONS_API = \
	movie_definition.h \
	$(NULL)

if SDKINSTALL
noinst_HEADERS += $(EXTENSIONS_API)
else
instdir = $(includedir)/gnash
inst_HEADERS = $(EXTENSIONS_API)
endif

libgnashparser_la_LIBADD = \
	$(top_builddir)/libamf/libgnashamf.la \
	$(top_builddir)/libbase/libgnashbase.la

## $(top_builddir)/libmedia/libgnashmedia.la

libgnashparser_la_LDFLAGS = $(BOOST_LIBS) -export-dynamic # -release $(VERSION)

if WIN32
  libgnashparser_la_LDFLAGS += -no-undefined 
endif

if USE_GST_ENGINE
AM_CPPFLAGS += $(GSTREAMER_CFLAGS) \
		-I$(top_srcdir)/libmedia/gst
endif

if USE_FFMPEG_ENGINE
AM_CPPFLAGS += $(FFMPEG_CFLAGS) \
		-I$(top_srcdir)/libmedia/ffmpeg
endif

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

