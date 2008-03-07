# 
#   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = 

# this is where Gnash plugins get installed
pluginsdir = $(prefix)/lib/gnash/plugins

if INSTALL_LTDL
include_HEADERS =  $(top_srcdir)/libltdl/ltdl.h
LIBLTDLLIB = libltdl.la
LIBLTDLHEAD =  $(top_srcdir)/libltdl/ltdl.h
endif

# If we are using an installable libltdl, then it needs to be built
# before libgnash, as libgnash depends on it for extensions.
pkglib_LTLIBRARIES =  $(LIBLTDLLIB) libgnashbase.la

libgnashbase_la_CPPFLAGS = -DPLUGINSDIR=\"$(pluginsdir)\" \
	-I.. -I$(srcdir) \
	-I$(top_srcdir)	\
	-I$(top_srcdir)/libamf	\
	-I$(top_srcdir)/server	\
	-I$(top_srcdir)/server/swf	\
	$(PTHREAD_CFLAGS) \
	$(LIBXML_CFLAGS) \
	$(PNG_CFLAGS) \
	$(SDL_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GSTREAMER_CFLAGS) \
	$(CURL_CFLAGS) \
	$(Z_CFLAGS) \
	$(JPEG_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(INCLTDL) \
	$(NULL)

# These headers get installed
# include_HEADERS = log.h err.h proc.h serial.h xantrex.h outback.h

# Don't build Dmalloc support if it's not configured in
if HAVE_DMALLOC
DMALLOC_FILE = dlmalloc.c
endif

libgnashbase_la_LIBADD = \
	$(JPEG_LIBS) \
	$(Z_LIBS) \
	$(CURL_LIBS) \
	$(OPENGL_LIBS) \
	$(LIBINTL) \
	$(LIBLTDL) \
	$(BOOST_LIBS) \
	$(PTHREAD_LIBS) \
	$(NULL)

if HAVE_SDL
SDL_FILE = tu_file_SDL.cpp
libgnashbase_la_LIBADD += $(SDL_LIBS)
else
EXTRA_DIST += tu_file_SDL.cpp 
endif

libgnashbase_la_SOURCES = \
	config.cpp \
	$(DMALLOC_FILE)	\
	extension.cpp \
	image.cpp \
	jpeg.cpp \
	lirc.cpp	\
	log.cpp	\
	membuf.cpp \
	memory.cpp \
	network.cpp \
	postscript.cpp \
	rc.cpp \
	shm.cpp \
	sharedlib.cpp \
	string_table.cpp \
	tu_file.cpp \
	$(SDL_FILE) \
	tu_timer.cpp \
	WallClockTimer.cpp \
	utf8.cpp \
	utility.cpp \
	curl_adapter.cpp \
	noseek_fd_adapter.cpp \
	zlib_adapter.cpp \
	URL.cpp \
	LoadThread.cpp \
	FLVParser.cpp \
	GC.cpp \
	BitsReader.cpp \
	$(NULL)

noinst_HEADERS = \
	$(LIBLTDLHEAD) \
	container.h \
	dlmalloc.h \
	extension.h \
	GnashException.h \
	gettext.h \
	grid_index.h \
	image.h	\
	jpeg.h \
	membuf.h \
	memory.h \
	network.h \
	lirc.h \
	log.h \
	ogl.h \
	postscript.h \
	rc.h \
	ref_counted.h \
	shm.h \
	smart_ptr.h \
	sharedlib.h \
	string_table.h \
	tree.hh \
	tu_config.h \
	tu_file.h \
	tu_math.h \
	tu_opengl_includes.h \
	tu_timer.h \
	WallClockTimer.h \
	utf8.h \
	utility.h \
	curl_adapter.h \
	noseek_fd_adapter.h \
	zlib_adapter.h \
	URL.h \
	LoadThread.h \
	FLVParser.h \
	GC.h \
	BitsReader.h \
	$(NULL)

libgnashbase_la_LDFLAGS = -release $(VERSION)

libltdlc_la_CPPFLAGS = $(LTDLINCL)
if CONVENIENCE_LTDL
noinst_LTLIBRARIES = libltdlc.la
libltdlc_la_SOURCES = $(top_srcdir)/libltdl/ltdl.c
endif
CLEANFILES = libltdl.la libltdlc.la

libltdl_la_SOURCES = $(top_srcdir)/libltdl/ltdl.c
libltdl_la_LDFLAGS = -no-undefined -version-info 4:4:1
libltdl_la_LIBADD = $(LIBADD_DL)

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.* *.obj
