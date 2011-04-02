# 
# Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

AUTOMAKE_OPTIONS = foreign

pkglib_LTLIBRARIES = libgnashvaapi.la

libgnashvaapi_la_CPPFLAGS = \
	-I$(top_srcdir)/libbase	\
	$(BOOST_CFLAGS)	\
	$(FFMPEG_CFLAGS) \
	$(LIBVA_CFLAGS)	\
	$(LIBVA_X11_CFLAGS) \
	$(NULL)

libgnashvaapi_la_LIBADD = \
	$(X11_LIBS) \
	$(BOOST_LIBS) \
	$(FFMPEG_LIBS) \
	$(LIBVA_LIBS) \
	$(LIBVA_X11_LIBS) \
	$(NULL)

libgnashvaapi_la_SOURCES = \
	vaapi_common.h \
	vaapi_utils.cpp	\
	vaapi_utils.h \
	VaapiContext.cpp \
	VaapiContext.h \
	VaapiDisplay.cpp \
	VaapiDisplay.h \
	VaapiDisplayX11.h \
	VaapiException.h \
	VaapiGlobalContext.cpp \
	VaapiGlobalContext.h \
	VaapiImage.cpp \
	VaapiImage.h \
	VaapiImageFormat.cpp \
	VaapiImageFormat.h \
	VaapiSubpicture.cpp \
	VaapiSubpicture.h \
	VaapiSurface.cpp \
	VaapiSurface.h \
	VaapiSurfaceProxy.cpp \
	VaapiSurfaceProxy.h \
	$(NULL)

if HAVE_VAAPI_GLX
libgnashvaapi_la_CPPFLAGS += \
	$(LIBVA_GLX_CFLAGS) \
	$(NULL)

libgnashvaapi_la_LIBADD += \
	$(LIBVA_GLX_LIBS) \
	$(NULL)

libgnashvaapi_la_SOURCES += \
	VaapiDisplayGLX.h \
	VaapiSurfaceGLX.cpp \
	VaapiSurfaceGLX.h \
	$(NULL)
endif

libgnashvaapi_la_LDFLAGS = -release $(VERSION)

