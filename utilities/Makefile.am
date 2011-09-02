# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
#   2011 Free Software Foundation, Inc.
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
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/libmedia/libgnashmedia.la \
	$(top_builddir)/libsound/libgnashsound.la \
	$(top_builddir)/librender/libgnashrender.la \
	$(NULL)

AM_LDFLAGS = \
	$(LIBINTL) \
	$(LIBADD_DL) \
	$(PTHREAD_LIBS) \
	$(NULL)

localedir = $(datadir)/locale

AM_CPPFLAGS = \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/libdevice \
        -I$(top_srcdir)/librender \
	-I$(top_srcdir)/librender/agg \
	-I$(top_srcdir)/librender/cairo \
	-I$(top_srcdir)/librender/opengl \
        -I$(top_srcdir)/libcore \
        -I$(top_srcdir)/libcore/asobj \
        -I$(top_srcdir)/libcore/swf \
        -I$(top_srcdir)/libcore/parser \
        -I$(top_srcdir)/libcore/vm \
        -I$(top_srcdir)/libmedia \
        -I$(top_srcdir)/libsound \
        -DLOCALEDIR=\"$(localedir)\" \
	$(GSTREAMER_CFLAGS) \
	$(GSTREAMER_PLUGINS_BASE_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(PTHREAD_CFLAGS) \
	$(NULL)

if BUILD_EGL_DEVICE
AM_CPPFLAGS += $(EGL_CFLAGS)
endif
if BUILD_DIRECTFB_DEVICE
AM_CPPFLAGS += $(DIRECTFB_CFLAGS)
endif

if WIN32
  # bzip2 is a dependency of some libraries with mingw32
  AM_LDFLAGS += $(BZ2_LIBS)
  GNASH_LIBS += -lintl -lz -lws2_32
endif

bin_PROGRAMS = gprocessor rtmpget

if CYGNAL
AM_CPPFLAGS += \
	-I$(top_srcdir)/cygnal/libamf \
	-I$(top_srcdir)/cygnal/libnet
bin_PROGRAMS += soldumper flvdumper

CYGNAL_LIBS = \
	$(top_builddir)/cygnal/libamf/libgnashamf.la \
	$(top_builddir)/cygnal/libnet/libgnashnet.la \
	$(BOOST_CYGNAL_LIBS) \
	$(NULL)

soldumper_SOURCES = soldumper.cpp
soldumper_LDADD = $(GNASH_LIBS) $(CYGNAL_LIBS) $(AM_LDFLAGS)

flvdumper_SOURCES = flvdumper.cpp
flvdumper_LDADD = $(GNASH_LIBS) $(CYGNAL_LIBS) $(AM_LDFLAGS)

endif

#check_PROGRAMS = gdebug.swf

if USE_GST_ENGINE
 # seems kindof silly to build these for an embedded target
if !CROSS_COMPILING
   bin_PROGRAMS += findwebcams
   bin_PROGRAMS += findmicrophones
endif
endif

gprocessor_SOURCES = processor.cpp
# export our symbols so they can be used by Gnash plugins
gprocessor_LDFLAGS = -export-dynamic
gprocessor_LDADD = $(GNASH_LIBS) $(AM_LDFLAGS)

rtmpget_SOURCES = rtmpget.cpp 
rtmpget_LDADD = $(top_builddir)/libbase/libgnashbase.la $(AM_LDFLAGS)

#dumpshm_SOURCES = dumpshm.cpp
#dumpshm_LDADD = $(GNASH_LIBS) $(AM_LDFLAGS)

if USE_GST_ENGINE
 GST_STUFF = \
	$(GSTREAMER_PBUTILS_LIBS) \
	$(GSTREAMER_PLUGINS_BASE_LIBS) \
	$(GSTREAMER_LIBS) \
	$(GSTAPP_CFLAGS) \
	$(NULL)

 findwebcams_SOURCES = findwebcams.cpp
 findwebcams_LDADD = $(GST_STUFF) $(top_builddir)/libbase/libgnashbase.la \
                     $(AM_LDFLAGS)

 findmicrophones_SOURCES = findmicrophones.cpp
 findmicrophones_LDADD = $(GST_STUFF) $(top_builddir)/libbase/libgnashbase.la \
                         $(AM_LDFLAGS)
endif

# Wrap a single .as tests in dejagnu_so importer.
# At play time, if the relative url '../ming-misc.all/Dejagnu.swf' 
# takes to the Dejagnu.swf shared library we'll get visual traces,
# otherwise normal traces will be used.
SUFFIXES = as swf
.as.swf: 
	$(MAKESWF) $(DEF_MAKESWF_FLAGS)	$(MAKESWF_FLAGS) -o $@ $<
