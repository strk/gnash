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

AUTOMAKE_OPTIONS = 

# this is where Gnash plugins get installed
pluginsdir = $(libdir)/gnash/plugins

plugins_LTLIBRARIES = fileio.la

DEF_MAKESWF_FLAGS = -I $(top_srcdir)/testsuite/actionscript.all

INCLUDES = -I$(srcdir)  \
            -I$(top_srcdir)/libbase \
	    -I$(top_srcdir)/libgeometry \
            -I$(top_srcdir)/server \
            -I$(top_srcdir)/server/vm \
	    -I$(top_srcdir)/server/parser \
	    -I$(top_srcdir)/server/asobjs \
	    $(INCLTDL)

AM_LDFLAGS =  \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/server/vm/libgnashvm.la \
	$(top_builddir)/backend/libgnashbackend.la \
	$(top_builddir)/server/libgnashserver.la \
	$(PTHREAD_LIBS) \
	$(NULL)

fileio_la_SOURCES = fileio.cpp fileio.h
fileio_la_LDFLAGS = -module -avoid-version -no-undefined
fileio_la_LIBDADD = $(LIBADD_DL) $(LIBLTDL) $(AM_LDFLAGS)


check_PROGRAMS = test
CLEANFILES = \
	gnash-dbg.log \
	test.swf.frame0.pp \
	test.swf \
	logfile

install-pluginsLTLIBRARIES: $(plugins_LTLIBRARIES)
	test -d "$(DESTDIR)$(pluginsdir)" || $(mkinstalldirs) "$(DESTDIR)$(pluginsdir)"
	$(LIBTOOL) --mode=install $(INSTALL) $(INSTALL_STRIP_FLAG) $(plugins_LTLIBRARIES) "$(DESTDIR)$(pluginsdir)/$(plugins_LTLIBRARIES)"
	$(RM) $(DESTDIR)$(pluginsdir)/*.a 

test: test.swf
	../../utilities/gprocessor test.swf

test.swf: test.as

# Wrap a single .as tests in dejagnu_so importer.
# At play time, if the relative url '../ming-misc.all/Dejagnu.swf' 
# takes to the Dejagnu.swf shared library we'll get visual traces,
# otherwise normal traces will be used.
SUFFIXES = as swf
.as.swf: 
	$(MAKESWF) $(DEF_MAKESWF_FLAGS)	$(MAKESWF_FLAGS) -o $@ $<
