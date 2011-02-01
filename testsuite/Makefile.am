# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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
# 

AUTOMAKE_OPTIONS = 

DIR_MING=

if ENABLE_MING
DIR_MING += misc-ming.all actionscript.all
endif

if HAS_NETCAT
# don't build the network tests yet till they're fully implemented
DIR_MING += # network.all
endif

if ENABLE_MTASC
DIR_MTASC = misc-mtasc.all
endif

if ENABLE_HAXE
DIR_HAXE = misc-haxe.all
endif

if ENABLE_SWFMILL
DIR_SWFMILL = misc-swfmill.all
endif

if ENABLE_SWFC
DIR_SWFC = misc-swfc.all
endif

if ENABLE_SWFDEC_TESTSUITE
DIR_SWFDEC_TESTSUITE = swfdec
endif

if ENABLE_AVM2
if ENABLE_AS3COMPILE 
DIR_AS3 = as3compile.all
endif
endif

DIST_SUBDIRS = \
	media \
	as3compile.all \
	actionscript.all \
	libbase.all	\
	libcore.all \
	libmedia.all \
	network.all \
	samples	\
	swfdec \
	misc-ming.all \
	misc-mtasc.all \
	misc-haxe.all \
	misc-swfmill.all \
	misc-swfc.all \
	movies.all \
	$(NULL)

SUBDIRS = \
	.	\
	media \
	$(DIR_AS3) \
	$(DIR_SWFDEC_TESTSUITE) \
	samples	\
	$(DIR_MING) \
	$(DIR_MTASC) \
	$(DIR_HAXE) \
	$(DIR_SWFMILL) \
	$(DIR_SWFC) \
	movies.all \
	libbase.all	\
	libmedia.all \
	libcore.all \
	$(NULL)

EXTRA_DIST = check.h \
	DummyMovieDefinition.h \
	DummyCharacter.h \
	gnashrc.in \
	simple.exp \
	analyse-results.sh \
	generic-testrunner.sh \
	gst_install_plugins_helper.sh \
	simultaneous-testrunner.sh


dist_noinst_SCRIPTS = analyse-results.sh generic-testrunner.sh

abs_mediadir = `cd $(srcdir)/media; pwd`
abs_tmpsodir = `cd tmpSharedObject; pwd`

# could not find a way to only build this on make check
check_LTLIBRARIES = libtestsuite.la 

libtestsuite_la_SOURCES = \
	MovieTester.h \
	MovieTester.cpp \
	FuzzyPixel.cpp \
	FuzzyPixel.h \
	$(NULL)
libtestsuite_la_LIBADD = \
	$(RENDERER_LIBS) \
	$(top_builddir)/libmedia/libgnashmedia.la \
	$(top_builddir)/libsound/libgnashsound.la \
	$(top_builddir)/libcore/libgnashcore.la \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/librender/libgnashrender.la \
	$(NULL)

if LIBLTDL2
libtestsuite_la_LIBADD += $(top_builddir)/libltdl/libltdlc.la
endif
if LIBLTDL1
libtestsuite_la_LIBADD += $(top_builddir)/libbase/libltdlc.la
endif

libtestsuite_la_CXXFLAGS = \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libsound \
	-I$(top_srcdir)/librender \
	-I$(top_srcdir)/librender/agg \
	-I$(top_srcdir)/librender/cairo \
	-I$(top_srcdir)/librender/opengl \
	-I$(top_srcdir)/libcore  \
	-I$(top_srcdir)/libcore/asobj  \
	-I$(top_srcdir)/libcore/swf  \
	-I$(top_srcdir)/libcore/parser  \
	-I$(top_srcdir)/libcore/vm  \
	$(BOOST_CFLAGS) \
	$(CAIRO_CFLAGS) \
	$(NULL)

tmpSharedObject: 
	mkdir tmpSharedObject

gnashrc: gnashrc.in Makefile tmpSharedObject
	sed -e 's#@@MEDIADIR@@#'$(abs_mediadir)'#g' -e 's#@@TMPSODIR@@#'$(abs_tmpsodir)'#g' $(srcdir)/gnashrc.in > $@

if ENABLE_MING

.PHONY: Dejagnu.swf
Dejagnu.swf:
	$(MAKE) -C misc-ming.all $@
	cp misc-ming.all/Dejagnu.swf $@

check-local: gnashrc Dejagnu.swf
else
check-local: gnashrc
endif

clean-local:
	rm -f Dejagnu.swf gnashrc
	rm -rf tmpSharedObject

