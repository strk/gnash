## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2008, 2009, 2010 Free Software Foundation, Inc.
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

IDL_OUTPUT = \
	iGnashComponent.xpt \
	iGnashComponent.h

EXTRA_DIST = iGnashComponent.idl

INCLUDES = \
        -DLOCALEDIR=\"$(localedir)\" \
        $(XPCOM_CFLAGS)

AM_LDFLAGS = \
	$(XPCOM_LIBS)

AM_CXXFLAGS=-fshort-wchar

#GECKO_INCLUDES = -I/usr/include/xulrunner-sdk-1.9/stable -I/usr/include/nspr4 -I/usr/include/xulrunner-sdk-1.9/

#GECKO_LDFLAGS =  -L/usr/lib/xulrunner-sdk-1.9/sdk/lib -lxpcomglue \
                 -lnspr4      \
                 -lplds4     

BUILT_SOURCES = $(IDL_OUTPUT)

pkglib_LTLIBRARIES = libgnashcomponent.la
libgnashcomponent_la_SOURCES = GnashComponent.cpp GnashComponentModule.cpp
libgnashcomponent_la_CFLAGS = -DXPCOM_GLUE -fno-rtti -fno-exceptions
libgnashcomponent_la_LDFLAGS = -avoid-version -module -no-undefined \
	$(XPCOM_LIBS)

noinst_HEADERS = GnashComponent.h

CLEANFILES = \
	$(IDL_OUTPUT) \
	*.loT 

# Build the output files from the IDL file
SUFFIXES = idl xpt
.idl.h:
	$(XPIDL) -m header  -v $(XPCOM_IDL_CFLAGS) $<
.idl.xpt:
	$(XPIDL) -m typelib -v $(XPCOM_IDL_CFLAGS) $<

EXT_DIR = "`dirname $(FIREFOX_PLUGINS)/extensions/{2b70f2b1-fc72-4734-bb81-4eb2a7713e49}`"

install-pkglib: iGnashComponent.h $(pkglib_LTLIBRARIES)
	@test -d "$(DESTDIR)$(EXT_DIR)" || $(mkinstalldirs) "$(DESTDIR)$(EXT_DIR)"
	@if test -d .libs; then \
	  cp -f .libs/libgnashcomponent.so* $(DESTDIR)$(EXT_DIR)/libgnashcomponent.so; \
	  cp -f iGnashComponent.xpt $(DESTDIR)$(EXT_DIR)/iGnashComponent.xpt; \
	fi

force:
