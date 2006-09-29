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

# Linking Gnash statically or dynamically with other modules is making a
# combined work based on Gnash. Thus, the terms and conditions of the GNU
# General Public License cover the whole combination.
#
# As a special exception, the copyright holders of Gnash give you
# permission to combine Gnash with free software programs or libraries
# that are released under the GNU LGPL and with code included in any
# release of Talkback distributed by the Mozilla Foundation. You may
# copy and distribute such a system following the terms of the GNU GPL
# for all but the LGPL-covered parts and Talkback, and following the
# LGPL for the LGPL-covered parts.
#
# Note that people who make modified versions of Gnash are not obligated
# to grant this special exception for their modified versions; it is their
# choice whether to do so. The GNU General Public License gives permission
# to release a modified version without this exception; this exception
# also makes it possible to release a modified version which carries
# forward this exception.
# 
#

AUTOMAKE_OPTIONS = 

EXTRA_DIST = 

INCLUDES = -I.. -I$(srcdir) \
        -I$(top_srcdir)	\
        -I$(top_srcdir)/server	\
	$(PTHREAD_CFLAGS) \
        $(OPENGL_CFLAGS) \
        $(LIBXML_CFLAGS) \
        $(PNG_CFLAGS) \
        $(SDL_CFLAGS) \
	$(SDL_MIXER_CFLAGS) \
	$(CURL_CFLAGS) \
        $(ZLIB_CFLAGS) \
        $(JPEG_CFLAGS)

AM_CXXFLAGS = $(INCLUDES)
AM_CPPFLAGS = #-Wall

lib_LTLIBRARIES = libgnashbase.la

# These headers get installed
# include_HEADERS = log.h err.h proc.h serial.h xantrex.h outback.h

# Don't build Dmalloc support if it's not configured in
if DMALLOC
DMALLOC_FILE = dlmalloc.c
endif

if HAS_SDL
SDL_FILE = tu_file_SDL.cpp
else
EXTRA_DIST += tu_file_SDL.cpp
endif

libgnashbase_la_SOURCES = \
	config.cpp \
	container.cpp \
	$(DMALLOC_FILE)	\
	image.cpp \
	image_filters.cpp \
	jpeg.cpp \
	log.cpp	\
	membuf.cpp \
	network.cpp \
	postscript.cpp \
	rc.cpp \
	triangulate_float.cpp \
	triangulate_sint32.cpp \
	tu_file.cpp \
	$(SDL_FILE) \
	tu_random.cpp \
	tu_timer.cpp \
	tu_types.cpp \
	utf8.cpp \
	utility.cpp \
	curl_adapter.cpp \
	noseek_fd_adapter.cpp \
	zlib_adapter.cpp \
	Thread.cpp \
	URL.cpp

libgnashbase_la_LIBADD = \
	$(JPEG_LIBS) \
	$(ZLIB_LIBS) \
	$(CURL_LIBS) \
	$(SDL_LIBS) \
	$(OPENGL_LIBS)

noinst_HEADERS = \
	container.h \
	demo.h \
	dlmalloc.h \
	GnashException.h \
	grid_index.h \
	image.h	\
	jpeg.h \
	membuf.h \
	network.h \
	log.h \
	ogl.h \
	postscript.h \
	rc.h \
	smart_ptr.h \
	triangulate.h \
	triangulate_impl.h \
	tu_config.h \
	tu_file.h \
	tu_math.h \
	tu_opengl_includes.h \
	tu_random.h \
	tu_swap.h \
	tu_timer.h \
	tu_types.h \
	utf8.h \
	utility.h \
	curl_adapter.h \
	noseek_fd_adapter.h \
	zlib_adapter.h \
	Thread.h \
	URL.h

#libbase_la_LDFLAGS = -module -avoid-version -no-undefined
#libbase_la_LIBDADD =  #@DL_LIB@  # $(LIBLTDL)

# We use our own rule to build the demo so it isn't built by default.
# Since it doesn't contain a main() routine, we only try to compile,
# and not link it.
#noinst_PROGRAMS = demo
#demo_SOURCES = demo.cpp
#demo_LDADD = libgnashbase.la
demo$(EXEEXT): demo.o $(lib_LTLIBRARIES)
# 	@$(RM) demo$(EXEEXT)
# 	$(CXXLINK) $(AM_LDFLAGS) demo.o $(lib_LTLIBRARIES) $(LIBS)

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.* *.obj
