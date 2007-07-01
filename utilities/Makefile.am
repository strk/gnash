# 
#   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
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
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/backend/libgnashbackend.la \
	$(top_builddir)/libamf/libgnashamf.la

## WARNING: make sure GLIB_LIBS appears first
## See: http://lists.gnu.org/archive/html/gnash-dev/2006-07/msg00076.html
AM_LDFLAGS = \
	$(LIBADD_DL) \
	$(LIBLTDL) \
        $(MYSQL_LIBS) \
	$(LIBXML_LIBS) \
	$(FFMPEG_LIBS) \
	$(CURL_LIBS) \
	$(BOOST_LIBS) \
	$(PTHREAD_LIBS) \
	$(NULL)

localedir = $(datadir)/locale

#AM_CPPFLAGS = # -Wall
AM_CPPFLAGS = -I.. \
        -I$(top_srcdir)	\
        -I$(top_srcdir)/libgeometry \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/server \
        -I$(top_srcdir)/server/parser \
        -I$(top_srcdir)/server/vm \
        -DLOCALEDIR=\"$(localedir)\" \
	$(BOOST_CFLAGS) \
        $(LIBXML_CFLAGS) \
        $(MYSQL_CFLAGS) \
	$(NULL)

if USE_FFMPEG_ENGINE
 AM_CPPFLAGS += $(FFMPEG_INCLS) 
 GNASH_LIBS += $(FFMPEG_LIBS)
endif

if USE_MAD_ENGINE
 AM_CPPFLAGS += $(MAD_INCLS) 
 GNASH_LIBS += $(MAD_LIBS)
endif

#if USE_SOUND_GST
 GNASH_LIBS += $(GLIB_LIBS) $(GSTREAMER_LIBS)
#endif

bin_PROGRAMS = gparser gprocessor
noinst_PROGRAMS = dumpshm
#check_PROGRAMS = gdebug.swf

gparser_SOURCES = parser.cpp
# export our symbols so they can be used by Gnash plugins
gparser_LDFLAGS = -export-dynamic $(LIBLTDL)
gparser_LDADD = $(GNASH_LIBS) 
#gparser_DEPENDENCIES = 

gprocessor_SOURCES = processor.cpp
# export our symbols so they can be used by Gnash plugins
gprocessor_LDFLAGS = -export-dynamic $(LIBLTDL)
gprocessor_LDADD = $(GNASH_LIBS) 
#gprocessor_DEPENDENCIES = 

dumpshm_SOURCES = dumpshm.cpp
dumpshm_LDADD = $(GNASH_LIBS)
#gparser_LDFLAGS = $(AM_LDFLAGS)

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

#gdebug.swf: gdebug.as gdebcmds.as

# Wrap a single .as tests in dejagnu_so importer.
# At play time, if the relative url '../ming-misc.all/Dejagnu.swf' 
# takes to the Dejagnu.swf shared library we'll get visual traces,
# otherwise normal traces will be used.
SUFFIXES = as swf
.as.swf: 
	$(MAKESWF) $(DEF_MAKESWF_FLAGS)	$(MAKESWF_FLAGS) -o $@ $<
