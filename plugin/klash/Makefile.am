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

# $Id: Makefile.am,v 1.22 2007/01/17 12:16:28 bjacques Exp $

AUTOMAKE_OPTIONS = 

AM_CPPFLAGS= -I$(kde_includes)/kio $(all_includes)

METASOURCES= AUTO

kde_module_LTLIBRARIES= libklashpart.la

libklashpart_la_SOURCES = klash_part.cpp klash_part.h
#nodist_libklashpart_la_SOURCES= klash_part.moc
libklashpart_la_LDFLAGS = -avoid-version $(all_libraries) $(KDE_RPATH)
libklashpart_la_DEPENDENCIES = klash_part.moc

libklashpart.la: $(libklashpart_la_DEPENDENCIES) $(libklashpart_la_OBJECTS) 
	$(CXXLINK) -rpath $(kde_moduledir) $(libklashpart_la_LDFLAGS) $(libklashpart_la_OBJECTS) $(libklashpart_la_LIBADD) $(LIBS)

#EXTRA_PROGRAMS = klashplayer
#klashplayer_app = klashplayer

#bin_PROGRAMS= $(klashplayer_app)

#klashplayer_LDADD=  #$(LIB_XINE) -lDCOP
#klashplayer_CFLAGS= #$(CFLAGS_XINE)
#klashplayer_LDFLAGS= $(all_libraries) $(KDE_RPATH)
#klashplayer_SOURCES= glashplayer.cpp

#xdg_apps_DATA = klash.desktop

dist_kde_services_DATA = klash_part.desktop

dist_rc_DATA = klashpartui.rc
rcdir = $(kde_datadir)/klash

dist_conf_DATA = klashrc
confdir = $(kde_confdir)

appsdatadir=$(kde_datadir)/klash
dist_appsdata_DATA= pluginsinfo

dummy.cpp:
	echo > dummy.cpp

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/klash.pot

if KLASH
#bin_PROGRAMS = klash

klash_SOURCES = klash.cpp
klash_CPPFLAGS = \
        -I.. \
        -I$(top_srcdir) \
        -I$(top_srcdir)/server  \
        -I$(top_srcdir)/server/parser \
        -I$(top_srcdir)/server/asobj \
        -I$(top_srcdir)/server/vm \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/libgeometry \
        -I$(top_srcdir)/backend \
	$(LIBXML_CFLAGS) \
	$(OPENGL_CFLAGS) \
	$(KDE_CFLAGS) \
	-DUSE_KDE

klash_LDADD = \
	$(KDE_LIBS) \
	$(OPENGL_LIBS) \
	$(LIBXML_LIBS) \
	$(PTHREAD_LIBS) \
        ../../backend/libgnashbackend.la \
        ../../server/libgnashserver.la \
        ../../server/asobj/libgnashasobjs.la \
        ../../libgeometry/libgnashgeo.la \
        ../../libbase/libgnashbase.la

klash_DEPENDENCIES =  klash.moc
BUILT_SOURCES = klash_part.moc klash.moc

CLEANFILES = klash_part.moc klash.moc
endif

klash_part.moc: $(srcdir)/klash_part.h
	@if test x"$(MOC)" != x; then \
	  echo "Generating MOC file..."; \
	  $(MOC) $(srcdir)/klash_part.h -o klash_part.moc; \
	else  \
	  echo "WARNING: Install QT's moc tool! Linking to default MOC file"; \
	  ln -s $(srcdir)/klash_part.moc.in klash_part.moc; \
	fi

klash.moc: $(srcdir)/klash.cpp
	@if test x"$(MOC)" != x; then \
	  echo "Generating MOC file..."; \
	  $(MOC) $(srcdir)/klash.cpp -o klash.moc; \
	else  \
	  echo "WARNING: Install QT's moc tool! Linking to default MOC file"; \
	  ln -s $(srcdir)/klash.moc.in klash.moc; \
	fi
