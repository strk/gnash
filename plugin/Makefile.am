## Process this file with automake to generate Makefile.in
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


AUTOMAKE_OPTIONS = 

SUBDIRS =

if KPARTS3
SUBDIRS += klash
endif

if KPARTS4
SUBDIRS += klash4
endif

if NPAPI
SUBDIRS += npapi
if WIN32
SUBDIRS += win32
endif
if AMIGAOS4
SUBDIRS += aos4
endif
endif # NPAPI

# make sure these files make it into the distribution tarball.
DIST_SUBDIRS = klash klash4 npapi win32 aos4

install-plugin install-plugins: $(plugin_LTLIBRARIES) $(PKGLIB)
if !WIN32
if NPAPI
	$(MAKE) -C npapi install-plugin
endif
if KPARTS4
	$(MAKE) -C klash4 install-plugins
endif
if KPARTS3
	$(MAKE) -C klash install-plugins
endif
else	# windoze
	: # do nothing
endif	# end of if windoze

uninstall-plugins: 
if NPAPI
	$(MAKE) -C npapi $@
endif
if KPARTS4
	$(MAKE) -C klash4 $@
endif
if KPARTS3
	$(MAKE) -C klash $@
endif
