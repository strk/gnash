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

AUTOMAKE_OPTIONS = 

# this is where Gnash plugins get installed
pluginsdir = $(libdir)/gnash/plugins

plugins_LTLIBRARIES = gtkext.la
DEF_MAKESWF_FLAGS = -I $(top_srcdir)/testsuite/actionscript.all

AM_CPPFLAGS = \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/gui \
	-I$(top_srcdir)/libcore \
	-I$(top_srcdir)/libcore/abc \
        -I$(top_srcdir)/libcore/swf \
	-I$(top_srcdir)/libcore/vm \
	-I$(top_srcdir)/libcore/parser \
	-I$(top_srcdir)/libcore/asobj \
	$(SDL_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(PTHREAD_CFLAGS) \
	$(GTK2_CFLAGS) \
	$(GLEXT_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(PANGO_CFLAGS) \
	$(ATK_CFLAGS) \
	$(CAIRO_CFLAGS) \
	$(INCLTDL)

AM_LDFLAGS =  \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/libcore/libgnashcore.la \
	$(GTK2_LIBS) \
	$(GLEXT_LIBS) \
	$(CAIRO_LIBS) \
	$(PANGO_LIBS) \
	$(ATK_LIBS) \
	$(GLIB_LIBS) \
	$(EXPAT_LIBS) \
	$(BOOST_LIBS) \
	$(PTHREAD_LIBS) \
	$(NULL)

gtkext_la_SOURCES = gtkext.cpp gtkext.h
gtkext_la_CPPFLAGS = $(AM_CPPFLAGS)
gtkext_la_LDFLAGS = -module -avoid-version # -no-undefined
gtkext_la_LIBDADD = $(LIBADD_DL) $(AM_LDFLAGS)

# check_PROGRAMS = hello
CLEANFILES = \
	gnash-dbg.log\
	hello.swf.frame0.pp \
	hello.swf \
	logfile

# hello: hello.swf
# 	../../utilities/gprocessor hello.swf

# noinst_SCRIPTS = hello.as
# hello.swf: hello.as

SUFFIXES = as swf
.as.swf: 
	$(MAKESWF) $(DEF_MAKESWF_FLAGS)	$(MAKESWF_FLAGS) -o $@ $<

install-pluginsLTLIBRARIES: $(plugins_LTLIBRARIES)
	test -d "$(DESTDIR)$(pluginsdir)" || $(mkinstalldirs) "$(DESTDIR)$(pluginsdir)"
	$(LIBTOOL) --mode=install $(INSTALL) $(INSTALL_STRIP_FLAG) $(plugins_LTLIBRARIES) "$(DESTDIR)$(pluginsdir)/$(plugins_LTLIBRARIES)"
	$(RM) $(DESTDIR)$(pluginsdir)/*.a 

uninstall-local:
	$(RM) $(DESTDIR)$(pluginsdir)/gtk2.so
