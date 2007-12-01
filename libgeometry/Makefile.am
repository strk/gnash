## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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

# 
#

# $Id: Makefile.am,v 1.33 2007/12/01 22:11:24 strk Exp $

AUTOMAKE_OPTIONS = 

# AM_CXXFLAGS =  $(CXXFLAGS) $(SDL_CFLAGS) $(AM_CPPFLAGS)
# AM_CPPFLAGS = #-Wall
# AM_LDFLAGS  = $(OPENGL_LIBNS) $(SDL_LIBS)

# noinst_LTLIBRARIES = libgeometry.la
pkglib_LTLIBRARIES = libgnashgeo.la
# noinst_LIBRARIES = libgeometry.a

# These headers get installed
# include_HEADERS = log.h err.h proc.h serial.h xantrex.h outback.h

AM_CPPFLAGS = -I.. -I$(srcdir)	\
        -I$(top_srcdir)/libbase \
	$(BOOST_CFLAGS)		\
	$(PTHREAD_CFLAGS)	\
        $(NULL)

#        -I$(top_srcdir) 	\
#	$(PTHREAD_CFLAGS)	\
#        $(SDL_CFLAGS) 		\
#        $(OPENGL_CFLAGS) 	\
#

libgnashgeo_la_SOURCES = 	\
	axial_box.cpp		\
	bsp.cpp			\
	collision.cpp		\
	cull.cpp		\
	geometry.cpp		\
	tqt.cpp

noinst_HEADERS = \
	Range2d.h	\
	Point2d.h	\
	snappingrange.h \
	axial_box.h	\
	bsp.h		\
	collision.h	\
	cull.h		\
	geometry.h	\
	tqt.h		\
	view_state.h	\
	visual.h

#libgeometry_la_LIBDADD =  #@DL_LIB@  # $(LIBLTDL)
libgnashgeo_la_LIBADD = $(top_builddir)/libbase/libgnashbase.la
libgnashgeo_la_LDFLAGS = -release $(VERSION)


# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

