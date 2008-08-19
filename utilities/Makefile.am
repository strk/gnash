# 
#   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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
	$(top_builddir)/libcore/libgnashcore.la \
	$(top_builddir)/libamf/libgnashamf.la \
	$(top_builddir)/libnet/libgnashnet.la \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/libmedia/libgnashmedia.la

## WARNING: make sure GLIB_LIBS appears first
## See: http://lists.gnu.org/archive/html/gnash-dev/2006-07/msg00076.html
AM_LDFLAGS = \
	$(LIBADD_DL) \
	$(LIBLTDL) \
    $(MYSQL_LIBS) \
	$(LIBXML_LIBS) \
	$(CURL_LIBS) \
	$(BOOST_LIBS) \
	$(PTHREAD_LIBS) \
	$(LIBINTL) \
	$(NULL)

localedir = $(datadir)/locale

#AM_CPPFLAGS = # -Wall
AM_CPPFLAGS = \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/libnet \
        -I$(top_srcdir)/libamf \
        -I$(top_srcdir)/libcore \
        -I$(top_srcdir)/libcore/asobj \
        -I$(top_srcdir)/libcore/parser \
        -I$(top_srcdir)/libcore/vm \
        -I$(top_srcdir)/libmedia \
        -DLOCALEDIR=\"$(localedir)\" \
		$(PTHREAD_CFLAGS) \
		$(BOOST_CFLAGS) \
        $(LIBXML_CFLAGS) \
        $(MYSQL_CFLAGS) \
		$(NULL)

if WIN32
 GNASH_LIBS += -lintl -lz -lws2_32
endif

if USE_FFMPEG_ENGINE
 AM_CPPFLAGS += $(FFMPEG_CFLAGS) \
		-I$(top_srcdir)/libmedia/ffmpeg 
 GNASH_LIBS += $(FFMPEG_LIBS)
endif

if USE_MAD_ENGINE
 AM_CPPFLAGS += $(MAD_INCLS) 
 GNASH_LIBS += $(MAD_LIBS)
endif

#if USE_SOUND_GST
 AM_CPPFLAGS += $(GSTREAMER_CFLAGS) \
		-I$(top_srcdir)/libmedia/gst
 GNASH_LIBS += $(GLIB_LIBS) $(GSTREAMER_LIBS)
#endif

bin_PROGRAMS = gprocessor soldumper dumpshm flvdumper rtmpget
#check_PROGRAMS = gdebug.swf

gprocessor_SOURCES = processor.cpp
gprocessor_CPPFLAGS = $(AM_CPPFLAGS)
# export our symbols so they can be used by Gnash plugins
gprocessor_LDFLAGS = -export-dynamic $(AM_LDFLAGS)
gprocessor_LDADD = $(GNASH_LIBS) 
#gprocessor_DEPENDENCIES = 

dumpshm_SOURCES = dumpshm.cpp
dumpshm_LDADD = $(GNASH_LIBS)

soldumper_SOURCES = soldumper.cpp
soldumper_LDADD = $(GNASH_LIBS)

flvdumper_SOURCES = flvdumper.cpp
flvdumper_LDADD = $(GNASH_LIBS)

rtmpget_SOURCES = rtmpget.cpp
rtmpget_LDADD = $(GNASH_LIBS)

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
