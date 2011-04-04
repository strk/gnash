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

noinst_LTLIBRARIES = libgnashvm.la 

AM_CPPFLAGS = \
	-I$(top_srcdir)/librender \
	-I$(top_srcdir)/libdevice \
	-I$(top_srcdir)/libcore \
	-I$(top_srcdir)/libcore/asobj \
	-I$(top_srcdir)/libcore/asobj/flash \
	-I$(top_srcdir)/libcore/swf \
	-I$(top_srcdir)/libcore/abc \
	-I$(top_srcdir)/libcore/parser \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libsound \
	$(PTHREAD_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(NULL)

if BUILD_EGL_DEVICE
AM_CPPFLAGS += $(EGL_CFLAGS)
endif
if BUILD_DIRECTFB_DEVICE
AM_CPPFLAGS += $(DIRECTFB_CFLAGS)
endif

libgnashvm_la_SOURCES = \
	ASHandlers.cpp \
	ActionExec.cpp \
	VM.cpp		\
	CallStack.cpp \
	$(NULL)

if ENABLE_AVM2
libgnashvm_la_SOURCES += \
	Machine.cpp
	$(NULL)
endif

inst_HEADERS = \
	ASHandlers.h \
	ActionExec.h \
	ExecutableCode.h \
	$(NULL)

EXTENSIONS_API = \
	fn_call.h \
	CallStack.h \
	SafeStack.h \
	VM.h \
	$(NULL)

if ENABLE_AVM2
inst_HEADERS += \
	Machine.h \
	$(NULL)
endif

instdir = $(includedir)/gnash/vm
inst_HEADERS += $(EXTENSIONS_API)

libgnashvm_la_LIBADD = \
	$(top_builddir)/libbase/libgnashbase.la 

libgnashvm_la_LDFLAGS = -export-dynamic # -release $(VERSION)

if WIN32
  libgnashvm_la_LDFLAGS += -no-undefined
endif

if ENABLE_PCH
AM_CXXFLAGS = $(PCH_FLAGS)
endif

