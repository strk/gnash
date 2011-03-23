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

plugins_LTLIBRARIES = mysql.la

AM_CPPFLAGS = \
            -I$(top_srcdir)/libbase \
            -I$(top_srcdir)/libcore \
            -I$(top_srcdir)/libcore/abc \
            -I$(top_srcdir)/libcore/swf \
            -I$(top_srcdir)/libcore/vm \
            -I$(top_srcdir)/libcore/asobj \
            $(BOOST_CFLAGS) \
            $(MYSQL_CFLAGS) \
	    $(NULL)

AM_LDFLAGS =  \
        $(MYSQL_LIBS) \
	$(top_builddir)/libcore/libgnashcore.la \
        $(BOOST_LIBS) \
	$(PTHREAD_LIBS) \
	$(NULL)

mysql_la_SOURCES = mysql_db.cpp mysql_db.h
mysql_la_LDFLAGS = -module -avoid-version -no-undefined
mysql_la_LIBADD = $(AM_LDFLAGS)

CLEANFILES = \
      gnash-dbg.log

# SharedTest_SOURCES = SharedTest.cpp
# SharedTest_CPPFLAGS = '-DINPUT="$(srcdir)/SharedTest.cpp"'
# SharedTest_LDFLAGS = -export-dynamic

install-pluginsLTLIBRARIES: $(plugins_LTLIBRARIES)
	test -d "$(DESTDIR)$(pluginsdir)" || $(mkinstalldirs) "$(DESTDIR)$(pluginsdir)"
	$(LIBTOOL) --mode=install $(INSTALL) $(INSTALL_STRIP_FLAG) $(plugins_LTLIBRARIES) "$(DESTDIR)$(pluginsdir)/$(plugins_LTLIBRARIES)"
	$(RM) $(DESTDIR)$(pluginsdir)/*.*a 

uninstall-local:
	$(RM) $(DESTDIR)$(pluginsdir)/mysql.so
