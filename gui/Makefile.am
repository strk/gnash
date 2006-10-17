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

# $Id: Makefile.am,v 1.37 2006/10/17 13:50:39 bjacques Exp $

AUTOMAKE_OPTIONS = 

INCLUDES = -I.. \
        -I$(top_srcdir) \
        -I$(top_srcdir)/server \
        -I$(top_srcdir)/server/parser \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/backend \
        -I$(top_srcdir)/libgeometry \
        $(PTHREAD_CFLAGS) \
        $(SDL_CFLAGS) \
        $(PANGO_CFLAGS) \
        $(GLIB_CFLAGS) \
        $(ATK_CFLAGS) \
        $(DMALLOC_CFLAGS) \
        $(PNG_CFLAGS) \
        $(JPEG_CFLAGS) \
        $(MP3_CFLAGS) \
        $(OGG_CFLAGS) \
	$(BOOST_CFLAGS)

## WARNING: make sure GLIB_LIBS appears first
## See: http://lists.gnu.org/archive/html/gnash-dev/2006-07/msg00076.html
AM_LDFLAGS =  \
	$(GLIB_LIBS) \
	$(PANGO_LIBS) \
	$(PTHREAD_LIBS) \
	$(NULL)


if USE_RENDERER_OPENGL
 INCLUDES += $(OPENGL_CFLAGS) 
if USE_GUI_GTK
  AM_LDFLAGS += $(GLEXT_LIBS)
  INCLUDES += $(GLEXT_CFLAGS) 
  GTK_OPENGL_SRCS = gtk_glue_gtkglext.cpp gtk_glue_gtkglext.h
endif # USE_GUI_GTK
if USE_GUI_SDL
  SDL_OPENGL_SRCS = sdl_ogl_glue.cpp sdl_ogl_glue.h
endif # USE_GUI_SDL
if USE_GUI_KDE
  KDE_OPENGL_SRCS = kde_glue_opengl.cpp kde_glue_opengl.h
  AM_LDFLAGS += \
        $(KDE_LDFLAGS) \
	$(LIB_KDECORE) \
	$(LIB_KDEUI)
  BUILT_SOURCES = klash.moc
endif # USE_GUI_KDE
else # ! USE_RENDERER_OPENGL
 GTK_OPENGL_SRCS =
 KDE_OPENGL_SRCS =
 SDL_OPENGL_SRCS =
endif # ! USE_RENDERER_OPENGL

if USE_RENDERER_CAIRO
 INCLUDES += $(CAIRO_CFLAGS) 
 AM_LDFLAGS += $(CAIRO_LIBS) 
 GTK_CAIRO_SRCS = gtk_glue_cairo.cpp gtk_glue_cairo.h
 KDE_CAIRO_SRCS = #kde_glue_cairo.cpp kde_glue_cairo.h
 SDL_CAIRO_SRCS = sdl_cairo_glue.cpp sdl_cairo_glue.h
else
 GTK_CAIRO_SRCS =
 KDE_CAIRO_SRCS =
 SDL_CAIRO_SRCS =
endif

if USE_RENDERER_AGG
  INCLUDES += $(AGG_CFLAGS)
  AM_LDFLAGS += $(AGG_LIBS)
  GTK_AGG_SRCS = gtk_glue_agg.cpp gtk_glue_agg.h
  SDL_AGG_SRCS = sdl_agg_glue.cpp sdl_agg_glue.h
else
  GTK_AGG_SRCS =
  SDL_AGG_SRCS =
endif

if USE_GUI_GTK
 GTK_SRCS = gtk.cpp gtksup.h gtk_glue.h $(GTK_CAIRO_SRCS) $(GTK_OPENGL_SRCS) $(GTK_AGG_SRCS) 
 AM_LDFLAGS += $(GTK2_LIBS) 
 INCLUDES += $(GTK2_CFLAGS)
else
 GTK_SRCS = 
endif

if USE_GUI_SDL
 SDL_SRCS = sdl.cpp sdlsup.h sdl_glue.h $(SDL_CAIRO_SRCS) $(SDL_OPENGL_SRCS) $(SDL_AGG_SRCS)
else
 SDL_SRCS =
endif

if USE_GUI_KDE
 KDE_SRCS = kde.cpp kdesup.h $(KDE_CAIRO_SRCS) $(KDE_OPENGL_SRCS)
 INCLUDES += -I$(kde_includes)/kio $(all_includes) 
else
 KDE_SRCS = 
endif

if USE_GUI_FB
 FB_SRCS = fb.cpp fbsup.h 
 INCLUDES += $(AGG_CFLAGS)
else
 FB_SRCS = 
endif

if USE_FFMPEG_ENGINE
 INCLUDES += $(FFMPEG_INCLS) 
 AM_LDFLAGS += $(FFMPEG_LIBS)
endif

if USE_MAD_ENGINE
 INCLUDES += $(MAD_INCLS) 
 AM_LDFLAGS += $(MAD_LIBS)
endif

#if USE_GUI_FLTK
# FLTK_SRCS = fltk.cpp fltksup.h
# INCLUDES += $(FLTK_INCLS) 
#else
# FLTK_SRCS = 
#endif


AM_CPPFLAGS = $(INCLUDES) -DPKGDATADIR=\"$(pkgdatadir)\"

dist_images_DATA = images/gnash_128_96.ico
#dist_images_DATA = images/gnash_G_v3.ico
imagesdir = $(pkgdatadir)

lib_LTLIBRARIES = libgnashgui.la libgnashplayer.la
libgnashgui_la_SOURCES = \
	$(SDL_SRCS) \
	$(FLTK_SRCS) \
	$(GTK_SRCS) \
	$(GTK2_SRCS) \
	$(KDE_SRCS) \
	$(FB_SRCS) \
	gui.cpp gui.h \
	NullGui.cpp \
	NullGui.h

libgnashgui_la_LIBADD = \
	$(top_builddir)/server/libgnashserver.la \
	$(top_builddir)/backend/libgnashbackend.la \
	$(AM_LDFLAGS)

if USE_GUI_KDE
bin_PROGRAMS = gnash klash
else
bin_PROGRAMS = gnash
endif

GNASH_LIBS = \
        libgnashgui.la \
	$(top_builddir)/libamf/libgnashamf.la \
	$(top_builddir)/backend/libgnashbackend.la \
	$(top_builddir)/server/libgnashserver.la \
	$(top_builddir)/server/asobj/libgnashasobjs.la \
	$(top_builddir)/libgeometry/libgnashgeo.la \
	$(top_builddir)/libbase/libgnashbase.la

libgnashplayer_la_SOURCES = \
	Player.cpp Player.h

libgnashplayer_la_LIBADD = \
	$(top_builddir)/server/libgnashserver.la \
	$(top_builddir)/backend/libgnashbackend.la \
	libgnashgui.la \
	$(AM_LDFLAGS) 

gnash_SOURCES = gnash.cpp 
gnash_LDFLAGS = -module -avoid-version -no-undefined
gnash_LDADD = \
	$(GNASH_LIBS) \
	libgnashplayer.la

klash_SOURCES = gnash.cpp
klash_CPPFLAGS =	\
	$(KDE_INCLS)	\
	-DUSE_KDE
klash_LDFLAGS = -module -avoid-version -no-undefined
klash_LDADD = \
	$(LIB_KDECORE) \
	$(LIB_KDEUI) \
	$(GNASH_LIBS) \
	libgnashplayer.la

klash_DEPENDENCIES = libgnashgui.la

CLEANFILES = klash.moc
klash.moc: $(srcdir)/kdesup.h
	@if test x"$(MOC)" != x; then \
	  echo "Generating MOC file..."; \
	  $(MOC) $(srcdir)/kdesup.h -o klash.moc; \
	else  \
	  echo "WARNING: Install QT's moc tool! Linking to default MOC file"; \
	  ln -sf $(srcdir)/klash.moc.in klash.moc; \
	fi

#noinst_PROGRAMS = guitest # klash flak
#guitest_SOURCES = guitest.cpp #fltk.cpp #fltksup.h fltksupp.h gnash.cpp gtk.cpp 
#	gtksup.h gui.cpp gui.h menu.cpp menu.h sdl.cpp sdlsup.h [21~sdlsupp.h
#guitest_LDADD = $(gnash_LDADD)


# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

