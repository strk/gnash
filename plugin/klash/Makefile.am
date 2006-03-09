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
# Linking Gnash statically or dynamically with other modules is making
# a combined work based on Gnash. Thus, the terms and conditions of
# the GNU General Public License cover the whole combination.
# 
# In addition, as a special exception, the copyright holders of Gnash give
# you permission to combine Gnash with free software programs or
# libraries that are released under the GNU LGPL and/or with Mozilla, 
# so long as the linking with Mozilla, or any variant of Mozilla, is
# through its standard plug-in interface. You may copy and distribute
# such a system following the terms of the GNU GPL for Gnash and the
# licenses of the other code concerned, provided that you include the
# source code of that other code when and as the GNU GPL requires
# distribution of source code. 
# 
# Note that people who make modified versions of Gnash are not obligated
# to grant this special exception for their modified versions; it is
# their choice whether to do so.  The GNU General Public License gives
# permission to release a modified version without this exception; this
# exception also makes it possible to release a modified version which
# carries forward this exception.
#
AUTOMAKE_OPTIONS = 

AM_CPPFLAGS= -I$(kde_includes)/kio $(all_includes)

METASOURCES= AUTO

kde_module_LTLIBRARIES= libklashpart.la

libklashpart_la_SOURCES = klash_part.cpp
nodist_libklashpart_la_SOURCES= klash_part.moc
libklashpart_la_LDFLAGS = -avoid-version $(all_libraries) $(KDE_RPATH)
libklashpart_la_DEPENDANCIES = klash_part.moc

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

