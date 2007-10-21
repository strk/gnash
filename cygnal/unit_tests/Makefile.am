# 
#   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
# $Id: Makefile.am,v 1.2 2007/10/21 20:24:00 eh9 Exp $

#----------------------------------------
# Makefile.am for Cygnal/unit_tests 
#----------------------------------------

AUTOMAKE_OPTIONS = #dejagnu

AM_CPPFLAGS = # -Wall

## WARNING: make sure GLIB_LIBS appears first
## See: http://lists.gnu.org/archive/html/gnash-dev/2006-07/msg00076.html
AM_LDFLAGS = \
	$(GLIB_LIBS) \
	$(BOOST_LIBS)

localedir = $(datadir)/locale

INCLUDES = -I$(top_srcdir)/cygnal	\
	-DLOCALEDIR=\"$(localedir)\" \
	$(BOOST_CFLAGS)

lib_LTLIBRARIES = libcygnal_test.la
libcygnal_test_la_SOURCES= \
	Random_Permutation.cpp \
	Test_Support.cpp \
	main.cpp

noinst_HEADERS = \
	Permutation.hpp \
	Random_Permutation.hpp \
	Test_Support.hpp

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) $(MUDFLAP_OPT)" LIBS="$(LIBS) $(MUDFLAP_LIB)"
