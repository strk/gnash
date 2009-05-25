## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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
	-I$(top_srcdir)/libnet \
	-I$(top_srcdir)/backend \
	-I$(top_srcdir)/libcore \
	-I$(top_srcdir)/libcore/asobj \
	-I$(top_srcdir)/libcore/asobj/flash \
	-I$(top_srcdir)/libcore/swf \
	-I$(top_srcdir)/libcore/parser \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/libamf \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libsound \
	-I$(top_srcdir)/libltdl \
	$(GLIB_CFLAGS) \
	$(PTHREAD_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(FFMPEG_CFLAGS) \
	$(NULL)

libgnashvm_la_SOURCES = \
	ASHandlers.cpp \
	ActionExec.cpp \
	VM.cpp		\
	action.cpp \
	CallStack.cpp \
	$(NULL)

if ENABLE_AVM2
libgnashvm_la_SOURCES += \
	CodeStream.cpp \
	Machine.cpp
	$(NULL)
endif

noinst_HEADERS = \
	ASHandlers.h \
	CodeStream.h \
	asName.h \
	ActionExec.h \
	ExecutableCode.h \
	action.h \
	with_stack_entry.h \
	$(NULL)

EXTENSIONS_API = \
	fn_call.h \
	CallStack.h \
	SafeStack.h \
	VM.h \
	Machine.h \
	$(NULL)

if ENABLE_AVM2
noinst_HEADERS += \
	asName.h \
	CodeStream.h \
	Machine.h \
	$(NULL)
endif

if SDKINSTALL
noinst_HEADERS += $(EXTENSIONS_API)
else
instdir = $(includedir)/gnash
inst_HEADERS = $(EXTENSIONS_API)
endif

libgnashvm_la_LIBADD = \
	$(top_builddir)/libbase/libgnashbase.la 

if USE_FFMPEG_ENGINE
	libgnashvm_la_CPPFLAGS += $(FFMPEG_CFLAGS)
endif

libgnashvm_la_LDFLAGS = -export-dynamic # -release $(VERSION)

if WIN32
  libgnashvm_la_LDFLAGS += -no-undefined
endif

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

