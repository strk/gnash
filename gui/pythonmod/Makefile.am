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

pkgpythondir = $(pyexecdir)/gtk-2.0
pkgpyexecdir = $(pyexecdir)/gtk-2.0

GNASH_INCLUDES = \
	-I$(top_srcdir)/gui \
	-I$(top_srcdir)/gui/gtk \
	-I$(top_srcdir)/gui/pythonmod \
	-I$(top_srcdir)/libbase	\
	-I$(top_srcdir)/libdevice \
	-I$(top_srcdir)/librender \
	-I$(top_srcdir)/librender/opengl \
	-I$(top_srcdir)/librender/cairo \
	-I$(top_srcdir)/librender/agg \
	-I$(top_srcdir)/libcore	\
	-I$(top_srcdir)/libcore/swf \
	-I$(top_srcdir)/libcore/vm \
	-I$(top_srcdir)/libcore/parser \
	-I$(top_srcdir)/libcore/asobj \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libdevice/vaapi \
	-I$(top_srcdir)/libsound

AM_CPPFLAGS = \
	$(PYTHON_CFLAGS) \
	$(PYGTK_CFLAGS) \
	$(GTK2_CFLAGS) \
	$(GNASH_INCLUDES)

pkgpyexec_LTLIBRARIES = gnash.la

GNASH_LIBS = \
	$(top_builddir)/librender/libgnashrender.la \
	$(top_builddir)/libcore/libgnashcore.la \
	$(top_builddir)/libbase/libgnashbase.la

if BUILD_LIBMEDIA
GNASH_LIBS += $(top_builddir)/libmedia/libgnashmedia.la
endif
if BUILD_LIBSOUND
GNASH_LIBS += $(top_builddir)/libsound/libgnashsound.la
endif

GNASH_LIBS += $(RENDERER_LIBS)

gnash_la_LDFLAGS = -module -avoid-version initgnash -export-dynamic #-no-undefined
gnash_la_LIBADD = \
	$(GNASH_LIBS) \
	$(GTK2_LIBS) \
	$(DEPS_LIBS) \
	$(LIBVA_LIBS) $(LIBVA_X11_LIBS) 

instdir = $(includedir)/gnash
inst_HEADERS = gnash-view.h

RENDERER_GLUE =
if BUILD_OGL_RENDERER
RENDERER_GLUE += ../gtk/gtk_glue_gtkglext.cpp
AM_CPPFLAGS += $(GLEXT_CFLAGS)
gnash_la_LIBADD += $(GLEXT_LIBS)
endif
if BUILD_AGG_RENDERER
RENDERER_GLUE += ../gtk/gtk_glue_agg.cpp
endif
if BUILD_CAIRO_RENDERER
RENDERER_GLUE += ../gtk/gtk_glue_cairo.cpp
endif

if USE_PYTHON
BUILT_SOURCES = gen-gnash.c
else
BUILT_SOURCES = 
endif

nodist_gnash_la_SOURCES = $(BUILT_SOURCES)
gnash_la_CPPFLAGS = $(AM_CPPFLAGS) $(GNASH_INCLUDES)
gnash_la_SOURCES = \
	gnashmodule.c \
	gnash-view.cpp \
	../gtk/gtk_canvas.cpp \
	$(RENDERER_GLUE)

gnash.c: gnash.defs gnash.override

CLEANFILES = gen-gnash.c
EXTRA_DIST = gnash.override gnash.defs

gen-gnash.c:   gnash.override gnash.defs
if HAS_PYTHON
	if test x"$(PYGOBJECT_CODEGEN)" != xno; then \
	  $(PYGOBJECT_CODEGEN) \
	    --register $(PYGTK_DEFSDIR)/gdk-types.defs \
	    --register $(PYGTK_DEFSDIR)/gtk-types.defs \
	    --override $(srcdir)/gnash.override \
	    --prefix pygnash $(srcdir)/gnash.defs > gen-gnash.c; \
	else if test x"$(PYGTK_CODEGEN)" != xno; then \
	  $(PYGTK_CODEGEN) \
	    --register $(PYGTK_DEFSDIR)/gdk-types.defs \
	    --register $(PYGTK_DEFSDIR)/gtk-types.defs \
	    --override $(srcdir)/gnash.override \
	    --prefix pygnash $(srcdir)/gnash.defs > gen-gnash.c; \
	 fi; fi
endif

# Remove libtool .la files
install-exec-hook:
	$(RM) $(DESTDIR)$(pyexecdir)/gtk-2.0/gnash.la

uninstall-local:
	$(RM) $(DESTDIR)$(pyexecdir)/gtk-2.0/gnash.so
