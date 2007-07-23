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

# $Id: Makefile.am,v 1.86 2007/07/23 13:49:43 strk Exp $

AUTOMAKE_OPTIONS = -Wno-portability

localedir = $(datadir)/locale

# this is where Gnash plugins get installed
pluginsdir = $(libdir)/gnash/plugins

lib_LTLIBRARIES = libgnashgui.la
BUILT_SOURCES = klash.moc .configline

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
	-DPLUGINSDIR=\"$(pluginsdir)\" \
        $(LIBXML_CFLAGS) \
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

## WARNING: make sure GLIB_LIBS appears first
## See: http://lists.gnu.org/archive/html/gnash-dev/2006-07/msg00076.html
AM_LDFLAGS =  \
	$(GLIB_LIBS) \
	$(LIBLTDL) \
	$(FFMPEG_LIBS) \
	$(CURL_LIBS) \
	$(LIBXML_LIBS) \
	$(INCLTDL) \
	$(BOOST_LIBS) \
	$(PTHREAD_LIBS) \
	$(NULL)

GNASH_LIBS = \
	$(top_builddir)/libamf/libgnashamf.la \
	$(top_builddir)/backend/libgnashbackend.la \
	$(top_builddir)/server/libgnashserver.la \
	$(top_builddir)/libgeometry/libgnashgeo.la \
	$(top_builddir)/libbase/libgnashbase.la

# The player class has to be added to each GUI library because it
# requires an ifdef to change between the various GUI toolkits.
PLAYER_SRCS = Player.cpp Player.h

#
# Build the libraries that hold the combined GUI toolkit and
# renderer. These are built as static libraries if doing a static
# build for debugging. Otherwise they are built as a loadable shared
# library module so we can dlopen() them at runtime.
#

plugins_LTLIBRARIES =		# set later on by conditionals

# Only build loadable modules unless statically linking
if DYNAMIC_GUI # make shared libraries suitable for dlopen().
if  ENABLE_SHARED
     LIB_FLAGS = -module -avoid-version -no-undefined
else
     LIB_FLAGS = -release $(VERSION)
endif # ENABLE_SHARED
else # make shared libraries suitable for linking.
     LIB_FLAGS = -release $(VERSION)
endif # DYNAMIC_GUI

# AQUA Toolkit
if BUILD_AQUA_GUI
if  HAVE_OPENGL
     plugins_LTLIBRARIES += libgnashaqua_gl.la
     libgnashaqua_gl_la_CPPFLAGS = -DGUI_AQUA $(AM_CPPFLAGS) \
	$(OPENGL_CFLAGS)
     libgnashaqua_gl_la_SOURCES = $(PLAYER_SRCS) \
	aqua_ogl_glue.cpp aqua_ogl_glue.h \
	aqua.cpp aquasup.h aqua_glue.h
     libgnashaqua_gl_la_LDFLAGS = $(LIB_FLAGS)
     libgnashaqua_gl_la_LIBADD = \
	$(top_builddir)/backend/libgnashogl.la libgnashgui.la
endif # HAVE_OPENGL
endif # BUILD_AQUA_GUI

# GTK2 Toolkit
if BUILD_GTK_GUI
if  HAVE_OPENGL
     plugins_LTLIBRARIES += libgnashgtk_gl.la
     libgnashgtk_gl_la_CPPFLAGS = -DGUI_GTK $(AM_CPPFLAGS) \
	$(GTK_CFLAGS) $(OPENGL_CFLAGS)
     libgnashgtk_gl_la_SOURCES = $(PLAYER_SRCS) \
	gtk_glue_gtkglext.cpp gtk_glue_gtkglext.h \
	gtk.cpp gtksup.h gtk_glue.h
     libgnashgtk_gl_la_LDFLAGS = $(LIB_FLAGS)
     libgnashgtk_gl_la_LIBADD = \
	$(top_builddir)/backend/libgnashogl.la libgnashgui.la $(GTK_LIBS)
endif # HAVE_OPENGL
if  HAVE_AGG
     plugins_LTLIBRARIES += libgnashgtk_agg.la
     libgnashgtk_agg_la_CPPFLAGS = -DGUI_GTK $(AM_CPPFLAGS) \
	$(GTK_CFLAGS) $(AGG_CFLAGS)
     libgnashgtk_agg_la_SOURCES = $(PLAYER_SRCS) \
	gtk_glue_agg.cpp gtk_glue_agg.h \
	gtk.cpp gtksup.h gtk_glue.h
     libgnashgtk_agg_la_LDFLAGS = $(LIB_FLAGS)
     libgnashgtk_agg_la_LIBADD = \
	$(top_builddir)/backend/libgnashagg.la libgnashgui.la \
	$(GTK_LIBS) $(AGG_LIBS)
endif # HAVE_AGG
if  HAVE_CAIRO
     plugins_LTLIBRARIES += libgnashgtk_cairo.la
     libgnashgtk_cairo_la_CPPFLAGS = -DGUI_GTK $(AM_CPPFLAGS) \
	$(GTK_CFLAGS) $(CAIRO_CFLAGS)
     libgnashgtk_cairo_la_SOURCES = $(PLAYER_SRCS) \
	gtk_glue_cairo.cpp gtk_glue_cairo.h \
	gtk.cpp gtksup.h gtk_glue.h
     libgnashgtk_cairo_la_LDFLAGS = $(LIB_FLAGS)
     libgnashgtk_cairo_la_LIBADD = \
	$(top_builddir)/backend/libgnashcairo.la  libgnashgui.la \
	$(GTK_LIBS) $(CAIRO_LIBS)
endif # HAVE_CAIRO
endif # BUILD_GTK_GUI

# KDE/QT Toolkit
if BUILD_KDE_GUI
if  HAVE_OPENGL
     plugins_LTLIBRARIES += libgnashkde_gl.la
     libgnashkde_gl_la_CPPFLAGS = -DGUI_KDE $(AM_CPPFLAGS) \
	$(OPENGL_FLAGS) $(KDE_CFLAGS) $(QT_CFLAGS)
     libgnashkde_gl_la_SOURCES = $(PLAYER_SRCS) \
	kde_glue_opengl.cpp kde_glue.h \
	kde.cpp kdesup.h kde_glue.h
     libgnashkde_gl_la_LDFLAGS = $(LIB_FLAGS)
     libgnashkde_gl_la_LIBADD = libgnashgui.la \
	$(top_builddir)/backend/libgnashogl.la \
	$(KDE_LIBS) $(QT_LIBS) $(OPENGL_LIBS)
endif # HAVE_OPENGL
if  HAVE_AGG
     plugins_LTLIBRARIES += libgnashkde_agg.la
     libgnashkde_agg_la_CPPFLAGS = -DGUI_KDE $(AM_CPPFLAGS) \
	$(AGG_CFLAGS) $(KDE_CFLAGS) $(QT_CFLAGS)
     libgnashkde_agg_la_SOURCES = $(PLAYER_SRCS) \
	kde_glue_agg.cpp kde_glue_agg.h \
	kde.cpp kdesup.h kde_glue.h
     libgnashkde_agg_la_LDFLAGS = $(LIB_FLAGS)
     libgnashkde_agg_la_LIBADD = libgnashgui.la \
	$(top_builddir)/backend/libgnashagg.la \
	$(KDE_LIBS) $(QT_LIBS) $(AGG_LIBS)
endif # HAVE_AGG
endif # BUILD_KDE_GUI

# SDL Toolkit
if BUILD_SDL_GUI
if  HAVE_OPENGL
     plugins_LTLIBRARIES += libgnashsdl_gl.la
     libgnashsdl_gl_la_CPPFLAGS = -DGUI_SDL $(AM_CPPFLAGS) \
	$(SDL_CFLAGS) $(OPENGL_CFLAGS)
     libgnashsdl_gl_la_SOURCES = $(PLAYER_SRCS) \
	sdl_ogl_glue.cpp sdl_ogl_glue.h \
	sdl.cpp sdlsup.h sdl_glue.h
     libgnashsdl_gl_la_LDFLAGS = $(LIB_FLAGS)
     libgnashsdl_gl_la_LIBADD = \
	$(top_builddir)/backend/libgnashogl.la libgnashgui.la \
	$(SDL_LIBS) $(OPENGL_LIBS)
endif # HAVE_OPENGL
if  HAVE_AGG
     plugins_LTLIBRARIES += libgnashsdl_agg.la
     libgnashsdl_agg_la_CPPFLAGS = -DGUI_SDL $(AM_CPPFLAGS) \
	$(SDL_CFLAGS) $(AGG_CFLAGS)
     libgnashsdl_agg_la_SOURCES = $(PLAYER_SRCS) \
	sdl_agg_glue.cpp sdl_agg_glue.h \
	sdl.cpp sdlsup.h sdl_glue.h
     libgnashsdl_agg_la_LDFLAGS = $(LIB_FLAGS)
     libgnashsdl_agg_la_LIBADD = \
	$(top_builddir)/backend/libgnashagg.la libgnashgui.la \
	$(SDL_LIBS) $(AGG_LIBS) 
endif # HAVE_AGG
if  HAVE_CAIRO
     plugins_LTLIBRARIES += libgnashsdl_cairo.la
     libgnashsdl_cairo_la_CPPFLAGS = -DGUI_SDL $(AM_CPPFLAGS) \
	$(SDL_CFLAGS) $(CAIRO_CFLAGS)
     libgnashsdl_cairo_la_SOURCES = $(PLAYER_SRCS) \
	sdl_cairo_glue.cpp sdl_cairo_glue.h \
	sdl.cpp sdlsup.h sdl_glue.h
     libgnashsdl_cairo_la_LDFLAGS = $(LIB_FLAGS)
     libgnashsdl_cairo_la_LIBADD = \
	$(top_builddir)/backend/libgnashcairo.la libgnashgui.la \
	$(SDL_LIBS) $(CAIRO_LIBS)
endif # HAVE_CAIRO
endif # BUILD_SDL_GUI

# FLTK2 Toolkit
if BUILD_FLTK_GUI
if  HAVE_AGG
     plugins_LTLIBRARIES += libgnashfltk_agg.la
     libgnashfltk_agg_la_CPPFLAGS = -DGUI_FLTK $(AM_CPPFLAGS) \
	$(FLTK2_CFLAGS) $(AGG_CFLAGS)
     libgnashfltk_agg_la_SOURCES = $(PLAYER_SRCS) \
	fltk_glue_agg.cpp fltk_glue_agg.h \
	fltk.cpp fltksup.h 
     libgnashfltk_agg_la_LDFLAGS = $(LIB_FLAGS)
     libgnashfltk_agg_la_LIBADD = \
	$(top_builddir)/backend/libgnashagg.la libgnashgui.la \
	$(FLTK2_LIBS) $(AGG_LIBS) $(XFT_LIBS) $(X11_LIBS)
endif # HAVE_AGG
if  HAVE_CAIRO
     plugins_LTLIBRARIES += libgnashfltk_cairo.la
     libgnashfltk_agg_la_CPPFLAGS = -DGUI_FLTK $(AM_CPPFLAGS) \
	$(FLTK2_CFLAGS) $(CAIRO_CFLAGS)
     libgnashfltk_cairo_la_SOURCES = $(PLAYER_SRCS) \
	fltk_glue_cairo.cpp fltk_glue_cairo.h \
	fltk.cpp fltksup.h 
     libgnashfltk_cairo_la_LDFLAGS = $(LIB_FLAGS)
     libgnashfltk_cairo_la_LIBADD = \
	$(top_builddir)/backend/libgnashcairo.la libgnashgui.la \
	$(FLTK2_LIBS) $(CAIRO_LIBS) $(XFT_LIBS) $(X11_LIBS)
endif # HAVE_CAIRO
endif # BUILD_FLTK_GUI

# Framebuffer, no toolkit
if BUILD_FB_GUI
if  HAVE_AGG
     libgnashsdl_agg_la_CPPFLAGS = -DGUI_FB $(AM_CPPFLAGS) $(SDL_CFLAGS)
     plugins_LTLIBRARIES += libgnashfb_agg.la
     libgnashfb_agg_la_SOURCES = $(PLAYER_SRCS) \
	fb.cpp fbsup.h
     libgnashfb_agg_la_LDFLAGS = $(LIB_FLAGS)
     libgnashfb_agg_la_LIBADD = \
	$(top_builddir)/backend/libgnashagg.la libgnashgui.la \
	$(SDL_LIBS) $(AGG_LIBS)
endif # HAVE_AGG
endif # BUILD_FB_GUI

#
# When static linking, we have to link in the correct library. These
# are built as static libraries by the above section,
#

#
# Setup the GUIs that have OpenGL support
#
if STATIC_GUI
if  BUILD_OGL_RENDERER
if   BUILD_GTK_GUI
      GNASH_LIBS += libgnashgtk_gl.la $(GTK_LIBS) $(OPENGL_LIBS)
endif # BUILD_GTK_GUI
if   BUILD_AQUA_GUI
      GNASH_LIBS += libgnashaqua_gl.la $(AQUA_LIBS) $(OPENGL_LIBS)
endif # BUILD_AQUA_GUI
if   BUILD_SDL_GUI
      GNASH_LIBS += libgnashsdl_gl.la $(SDL_LIBS) $(OPENGL_LIBS)
endif # BUILD_SDL_GUI
if BUILD_KDE_GUI
     GNASH_LIBS += libgnashkde_gl.la $(KDE_LIBS) $(QT_LIBS) $(OPENGL_LIBS)
endif # BUILD_KDE_GUI
endif # BUILD_OPENGL_RENDERER

#
# Setup the GUIs that have Cairo support
#
if  BUILD_CAIRO_RENDERER
if   BUILD_GTK_GUI
      GNASH_LIBS += libgnashgtk_cairo.la $(GTK_LIBS) $(CAIRO_LIBS)
endif # BUILD_GTK_GUI
if   BUILD_SDL_GUI
      GNASH_LIBS += libgnashsdl_cairo.la $(SDL_LIBS) $(CAIRO_LIBS)
endif # BUILD_SDL
if   BUILD_FLTK_GUI
      GNASH_LIBS += libgnashfltk_cairo.la $(FLTK2_LIBS) $(CAIRO_LIBS) $(XFT_LIBS) $(X11_LIBS)
endif # BUILD_FLTK_GUI
endif # BUILD_CAIRO_RENDERER

#
# Setup the GUIs that have AntiGrain (AGG) support
#
if  BUILD_AGG_RENDERER
if   BUILD_GTK_GUI
      GNASH_LIBS += libgnashgtk_agg.la $(AGG_LIBS) $(GTK_LIBS)
endif # BUILD_GTK_GUI
if  BUILD_AQUA_GUI
     GNASH_LIBS += libgnashaqua_agg.la $(AQUA_LIBS) $(AGG_LIBS)
endif # BUILD_AQUA_GUI
if  BUILD_SDL_GUI
     GNASH_LIBS += libgnashsdl_agg.la $(SDL_LIBS) $(AGG_LIBS)
endif # BUILD_SDL_GUI
if  BUILD_FLTK_GUI
     GNASH_LIBS += libgnashfltk_agg.la $(FLTK2_LIBS) $(AGG_LIBS) $(XFT_LIBS) $(X11_LIBS)
endif # BUILD_FLTK_GUI
if  BUILD_KDE_GUI
     GNASH_LIBS += libgnashkde_agg.la $(KDE_LIBS) $(QT_LIBS) $(AGG_LIBS)
endif # BUILD_KDE_GUI
if  BUILD_RISCOS_GUI
     GNASH_LIBS += libgnashriscos_agg.la $(RISCOS_LIBS) $(AGG_LIBS)
     AM_LDFLAGS += -lOSLib32
endif # BUILD_RISCOS_GUI
endif # BUILD_AGG_RENDERER
endif # STATIC_GUI

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

# if USE_GUI_FLTK
#  FLTK_SRCS = fltk.cpp fltksup.h $(FLTK_AGG_SRCS) $(FLTK_CAIRO_SRCS)
#  AM_CPPFLAGS += $(FLTK2_CFLAGS) $(X11_CFLAGS)
#  AM_LDFLAGS += $(FLTK2_LIBS) $(XFT_LIBS) $(X11_LIBS)
# else
#  FLTK_SRCS = 
# endif


AM_CPPFLAGS += -DPKGDATADIR=\"$(pkgdatadir)\"
AM_CPPFLAGS += -DRENDERER_CONFIG=\"$(RENDERER_CONFIG)\"
AM_CPPFLAGS += -DGUI_CONFIG=\"$(GUI_CONFIG)\"
AM_CPPFLAGS += -DMEDIA_CONFIG=\"$(MEDIA_CONFIG)\"
AM_CPPFLAGS += -DTARGET_CONFIG=\"$(target)\"
AM_CPPFLAGS += -DCONFIG_CONFIG="\"$(shell cat .configline)\""

dist_images_DATA = images/gnash_128_96.ico images/GnashG.png
#dist_images_DATA = images/gnash_G_v3.ico
imagesdir = $(pkgdatadir)

libgnashgui_la_SOURCES = \
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
	$(AM_LDFLAGS)

bin_PROGRAMS = gnash

gnash_SOURCES = gnash.cpp
# export our symbols so they can be used by Gnash plugins
gnash_LDFLAGS = $(LIBLTDL) -export-dynamic 
gnash_DEPENDENCIES = libgnashgui.la .configline
gnash_LDADD = \
	libgnashgui.la \
	$(MYSQL_LIBS) \
	$(GNASH_LIBS) \
	$(AM_LDFLAGS)
#	$(LIBLTDL) "-dlopen" self "-dlopen"

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

if DYNAMIC_GUI
install-pluginsLTLIBRARIES: $(plugins_LTLIBRARIES)
	test -d "$(DESTDIR)$(plugindir)" || $(mkinstalldirs) "$(DESTDIR)$(plugindir)"
	for i in $(plugins_LTLIBRARIES); do \
	  $(LIBTOOL) --mode=install $(INSTALL) $(INSTALL_STRIP_FLAG) $$i "$(DESTDIR)$(plugindir)/$$i" ; \
	done
endif
