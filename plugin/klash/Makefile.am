## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
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

AUTOMAKE_OPTIONS = 

AM_CPPFLAGS= -I$(kde_includes)/kio $(all_includes)

METASOURCES= AUTO

kde_module_LTLIBRARIES= libklashpart.la

libklashpart_la_SOURCES= klash_part.cpp
libklashpart_la_LDFLAGS= -avoid-version $(all_libraries) $(KDE_RPATH)
libklashpart_la_LIBADD=

#EXTRA_PROGRAMS = klashplayer
#klashplayer_app = klashplayer

#bin_PROGRAMS= $(klashplayer_app)

#klashplayer_LDADD=  #$(LIB_XINE) -lDCOP
#klashplayer_CFLAGS= #$(CFLAGS_XINE)
#klashplayer_LDFLAGS= $(all_libraries) $(KDE_RPATH)
#klashplayer_SOURCES= glashplayer.cpp

#xdg_apps_DATA = klash.desktop

kde_services_DATA = klash_part.desktop

rc_DATA = klashpartui.rc
rcdir = $(kde_datadir)/klash

conf_DATA = klashrc
confdir = $(kde_confdir)

appsdatadir=$(kde_datadir)/klash
appsdata_DATA= pluginsinfo

CLEANFILES = klash_part.moc
dummy.cpp:
	echo > dummy.cpp

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/klash.pot

klash_part.moc: $(srcdir)/klash_part.h
	$(MOC) $(srcdir)/klash_part.h -o klash_part.moc

