## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

# $Id: Makefile.am,v 1.82 2007/07/17 23:16:06 strk Exp $

AUTOMAKE_OPTIONS = 
localedir = $(datadir)/locale

AM_CPPFLAGS = -I.. \
        -I$(top_srcdir) \
        -I$(top_srcdir)/server \
        -I$(top_srcdir)/server/parser \
        -I$(top_srcdir)/server/vm \
	-I$(top_srcdir)/libltdl \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/backend \
        -I$(top_srcdir)/libgeometry \
        -DLOCALEDIR=\"$(localedir)\" \
        $(LIBXML_CFLAGS) \
        $(SDL_CFLAGS) \
        $(DMALLOC_CFLAGS) \
	$(BOOST_CFLAGS) \
        $(PTHREAD_CFLAGS) \
	$(NULL)

GTK_CFLAGS = \
	$(GTK2_CFLAGS) \
	$(GLEXT_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(PANGO_CFLAGS) \
	$(ATK_CFLAGS) \
	$(CAIRO_CFLAGS)

GTK_LIBS =  \
	$(GTK2_LIBS) \
	$(GLEXT_LIBS) \
	$(CAIRO_LIBS) \
	$(PANGO_LIBS) \
	$(ATK_LIBS) \
	$(GLIB_LIBS) \
	$(EXPAT_LIBS)
#        $(PNG_CFLAGS) \
#        $(JPEG_CFLAGS) \
#        $(MP3_CFLAGS) \
#        $(OGG_CFLAGS)

## WARNING: make sure GLIB_LIBS appears first
## See: http://lists.gnu.org/archive/html/gnash-dev/2006-07/msg00076.html
AM_LDFLAGS =  \
	$(LIBLTDL) \
	$(FFMPEG_LIBS) \
	$(CURL_LIBS) \
	$(LIBXML_LIBS) \
	$(INCLTDL) \
	$(BOOST_LIBS) \
	$(PTHREAD_LIBS) \
	$(NULL)


if USE_RENDERER_OPENGL
 AM_CPPFLAGS += $(OPENGL_CFLAGS) 
if USE_GUI_GTK
  AM_LDFLAGS += $(GLEXT_LIBS) $(OPENGL_LIBS)
  AM_CPPFLAGS += $(GLEXT_CFLAGS)
  GTK_OPENGL_SRCS = gtk_glue_gtkglext.cpp gtk_glue_gtkglext.h
endif # USE_GUI_GTK
if USE_GUI_AQUA
  AQUA_OPENGL_SRCS = aqua_ogl_glue.cpp aqua_ogl_glue.h
  AM_LDFLAGS += $(OPENGL_LIBS)
endif # USE_GUI_AQUA
if USE_GUI_SDL
  SDL_OPENGL_SRCS = sdl_ogl_glue.cpp sdl_ogl_glue.h
endif # USE_GUI_SDL
if USE_GUI_KDE
  KDE_OPENGL_SRCS = kde_glue_opengl.cpp kde_glue_opengl.h
  AM_CPPFLAGS += $(KDE_CFLAGS) $(QT_CFLAGS)
  AM_LDFLAGS +=  $(KDE_LIBS) $(QT_LIBS)
  BUILT_SOURCES = klash.moc
endif # USE_GUI_KDE
else # ! USE_RENDERER_OPENGL
 GTK_OPENGL_SRCS =
 KDE_OPENGL_SRCS =
 SDL_OPENGL_SRCS =
 AQUA_OPENGL_SRCS =
endif # ! USE_RENDERER_OPENGL

if USE_RENDERER_CAIRO
 AM_LDFLAGS += $(CAIRO_LIBS) 
 GTK_CAIRO_SRCS = gtk_glue_cairo.cpp gtk_glue_cairo.h gtk_cairo_create.h
 KDE_CAIRO_SRCS = #kde_glue_cairo.cpp kde_glue_cairo.h
 SDL_CAIRO_SRCS = sdl_cairo_glue.cpp sdl_cairo_glue.h
 FLTK_CAIRO_SRCS = fltk_glue_cairo.cpp fltk_glue_cairo.h
 AM_CPPFLAGS += $(CAIRO_CFLAGS)
else
 GTK_CAIRO_SRCS =
 KDE_CAIRO_SRCS =
 SDL_CAIRO_SRCS =
 FLTK_CAIRO_SRCS =
endif

if USE_RENDERER_AGG
  AM_CPPFLAGS += $(AGG_CFLAGS)
  AM_LDFLAGS += $(AGG_LIBS)
  GTK_AGG_SRCS = gtk_glue_agg.cpp gtk_glue_agg.h
  RISCOS_AGG_SRCS = riscos_glue_agg.cpp riscos_glue_agg.h
  SDL_AGG_SRCS = sdl_agg_glue.cpp sdl_agg_glue.h
  FLTK_AGG_SRCS = fltk_glue_agg.cpp fltk_glue_agg.h
  KDE_AGG_SRCS = kde_glue_agg.cpp kde_glue_agg.h
else
  GTK_AGG_SRCS =
  RISCOS_AGG_SRCS =
  SDL_AGG_SRCS =
  FLTK_AGG_SRCS =
  KDE_AGG_SRCS =
endif

if USE_GUI_GTK
  AM_LDFLAGS += $(CAIRO_LIBS)
  GTK_SRCS = gtk.cpp gtksup.h gtk_glue.h $(GTK_CAIRO_SRCS) $(GTK_OPENGL_SRCS) $(GTK_AGG_SRCS) 
  AM_LDFLAGS += $(GTK_LIBS) $(X11_LIBS)
  AM_CPPFLAGS += $(GTK_CFLAGS)
else
 GTK_SRCS = 
endif

if USE_GUI_RISCOS
 RISCOS_SRCS = riscos.cpp riscossup.h riscos_glue.h $(RISCOS_AGG_SRCS)
 AM_LDFLAGS += -lOSLib32
else
 RISCS_SRCS =
endif

if USE_GUI_AQUA
 AQUA_SRCS = aqua.cpp aquasup.h aqua_glue.h $(AQUA_OPENGL_SRCS)
else
 AQUA_SRCS =
endif

if USE_GUI_SDL
 SDL_SRCS = sdl.cpp sdlsup.h sdl_glue.h $(SDL_CAIRO_SRCS) $(SDL_OPENGL_SRCS) $(SDL_AGG_SRCS)
else
 SDL_SRCS =
endif

if USE_GUI_KDE
 KDE_SRCS = kde.cpp kdesup.h kde_glue.h klash.moc $(KDE_CAIRO_SRCS) $(KDE_OPENGL_SRCS) $(KDE_AGG_SRCS)
 AM_CPPFLAGS += $(KDE_CFLAGS) $(QT_CFLAGS)
 AM_LDFLAGS += $(KDE_LIBS) $(QT_LIBS)
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

if USE_SOUND_GST
AM_CPPFLAGS += $(GSTREAMER_CFLAGS)
endif

if USE_GUI_FLTK
 FLTK_SRCS = fltk.cpp fltksup.h $(FLTK_AGG_SRCS) $(FLTK_CAIRO_SRCS)
 AM_CPPFLAGS += $(FLTK2_CFLAGS) $(X11_CFLAGS)
 AM_LDFLAGS += $(FLTK2_LIBS) $(XFT_LIBS) $(X11_LIBS)
else
 FLTK_SRCS = 
endif


AM_CPPFLAGS += -DPKGDATADIR=\"$(pkgdatadir)\"
AM_CPPFLAGS += -DRENDERER_CONFIG=\"$(RENDERER_CONFIG)\"
AM_CPPFLAGS += -DGUI_CONFIG=\"$(GUI_CONFIG)\"
AM_CPPFLAGS += -DMEDIA_CONFIG=\"$(MEDIA_CONFIG)\"
AM_CPPFLAGS += -DTARGET_CONFIG=\"$(target)\"
AM_CPPFLAGS += -DCONFIG_CONFIG="\"$(shell cat .configline)\""

dist_images_DATA = images/gnash_128_96.ico images/GnashG.png
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
	$(AQUA_SRCS) \
	$(RISCOS_SRCS) \
	gui.cpp gui.h \
	NullGui.cpp \
	NullGui.h

libgnashgui_la_LIBADD = \
	$(top_builddir)/server/libgnashserver.la \
	$(top_builddir)/backend/libgnashbackend.la \
	$(top_builddir)/libamf/libgnashamf.la 

# is LDFLAGS still needed when AM_LDFLAGS is defined already ?
libgnashgui_la_LDFLAGS = \
	-release $(VERSION) \
	-no-undefined \
	-export-dynamic \
	$(AM_LDFLAGS)

if USE_GUI_KDE
bin_PROGRAMS = gnash # klash
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
	libgnashplayer.la \
	libgnashgui.la

libgnashplayer_la_SOURCES = \
	Player.cpp Player.h

libgnashplayer_la_LIBADD = \
	$(top_builddir)/server/libgnashserver.la \
	$(top_builddir)/backend/libgnashbackend.la \
	libgnashgui.la 

libgnashplayer_la_LDFLAGS = \
	-release $(VERSION) \
	-no-undefined \
	-export-dynamic \
	$(AM_LDFLAGS) 

gnash_SOURCES = gnash.cpp
# export our symbols so they can be used by Gnash plugins
gnash_LDFLAGS = $(LIBLTDL) -export-dynamic 
gnash_DEPENDENCIES = $(GNASH_LIBS)  .configline
gnash_LDADD = \
	$(MYSQL_LIBS) \
	$(GNASH_LIBS)
#	$(LIBLTDL) "-dlopen" self "-dlopen"

#klash_SOURCES = gnash.cpp
#klash_CPPFLAGS =	\
#	$(KDE_INCLS)	\
#	-DUSE_KDE \
#	$(AM_CPPFLAGS)
# export our symbols so they can be used by Gnash plugins
#klash_LDFLAGS = -export-dynamic
#klash_LDADD = \
#	$(LIB_KDECORE) \
#	$(LIB_KDEUI) \
#	$(GNASH_LIBS)

#klash_DEPENDENCIES = \
#	$(GNASH_LIBS)  .configline
#	 $(LIBLTDL) "-dlopen" self "-dlopen"

CLEANFILES = klash.moc .configline
klash.moc: $(srcdir)/kdesup.h
	@if test x"$(MOC)" != x; then \
	  echo "Generating MOC file..."; \
	  $(MOC) $(srcdir)/kdesup.h -o klash.moc; \
	else  \
	  echo "WARNING: Install QT's moc tool! Linking to default MOC file"; \
	  ln -sf $(srcdir)/klash.moc.in klash.moc; \
	fi

.configline: ../config.log
	-@rm -f .configline
	@head ../config.log | grep " .*/configure " | sed -e 's:^  . .*configure ::' > .configline

#noinst_PROGRAMS = guitest # klash flak
#guitest_SOURCES = guitest.cpp #fltk.cpp #fltksup.h fltksupp.h gnash.cpp gtk.cpp 
#	gtksup.h gui.cpp gui.h menu.cpp menu.h sdl.cpp sdlsup.h [21~sdlsupp.h
#guitest_LDADD = $(gnash_LDADD)

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.* $(CLEANFILES)

