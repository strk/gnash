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

SUBDIRS =

if KPARTS3
SUBDIRS += klash
endif

if KPARTS4
SUBDIRS += klash4
endif

if NPAPI
SUBDIRS += mozilla-sdk

if XPCOM
AM_CXXFLAGS=-fshort-wchar
INCLUDES = $(XPCOM_CFLAGS)
# Adding XPCOM_LIBS here breaks loading of plugin under iceapi from lenny.
# See bug #24469
# AM_LDFLAGS = $(XPCOM_LIBS)
SUBDIRS += xpcom
endif

if WIN32
SUBDIRS += win32
endif
endif # NPAPI

# make sure these files make it into the distribution tarball.
DIST_SUBDIRS = klash klash4 mozilla-sdk xpcom win32

# This is a hard coded path to where the plugin gets installed.
GNASHBINDIR = $(bindir)

# This is the directory the plugin gets installed.
plugindir   = $(FIREFOX_PLUGINS)

# XP_UNIX	This needs to get defined for npapi.h on unix platforms.
# PLUGIN_TRACE	Enable this define to get debug prints whenever the plugin
#		api gets control.
AM_CPPFLAGS = -DPLUGIN_TRACE -DXP_UNIX -DMOZ_X11 -DGNASHBINDIR=\"$(GNASHBINDIR)\" \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
        -I$(top_srcdir)/libcore \
        -I$(top_srcdir)/libcore/parser \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/backend \
	-I$(srcdir)/mozilla-sdk \
	-I$(srcdir)/mozilla-sdk/include \
	$(GLIB_CFLAGS) \
	$(X11_CFLAGS) \
	$(GTK2_CFLAGS) \
        $(CAIRO_CFLAGS) \
        $(GLIB_CFLAGS) \
        $(ATK_CFLAGS) \
        $(PANGO_CFLAGS) \
	$(NULL)

# AM_CXXFLAGS = $(AM_CPPFLAGS)

noinst_HEADERS = plugin.h \
	 mozilla-sdk/pluginbase.h \
	 mozilla-sdk/npplat.h \
         mozilla-sdk/include/jri.h \
	 mozilla-sdk/include/npapi.h \
         mozilla-sdk/include/npupp.h \
         mozilla-sdk/include/prtypes.h \
         mozilla-sdk/include/jni.h \
         mozilla-sdk/include/jri_md.h \
         mozilla-sdk/include/npruntime.h \
         mozilla-sdk/include/jni_md.h \
         mozilla-sdk/include/jritypes.h \
         mozilla-sdk/include/nptypes.h \
         mozilla-sdk/include/prcpucfg.h \
         mozilla-sdk/include/prcpucfg-glibc.h \
         mozilla-sdk/include/prcpucfg-win32.h \
         mozilla-sdk/include/prlock.h \
         mozilla-sdk/include/prcvar.h \
         mozilla-sdk/include/prthread.h \
         mozilla-sdk/include/prinit.h \
         mozilla-sdk/include/prerr.h \
         mozilla-sdk/include/prerror.h \
         mozilla-sdk/include/prinrval.h \
	 mozilla-sdk/include/prwin16.h \
         mozilla-sdk/include/obsolete/protypes.h 

if !WIN32
if NPAPI

plugin_LTLIBRARIES = libgnashplugin.la

libgnashplugin_la_SOURCES  = plugin.cpp \
	mozilla-sdk/npn_gate.cpp \
        mozilla-sdk/npp_gate.cpp \
        mozilla-sdk/np_entry.cpp

libgnashplugin_la_LIBADD   = \
	$(GLIB_LIBS) \
	$(NULL)

if PLUGIN_LINK_UNDEFINED
libgnashplugin_la_LDFLAGS = -avoid-version -L$(plugindir)
else
libgnashplugin_la_LDFLAGS = -module -avoid-version -L$(plugindir)
endif
endif # NPAPI
endif # WIN32

if XPCOM
  PKGLIB = install-pkglib
else
  PKGLIB = 
endif
test_perms:
	@if test ! -w $(DESTDIR)$(plugindir); then \
	  echo "WARNING: Improper permissions for $(DESTDIR)$(plugindir)!"; \
	  echo "You probably need to be root"; \
	  echo "Or configure with --with-npapi-plugindir= set to a writable directory"; \
	exit 1; \
	fi

install-pluginLTLIBRARIES: $(plugin_LTLIBRARIES)
	@echo "Nothing to install here now."
	@echo "Use \"make install-plugin\" to install the Firefox plugin to $(DESTDIR)$(plugindir)."
	$(RM) $(DESTDIR)$(plugindir)/*.a $(DESTDIR)$(plugindir)/*.la

if !WIN32
install-plugin install-plugins: $(plugin_LTLIBRARIES) $(PKGLIB)
	@test -d "$(DESTDIR)$(plugindir)" || $(mkinstalldirs) "$(DESTDIR)$(plugindir)"
	@echo "Installing plugin to $(DESTDIR)$(plugindir)"
	@if test -d .libs; then \
	  cp -f .libs/libgnashplugin.so* $(DESTDIR)$(plugindir)/libgnashplugin.so; \
	else \
	  cp -f libgnashplugin.so* $(DESTDIR)$(plugindir)/libgnashplugin.so; \
	fi
if KPARTS4
	$(MAKE) -C klash4 install-plugins
endif
if KPARTS3
	$(MAKE) -C klash install-plugins
endif
else
install-plugin install-plugins: 
	: # do nothing
endif	# end of if not windoze

# install-plugins: install-plugin $(PKGLIB)
# 	$(MAKE) -C klash install-plugins
# 	$(MAKE) -C klash4 install-plugins
# 	$(MAKE) -C win32 install-plugins
# if XPCOM
# 	$(MAKE) -C xpcom install-pkglib
# endif

install-pkglib:
	$(MAKE) -C xpcom install-pkglib

uninstall-plugin uninstall-plugins: 
	$(RM) -f $(DESTDIR)$(plugindir)/libgnashplugin.so

.PHONY : install-plugins install-plugin test_perms

