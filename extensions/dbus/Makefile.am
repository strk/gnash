## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006, 2009, 2010, 2011 Free Software Foundation, Inc.
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

plugins_LTLIBRARIES = dbus.la

AM_CPPFLAGS =  \
		-I$(top_srcdir)/libbase \
		-I$(top_srcdir)/libcore \
		-I$(top_srcdir)/libcore/abc \
		-I$(top_srcdir)/libcore/swf \
		-I$(top_srcdir)/libcore/vm \
	    -I$(top_srcdir)/libcore/asobj \
	    $(BOOST_CFLAGS) \
		$(NULL)

dbus_la_SOURCES = dbus_ext.cpp dbus_ext.h
dbus_la_CPPFLAGS = $(AM_CPPFLAGS)
dbus_la_LDFLAGS = -module -avoid-version -no-undefined
dbus_la_LIBDADD = $(LIBADD_DL) \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/libcore/libgnashcore.la \
	$(BOOST_LIBS) \
	$(PTHREAD_LIBS) \
	$(NULL)

check_PROGRAMS = # SharedTest
CLEANFILES = \
      gnash-dbg.log

install-pluginsLTLIBRARIES: $(plugins_LTLIBRARIES)
	test -d "$(DESTDIR)$(pluginsdir)" || $(mkinstalldirs) "$(DESTDIR)$(pluginsdir)"
	$(LIBTOOL) --mode=install $(INSTALL) $(INSTALL_STRIP_FLAG) $(plugins_LTLIBRARIES) "$(DESTDIR)$(pluginsdir)/$(plugins_LTLIBRARIES)"
	$(RM) $(DESTDIR)$(pluginsdir)/*.a 

uninstall-local:
	$(RM) $(DESTDIR)$(pluginsdir)/dbus.so
