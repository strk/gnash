## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
#   2011 Free Software Foundation, Inc.
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
# build gtk-gnash, qt4-gnash, sdl-gnash, fltk-gnash, aqua-gnash, fb-gnash,
# haiku-gnash and dump-gnash
#

AUTOMAKE_OPTIONS = -Wno-portability

SUBDIRS = .

# If python support is enabled, built that too
if HAS_PYTHON
SUBDIRS += pythonmod
endif

# if BUILD_DUMP_GUI
# SUBDIRS += dump
# endif

localedir = $(datadir)/locale

# this is where Gnash plugins get installed
pluginsdir = $(libdir)/gnash/plugins

# noinst_LTLIBRARIES = libgnashgui.la
BUILT_SOURCES =

DIST_SUBDIRS = pythonmod
EXTRA_DIST = gnash.in \
	qt/klash3.moc.in \
	qt/Qt4Gui.moc.in \
	qt/Qt4Glue.moc.in

noinst_HEADERS =
noinst_LTLIBRARIES = 

# top_builddir and top_srcdir are for revno.h; its location depends on
# whether we're building from a snapshot or a checkout.
AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libnet \
	-I$(top_srcdir)/libcore \
	-I$(top_srcdir)/libcore/swf \
	-I$(top_srcdir)/libcore/parser \
	-I$(top_srcdir)/libcore/vm \
	-I$(top_srcdir)/libcore/asobj \
	-I$(top_srcdir)/libltdl \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/libdevice \
	-I$(top_srcdir)/librender \
	-I$(top_srcdir)/librender/agg \
	-I$(top_srcdir)/librender/cairo \
	-I$(top_srcdir)/librender/opengl \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libsound \
	-DLOCALEDIR=\"$(localedir)\" \
	-DPLUGINSDIR=\"$(pluginsdir)\" \
	-DLIBAVCODEC_IDENT=\"$(LIBAVCODEC_IDENT)\" \
	$(BOOST_CFLAGS) \
	$(PTHREAD_CFLAGS) \
	$(NULL)

if BUILD_EGL_DEVICE
AM_CPPFLAGS += $(EGL_CFLAGS)
endif
if BUILD_DIRECTFB_DEVICE
AM_CPPFLAGS += $(DIRECTFB_CFLAGS)
endif

GTK_CFLAGS = \
	$(GTK2_CFLAGS) \
	$(GLEXT_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(PANGO_CFLAGS) \
	$(ATK_CFLAGS) \
	$(CAIRO_CFLAGS)

GTK_LIBS =  \
	$(GLIB_LIBS) \
	$(GTK2_LIBS) \
	$(GLEXT_LIBS) \
	$(CAIRO_LIBS) \
	$(PANGO_LIBS) \
	$(ATK_LIBS) \
	$(EXPAT_LIBS) \
	$(X11_LIBS)

# The following will be filled up later
GUI_CPPFLAGS = 

AM_CXXFLAGS = $(CROSS_CXXFLAGS)

GNASH_LIBS = \
	$(top_builddir)/libcore/libgnashcore.la \
	$(top_builddir)/libdevice/libgnashdevice.la \
	$(top_builddir)/librender/libgnashrender.la \
	$(top_builddir)/libbase/libgnashbase.la

if BUILD_LIBMEDIA
GNASH_LIBS += $(top_builddir)/libmedia/libgnashmedia.la
endif
if BUILD_LIBSOUND
GNASH_LIBS += $(top_builddir)/libsound/libgnashsound.la
endif

if HAVE_VAAPI
GNASH_LIBS += \
	$(top_builddir)/libdevice/libgnashvaapi.la \
	$(LIBVA_LIBS) \
	$(LIBVA_X11_LIBS)
if BUILD_OGL_RENDERER
GNASH_LIBS += $(LIBVA_GLX_LIBS)
endif   # BUILD_OGL_RENDERER
endif   # HAVE_VAAPI

LIB_FLAGS = -release $(VERSION)
if WIN32
LIB_FLAGS += --enable-auto-import
endif

AM_CPPFLAGS += -DPKGDATADIR="\"$(pkgdatadir)\""
AM_CPPFLAGS += -DRENDERER_CONFIG="\"$(RENDERER_CONFIG)\""
AM_CPPFLAGS += -DHWACCEL_CONFIG="\"$(HWACCEL_CONFIG)\""
AM_CPPFLAGS += -DMEDIA_CONFIG="\"$(MEDIA_CONFIG)\""
AM_CPPFLAGS += -DCONFIG_CONFIG="\"$(shell cat ../.configline)\""
AM_CPPFLAGS += -DCXXFLAGS="\"$(CXXFLAGS)\"" 

dist_images_DATA = images/gnash_128_96.ico images/GnashG.png
imagesdir = $(pkgdatadir)

bin_SCRIPTS = gnash
bin_PROGRAMS = 

gnash: gnash.in
	cp $(srcdir)/gnash.in $@
	chmod +x $@

GUI_SRCS = gnash.cpp \
	gui.cpp gui.h \
	Player.cpp Player.h \
	NullGui.cpp NullGui.h \
	ScreenShotter.cpp ScreenShotter.h \
	$(NULL)

if BUILD_DUMP_GUI
include $(srcdir)/dump/dump.am
endif

if BUILD_AQUA_GUI
BUILT_SOURCES += bundle
EXTRA_DIST += Gnash.app
endif

# Build the FB gui
if BUILD_FB_GUI
include $(srcdir)/fb/fb.am
endif

# Build the QTOPIA gui
if BUILD_QTOPIA3_GUI
include $(srcdir)/qt/qtopia3.am
endif

# Build the KDE3 gui
if BUILD_KDE3_GUI
BUILT_SOURCES += klash3.moc 
include $(srcdir)/qt/kde3.am
endif

# Build the QT4 gui
if BUILD_QT4_GUI
BUILT_SOURCES += Qt4Gui.moc
BUILT_SOURCES += Qt4Glue.moc
include $(srcdir)/qt/qt4.am
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
include $(srcdir)/aqua/aqua.am
endif

# Build the GTK gui
if BUILD_GTK_GUI
include $(srcdir)/gtk/gtk.am
endif

CLEANFILES = klash3.moc Qt4Gui.moc Qt4Glue.moc gnash

bundle: Info.plist aqua-gnash
	@echo " Building: $(bundle_name)" 
	rm -rf $(bundle_name)/Contents
	mkdir -p $(bundle_name)/Contents/MacOS
	mkdir -p $(bundle_name)/Contents/Resources
	echo "APPL????" > $(bundle_name)/Contents/PkgInfo
	cp Info.plist $(bundle_name)/Contents/Info.plist
	cp -f aqua-gnash $(bundle_name)/Contents/MacOS/Gnash
	cp -f images/GnashG.icns $(bundle_name)/Contents/Resources/

clean-hook:
	-rm -f core.* $(CLEANFILES)

if ENABLE_DEVELOPER_TESTS
check_PROGRAMS = test_glue

test_glue_SOURCES = \
	test_glue.cpp \
	fb/fb_glue_agg.cpp \
	fb/fb_glue_ovg.cpp
test_glue_CPPFLAGS = \
	-DFAKEFB=\"$(FAKEFB)\" \
	$(AM_CPPFLAGS) \
	$(DIRECTFB_CFLAGS)
#       -DUSE_TESTSUITE
test_glue_LDADD = $(GNASH_LIBS)
endif
