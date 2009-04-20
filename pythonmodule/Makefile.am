## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2008, 2009 Free Software Foundation, Inc.
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

if USE_PYTHON

pkglib_LTLIBRARIES = libgnashpython.la gnash.la

EXTRA_DIST = gnashPythonExample.py

BOOSTPYTHON_LIBS = -lboost_python  $(PYTHON_LIBS)
BOOSTPYTHON_CFLAGS = $(PYTHON_CFLAGS)

libgnashpython_la_SOURCES = \
	gnashpython.cpp \
	$(NULL)

libgnashpython_la_LDFLAGS = \
	$(GLIB_LIBS) \
	$(LIBLTDL) \
	$(FFMPEG_LIBS) \
	$(GSTREAMER_LIBS) \
	$(CURL_LIBS) \
	$(INCLTDL) \
	$(BOOST_LIBS) \
	$(PTHREAD_LIBS) \
	$(NULL)

libgnashpython_la_CPPFLAGS = \
        -I$(top_srcdir)/libcore \
        -I$(top_srcdir)/libcore/swf \
        -I$(top_srcdir)/libcore/parser \
        -I$(top_srcdir)/libcore/vm \
	-I$(top_srcdir)/libltdl \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/backend \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libmedia/ffmpeg \
	-I$(top_srcdir)/libmedia/gst \
	-DLOCALEDIR=\"$(localedir)\" \
	$(BOOSTPYTHON_CFLAGS) \
	$(LIRC_CFLAGS) \
	$(BOOST_CFLAGS) \
        $(PTHREAD_CFLAGS) \
	$(NULL)

gnash_la_SOURCES = \
	pyGnash.cpp
	$(NULL)

gnash_la_LDFLAGS = \
	$(GLIB_LIBS) \
	$(LIBLTDL) \
	$(BOOSTPYTHON_LIBS) \
	$(NULL)

gnash_la_LDFLAGS += -module -lgnashpython

gnash_la_CPPFLAGS = -I.. \
        -I$(top_srcdir) \
        -I$(top_srcdir)/libcore \
        -I$(top_srcdir)/libcore/swf \
        -I$(top_srcdir)/libcore/parser \
        -I$(top_srcdir)/libcore/vm \
	-I$(top_srcdir)/libltdl \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/backend \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libmedia/ffmpeg \
	-I$(top_srcdir)/libmedia/gst \
	-DLOCALEDIR=\"$(localedir)\" \
	-DBOOST_PYTHON_DYNAMIC_LIB \
	$(BOOSTPYTHON_CFLAGS) \
	$(LIRC_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(NULL)

libgnashpython_la_LIBADD = $(top_builddir)/libcore/libgnashcore.la \
	$(RENDERER_LIBS) \
	$(GNASH_LIBS)

noinst_HEADERS = \
	gnashpython.h

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.* $(CLEANFILES)

endif
