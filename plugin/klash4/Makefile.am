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

if KPARTS4
AM_CPPFLAGS = \
	$(KDE4_CFLAGS) \
	$(QT4_CFLAGS) \
	$(X11_CFLAGS) \
	-DGNASHBINDIR=\"${bindir}\"

plugindir   = $(KDE4_PLUGINDIR)
kde_servicesdir = $(KDE4_SERVICESDIR)
kde_rcdir       = $(KDE4_APPSDATADIR)
#$(prefix)/share/apps/klash
kde_appsdatadir = $(KDE4_APPSDATADIR)
#$(prefix)/share/apps/klash
kde_configdir   = $(KDE4_CONFIGDIR)
#$(prefix)/share/config

plugin_LTLIBRARIES= libklashpart.la

libklashpart_la_SOURCES = klash_part.cpp klash_part.h
libklashpart_la_DEPENDENCIES = klash_part.moc
if PLUGIN_LINK_UNDEFINED
libklashpart_la_LDFLAGS = -avoid-version -L$(plugindir) $(KDE4_LIBS) $(QT4_LIBS)
else
libklashpart_la_LDFLAGS = -module -avoid-version -L$(plugindir) $(KDE4_LIBS) $(QT4_LIBS)
endif

dist_kde_services_DATA = klash_part.desktop
dist_kde_rc_DATA = klashpartui.rc
dist_kde_appsdata_DATA = pluginsinfo
#dist_kde_config_DATA = klashrc

# DIST_COMMON = $(dist_kde_appsdata_DATA) $(dist_kde_services_DATA) $(dist_kde_rc_DATA)

BUILT_SOURCES = klash_part.moc
CLEANFILES = klash_part.moc

klash_part.moc: $(srcdir)/klash_part.h
	@if test x"$(MOC4)" != x; then \
	  echo "Generating MOC 4.x file..."; \
	$(MOC4) $(srcdir)/klash_part.h -o klash_part.moc; \
	else  \
	  echo "WARNING: Install QT 4.x moc tool! Linking to default MOC file"; \
	  ln -s $(srcdir)/klash_part.moc.in klash_part.moc; \
	fi

install-data: $(dist_kde_services_DATA)
	test -d $(DESTDIR)$(kde_servicesdir) || $(mkinstalldirs) $(DESTDIR)$(kde_servicesdir)
	$(INSTALL) $(dist_kde_services_DATA) $(DESTDIR)$(kde_servicesdir)

test_perms:
	@if test ! -w $(DESTDIR)$(kde_appsdatadir) -o ! -w $(DESTDIR)$(kde_servicesdir); then \
	  if test -d $(DESTDIR)$(kde_appsdatadir); then \
	    echo "WARNING: Improper permissions for $(DESTDIR)$(kde_appsdatadir)!"; \
	    echo "You probably need to be root"; \
	    echo "Or configure with --with-kde-prefix= set to a writable directory"; \
	    exit 1; \
          fi; \
	  if test -d $(DESTDIR)$(kde_servicesdir) -a -d $(DESTDIR)$(kde_appsdatadir); then \
	    echo "WARNING: Improper permissions for $(DESTDIR)$(kde_appsdatadir) and $(DESTDIR)$(kde_servicesdir)!"; \
	    echo "You probably need to be root"; \
	    echo "Or configure with --with-kde-prefix= set to a writable directory"; \
	    exit 1; \
	  fi; \
	fi

install-data-am: $(dist_kde_services_DATA) $(dist_kde_appsdata_DATA)
	test -d $(DESTDIR)$(kde_servicesdir) || $(mkinstalldirs) $(DESTDIR)$(kde_servicesdir)
	$(INSTALL) $(srcdir)/$(dist_kde_services_DATA) $(DESTDIR)$(kde_servicesdir)
	test -d $(DESTDIR)$(kde_appsdatadir) || $(mkinstalldirs) $(DESTDIR)$(kde_appsdatadir)
	$(INSTALL) $(srcdir)/$(dist_kde_appsdata_DATA) $(DESTDIR)$(kde_appsdatadir)

install-pluginLTLIBRARIES: $(plugin_LTLIBRARIES)
	@echo "Nothing to install here now."
	@echo "Use \"make install-plugin\" to install the KDE plugin to $(DESTDIR)$(plugindir)."

install-plugin install-plugins: test_perms $(plugin_LTLIBRARIES) $(dist_kde_services_DATA) \
	install-dist_kde_appsdataDATA install-dist_kde_rcDATA \
	install-dist_kde_servicesDATA
	test -d $(DESTDIR)$(plugindir) || $(mkinstalldirs) $(DESTDIR)$(plugindir)
	$(LIBTOOL) --mode=install $(INSTALL) $(INSTALL_STRIP_FLAG) $(plugin_LTLIBRARIES) $(DESTDIR)$(plugindir)

uninstall-plugins: 
	$(RM) -f $(DESTDIR)$(plugindir)/libklashpart.so
	$(RM) -f $(DESTDIR)$(plugindir)/libklashpart.la
	$(RM) -f $(DESTDIR)$(plugindir)/libklashpart.a

endif

force:

.PHONY : install-plugins install-plugin test_perms
