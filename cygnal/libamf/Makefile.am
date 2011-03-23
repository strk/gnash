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

pkglib_LTLIBRARIES = libgnashamf.la
libgnashamf_la_SOURCES = \
	amf.cpp \
	amf_msg.cpp \
	buffer.cpp \
	element.cpp \
	sol.cpp \
	lcshm.cpp \
	flv.cpp

libgnashamf_la_LIBADD = \
	$(top_builddir)/libbase/libgnashbase.la

libgnashamf_la_LDFLAGS =  -release $(VERSION) ${AM_LDFLAGS} 

if WIN32
  libgnashamf_la_LDFLAGS += -no-undefined
  libgnashamf_la_LIBADD += -lws2_32 -lintl
endif

# noinst_HEADERS = amf.h amfutf8.h

AM_CPPFLAGS = \
	-I$(top_srcdir)/cygnal/libamf \
	-I$(top_srcdir)/cygnal/libnet \
	-I$(top_srcdir)/libcore \
	-I$(top_srcdir)/libbase \
	$(BOOST_CFLAGS) \
	$(PTHREAD_CFLAGS)

AM_LDFLAGS = $(BOOST_LIBS)

noinst_HEADERS = \
	amfutf8.h \
	amf.h \
	amf_msg.h \
	lcshm.h \
	buffer.h \
	element.h \
	flv.h \
	protocol.h \
	sol.h

if ENABLE_PCH
AM_CXXFLAGS = $(PCH_FLAGS)
endif

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

# Remove libtool .la files
install-exec-hook:
	$(RM) $(DESTDIR)$(libdir)/gnash/libgnashamf.la

uninstall-local:
	$(RM) $(DESTDIR)$(libdir)/gnash/libgnashamf-*.so
