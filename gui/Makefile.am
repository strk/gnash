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

# $Id: Makefile.am,v 1.50 2006/11/28 12:20:16 nihilus Exp $

AUTOMAKE_OPTIONS = 

AM_CPPFLAGS = -I.. \
        -I$(top_srcdir) \
        -I$(top_srcdir)/server \
        -I$(top_srcdir)/server/parser \
        -I$(top_srcdir)/server/vm \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/backend \
        -I$(top_srcdir)/libgeometry \
        $(PTHREAD_CFLAGS) \
        $(SDL_CFLAGS) \
        $(DMALLOC_CFLAGS) \
	$(BOOST_CFLAGS)

#        $(GLIB_CFLAGS) \
#        $(PANGO_CFLAGS) \
#        $(ATK_CFLAGS) \
#        $(PNG_CFLAGS) \
#        $(JPEG_CFLAGS) \
#        $(MP3_CFLAGS) \
#        $(OGG_CFLAGS) \
#

## WARNING: make sure GLIB_LIBS appears first
## See: http://lists.gnu.org/archive/html/gnash-dev/2006-07/msg00076.html
AM_LDFLAGS =  \
	$(GLIB_LIBS) \
	$(CURL_LIBS) \
	$(PANGO_LIBS) \
	$(PTHREAD_LIBS) \
	$(LIBLTDL) \
	$(NULL)


if USE_RENDERER_OPENGL
 AM_CPPFLAGS += $(OPENGL_CFLAGS) 
if USE_GUI_GTK
  AM_LDFLAGS += $(GLEXT_LIBS) $(OPENGL_LIBS)
  AM_CPPFLAGS += $(GLEXT_CFLAGS)
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
 AM_LDFLAGS += $(CAIRO_LIBS) 
 GTK_CAIRO_SRCS = gtk_glue_cairo.cpp gtk_glue_cairo.h
 KDE_CAIRO_SRCS = #kde_glue_cairo.cpp kde_glue_cairo.h
 SDL_CAIRO_SRCS = sdl_cairo_glue.cpp sdl_cairo_glue.h
 AM_CPPFLAGS += $(CAIRO_CFLAGS)
else
 GTK_CAIRO_SRCS =
 KDE_CAIRO_SRCS =
 SDL_CAIRO_SRCS =
endif

if USE_RENDERER_AGG
  AM_CPPFLAGS += $(AGG_CFLAGS)
  AM_LDFLAGS += $(AGG_LIBS)
  GTK_AGG_SRCS = gtk_glue_agg.cpp gtk_glue_agg.h
  RISCOS_AGG_SRCS = ro_glue_agg.cpp ro_glue_agg.h
  SDL_AGG_SRCS = sdl_agg_glue.cpp sdl_agg_glue.h
else
  GTK_AGG_SRCS =
  RISCOS_AGG_SRCS =
  SDL_AGG_SRCS =
endif

if USE_GUI_GTK
  AM_LDFLAGS += $(CAIRO_LIBS)
 GTK_SRCS = gtk.cpp gtksup.h gtk_glue.h $(GTK_CAIRO_SRCS) $(GTK_OPENGL_SRCS) $(GTK_AGG_SRCS) 
 AM_LDFLAGS += $(GTK2_LIBS) 
 AM_CPPFLAGS += $(GTK2_CFLAGS)
else
 GTK_SRCS = 
endif

if USE_GUI_RISCOS
 RISCOS_SRCS = riscos.cpp riscossup.h riscos_glue.h $(RISCOS_AGG_SRCS)
 AM_LDFLAGS += -lOSLib32
else
 RISCS_SRCS =
endif

if USE_GUI_SDL
 SDL_SRCS = sdl.cpp sdlsup.h sdl_glue.h $(SDL_CAIRO_SRCS) $(SDL_OPENGL_SRCS) $(SDL_AGG_SRCS)
else
 SDL_SRCS =
endif

if USE_GUI_KDE
 KDE_SRCS = kde.cpp kdesup.h $(KDE_CAIRO_SRCS) $(KDE_OPENGL_SRCS)
 AM_CPPFLAGS += -I$(kde_includes)/kio $(all_includes) 
else
 KDE_SRCS = 
endif

if USE_GUI_FB
 FB_SRCS = fb.cpp fbsup.h 
 AM_CPPFLAGS += $(AGG_CFLAGS)
else
 FB_SRCS = 
endif

if USE_SOUND_SDL
if USE_FFMPEG_ENGINE
AM_LDFLAGS += $(FFMPEG_LIBS)
AM_CPPFLAGS += $(FFMPEG_CFLAGS)
endif
if USE_MAD_ENGINE
AM_LDFLAGS += $(MAD_LIBS)
AM_CPPFLAGS += $(MAD_CFLAGS)
endif
endif


#if USE_GUI_FLTK
# FLTK_SRCS = fltk.cpp fltksup.h
# AM_CPPFLAGS += $(FLTK_INCLS) 
#else
# FLTK_SRCS = 
#endif


AM_CPPFLAGS += -DPKGDATADIR=\"$(pkgdatadir)\"
AM_CPPFLAGS += -DRENDERER_CONFIG=\"$(RENDERER_CONFIG)\"
AM_CPPFLAGS += -DGUI_CONFIG=\"$(GUI_CONFIG)\"
AM_CPPFLAGS += -DSOUND_CONFIG=\"$(SOUND_CONFIG)\"
AM_CPPFLAGS += -DDECODER_CONFIG=\"$(DECODER_CONFIG)\"
AM_CPPFLAGS += -DTARGET_CONFIG=\"$(target)\"

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
	$(RISCOS_SRCS) \
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
	$(top_builddir)/libgeometry/libgnashgeo.la \
	$(top_builddir)/libbase/libgnashbase.la \
	libgnashplayer.la

libgnashplayer_la_SOURCES = \
	Player.cpp Player.h

libgnashplayer_la_LIBADD = \
	$(top_builddir)/server/libgnashserver.la \
	$(top_builddir)/backend/libgnashbackend.la \
	libgnashgui.la \
	$(AM_LDFLAGS) 

gnash_SOURCES = gnash.cpp 
# export our symbols so they can be used by Gnash plugins
gnash_LDFLAGS = -export-dynamic
gnash_DEPENDENCIES = $(GNASH_LIBS)
gnash_LDADD = \
	$(GNASH_LIBS)
#	$(LIBLTDL) "-dlopen" self "-dlopen"

klash_SOURCES = gnash.cpp
klash_CPPFLAGS =	\
	$(KDE_INCLS)	\
	-DUSE_KDE
# export our symbols so they can be used by Gnash plugins
klash_LDFLAGS = -export-dynamic
klash_LDADD = \
	$(LIB_KDECORE) \
	$(LIB_KDEUI) \
	$(GNASH_LIBS)

klash_DEPENDENCIES = \
	$(GNASH_LIBS) 
#	 $(LIBLTDL) "-dlopen" self "-dlopen"

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
libgnashplayer_la_LDFLAGS = -avoid-version
libgnashgui_la_LDFLAGS = -avoid-version

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

