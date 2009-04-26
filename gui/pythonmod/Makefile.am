## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

pkgpythondir = $(pyexecdir)/gtk-2.0
pkgpyexecdir = $(pyexecdir)/gtk-2.0

GNASH_INCLUDES = \
	-I$(top_srcdir)/gui \
	-I$(top_srcdir)/libbase	\
	-I$(top_srcdir)/backend	\
	-I$(top_srcdir)/libcore	\
	-I$(top_srcdir)/libcore/swf \
	-I$(top_srcdir)/libcore/vm \
	-I$(top_srcdir)/libcore/parser \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libmedia/gst \
	-I$(top_srcdir)/libmedia/ffmpeg \
	-I$(top_srcdir)/libsound

INCLUDES = \
	$(PYTHON_CFLAGS) \
	$(PYGTK_CFLAGS) \
	$(GTK2_CFLAGS) \
	$(GNASH_INCLUDES)

pkgpyexec_LTLIBRARIES = gnash.la

GNASH_LIBS = \
	$(top_builddir)/libamf/libgnashamf.la \
	$(top_builddir)/libnet/libgnashnet.la \
	$(top_builddir)/libmedia/libgnashmedia.la \
	$(top_builddir)/libsound/libgnashsound.la \
	$(top_builddir)/libcore/libgnashcore.la \
	$(top_builddir)/libbase/libgnashbase.la \
	$(RENDERER_LIBS)

gnash_la_LDFLAGS = -module -avoid-version initgnash -export-dynamic #-no-undefined
gnash_la_LIBADD = \
	$(GNASH_LIBS) \
	$(GTK2_LIBS) \
	$(DEPS_LIBS)

if BUILD_OGL_RENDERER
RENDERER_GLUE = $(top_srcdir)/gui/gtk_glue_gtkglext.cpp
endif
if BUILD_AGG_RENDERER
RENDERER_GLUE = $(top_srcdir)/gui/gtk_glue_agg.cpp
endif
if BUILD_CAIRO_RENDERER
RENDERER_GLUE = $(top_srcdir)/gui/gtk_glue_cairo.cpp
endif

gnash_la_SOURCES = \
	gen-gnash.c	\
	gnashmodule.c \
	gnash-view.cpp\
	$(top_srcdir)/gui/gtk_canvas.cpp \
	$(RENDERER_GLUE)

gnash.c: gnash.defs gnash.override

CLEANFILES = gen-gnash.c
EXTRA_DIST = gnash.override gnash.defs
BUILT_SOURCES = gen-gnash.c

gen-gnash.c:   gnash.override gnash.defs
	if test x"$(PYGOBJECT_CODEGEN)" != x; then \
	  $(PYGOBJECT_CODEGEN) \
	    --register $(PYGTK_DEFSDIR)/gdk-types.defs \
	    --register $(PYGTK_DEFSDIR)/gtk-types.defs \
	    --override $(srcdir)/gnash.override \
	    --prefix pygnash $(srcdir)/gnash.defs > gen-gnash.c; \
	 fi
