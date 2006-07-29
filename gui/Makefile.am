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
#

AUTOMAKE_OPTIONS =

if USE_RENDERER_OPENGL
if USE_GUI_GTK
GTK_OPENGL_SRCS = gtk_glue_gtkglext.cpp gtk_glue_gtkglext.h
endif # USE_GUI_GTK
if USE_GUI_KDE
KDE_OPENGL_SRCS = kde_glue_opengl.cpp kde_glue_opengl.h
endif # USE_GUI_KDE
else # ! USE_RENDERER_OPENGL
GTK_OPENGL_SRCS =
KDE_OPENGL_SRCS =
endif # ! USE_RENDERER_OPENGL

if USE_RENDERER_CAIRO
GTK_CAIRO_SRCS = gtk_glue_cairo.cpp gtk_glue_cairo.h
KDE_CAIRO_SRCS = kde_glue_cairo.cpp kde_glue_cairo.h
else
GTK_CAIRO_SRCS =
KDE_CAIRO_SRCS =
endif

if USE_GUI_GTK
GTK_SRCS = gtk.cpp gtksup.h $(GTK_CAIRO_SRCS) $(GTK_OPENGL_SRCS)
else
GTK_SRCS = 
endif

if USE_GUI_SDL
SDL_SRCS = sdl.cpp sdlsup.h # glow for SDL widgets
else
SDL_SRCS =
endif

if USE_GUI_KDE
KDE_SRCS = kde.cpp kdesup.h $(KDE_CAIRO_SRCS) $(KDE_OPENGL_SRCS)
else
KDE_SRCS = 
endif

#if USE_GUI_FLTK
#FLTK_SRCS = fltk.cpp fltksup.h
#else
#FLTK_SRCS = 
#endif

INCLUDES = -I.. \
	-I$(kde_includes)/kio $(all_includes) \
        -I$(top_srcdir) \
        -I$(top_srcdir)/server \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/backend \
        -I$(top_srcdir)/libgeometry \
        $(PTHREAD_CFLAGS) \
        $(SDL_CFLAGS) \
        $(SDL_MIXER_CFLAGS) \
        $(LIBXML_CFLAGS) \
        $(OPENGL_CFLAGS) \
        $(GLEXT_CFLAGS) \
        $(GTK2_CFLAGS) \
        $(PANGO_CFLAGS) \
        $(GLIB_CFLAGS) \
	$(GTK2_INCLS) \
	$(FLTK_INCLS) \
	$(KDE_INCLS) \
        $(CAIRO_CFLAGS) \
        $(ATK_CFLAGS) \
        $(DMALLOC_CFLAGS) \
        $(PNG_CFLAGS) \
        $(JPEG_CFLAGS) \
        $(MP3_CFLAGS) \
        $(OGG_CFLAGS)

AM_CPPFLAGS = $(INCLUDES)

#AM_LDFLAGS =  \
#        ../libbase/libgnashbase.la \
#	$(JPEG_LIBS) \
#	$(PNG_LIBS) \
#        $(OGG_LIBS) \
#	$(ZLIB_LIBS) \
#        $(LIBXML_LIBS) \
#        $(SDL_LIBS) \
#        $(SDL_MIXER_LIBS) \
#        $(OPENGL_LIBS) \
#	$(GLEXT_LIBS) \
#	$(GTK2_LIBS) \
#	$(FLTK_LIBS) \
#	$(XFT_LIBS) \
#	$(PANGO_LIBS) \
#	$(CAIRO_LIBS) \
#	$(ATK_LIBS) \
#	$(GLIB_LIBS) \
#	$(X_LIBS) \
#	$(DMALLOC_LIBS)	\
#	$(MP3_LIBS) \
#	$(PTHREAD_LIBS) \
#	$(KDE_LDFLAGS) \
#	$(all_libraries) \
#	$(KDE_RPATH)

lib_LTLIBRARIES = libgnashgui.la
libgnashgui_la_SOURCES = \
	$(SDL_SRCS) \
	$(FLTK_SRCS) \
	$(GTK_SRCS) \
	$(KDE_SRCS) \
	gui.cpp gui.h

libgnashgui_la_LIBADD = ../server/libgnashserver.la

if USE_GUI_KDE
bin_PROGRAMS = gnash klash
else
bin_PROGRAMS = gnash
endif

gnash_SOURCES = gnash.cpp # $(libgnashgui_la_SOURCES)
gnash_CPPFLAGS = -DUSE_GTK
gnash_LDFLAGS = -module -avoid-version -no-undefined
gnash_LDADD = \
        libgnashgui.la \
	../backend/libgnashbackend.la \
	../server/libgnashserver.la \
	../server/asobj/libgnashasobjs.la \
	../libgeometry/libgnashgeo.la \
	../libbase/libgnashbase.la

klash_SOURCES = gnash.cpp
klash_CPPFLAGS = -DUSE_KDE
klash_LDFLAGS = -module -avoid-version -no-undefined
klash_LDADD = \
	libgnashgui.la \
	$(LIB_KDECORE) \
	$(LIB_KDEUI) \
	../backend/libgnashbackend.la \
	../server/libgnashserver.la \
	../server/asobj/libgnashasobjs.la \
	../libgeometry/libgnashgeo.la \
	../libbase/libgnashbase.la
klash_DEPENDANCIES = klash.moc libgnashgui.la

CLEANFILES = klash.moc
klash.moc: $(srcdir)/kdesup.h
	@if test x"$(MOC)" != x; then \
	  echo "Generating MOC file..."; \
	  $(MOC) $(srcdir)/kdesup.h -o klash.moc; \
	else  \
	  echo "WARNING: Install QT's moc tool! Linking to default MOC file"; \
	  ln -s $(srcdir)/klash.moc.in klash.moc; \
	fi

#noinst_PROGRAMS = guitest # klash flak
#guitest_SOURCES = guitest.cpp #fltk.cpp #fltksup.h fltksupp.h gnash.cpp gtk.cpp 
#	gtksup.h gui.cpp gui.h menu.cpp menu.h sdl.cpp sdlsup.h sdlsupp.h
#guitest_LDADD = $(gnash_LDADD)


# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

