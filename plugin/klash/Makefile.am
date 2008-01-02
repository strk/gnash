## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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

# $Id: Makefile.am,v 1.38 2008/01/02 12:04:43 strk Exp $

AUTOMAKE_OPTIONS = 

if KPARTS
AM_CPPFLAGS= $(KDE_CFLAGS) $(QT_CFLAGS) -DGNASHBINDIR=\"${bindir}\"

plugindir   = $(KDE_PLUGINDIR)
kde_servicesdir = $(KDE_SERVICESDIR)
kde_rcdir       = $(KDE_APPSDATADIR)/klash
#$(prefix)/share/apps/klash
kde_appsdatadir = $(KDE_APPSDATADIR)/klash
#$(prefix)/share/apps/klash
kde_configdir   = $(KDE_CONFIGDIR)
#$(prefix)/share/config

plugin_LTLIBRARIES= libklashpart.la

libklashpart_la_SOURCES = klash_part.cpp klash_part.h
libklashpart_la_DEPENDENCIES = klash_part.moc
if PLUGIN_LINK_UNDEFINED
libklashpart_la_LDFLAGS = -avoid-version -L$(plugindir)
else
libklashpart_la_LDFLAGS = -module -avoid-version -L$(plugindir)
endif

dist_kde_services_DATA = klash_part.desktop
dist_kde_rc_DATA = klashpartui.rc
dist_kde_appsdata_DATA = pluginsinfo
#dist_kde_config_DATA = klashrc

# DIST_COMMON = $(dist_kde_appsdata_DATA) $(dist_kde_services_DATA) $(dist_kde_rc_DATA)

BUILT_SOURCES = klash_part.moc
CLEANFILES = klash_part.moc

klash_part.moc: $(srcdir)/klash_part.h
	@if test x"$(MOC)" != x; then \
	  echo "Generating MOC file..."; \
	  $(MOC) $(srcdir)/klash_part.h -o klash_part.moc; \
	else  \
	  echo "WARNING: Install QT's moc tool! Linking to default MOC file"; \
	  ln -s $(srcdir)/klash_part.moc.in klash_part.moc; \
	fi

install-data: $(dist_kde_services_DATA)
	$(INSTALL) $(dist_kde_services_DATA) $(kde_servicesdir)

install-pluginLTLIBRARIES: $(plugin_LTLIBRARIES)
	test -d "$(DESTDIR)$(plugindir)" || $(mkinstalldirs) "$(DESTDIR)$(plugindir)"
#	cp .libs/libklashpart.so $(DESTDIR)$(plugindir)/libklashpart.so
	$(LIBTOOL) --mode=install $(INSTALL) $(INSTALL_STRIP_FLAG) $(plugin_LTLIBRARIES) $(DESTDIR)$(plugindir)
#	$(RM) libklashpart.*a $(DESTDIR)$(plugindir)/libklashpart.so.0

uninstall-pluginLTLIBRARIES: $(plugin_LTLIBRARIES)
	$(RM) -f $(DESTDIR)$(plugindir)/libklashpart.so
	$(RM) -f $(DESTDIR)$(plugindir)/libklashpart.la
	$(RM) -f $(DESTDIR)$(plugindir)/libklashpart.a

endif

force:
