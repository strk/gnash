## Process this file with automake to generate Makefile.in
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
# 
# Linking Gnash statically or dynamically with other modules is making
# a combined work based on Gnash. Thus, the terms and conditions of
# the GNU General Public License cover the whole combination.
# 
# In addition, as a special exception, the copyright holders of Gnash give
# you permission to combine Gnash with free software programs or
# libraries that are released under the GNU LGPL and/or with Mozilla, 
# so long as the linking with Mozilla, or any variant of Mozilla, is
# through its standard plug-in interface. You may copy and distribute
# such a system following the terms of the GNU GPL for Gnash and the
# licenses of the other code concerned, provided that you include the
# source code of that other code when and as the GNU GPL requires
# distribution of source code. 
# 
# Note that people who make modified versions of Gnash are not obligated
# to grant this special exception for their modified versions; it is
# their choice whether to do so.  The GNU General Public License gives
# permission to release a modified version without this exception; this
# exception also makes it possible to release a modified version which
# carries forward this exception.
#

AUTOMAKE_OPTIONS = 

AM_CXXFLAGS =  $(CFLAGS) $(SDL_CFLAGS) $(INCLUDES)
AM_CPPFLAGS = #-Wall
# AM_LDFLAGS  = $(OPENGL_LIBNS) $(SDL_LIBS)

# noinst_LTLIBRARIES = libgeometry.la
lib_LTLIBRARIES = libgnashgeo.la
# noinst_LIBRARIES = libgeometry.a

# These headers get installed
# include_HEADERS = log.h err.h proc.h serial.h xantrex.h outback.h

INCLUDES = -I.. -I$(srcdir)	\
        -I$(top_srcdir) 	\
        -I$(top_srcdir)/libbase \
	$(PTHREAD_CFLAGS)	\
        $(SDL_CFLAGS) 		\
        $(OPENGL_CFLAGS) 	\
        $(ENGINE_INCLUDE)

libgnashgeo_la_SOURCES = 	\
	axial_box.cpp		\
	bsp.cpp			\
	collision.cpp		\
	cull.cpp		\
	geometry.cpp		\
	kd_tree_dynamic.cpp	\
	kd_tree_packed.cpp	\
	tqt.cpp

noinst_HEADERS = \
	axial_box.h	\
	bsp.h		\
	collision.h	\
	cull.h		\
	geometry.h	\
	kd_tree_dynamic.h	\
	kd_tree_packed.h	\
	tqt.h		\
	view_state.h	\
	visual.h

#libgeometry_la_LDFLAGS = -module -avoid-version -no-undefined
#libgeometry_la_LIBDADD =  #@DL_LIB@  # $(LIBLTDL)
libgnashgeo_la_LIBADD = -L../libbase -lgnashbase

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

