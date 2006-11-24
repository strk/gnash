## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

# 
#

# $Id: Makefile.am,v 1.2 2006/11/24 11:49:18 strk Exp $

AUTOMAKE_OPTIONS = 

AM_CPPFLAGS = 

noinst_LTLIBRARIES = libgnashvm.la 

INCLUDES = \
	-I$(top_srcdir)/backend \
	-I$(top_srcdir)/server \
	-I$(top_srcdir)/server/asobj \
	-I$(top_srcdir)/server/swf \
	-I$(top_srcdir)/server/parser \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/libamf \
        -I$(top_srcdir)	\
	$(PTHREAD_CFLAGS) \
	$(DMALLOC_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(LIBXML_CFLAGS) \
	$(FFMPEG_CFLAGS) \
	$(NULL)

libgnashvm_la_SOURCES = \
	ASHandlers.cpp \
	ActionExec.cpp \
	VM.cpp		\
        action.cpp \
	$(NULL)

noinst_HEADERS =		\
	ASHandlers.h		\
	ActionExec.h		\
	VM.h			\
	action.h		\
	fn_call.h		\
	with_stack_entry.h 	\
	$(NULL)

libgnashvm_la_LIBADD = \
	$(top_builddir)/libamf/libgnashamf.la \
	$(top_builddir)/libbase/libgnashbase.la \
	$(FFMPEG_LIBS)

libgnashvm_la_LDFLAGS = -avoid-version

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

