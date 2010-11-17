# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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

## WARNING: make sure GLIB_LIBS appears first
## See: http://lists.gnu.org/archive/html/gnash-dev/2006-07/msg00076.html
AM_CPPFLAGS = -I.. \
	-I$(srcdir) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libcore \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libcore/parser \
	-I$(top_srcdir)/libcore/swf \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/librender \
	$(PTHREAD_CFLAGS) \
	$(SDL_CFLAGS) \
	$(PANGO_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GTK2_CFLAGS) \
	$(ATK_CFLAGS) \
	$(DMALLOC_CFLAGS) \
	$(PNG_CFLAGS) \
	$(JPEG_CFLAGS) \
	$(OGG_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(OPENGL_CFLAGS) \
	$(AGG_CFLAGS) \
	$(CAIRO_CFLAGS) \
	$(NULL)

GNASH_LIBS = \
	$(top_builddir)/libcore/libgnashcore.la \
	$(top_builddir)/libbase/libgnashbase.la \
	$(NULL)

instdir = $(includedir)/gnash

noinst_HEADERS = \
	agg/Renderer_agg.h \
	agg/Renderer_agg_bitmap.h \
	agg/Renderer_agg_style.h \
	cairo/Renderer_cairo.h \
	cairo/PathParser.h
	GnashDevice.h \
	eglDevice.h \
	opengl/tu_opengl_includes.h \
	directfb/Renderer_DirectFB.h \
	directfb/DirectFBDevice.h \
	vaapi/VaapiDevice.h \
	opengl/Renderer_ogl.h \
	opengles1/Renderer_gles1.h \
	opengles1/render_handler_gles.h \
	opengles2/Renderer_gles2.h \
	openvg/Renderer_ovg.h \
	openvg/Renderer_ovg_bitmap.h \
	opengl/Renderer_ogl.h \
	$(NULL)

pkglib_LTLIBRARIES = libgnashrender.la

libgnashrender_la_CPPFLAGS = $(AM_CPPFLAGS)
libgnashrender_la_LIBADD = $(LIBVA_LIBS) $(LIBVA_X11_LIBS) $(LIBVA_GLX_LIBS)
libgnashrender_la_LDFLAGS =  -release $(VERSION) 
libgnashrender_la_SOURCES =

if BUILD_OGL_RENDERER
libgnashrender_la_SOURCES += opengl/Renderer_ogl.cpp
libgnashrender_la_LIBADD += $(OPENGL_LIBS)
endif

if  BUILD_AGG_RENDERER
libgnashrender_la_SOURCES += agg/Renderer_agg.cpp 
libgnashrender_la_LIBADD += $(AGG_LIBS) $(LIBVA)
endif

if  BUILD_OVG_RENDERER
libgnashrender_la_CPPFLAGS += $(OVG_CFLAGS)
libgnashrender_la_SOURCES += openvg/Renderer_ovg.cpp 
libgnashrender_la_LIBADD += $(OPENVG_LIBS)
endif

if  BUILD_GLES1_RENDERER
libgnashrender_la_SOURCES += opengles1/Renderer_gles1.cpp 
libgnashrender_la_LIBADD += $(GLES1_LIBS)
endif

if  BUILD_GLES2_RENDERER
libgnashrender_la_SOURCES += opengles2/Renderer_gles2.cpp 
libgnashrender_la_LIBADD += $(GLES2_LIBS)
endif

if  BUILD_CAIRO_RENDERER
libgnashrender_la_SOURCES += cairo/Renderer_cairo.cpp
libgnashrender_la_SOURCES += cairo/PathParser.cpp
libgnashrender_la_LIBADD += $(CAIRO_LIBS)
endif

if BUILD_EGL_DEVICE
libgnashrender_la_CPPFLAGS += $(EGL_CFLAGS)
libgnashrender_la_SOURCES += eglDevice.cpp
libgnashrender_la_LIBADD += $(EGL_LIBS) $(GTK2_LIBS)
endif

if BUILD_DIRECTFB_DEVICE
libgnashrender_la_CPPFLAGS += $(DIRECTFB_CFLAGS)
libgnashrender_la_SOURCES += \
	directfb/Renderer_DirectFB.cpp \
	directfb/DirectFBDevice.cpp
libgnashrender_la_LIBADD += $(DIRECTFB_LIBS)
endif

if BUILD_VAAPI_DEVICE
libgnashrender_la_CPPFLAGS += $(VAAPI_CFLAGS)
libgnashrender_la_SOURCES += \
	vaapi/VaapiDevice.cpp \
	vaapi/VaapiSurface.cpp
libgnashrender_la_LIBADD += $(VAAPI_LIBS)
endif

# if BUILD_X11_DEVICE
libgnashrender_la_CPPFLAGS += $(X11_CFLAGS)
libgnashrender_la_SOURCES += \
	x11/X11Device.cpp
libgnashrender_la_LIBADD += $(X11_LIBS)
# endif

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="`$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

if ENABLE_PCH
AM_CXXFLAGS = $(PCH_FLAGS)
endif

bin_PROGRAMS = testr
check_PROGRAMS =

if BUILD_EGL_DEVICE
check_PROGRAMS += test_egl
# this tests the low level EGL interface, used by OpenVG, OpenGLES1,
# and OpenGLES2.
test_egl_SOURCES = test_egl.cpp
test_egl_CPPFLAGS = $(AM_CPPFLAGS)
test_egl_LDADD = ../libbase/libgnashbase.la \
	libgnashrender.la \
	$(EGL_LIBS) \
	$(GNASH_LIBS) \
	$(GTK2_LIBS)
endif

if BUILD_VAAPI_DEVICE
check_PROGRAMS += test_vaapi
test_vaapi_SOURCES = vaapi/test_vaapi.cpp
test_vaapi_CPPFLAGS = $(AM_CPPFLAGS) $(VAAPI_CFLAGS)
test_vaapi_LDADD = ../libbase/libgnashbase.la \
	libgnashrender.la \
	$(VAAPI_LIBS) \
	$(GNASH_LIBS)
endif

if BUILD_DIRECTFB_DEVICE
check_PROGRAMS += test_dfb
test_dfb_SOURCES = directfb/test_dfb.cpp
test_dfb_CPPFLAGS = $(AM_CPPFLAGS) $(DIRECTFB_CFLAGS)
test_dfb_LDADD = ../libbase/libgnashbase.la \
	libgnashrender.la \
	$(DIRECTFB_LIBS) \
	$(GNASH_LIBS)
endif

# if BUILD_X11_DEVICE
check_PROGRAMS += test_x11
test_x11_SOURCES = x11/test_x11.cpp
test_x11_CPPFLAGS = $(AM_CPPFLAGS) $(X11_CFLAGS)
test_x11_LDADD = ../libbase/libgnashbase.la \
	libgnashrender.la \
	$(X11_LIBS) \
	$(GNASH_LIBS)
# endif

testr_SOURCES = testr.cpp testr_gtk.cpp
testr_CPPFLAGS = $(AM_CPPFLAGS) $(DIRECTFB_CFLAGS) # -DUSE_TESTSUITE
testr_LDADD = ../libbase/libgnashbase.la \
	libgnashrender.la \
	$(EGL_LIBS) \
	$(GNASH_LIBS) \
	$(GTK2_LIBS)

