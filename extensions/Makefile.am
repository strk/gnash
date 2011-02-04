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

EXTRA_DIST = gnashrc

# Don't try to build the MySQL extension unless MySQL is actually installed.
OTHER_DIRS =
if BUILD_MYSQL_EXT
OTHER_DIRS += mysql
endif

if BUILD_DEJAGNU_EXT
OTHER_DIRS += dejagnu
endif

if BUILD_FILEIO_EXT
OTHER_DIRS += fileio
endif

if BUILD_LIRC_EXT
OTHER_DIRS += lirc
endif

if BUILD_DBUS_EXT
OTHER_DIRS += dbus
endif

if BUILD_GTK_EXT
if BUILD_GTK_GUI
OTHER_DIRS += # gtk2
endif
endif
DIST_SUBDIRS = dejagnu fileio mysql gtk2 lirc dbus 
SUBDIRS = $(OTHER_DIRS)
