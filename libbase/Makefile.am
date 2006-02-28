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

AM_CXXFLAGS = $(SDL_CFLAGS) $(SDL_MIXER_CFLAGS) $(INCLUDES)
AM_CPPFLAGS = -Wall
# AM_LDFLAGS = $(OPENGL_LIBS) $(GLU_LIBS) $(SDL_LIBS)

# noinst_LTLIBRARIES = libbase.la
lib_LTLIBRARIES = libgnashbase.la
# noinst_LIBRARIES = libbase.a

# These headers get installed
# include_HEADERS = log.h err.h proc.h serial.h xantrex.h outback.h

# Don't build Dmalloc support if it's not configured in
if DMALLOC
DMALLOC_FILE = dlmalloc.c
endif

INCLUDES = -I.. -I$(srcdir)	\
	$(PTHREAD_CFLAGS)	\
        $(PNG_CFLAGS)		\
        $(ZLIB_CFLAGS)		\
        $(JPEG_CFLAGS)		\
        $(LIBXML_CFLAGS)	\
	$(OPENGL_CFLAGS)	\
        -I$(top_srcdir)

libgnashbase_la_SOURCES = \
	config.cpp		\
	container.cpp		\
	demo.cpp		\
	$(DMALLOC_FILE)		\
	image.cpp		\
	image_filters.cpp	\
	jpeg.cpp		\
	membuf.cpp		\
	ogl.cpp			\
	postscript.cpp		\
	triangulate_float.cpp	\
	triangulate_sint32.cpp	\
	tu_file.cpp		\
	tu_file_SDL.cpp		\
	tu_random.cpp		\
	tu_timer.cpp		\
	tu_types.cpp		\
	utf8.cpp		\
	utility.cpp		\
	zlib_adapter.cpp

noinst_HEADERS = \
	config.h		\
	container.h		\
	demo.h			\
	dlmalloc.h		\
	grid_index.h		\
	image.h			\
	jpeg.h			\
	membuf.h		\
	ogl.h			\
	postscript.h		\
	smart_ptr.h		\
	triangulate.h		\
	triangulate_impl.h	\
	tu_config.h		\
	tu_file.h		\
	tu_math.h		\
	tu_opengl_includes.h	\
	tu_random.h		\
	tu_swap.h		\
	tu_timer.h		\
	tu_types.h		\
	utf8.h			\
	utility.h		\
	zlib_adapter.h

#libbase_la_LDFLAGS = -module -avoid-version -no-undefined
#libbase_la_LIBDADD =  #@DL_LIB@  # $(LIBLTDL)

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

