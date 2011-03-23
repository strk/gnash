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

AUTOMAKE_OPTIONS = dejagnu

EXTRA_DIST = test.as

# this is where Gnash plugins get installed
pluginsdir = $(libdir)/gnash/plugins

plugins_LTLIBRARIES = fileio.la

DEF_MAKESWF_FLAGS = -I $(top_srcdir)/testsuite/actionscript.all

INCLUDES =  \
            -I$(top_srcdir)/libbase \
            -I$(top_srcdir)/libcore \
            -I$(top_srcdir)/libcore/abc \
            -I$(top_srcdir)/libcore/swf \
            -I$(top_srcdir)/libcore/vm \
	    -I$(top_srcdir)/libcore/parser \
	    -I$(top_srcdir)/libcore/asobj \
	    $(BOOST_CFLAGS) \
	    $(PTHREAD_CFLAGS) \
	    $(NULL)

AM_LDFLAGS =  \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/libcore/libgnashcore.la \
	$(BOOST_LIBS) \
	$(PTHREAD_LIBS) \
	$(NULL)

fileio_la_SOURCES = fileio.cpp fileio.h
fileio_la_LDFLAGS = -module -avoid-version -no-undefined
fileio_la_LIBDADD = $(LIBADD_DL) $(AM_LDFLAGS)


check_SCRIPTS =

if ENABLE_MING
 check_SCRIPTS += test
endif

CLEANFILES = \
	gnash-dbg.log \
	test.swf.frame0.pp \
	test.swf \
	test \
	logfile \
	testrun.log \
	testrun.sum \
	site.exp.bak

install-pluginsLTLIBRARIES: $(plugins_LTLIBRARIES)
	test -d "$(DESTDIR)$(pluginsdir)" || $(mkinstalldirs) "$(DESTDIR)$(pluginsdir)"
	$(LIBTOOL) --mode=install $(INSTALL) $(INSTALL_STRIP_FLAG) $(plugins_LTLIBRARIES) "$(DESTDIR)$(pluginsdir)/$(plugins_LTLIBRARIES)"
	$(RM) $(DESTDIR)$(pluginsdir)/*.*a 

test: $(srcdir)/../../testsuite/generic-testrunner.sh test.swf
	GNASH_PLUGINS=$(PWD)/.libs GNASHRC=$(srcdir)/../gnashrc sh $< $(top_builddir) test.swf > $@
	chmod +x $@

uninstall-local:
	$(RM) $(DESTDIR)$(pluginsdir)/fileio.so

# Wrap a single .as tests in dejagnu_so importer.
# At play time, if the relative url '../ming-misc.all/Dejagnu.swf' 
# takes to the Dejagnu.swf shared library we'll get visual traces,
# otherwise normal traces will be used.
SUFFIXES = as swf
.as.swf: 
	$(MAKESWF) $(DEF_MAKESWF_FLAGS)	$(MAKESWF_FLAGS) -o $@ $<

TEST_DRIVERS = ../../testsuite/simple.exp
TEST_CASES = \
	$(check_SCRIPTS)

site-update: site.exp
	@rm -fr site.exp.bak
	@cp site.exp site.exp.bak
	@sed -e '/testcases/d' site.exp.bak > site.exp
	@echo "# This is a list of the pre-compiled testcases" >> site.exp
	@echo "set testcases \"$(TEST_CASES)\"" >> site.exp

check-DEJAGNU: site-update $(TEST_CASES)
	@runtest=$(RUNTEST); \
	if $(SHELL) -c "$$runtest --version" > /dev/null 2>&1; then \
	    $$runtest $(RUNTESTFLAGS) $(TEST_DRIVERS); \
	else \
	  echo "WARNING: could not find \`runtest'" 1>&2; \
          for i in "$(TEST_CASES)"; do \
	    $(SHELL) $$i; \
	  done; \
	fi

