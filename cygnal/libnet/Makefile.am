# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
#   Free Software Foundation, Inc.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
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

## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = dejagnu

pkglib_LTLIBRARIES = libgnashnet.la

AM_LDFLAGS = \
	$(top_builddir)/libbase/libgnashbase.la \
	../libamf/libgnashamf.la \
	$(SSH_LIBS) \
	$(SSL_LIBS) \
	$(GLIB_LIBS) \
	$(CURL_LIBS) \
	$(LIBADD_DL) \
	$(BOOST_LIBS) \
	$(BOOST_CYGNAL_LIBS) \
	$(NULL)

libgnashnet_la_LDFLAGS =  -release $(VERSION) $(AM_LDFLAGS)

localedir = $(datadir)/locale

AM_CPPFLAGS = \
	-I$(top_srcdir)/cygnal/libamf \
	-I$(top_srcdir)/cygnal/libnet \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/libcore \
        -DLOCALEDIR=\"$(localedir)\" \
	$(SSL_CFLAGS) \
	$(SSH_CFLAGS) \
	$(CURL_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(PTHREAD_CFLAGS) \
	$(NULL)

noinst_HEADERS = \
	cque.h \
	lirc.h \
	http.h \
	network.h \
	netstats.h \
	rtmp.h \
	rtmp_msg.h \
	rtmp_client.h \
	statistics.h \
	diskstream.h \
	cache.h

libgnashnet_la_SOURCES = \
	cque.cpp \
	lirc.cpp \
	http.cpp \
	network.cpp \
	netstats.cpp \
	rtmp.cpp \
	rtmp_msg.cpp \
	rtmp_client.cpp \
	statistics.cpp \
	diskstream.cpp \
	cache.cpp

if BUILD_SSL
libgnashnet_la_SOURCES += sslclient.cpp sslserver.cpp
noinst_HEADERS += sslclient.h sslserver.h
endif

if BUILD_SSH
libgnashnet_la_SOURCES += sshclient.cpp sshserver.cpp
noinst_HEADERS += sshclient.h sshserver.h
endif

if WIN32
  AM_LDFLAGS += -no-undefined
endif

if ENABLE_PCH
AM_CXXFLAGS = $(PCH_FLAGS)
endif

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) $(MUDFLAP_OPT)" LIBS="$(LIBS) $(MUDFLAP_LIB)"

# Remove libtool .la files
install-exec-hook:
	$(RM) $(DESTDIR)$(libdir)/gnash/libgnashnet.la

uninstall-local:
	$(RM) $(DESTDIR)$(libdir)/gnash/libgnashnet-*.so
