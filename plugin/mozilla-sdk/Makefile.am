## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

AUTOMAKE_OPTIONS =

AM_CPPFLAGS = -DPLUGIN_TRACE

if WIN32
  AM_CPPFLAGS += -DXP_WIN -D_WINDOWS
  AM_LDFLAGS = -static -no-undefined
else
  AM_CPPFLAGS += -DXP_UNIX
endif

AM_CPPFLAGS += \
	-I$(srcdir)/include \
	-I$(top_srcdir)/libbase

# AM_CXXFLAGS = $(CXXFLAGS) $(AM_CPPFLAGS)
# AM_CPPFLAGS = -DPLUGIN_TRACE -DXP_UNIX # $(FIREFOX_DEFS)
# AM_LDFLAGS  = $(FIREFOX_LIBS)

# noinst_LTLIBRARIES = libmozsdk.la
pkglib_LTLIBRARIES = libmozsdk.la

noinst_HEADERS = \
	npplat.h \
	pluginbase.h \
	include/jri.h \
	include/npapi.h \
	include/npupp.h \
	include/prtypes.h \
	include/jni.h \
	include/jri_md.h \
	include/npruntime.h \
	include/jni_md.h \
	include/jritypes.h \
	include/nptypes.h \
	include/prcpucfg.h \
	include/prcpucfg-glibc.h \
	include/prcpucfg-win32.h \
	include/prlock.h \
	include/prcvar.h \
	include/prthread.h \
	include/prinit.h \
	include/prerr.h \
	include/prerror.h \
	include/prinrval.h \
	include/prwin16.h \
	include/obsolete/protypes.h 

libmozsdk_la_SOURCES = \
	npn_gate.cpp \
	npp_gate.cpp \
	np_entry.cpp
