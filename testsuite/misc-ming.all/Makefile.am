## Process this fill with automake to generate Makefile.in
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

AUTOMAKE_OPTIONS = dejagnu

# We don't need  --tool anymore
RUNTESTDEFAULTFLAGS = --srcdir $$srcdir

if ENABLE_MING

INCLUDES = -I.. \
        -I$(top_srcdir)		\
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/server  \
        -I$(top_srcdir)/libgeometry \
        $(MING_CFLAGS)


noinst_PROGRAMS = \
	moviecliploader_test \
	definebitsjpeg2 \
	spritehier

definebitsjpeg2_SOURCES = definebitsjpeg2.c
definebitsjpeg2_LDADD = $(MING_LIBS)

moviecliploader_test_SOURCES =  moviecliploader_test.c
moviecliploader_test_LDADD = $(MING_LIBS)

spritehier_SOURCES =  spritehier.c
spritehier_LDADD = $(MING_LIBS)

moviecliploader_test.swf: moviecliploader_test
	./moviecliploader_test $(top_srcdir)/testsuite/media

definebitsjpeg2.swf: definebitsjpeg2
	./definebitsjpeg2 $(srcdir)/../media/lynch.jpg

spritehier.swf: spritehier
	./spritehier

endif


