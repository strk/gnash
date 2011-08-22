# 
#   Copyright (C) 2010, 2011 Free Software Foundation, Inc.
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
# 

desktopdir = $(datadir)/applications
desktop_DATA =

icons32dir = $(datadir)/icons/hicolor/32x32/apps
icons32_DATA =

splashdir = $(pkgdatadir)
splash_DATA = gnash-splash.swf

bin_SCRIPTS =

EXTRA_DIST = gnash-splash.swf \
             gnash.desktop \
             klash.desktop \
             gnash-gtk-launcher.in \
             gnash-qt-launcher.in \
             gnash-thumbnailer.in \
             gnash.schemas.in

if BUILD_GTK_GUI
desktop_DATA += gnash.desktop
icons32_DATA += ../packaging/gnash.xpm
bin_SCRIPTS  += gnash-gtk-launcher
endif

if BUILD_QT4_GUI
desktop_DATA += klash.desktop
icons32_DATA += ../packaging/klash.xpm
bin_SCRIPTS  += gnash-qt-launcher
endif

if BUILD_DUMP_GUI
sysconf_DATA = gnashthumbnailrc
desktop_DATA += gnash.schemas
bin_SCRIPTS += gnash-thumbnailer
endif

gnash.schemas: gnash.schemas.in
	sed -e 's#@@BINDIR@@#$(bindir)#g' $(srcdir)/gnash.schemas.in > $@

gnash-thumbnailer: gnash-thumbnailer.in
	sed -e 's#@@BINDIR@@#$(bindir)#g;s#@@SYSCONFDIR@@#$(sysconfdir)#g' $(srcdir)/gnash-thumbnailer.in > $@
	chmod +x $@

gnashthumbnailrc: gnashthumbnailrc.in
	cp $(srcdir)/$@.in $@

validate-gnash-desktop: gnash.desktop
	( dfvalidate=`which desktop-file-validate`; \
	  if test x$$dfvalidate != x && test -x $$dfvalidate; then \
              $$dfvalidate $< || ( echo "* $< INVALID *"; exit 1 ) \
	  else \
	      echo "Can't find desktop-file-validate to validate $<; proceed with fingers crossed..."; \
          fi )

validate-klash-desktop: klash.desktop
	( dfvalidate=`which desktop-file-validate`; \
	  if test x$$dfvalidate != x && test -x $$dfvalidate; then \
              $$dfvalidate $< || ( echo "* $< INVALID *"; exit 1 ) \
	  else \
	      echo "Can't find desktop-file-validate to validate $<; proceed with fingers crossed..."; \
          fi )

install-data-hook: validate-gnash-desktop validate-klash-desktop install-schemas

uninstall-hook: uninstall-schemas

install-schemas: 
if BUILD_DUMP_GUI
if GCONF_SCHEMAS_INSTALL
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule gnash.schemas
endif
endif

uninstall-schemas: 
if GCONF_SCHEMAS_INSTALL
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-uninstall-rule gnash.schemas
endif

CLEANFILES = gnash.schemas gnash-thumbnailer gnashthumbnailrc
