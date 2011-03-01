## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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

# make sure these files make it into the distribution tarball.
EXTRA_DIST = \
	mozilla-sdk/np_entry.cpp \
	mozilla-sdk/npn_gate.cpp \
	mozilla-sdk/npp_gate.cpp \
	mozilla-sdk/npplat.h \
	mozilla-sdk/pluginbase.h

# This is a hard coded path to where the plugin gets installed.
GNASHBINDIR = $(bindir)

# This is the directory the plugin gets installed.
plugindir   = $(FIREFOX_PLUGINS)

# XP_UNIX	This needs to get defined for npapi.h on unix platforms.
# PLUGIN_TRACE	Enable this define to get debug prints whenever the plugin
#		api gets control.
AM_CPPFLAGS = -DPLUGIN_TRACE -DGNASHBINDIR=\"$(GNASHBINDIR)\" \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
        -I$(top_srcdir)/libcore \
        -I$(top_srcdir)/libcore/parser \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/librender \
	-I$(srcdir)/mozilla-sdk \
	$(MOZPLUGIN_CFLAGS) \
	$(X11_CFLAGS) \
        $(GLIB_CFLAGS) \
	$(NULL)

if HAIKU
  AM_CPPFLAGS += -DXP_HAIKU
else
if AMIGAOS4
  AM_CPPFLAGS += -DXP_AMIGAOS4
else
  AM_CPPFLAGS += -DXP_UNIX -DMOZ_X11
endif
endif

# AM_CXXFLAGS = $(AM_CPPFLAGS)

noinst_HEADERS = plugin.h \
	 mozilla-sdk/pluginbase.h \
	 mozilla-sdk/npplat.h

if !WIN32
if !AMIGAOS4
plugin_LTLIBRARIES = libgnashplugin.la

libgnashplugin_la_SOURCES  = plugin.cpp \
	mozilla-sdk/npn_gate.cpp \
        mozilla-sdk/npp_gate.cpp \
        mozilla-sdk/np_entry.cpp

libgnashplugin_la_LIBADD   = \
	$(GLIB_LIBS) \
	$(NULL)

# Scriptable plugin support
check_PROGRAMS = test
test_LDADD  = \
	$(GLIB_LIBS) \
	$(NULL)
test_SOURCES = test.cpp \
	external.cpp
test_CPPFLAGS = -I$(top_srcdir)/testsuite $(AM_CPPFLAGS) 

noinst_HEADERS += pluginScriptObject.h \
	callbacks.h \
	external.h \
	GnashNPVariant.h
libgnashplugin_la_SOURCES += pluginScriptObject.cpp \
	callbacks.cpp \
	external.cpp

check:
	./test

if PLUGIN_LINK_UNDEFINED
libgnashplugin_la_LDFLAGS = -avoid-version -L$(plugindir)
else
libgnashplugin_la_LDFLAGS = -module -avoid-version -L$(plugindir)
endif

if VISIBILITY_WORKS
libgnashplugin_la_CXXFLAGS = -fvisibility=hidden
endif

endif # AMIGAOS4
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
	$(RM) $(DESTDIR)$(plugindir)/*.a $(DESTDIR)$(plugindir)/*.la

if !WIN32
if !AMIGAOS4
install-plugin install-plugins: $(plugin_LTLIBRARIES) $(PKGLIB)
	@test -d "$(DESTDIR)$(plugindir)" || $(mkinstalldirs) "$(DESTDIR)$(plugindir)"
	@echo "Installing plugin to $(DESTDIR)$(plugindir)"
	@if test -d .libs; then \
	  cp -f .libs/libgnashplugin.so* $(DESTDIR)$(plugindir)/libgnashplugin.so; \
	else \
	  cp -f libgnashplugin.so* $(DESTDIR)$(plugindir)/libgnashplugin.so; \
	fi
else
install-plugin install-plugins: 
	: # do nothing
endif	# end of if not amigaos4
else
install-plugin install-plugins: 
	: # do nothing
endif	# end of if not windoze

uninstall-plugin uninstall-plugins: 
	$(RM) -f $(DESTDIR)$(plugindir)/libgnashplugin.so

.PHONY : install-plugins install-plugin test_perms

