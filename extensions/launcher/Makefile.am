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

AUTOMAKE_OPTIONS = 

# this is where Gnash plugins get installed
pluginsdir = $(libdir)/gnash/plugins

plugins_LTLIBRARIES = launcher_ext.la
bin_PROGRAMS = launcher-gnash

AM_CPPFLAGS = -DGNASHBINDIR=\"${bindir}\" \
            -I$(top_srcdir)/libbase \
            -I$(top_srcdir)/libcore \
            -I$(top_srcdir)/libcore/abc \
            -I$(top_srcdir)/libcore/swf \
            -I$(top_srcdir)/libcore/vm \
	    -I$(top_srcdir)/libcore/asobj \
	    $(BOOST_CFLAGS) \
	    $(NULL)

launcher_ext_la_SOURCES = launcher_ext.cpp launcher_ext.h
launcher_ext_la_LDFLAGS = -module -avoid-version -no-undefined
launcher_ext_la_LIBDADD = $(LIBADD_DL) \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/libcore/libgnashcore.la \
	$(BOOST_LIBS) \
	$(PTHREAD_LIBS) \
	$(NULL)

launcher_gnash_SOURCES = \
	launcher.cpp \
	md5.cpp md5.h
launcher_gnash_CPPFLAGS = $(AM_CPPFLAGS)
launcher_gnash_LDADD = $(top_builddir)/libbase/libgnashbase.la
launcher_gnash_DEPENDENCIES = sums

CLEANFILES = gnash-dbg.log

# this is a list of Gnash executables that we're allowed to execute
PROGS =

if BUILD_GTK_GUI
PROGS += gui/.libs/gtk-gnash
endif

if BUILD_KDE_GUI
PROGS += gui/.libs/kde-gnash
endif

if BUILD_SDL_GUI
PROGS += gui/.libs/sdl-gnash
endif

if BUILD_FLTK_GUI
PROGS += gui/.libs/fltk-gnash
endif

# we don't use a dependency here, as it's too messy for make. Instead, we just
# check to make sure the file exists, and is an executable
sums: 
	for i in $(PROGS); do \
	  if test -x ../../$$i;  then \
	    file=`basename $$i | tr '-' '_' `; \
	    rm -f $$file; \
	    md5=`$(MD5SUM) ../../$$i | cut -d ' ' -f 1`; \
	    echo "const char *$$file = \"$$md5\";" > $$file.h; \
	  fi; \
	done

check_PROGRAMS = # SharedTest

install-pluginsLTLIBRARIES: $(plugins_LTLIBRARIES)
	test -d "$(DESTDIR)$(pluginsdir)" || $(mkinstalldirs) "$(DESTDIR)$(pluginsdir)"
	$(LIBTOOL) --mode=install $(INSTALL) $(INSTALL_STRIP_FLAG) $(plugins_LTLIBRARIES) "$(DESTDIR)$(pluginsdir)/$(plugins_LTLIBRARIES)"
	$(RM) $(DESTDIR)$(pluginsdir)/*.a 

uninstall-local:
	$(RM) $(DESTDIR)$(pluginsdir)/launcher.so
