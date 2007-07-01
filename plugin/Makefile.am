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

# $Id: Makefile.am,v 1.60 2007/07/01 10:54:14 bjacques Exp $

AUTOMAKE_OPTIONS = 

if KLASH
KLASH = klash
endif
SUBDIRS = $(KLASH) # test mozilla-sdk

# if DMALLOC
# MALL_CFLAGS = $(DMALLOC_CFLAGS)
# MALL_LIBS = $(DMALLOC_LIBS)
# endif

plugindir   = $(FIREFOX_PLUGINS)

# XP_UNIX	This needs to get defined for npapi.h on unix platforms.
# PLUGIN_TRACE	Enable this define to get debug prints whenever the plugin
#		api gets control.
AM_CPPFLAGS = -DPLUGIN_TRACE -DXP_UNIX -DMOZ_X11 -DGNASHBINDIR=\"$(GNASHEXE)\" \
        -I.. \
        -I$(srcdir) \
        -I$(top_srcdir)/server \
        -I$(top_srcdir)/server/parser \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/backend \
	-I$(srcdir)/mozilla-sdk \
	-I$(srcdir)/mozilla-sdk/include \
	$(GLIB_CFLAGS) \
	$(GTK2_CFLAGS) \
        $(CAIRO_CFLAGS) \
        $(GLIB_CFLAGS) \
        $(ATK_CFLAGS) \
        $(PANGO_CFLAGS) \
	$(NULL)

# AM_CXXFLAGS = $(AM_CPPFLAGS)

plugin_LTLIBRARIES = libgnashplugin.la

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
         mozilla-sdk/include/prlock.h \
         mozilla-sdk/include/prcvar.h \
         mozilla-sdk/include/prthread.h \
         mozilla-sdk/include/prinit.h \
         mozilla-sdk/include/prerr.h \
         mozilla-sdk/include/prerror.h \
         mozilla-sdk/include/prinrval.h \
	 mozilla-sdk/include/prwin16.h \
         mozilla-sdk/include/obsolete/protypes.h 

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

# The default directory for the plugin is $(HOME)/.firefox/plugins.
install-pluginLTLIBRARIES: $(plugin_LTLIBRARIES)
	test -d "$(DESTDIR)$(plugindir)" || $(mkinstalldirs) "$(DESTDIR)$(plugindir)"
#	cp .libs/libgnashplugin.so $(DESTDIR)$(plugindir)/libgnashplugin.so
	$(LIBTOOL) --mode=install $(INSTALL) $(INSTALL_STRIP_FLAG) $(plugin_LTLIBRARIES) "$(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)"
#if PLUGIN_LINK_UNDEFINED
#cp $(DESTDIR)$(plugindir)/.libs/libgnashplugin.so.0.* $(DESTDIR)$(plugindir)/libgnashplugin.so
#endif
	$(RM) $(DESTDIR)$(plugindir)/libgnashplugin.*a $(DESTDIR)$(plugindir)/libgnashplugin.so.0 $(DESTDIR)$(plugindir)/libgnashplugin.*.T

# install-pluginLTLIBRARIES: $(plugin_LTLIBRARIES)
# 	if test -d $(HOME)/.firefox ; then \
# 	   $(mkinstalldirs) $(HOME)/.firefox/plugins; \
# 	   $(INSTALL) .libs/libgnash.so $(HOME)/.firefox/plugins/libgnash.so; \
# 	else \
# 	    $(mkinstalldirs) $(HOME)/.mozilla/plugins; \
# 	   $(INSTALL) .libs/libgnash.so $(HOME)/.mozilla/plugins/libgnash.so; \
# 	fi

uninstall-pluginLTLIBRARIES: $(plugin_LTLIBRARIES)
	$(RM) -f $(DESTDIR)$(plugindir)/libgnashplugin.so
