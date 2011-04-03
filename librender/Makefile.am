# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
#   2011 Free Software Foundation, Inc.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
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
#

## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS =

# this is where Gnash plugins get installed
pluginsdir = $(libdir)/gnash/plugins

AM_CPPFLAGS = -I.. \
	-I$(srcdir) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libcore \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libcore/parser \
	-I$(top_srcdir)/libcore/swf \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/librender \
	-I$(top_srcdir)/libdevice \
	$(BOOST_CFLAGS) \
	$(OPENGL_CFLAGS) \
	$(AGG_CFLAGS) \
	$(DIRECTFB_CFLAGS) \
	$(CAIRO_CFLAGS) \
	$(NULL)

GNASH_LIBS = \
	$(top_builddir)/libcore/libgnashcore.la \
	$(top_builddir)/libmedia/libgnashmedia.la \
	$(top_builddir)/libbase/libgnashbase.la \
	$(NULL)

instdir = $(includedir)/gnash

noinst_HEADERS = \
	Renderer.h \
	agg/Renderer_agg.h \
	agg/LinearRGB.h \
	agg/Renderer_agg_bitmap.h \
	agg/Renderer_agg_style.h \
	cairo/Renderer_cairo.h \
	cairo/PathParser.h \
	opengl/tu_opengl_includes.h \
	opengl/Renderer_ogl.h \
	opengles1/Renderer_gles1.h \
	opengles1/render_handler_gles.h \
	opengles2/Renderer_gles2.h \
	openvg/OpenVGRenderer.h \
	openvg/OpenVGBitmap.h \
	openvg/OpenVGStyle.h \
	opengl/Renderer_ogl.h \
	$(NULL)

EXTRA_DIST = $(noinst_HEADERS)

pkglib_LTLIBRARIES =  libgnashrender.la

libgnashrender_la_CPPFLAGS = $(AM_CPPFLAGS)
libgnashrender_la_LIBADD = \
	../libdevice/libgnashdevice.la \
	$(LIBVA_LIBS) \
	$(LIBVA_X11_LIBS) \
	$(LIBVA_GLX_LIBS) \
	$(GNASH_LIBS)
libgnashrender_la_LDFLAGS =  -release $(VERSION) 
libgnashrender_la_SOURCES =

if BUILD_OGL_RENDERER
libgnashrender_la_SOURCES += \
	opengl/Renderer_ogl.cpp \
	opengl/Renderer_ogl.h
libgnashrender_la_LIBADD += $(OPENGL_LIBS)
endif

if  BUILD_AGG_RENDERER
libgnashrender_la_SOURCES += \
	agg/Renderer_agg.cpp \
	agg/Renderer_agg.h
libgnashrender_la_LIBADD += $(AGG_LIBS) $(LIBVA)
endif

if  BUILD_OVG_RENDERER
libgnashrender_la_CPPFLAGS += $(OPENVG_CFLAGS)
libgnashrender_la_SOURCES += \
	openvg/OpenVGRenderer.cpp \
	openvg/OpenVGRenderer.h \
	openvg/OpenVGBitmap.h \
	openvg/OpenVGBitmap.cpp \
	openvg/OpenVGStyle.h
libgnashrender_la_LIBADD += $(OPENVG_LIBS)
endif

if  BUILD_GLES1_RENDERER
libgnashrender_la_SOURCES += \
	opengles1/Renderer_gles1.cpp \
	opengles1/Renderer_gles1.h
libgnashrender_la_LIBADD += $(GLES1_LIBS)
endif

if  BUILD_CAIRO_RENDERER
libgnashrender_la_SOURCES += \
	cairo/Renderer_cairo.cpp \
	cairo/Renderer_cairo.h \
	cairo/PathParser.cpp
libgnashrender_la_LIBADD += $(CAIRO_LIBS)
endif

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="`$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

if ENABLE_PCH
AM_CXXFLAGS = $(PCH_FLAGS)
endif

# Remove libtool .la files
install-exec-hook:
	$(RM) $(DESTDIR)$(libdir)/gnash/libgnashrender.la

uninstall-local:
	$(RM) $(DESTDIR)$(libdir)/gnash/libgnashrender-*.so

if BUILD_OGL_RENDERER
    libgnashrender_la_SOURCES += \
	GnashTexture.cpp \
	$(NULL)

noinst_HEADERS += \
	GnashTexture.h \
	$(NULL)
endif

if ENABLE_DEVELOPER_TESTS
check_PROGRAMS = testr

testr_SOURCES = testr.cpp # testr_gtk.cpp
testr_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(EGL_CFLAGS) \
	$(OPENVG_CFLAGS) \
	$(GLES1_CFLAGS) \
	$(DIRECTFB_CFLAGS)
testr_LDADD = \
	libgnashrender.la \
	../libdevice/libgnashdevice.la \
	../libsound/libgnashsound.la \
	$(GTK2_LIBS) \
	$(EXTRA_EGL_LIBS) \
	$(EGL_LIBS) \
	$(GNASH_LIBS) \
	$(OPENVG_LIBS) \
	$(GLES1_LIBS)
endif
