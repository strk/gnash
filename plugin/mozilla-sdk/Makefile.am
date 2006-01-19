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

sdkdir   = $(FIREFOX_PLUGINS)

INCLUDES = -I.. \
        -I$(srcdir) \
	$(FIREFOX_CFLAGS)

AM_CXXFLAGS = $(CFLAGS) $(INCLUDES)
AM_CPPFLAGS = -Wall  -DPLUGIN_TRACE -DXP_UNIX $(FIREFOX_DEFS)
# AM_LDFLAGS  = $(FIREFOX_LIBS)

sdk_LTLIBRARIES = libmozsdk.la
noinst_HEADERS = npplat.h pluginbase.h
libmozsdk_la_SOURCES = \
	npn_gate.cpp \
        npp_gate.cpp \
        np_entry.cpp

install-sdkLTLIBRARIES: $(sdk_LIBRARIES)
	cp .libs/libmozsdk.so $(sdkdir)/libgnash.so

uninstall-sdkLTLIBRARIES: $(sdk_LIBRARIES)
	rm -f $(sdkdir)/libmozsdk.so
