## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

# $Id: Makefile.am,v 1.25 2007/02/02 15:50:28 rsavoye Exp $

AUTOMAKE_OPTIONS = 

if KLASH
AM_CPPFLAGS= $(KDE_CFLAGS) $(QT_CFLAGS)

plugindir   = $(libdir)/kde3
plugin_LTLIBRARIES= libklashpart.la

libklashpart_la_SOURCES = klash_part.cpp klash_part.h
libklashpart_la_DEPENDENCIES = klash_part.moc
if PLUGIN_LINK_UNDEFINED
libklashpart_la_LDFLAGS = -avoid-version -L$(plugindir)
else
libklashpart_la_LDFLAGS = -module -avoid-version -L$(plugindir)
endif

BUILT_SOURCES = klash_part.moc klash.moc
CLEANFILES = klash_part.moc klash.moc

klash_part.moc: $(srcdir)/klash_part.h
	@if test x"$(MOC)" != x; then \
	  echo "Generating MOC file..."; \
	  $(MOC) $(srcdir)/klash_part.h -o klash_part.moc; \
	else  \
	  echo "WARNING: Install QT's moc tool! Linking to default MOC file"; \
	  ln -s $(srcdir)/klash_part.moc.in klash_part.moc; \
	fi

klash.moc: $(srcdir)/klash.cpp
	@if test x"$(MOC)" != x; then \
	  echo "Generating MOC file..."; \
	  $(MOC) $(srcdir)/klash.cpp -o klash.moc; \
	else  \
	  echo "WARNING: Install QT's moc tool! Linking to default MOC file"; \
	  ln -s $(srcdir)/klash.moc.in klash.moc; \
	fi

install-pluginLTLIBRARIES: $(plugin_LTLIBRARIES)
	test -d "$(DESTDIR)$(plugindir)" || $(mkinstalldirs) "$(DESTDIR)$(plugindir)"
#	cp .libs/libklashpart.so $(DESTDIR)$(plugindir)/libklashpart.so
	$(LIBTOOL) --mode=install $(INSTALL) $(INSTALL_STRIP_FLAG) $(plugin_LTLIBRARIES) "$(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)"
#	$(RM) libklashpart.*a $(DESTDIR)$(plugindir)/libklashpart.so.0

uninstall-pluginLTLIBRARIES: $(plugin_LTLIBRARIES)
	$(RM) -f $(DESTDIR)$(plugindir)/libklashpart.so

else
all:
	$echo "Building klash is disabled"
endif
