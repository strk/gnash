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

if NPAPI
if AMIGAOS4

AM_CPPFLAGS = \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libsound \
	-I$(top_srcdir)/libmedia/ffmpeg \
	-I$(top_srcdir)/librender \
	-I$(top_srcdir)/libcore \
	-I$(top_srcdir)/libcore/parser \
	-I$(top_srcdir)/libcore/vm \
	-I$(top_srcdir)/libcore/swf \
	-I$(top_srcdir)/gui \
	-I$(top_srcdir)/plugin/npapi/mozilla-sdk \
	-I$(top_srcdir)/plugin/npapi/mozilla-sdk/include \
	-I$(top_srcdir)/plugin/npapi \
	$(AGG_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(SDL_CFLAGS) \
	$(NULL)

AM_LDFLAGS = \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/libmedia/libgnashmedia.la \
	$(top_builddir)/libsound/libgnashsound.la \
	$(top_builddir)/librender/libgnashrender.la \
	$(top_builddir)/libcore/libgnashcore.la \
	$(top_builddir)/plugin/mozilla-sdk/np_entry.lo \
	$(top_builddir)/plugin/mozilla-sdk/npn_gate.lo \
	$(top_builddir)/plugin/mozilla-sdk/npp_gate.lo \
	$(AGG_LIBS) \
	$(BOOST_LIBS) \
	$(CURL_LIBS) \
	$(EXPAT_LIBS) \
	$(JPEG_LIBS) \
	$(PNG_LIBS) \
	$(SDL_LIBS) \
	$(NSPR_LIBS) \
	$(NULL)

plugindir = $(bindir)

plugin_LTLIBRARIES = libnpgnash.la

libnpgnash_la_SOURCES = plugin.cpp plugin.h
libnpgnash_la_LDFLAGS = -shared -no-undefined -version-info 0:0:0 \
	-o libnpgnash.so 

install-pluginLTLIBRARIES: $(plugin_LTLIBRARIES)
	@echo "Nothing to install here now."
	@echo "Use \"make install-plugin\" to install the npgnash plugin to $(bindir)."

install-plugin install-plugins: $(plugin_LTLIBRARIES)
	test -d $(bindir) || $(mkinstalldirs) $(bindir)
	$(INSTALL_PROGRAM) libnpgnash.so $(bindir)

uninstall-plugin uninstall-plugins: 
	$(RM) -f $(bindir)/libnpgnash.so

endif # AMIGAOS4
endif # NPAPI

force:

.PHONY : install-plugins install-plugin uninstall-plugin uninstall-plugins
