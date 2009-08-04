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

AUTOMAKE_OPTIONS = 

# this is where Gnash plugins get installed
cgidir = $(libdir)/gnash/cygnal

bin_PROGRAMS = echo # gateway

# this is where Gnash plugins get installed
pluginsdir = $(libdir)/cygnal/plugins

plugins_LTLIBRARIES = gateway.la

GNASH_LIBS = \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/libamf/libgnashamf.la \
	$(top_builddir)/libnet/libgnashnet.la \
	$(BOOST_CFLAGS) \
	$(PTHREAD_LIBS) \
	$(NULL)

INCLUDES = \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/libamf \
	-I$(top_srcdir)/libnet \
	-I$(top_srcdir)/libcore \
	-I$(top_srcdir)/cygnal \
	$(BOOST_CFLAGS)

echo_SOURCES = echo.cpp echo.h
echo_LDADD =  $(GNASH_LIBS) $(top_builddir)/cygnal/libcygnal.la

# gateway_SOURCES = gateway.cpp echo.h 
# gateway_LDADD = $(GNASH_LIBS) $(top_builddir)/cygnal/libcygnal.la

gateway_la_SOURCES = gateway.cpp gateway.h 
gateway_la_CPPFLAGS = -DPLUGIN
gateway_la_LDFLAGS = -module -avoid-version -no-undefined
gateway_la_LIBADD = $(GNASH_LIBS) $(top_builddir)/cygnal/libcygnal.la

CLEANFILES = \
      gnash-dbg.log

install-pluginsLTLIBRARIES: $(plugins_LTLIBRARIES)
	test -d "$(DESTDIR)$(pluginsdir)" || $(mkinstalldirs) "$(DESTDIR)$(pluginsdir)"
	$(LIBTOOL) --mode=install $(INSTALL) $(INSTALL_STRIP_FLAG) $(plugins_LTLIBRARIES) "$(DESTDIR)$(pluginsdir)/$(plugins_LTLIBRARIES)"
	$(RM) $(DESTDIR)$(pluginsdir)/*.a 
