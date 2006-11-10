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

## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = # dejagnu

# @@ shouldn't we drop package_LIBS and leave deps
#    on the gnash libraries side instead ?

GNASH_LIBS = \
	$(top_builddir)/server/libgnashserver.la \
	$(top_builddir)/libbase/libgnashbase.la

AM_CPPFLAGS = # -Wall

## WARNING: make sure GLIB_LIBS appears first
## See: http://lists.gnu.org/archive/html/gnash-dev/2006-07/msg00076.html
AM_LDFLAGS = \
	$(CURL_LIBS) \
	$(LIBADD_DL) \
	$(LIBLTDL) \
	$(NULL)
#	$(GLIB_LIBS) \
#	$(PTHREAD_LIBS)

INCLUDES = -I.. \
        -I$(top_srcdir)	\
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/server \
        -I$(top_srcdir)/server/parser \
        $(LIBXML_CFLAGS)

if USE_FFMPEG_ENGINE
 INCLUDES += $(FFMPEG_INCLS) 
 AM_LDFLAGS += $(FFMPEG_LIBS)
endif

if USE_MAD_ENGINE
 INCLUDES += $(MAD_INCLS) 
 AM_LDFLAGS += $(MAD_LIBS)
endif

bin_PROGRAMS = gparser gprocessor
noinst_PROGRAMS = dumpshm

gparser_SOURCES = parser.cpp
gparser_LDADD = $(GNASH_LIBS) $(LIBLTDL) "-dlopen" self "-dlopen"
gparser_DEPENDENCIES = 
# export our symbols so they can be used by Gnash plugins
gparser_LDFLAGS = -export-dynamic

gprocessor_SOURCES = processor.cpp
gprocessor_LDADD = $(GNASH_LIBS) $(LIBLTDL) "-dlopen" self "-dlopen"
gprocessor_DEPENDENCIES = 
# export our symbols so they can be used by Gnash plugins
gprocessor_LDFLAGS = -export-dynamic

dumpshm_SOURCES = dumpshm.cpp
dumpshm_LDADD = $(GNASH_LIBS)
#gparser_LDFLAGS = $(AM_LDFLAGS)

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*
