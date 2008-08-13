# 
#   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

if ENABLE_MING
DIR_MING = misc-ming.all actionscript.all
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

DIST_SUBDIRS = \
	actionscript.all \
	libbase	\
	libamf.all \
	libnet.all \
	libcore.all \
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
	$(DIR_SWFDEC_TESTSUITE) \
	libnet.all \
	libbase	\
	libamf.all \
	libcore.all \
	samples	\
	$(DIR_MING) \
	$(DIR_MTASC) \
	$(DIR_HAXE) \
	$(DIR_SWFMILL) \
	$(DIR_SWFC) \
	movies.all \
	$(NULL)

EXTRA_DIST = check.h DummyMovieDefinition.h DummyCharacter.h gnashrc.in simple.exp media/README media/Bitstream-Vera-Sans.fdb media/Bitstream-Vera-Sans.COPYRIGHT media/default.orc media/gnash.xml media/majorscale.sco media/offspring.swf media/offspring.jpg media/green.jpg media/green.swf media/lynch.swf media/lynch.jpg media/sound1.wav media/sound1.mp3 media/square.flv media/square.ogg media/vars.txt media/vars2.txt

dist_noinst_SCRIPTS = anaylse-results.sh generic-testrunner.sh

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
	$(top_builddir)/libcore/libgnashcore.la \
	$(top_builddir)/libbase/libgnashbase.la \
	$(NULL)
libtestsuite_la_CXXFLAGS = \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/backend \
	-I$(top_srcdir)/libcore  \
	-I$(top_srcdir)/libcore/parser  \
	-I$(top_srcdir)/libcore/vm  \
	$(BOOST_CFLAGS) \
	$(CAIRO_CFLAGS) \
	$(NULL)

tmpSharedObject: 
	mkdir tmpSharedObject

gnashrc: gnashrc.in Makefile tmpSharedObject
	sed -e 's#@@MEDIADIR@@#'$(abs_mediadir)'#g' -e 's#@@TMPSODIR@@#'$(abs_tmpsodir)'#g' $< > $@

if ENABLE_MING

.PHONY: Dejagnu.swf
Dejagnu.swf:
	$(MAKE) -C misc-ming.all $@
	cp misc-ming.all/Dejagnu.swf $@

check-local: Dejagnu.swf gnashrc

endif

clean-local:
	rm -f Dejagnu.swf gnashrc
	rm -rf tmpSharedObject

