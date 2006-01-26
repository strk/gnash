# 
# Copyright (C) 2005, 2006 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = dejagnu

STD_DIRS = libbase libgeometry server backend utilities testsuite
DIST_SUBDIRS = doc
if PLUGIN
SUBDIRS = $(STD_DIRS) plugin
else
SUBDIRS = $(STD_DIRS)
endif

# if HTTP
# SUBDIRS += net $(STD_DIRS)
# else
# SUBDIRS += $(STD_DIRS)
# endif

noinst_HEADERS = compatibility_include.h

install-doc:
	@cd doc/C ; $(MAKE) install-doc

doc:
	@cd doc/C ; $(MAKE) alldocs

# Build an RPM package
rpm redhat: dist-gzip $(srcdir)/rpm/gnash.spec
	cp -f $(srcdir)/rpm/gnash.spec /usr/src/redhat/SPECS/
	cp -f gnash-${VERSION}.tar.gz /usr/src/redhat/SOURCES/
	rpmbuild -ba /usr/src/redhat/SPECS/gnash.spec

# Build a Debian GNU/Linux package
deb debian: dist-bzip2 $(srcdir)/debian/rules
	rm -rf gnash-${VERSION}
	tar jxf gnash-${VERSION}.tar.bz2
	cp -fr $(srcdir)/debian gnash-${VERSION}/
	(cd gnash-${VERSION}; dpkg-buildpackage)

mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"
