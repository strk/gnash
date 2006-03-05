## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#

AUTOMAKE_OPTIONS = dejagnu

if KLASH
KLASH = klash
endif
SUBDIRS = mozilla-sdk $(KLASH) # test

if MP3
MP3_CFLAGS = $(MAD_CFLAGS)
MP3_LIBS = $(MAD_LIBS)
endif

# if DMALLOC
# MALL_CFLAGS = $(DMALLOC_CFLAGS)
# MALL_LIBS = $(DMALLOC_LIBS)
# endif

plugindir   = $(FIREFOX_PLUGINS)

INCLUDES = -I.. \
        -I$(srcdir)		\
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/libgeometry \
        -I$(top_srcdir)/server	\
	-I$(srcdir)/mozilla-sdk \
	-I$(srcdir)/mozilla-sdk/include \
	$(LIBXML_CFLAGS)	\
	$(GLEXT_CFLAGS)		\
	$(GTK2_CFLAGS)		\
	$(PANGO_CFLAGS)		\
	$(GLIB_CFLAGS)		\
	$(CAIRO_CFLAGS)		\
	$(ATK_CFLAGS)		\
	$(SDL_CFLAGS)		\
	$(SDL_MIXER_CFLAGS)	\
	$(X_CFLAGS)		\
	$(DMALLOC_CFLAGS)	\
	$(MP3_CFLAGS)		\
	$(OGG_CFLAGS)


AM_CXXFLAGS = $(INCLUDES)
# XP_UNIX	This needs to get defined for npapi.h on unix platforms.
# PLUGIN_TRACE	Enable this define to get debug prints whenever the plugin
#		api gets control.
AM_CPPFLAGS = -Wall -DPLUGIN_TRACE -DXP_UNIX -DMOZ_X11
AM_LDFLAGS  = $(X_LIBS) -lX11 -lXi -lXmu \
	$(SDL_LIBS)	\
	$(SDL_MIXER_LIBS)\
	$(OPENGL_LIBS)	\
	$(GLEXT_LIBS)	\
	$(GTK2_LIBS)	\
	$(PANGO_LIBS)	\
	$(CAIRO_LIBS)	\
	$(ATK_LIBS)	\
	$(GLIB_LIBS)	\
	$(LIBXML_LIBS)	\
	$(JPEG_LIBS)	\
	$(PNG_LIBS)	\
	$(DMALLOC_LIBS)	\
	$(OGG_LIBS)	\
	$(DMALLOC_LIBS)

# 	$(X_LIBS)\
# 	$(X_EXTRA_LIBS)

plugin_LTLIBRARIES = libgnashplugin.la

noinst_HEADERS = plugin.h player.h

libgnashplugin_la_SOURCES  = plugin.cpp player.cpp
libgnashplugin_la_LIBADD   = $(AM_LDFLAGS) \
	mozilla-sdk/libmozsdk.la\
	../backend/libgnashbackend.la\
	../server/libgnashasobjs.la \
	../server/libgnashserver.la\
	../libgeometry/libgnashgeo.la\
	../libbase/libgnashbase.la
libgnashplugin_la_LDFLAGS = -module -avoid-version -no-undefined -L$(plugindir)

# For now the plugin has to be installed in the root plugin
# directory for Firefox. For some reason if the same file is
# installed in the users $(HOME)/.firefox.plugins, the plugin
# won't load due to errors with an undefined MaiAtkObject.
install-pluginLTLIBRARIES: $(plugin_LTLIBRARIES)
	test -d "$(plugindir)" || mkdir "$(DESTDIR)$(plugindir)"
#       cp .libs/libgnashplugin.so $(DESTDIR)$(plugindir)/libgnashplugin.so
	$(LIBTOOL) --mode=install $(INSTALL) $(INSTALL_STRIP_FLAG) $(plugin_LTLIBRARIES) "$(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)"
	$(RM) $(DESTDIR)$(plugindir)/libgnashplugin.*a


# install-pluginLTLIBRARIES: $(plugin_LTLIBRARIES)
# 	if test -d $(HOME)/.firefox ; then \
# 	   $(mkinstalldirs) $(HOME)/.firefox/plugins; \
# 	   $(INSTALL) .libs/libgnash.so $(HOME)/.firefox/plugins/libgnash.so; \
# 	else \
# 	    $(mkinstalldirs) $(HOME)/.mozilla/plugins; \
# 	   $(INSTALL) .libs/libgnash.so $(HOME)/.mozilla/plugins/libgnash.so; \
# 	fi

uninstall-pluginLTLIBRARIES: $(plugin_LTLIBRARIES)
	rm -f $(DESTDIR)$(plugindir)/libgnashplugin.so
