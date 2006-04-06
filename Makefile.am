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

## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = dejagnu
ACLOCAL_AMFLAGS = -I macros

STD_DIRS = libbase libgeometry server backend utilities plugin
if DOCBOOK
DOC_DIRS = doc
endif
SUBDIRS      = $(STD_DIRS) $(DOC_DIRS)
# libamf isn't used yet, but we still want it in the distribution
DIST_SUBDIRS = $(STD_DIRS) doc testsuite libamf

#EXTRA_DIST = macros
# if HTTP
# SUBDIRS += net $(STD_DIRS)
# else
# SUBDIRS += $(STD_DIRS)
# endif

noinst_HEADERS = compatibility_include.h

# Build an RPM package
rpm redhat: dist-gzip $(srcdir)/packaging/redhat/gnash.spec
	cp -f $(srcdir)/packaging/redhat/gnash.spec /usr/src/redhat/SPECS/
	cp -f gnash-${VERSION}.tar.gz /usr/src/redhat/SOURCES/
	rpmbuild -ba /usr/src/redhat/SPECS/gnash.spec

# Build a Debian GNU/Linux package
deb debian: dist-bzip2 $(srcdir)/packaging/debian/rules
	rm -rf gnash-${VERSION}
	tar jxf gnash-${VERSION}.tar.bz2
	cp -fr $(srcdir)/packaging/debian gnash-${VERSION}/
	(cd gnash-${VERSION}; dpkg-buildpackage)

snapshot: distdir
	now=`date -u "+%Y%m%d"`; \
	mv gnash-${VERSION} gnash-$${now}; \
	tardir=gnash-$${now} && $(am__tar) | bzip2 -9 -c > gnash-$${now}.tar.bz2; \
	$(RM) -fr gnash-$${now}

mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

