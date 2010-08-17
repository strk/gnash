## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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
# build gtk-gnash, kde-gnash, sdl-gnash, fltk-gnash, aqua-gnash, fb-gnash,
# riscos-gnash, haiku-gnash and dump-gnash
#

AUTOMAKE_OPTIONS = -Wno-portability

# Build the current directory first. It's hard to see, but there is a '.' dot here.
SUBDIRS = .

localedir = $(datadir)/locale

# this is where Gnash plugins get installed
pluginsdir = $(libdir)/gnash/plugins

# noinst_LTLIBRARIES = libgnashgui.la
BUILT_SOURCES = .configline

EXTRA_DIST = gnash.in \
	kde/klash3.moc.in \
	kde/Kde4Gui.moc.in \
	kde/Kde4Glue.moc.in \
	am-frag

# If python support is enabled, built that too
if HAS_PYTHON
SUBDIRS += pythonmod
endif
DIST_SUBDIRS = pythonmod

noinst_HEADERS =
noinst_LTLIBRARIES = #libgnashgui.la
AM_CPPFLAGS = \
        -I../.. \
        -I$(top_srcdir)/libnet \
        -I$(top_srcdir)/libcore \
        -I$(top_srcdir)/libcore/swf \
        -I$(top_srcdir)/libcore/parser \
        -I$(top_srcdir)/libcore/vm \
        -I$(top_srcdir)/libcore/asobj \
	-I$(top_srcdir)/libltdl \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/librender \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libsound \
	-I$(top_srcdir)/libmedia/ffmpeg \
	-I$(top_srcdir)/libmedia/gst \
	-I$(top_srcdir)/libmedia/haiku \
       -DLOCALEDIR=\"$(localedir)\" \
	-DPLUGINSDIR=\"$(pluginsdir)\" \
	-DLIBAVCODEC_IDENT=\"$(LIBAVCODEC_IDENT)\" \
	$(GLIB_CFLAGS) \
	$(LIRC_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(PYTHON_LIBS) \
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
	$(EXPAT_LIBS) \
	$(X11_LIBS) \
	$(Z_LIBS)

DUMP_CFLAGS =

DUMP_LIBS =

# The following will be filled up later
GUI_CPPFLAGS = 

## WARNING: make sure GLIB_LIBS appears first
## See: http://lists.gnu.org/archive/html/gnash-dev/2006-07/msg00076.html
AM_LDFLAGS =  \
	$(GLIB_LIBS) \
	$(CURL_LIBS) \
	$(JPEG_LIBS) \
	$(PNG_LIBS) \
	$(LIRC_LIBS) \
	$(MYSQL_LIBS) \
	$(INTLLIBS) \
	$(TS_LIBS)
	$(BOOST_LIBS) \
	$(PTHREAD_LIBS) \
	$(NULL)

GNASH_LIBS = \
	$(top_builddir)/libmedia/libgnashmedia.la \
	$(top_builddir)/libcore/libgnashcore.la \
	$(top_builddir)/libsound/libgnashsound.la \
	$(top_builddir)/libbase/libgnashbase.la

if LIBLTDL2
GNASH_LIBS += $(top_builddir)/libltdl/libltdlc.la
endif
if LIBLTDL1
GNASH_LIBS += $(top_builddir)/libbase/libltdlc.la
endif

# Only build loadable modules unless statically linking
# if DYNAMIC_GUI # make shared libraries suitable for dlopen().
# if  ENABLE_SHARED
#      LIB_FLAGS = -module -avoid-version -no-undefined
# else
#      LIB_FLAGS = -release $(VERSION)
# endif # ENABLE_SHARED
# else # make shared libraries suitable for linking.
LIB_FLAGS = -release $(VERSION)
if WIN32
LIB_FLAGS += --enable-auto-import
endif

# endif # DYNAMIC_GUI

# AQUA Toolkit
if BUILD_AQUA_GUI
#OS X stuff goes here...
GUI_CPPFLAGS += -DGUI_AQUA

bundle_name = Gnash.app

if  HAVE_OPENGL
noinst_LTLIBRARIES += libgnashaqua_ogl.la
libgnashaqua_ogl_la_CPPFLAGS = $(AM_CPPFLAGS) \
	$(OPENGL_CFLAGS)
libgnashaqua_ogl_la_SOURCES = \
	aqua_ogl_glue.cpp aqua_ogl_glue.h \
	aqua.cpp aquasup.h aqua_glue.h
libgnashaqua_ogl_la_LDFLAGS = $(LIB_FLAGS)
libgnashaqua_ogl_la_LIBADD = 
endif # HAVE_OPENGL
endif # BUILD_AQUA_GUI

if USE_FFMPEG_ENGINE
AM_LDFLAGS += $(FFMPEG_LIBS) $(LIBVA_LIBS) $(LIBVA_X11_LIBS) 
AM_CPPFLAGS += $(FFMPEG_CFLAGS) 
endif

if USE_GST_ENGINE
GST_STUFF = $(GSTPBUTILS_LIBS) $(GSTREAMER_PLUGINS_BASE_LIBS) $(GSTREAMER_LIBS)
AM_CPPFLAGS += $(GSTREAMER_CFLAGS)
AM_LDFLAGS += $(GST_STUFF)
endif

# if USE_GUI_FLTK
#  FLTK_SRCS = fltk.cpp fltksup.h $(FLTK_AGG_SRCS) $(FLTK_CAIRO_SRCS)
#  AM_CPPFLAGS += $(FLTK2_CFLAGS) $(X11_CFLAGS)
#  AM_LDFLAGS += $(FLTK2_LIBS) $(XFT_LIBS) $(X11_LIBS)
# else
#  FLTK_SRCS = 
# endif

AM_CPPFLAGS += -DPKGDATADIR="\"$(pkgdatadir)\""
AM_CPPFLAGS += -DRENDERER_CONFIG="\"$(RENDERER_CONFIG)\""
AM_CPPFLAGS += -DHWACCEL_CONFIG="\"$(HWACCEL_CONFIG)\""
AM_CPPFLAGS += -DMEDIA_CONFIG="\"$(MEDIA_CONFIG)\""
AM_CPPFLAGS += -DCONFIG_CONFIG="\"$(shell cat .configline)\""
AM_CPPFLAGS += -DCXXFLAGS="\"$(CXXFLAGS)\"" 

dist_images_DATA = images/gnash_128_96.ico images/GnashG.png
#dist_images_DATA = images/gnash_G_v3.ico
imagesdir = $(pkgdatadir)

bin_SCRIPTS = gnash
bin_PROGRAMS = 

gnash: gnash.in
	cp $< $@
	chmod +x $@

# These are used to handle input events from a keyboard, mouse, or
# touchscreen. These are only used by the Framebuffer GUI, as without
# X11, it has no event handling. We do it this way so the optionally
# built source files still get included in the source tarball.
DEVICES =
if ENABLE_INPUT_DEVICES
DEVICES += InputDevice.cpp InputDevice.h

# Touchscreen
if ENABLE_TSLIB
DEVICES += TouchDevice.cpp
endif
# PS/2 Mouse
if ENABLE_MOUSE
DEVICES += MouseDevice.cpp
endif
# Linux input events
if ENABLE_INPUT_EVENTS
DEVICES += EventDevice.cpp
endif
endif

# Always include the device files in the source tarball
EXTRA_DIST += \
	TouchDevice.cpp \
	MouseDevice.cpp \
	EventDevice.cpp

GUI_SRCS = gnash.cpp \
	gui.cpp gui.h \
	Player.cpp Player.h \
	NullGui.cpp NullGui.h \
	$(DEVICES) \
	$(NULL)

if BUILD_AQUA_GUI
BUILT_SOURCES += bundle
EXTRA_DIST += Gnash.app
endif

# Build the QTOPIA gui
if BUILD_QTOPIA3_GUI
include $(srcdir)/kde/qtopia3.am
endif

# Build the KDE3 gui
if BUILD_KDE3_GUI
BUILT_SOURCES += klash3.moc 
include $(srcdir)/kde/kde3.am
endif

# Build the KDE4 gui
if BUILD_KDE4_GUI
BUILT_SOURCES += Kde4Gui.moc
BUILT_SOURCES += Kde4Glue.moc
include $(srcdir)/kde/kde4.am
endif

# Build the SDL gui
if BUILD_SDL_GUI
include $(srcdir)/sdl/sdl.am
endif

# Build the AmigaOS4 gui
if BUILD_AMIGAOS4_GUI
include $(srcdir)/aos4/aos4.am
endif

# Build the Haiku gui
if BUILD_HAIKU_GUI
include $(srcdir)/haiku/haiku.am
endif

# Build the FLTK gui
if BUILD_FLTK_GUI
include $(srcdir)/fltk/fltk.am
endif

# Build the AQUA gui
if BUILD_AQUA_GUI
include $(srcdir)/am-frag/aqua.am
endif

# Build the GTK gui
if BUILD_GTK_GUI
include $(srcdir)/gtk/gtk.am
endif

# if ALP				# Access Linux Platform
# gtk_gnash_LDFLAGS += "-shared -Xlinker -z -Xlinker defs $(GTK2_LIBS)"
# gtk_gnash_LDADD += $(ALP_LIBS)
# endif
#
# Build the GTK/ALP gui
if BUILD_ALP_GUI
include $(srcdir)/am-frag/alp.am
endif

# Build the GTK/HILDON gui
if BUILD_HILDON_GUI
include $(srcdir)/am-frag/hildon.am
endif

# Build the DUMP gui
if BUILD_DUMP_GUI
include $(srcdir)/am-frag/dump.am
endif

# Build the FB gui
if BUILD_FB_GUI
include $(srcdir)/fb/fb.am
endif

# Build the RISCOS gui
if BUILD_RISCOS_GUI
include $(srcdir)/am-frag/riscos.am
endif

CLEANFILES = klash3.moc Kde4Gui.moc Kde4Glue.moc .configline gnash

bundle: Info.plist aqua-gnash
	@echo " Building: $(bundle_name)" 
	rm -rf $(bundle_name)/Contents
	mkdir -p $(bundle_name)/Contents/MacOS
	mkdir -p $(bundle_name)/Contents/Resources
	echo "APPL????" > $(bundle_name)/Contents/PkgInfo
	cp Info.plist $(bundle_name)/Contents/Info.plist
	cp -f aqua-gnash $(bundle_name)/Contents/MacOS/Gnash
	cp -f images/GnashG.icns $(bundle_name)/Contents/Resources/


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

# if DYNAMIC_GUI
# install-pluginsLTLIBRARIES: $(plugins_LTLIBRARIES)
# 	test -d "$(DESTDIR)$(plugindir)" || $(mkinstalldirs) "$(DESTDIR)$(plugindir)"
# 	for i in $(plugins_LTLIBRARIES); do \
# 	  $(LIBTOOL) --mode=install $(INSTALL) $(INSTALL_STRIP_FLAG) $$i "$(DESTDIR)$(plugindir)/$$i" ; \
# 	done
# endif

