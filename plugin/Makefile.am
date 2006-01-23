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

AUTOMAKE_OPTIONS = dejagnu

SUBDIRS = mozilla-sdk # test

if MP3
MP3_CFLAGS = $(MAD_CFLAGS)
MP3_LIBS = $(MAD_LIBS)
endif

plugindir   = $(FIREFOX_PLUGINS)

INCLUDES = -I.. \
        -I$(srcdir)		\
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/libgeometry \
        -I$(top_srcdir)/server	\
	-I$(srcdir)/mozilla-sdk \
	$(LIBXML_CFLAGS)	\
	$(FIREFOX_CFLAGS)	\
	$(SDL_CFLAGS)		\
	$(SDL_MIXER_CFLAGS)	\
	$(X_CFLAGS)		\
	$(MP3_CFLAGS)		\
	$(OGG_CFLAGS)		\
	$(VORBIS_CFLAGS)


AM_CXXFLAGS = $(INCLUDES)
# XP_UNIX	This needs to get defined for npapi.h on unix platforms.
# PLUGIN_TRACE	Enable this define to get debug prints whenever the plugin
#		api gets control.
AM_CPPFLAGS = -Wall -DPLUGIN_TRACE -DXP_UNIX $(FIREFOX_DEFS)
AM_LDFLAGS  = $(FIREFOX_LIBS) \
	$(X_LIBS) -lX11 -lXi -lXmu \
	$(SDL_LIBS)		\
	$(SDL_MIXER_LIBS)	\
	$(OPENGL_LIBS)		\
	$(LIBXML_LIBS)		\
	$(JPEG_LIBS)		\
	$(PNG_LIBS)		\
	$(MP3_LIBS)		\
	$(OGG_LIBS)		\
        $(VORBIS_LIBS)


plugin_LTLIBRARIES = libgnash.la

noinst_HEADERS = plugin.h

libgnash_la_SOURCES  = plugin.cpp
libgnash_la_LIBADD   = 	$(AM_LDFLAGS) \
	mozilla-sdk/libmozsdk.la \
	../libbase/libbase.la \
	../libgeometry/libgeometry.la \
	../server/libserver.la \
	../backend/libgbackend.la

libgnash_la_DEPENDENCIES   = mozilla-sdk/libmozsdk.la

install-pluginLTLIBRARIES: $(plugin_LIBRARIES)
	cp .libs/libgnash.so $(plugindir)/libgnash.so

uninstall-pluginLTLIBRARIES: $(plugin_LIBRARIES)
	rm -f $(plugindir)/libgnash.so
