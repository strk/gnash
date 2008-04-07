## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

if KPARTS
SUBDIRS += klash
endif

if NSAPI
SUBDIRS += mozilla-sdk

if WIN32
SUBDIRS += win32
endif
endif # NSAPI

# if DMALLOC
# MALL_CFLAGS = $(DMALLOC_CFLAGS)
# MALL_LIBS = $(DMALLOC_LIBS)
# endif

plugindir   = $(FIREFOX_PLUGINS)

# XP_UNIX	This needs to get defined for npapi.h on unix platforms.
# PLUGIN_TRACE	Enable this define to get debug prints whenever the plugin
#		api gets control.
AM_CPPFLAGS = -DPLUGIN_TRACE -DXP_UNIX -DMOZ_X11 -DGNASHBINDIR=\"${bindir}\" \
        -I.. \
        -I$(srcdir) \
        -I$(top_srcdir)/server \
        -I$(top_srcdir)/server/parser \
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
if NSAPI

plugin_LTLIBRARIES = libgnashplugin.la

libgnashplugin_la_SOURCES  = plugin.cpp \
	mozilla-sdk/npn_gate.cpp \
        mozilla-sdk/npp_gate.cpp \
        mozilla-sdk/np_entry.cpp

libgnashplugin_la_LIBADD   = \
	$(NULL)

if PLUGIN_LINK_UNDEFINED
libgnashplugin_la_LDFLAGS = -avoid-version -L$(plugindir)
else
libgnashplugin_la_LDFLAGS = -module -avoid-version -L$(plugindir)
endif
endif # NSAPI
endif # WIN32

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

install-plugin: $(plugin_LTLIBRARIES)
	@test -d "$(DESTDIR)$(plugindir)" || $(mkinstalldirs) "$(DESTDIR)$(plugindir)"
	@echo "Installing plugin to $(DESTDIR)$(plugindir)"
	@if test -d .libs; then \
	  cp -f .libs/libgnashplugin.so* $(DESTDIR)$(plugindir)/libgnashplugin.so; \
	else \
	  cp -f libgnashplugin.so* $(DESTDIR)$(plugindir)/libgnashplugin.so; \
	fi

install-plugins: install-plugin
	$(MAKE) -C klash install-plugins

uninstall-pluginLTLIBRARIES: $(plugin_LTLIBRARIES)
	$(RM) -f $(DESTDIR)$(plugindir)/libgnashplugin.so

.PHONY : install-plugins install-plugin test_perms

