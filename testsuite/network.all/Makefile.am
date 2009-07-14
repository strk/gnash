## Process this file with automake to generate Makefile.in
# 
# Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

AUTOMAKE_OPTIONS = dejagnu -Wno-portability

abs_top_builddir=$(shell cd $(top_builddir); pwd)
abs_builddir=$(shell cd $(top_builddir)/testsuite/network.all; pwd)
abs_mediadir = $(shell cd $(srcdir)/../media; pwd)

CLEANFILES =  \
	gnash-dbg.log \
	site.exp.bak \
	testrun.sum \
	testrun.log

EXTRA_DIST = \
	http.as \
	rtmp.as \
	netstream.as

AM_CPPFLAGS = \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libsound \
	-I$(top_srcdir)/backend \
	-I$(top_srcdir)/libcore  \
	-I$(top_srcdir)/libcore/parser  \
	-I$(top_srcdir)/libcore/vm \
	-I$(top_srcdir)/libcore/asobj \
	-I$(top_srcdir)/libamf \
	-I$(top_srcdir)/libnet \
	-I$(top_srcdir)/testsuite \
	-I$(top_srcdir)/testsuite/misc-ming.all \
	$(MING_CFLAGS) \
	$(OPENGL_CFLAGS) \
	$(BOOST_CFLAGS) \
	-DMING_VERSION_CODE=$(MING_VERSION_CODE) \
	$(NULL)

AM_LDFLAGS = 
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/libamf/libgnashamf.la \
	$(top_builddir)/libnet/libgnashnet.la \
	$(OPENGL_LIBS) \
	$(BOOST_LIBS) \
	$(PTHREAD_LIBS)
	$(NULL)

check_PROGRAMS = \
	Dejagnu \
	Dejagnu.swf \
	test_ssl \
	test_allowDomain \
	test_ssh \
	$(NULL)

check_SCRIPTS = \
	http.swf \
	rtmp.swf

# if MAKESWF_SUPPORTS_PREBUILT_CLIPS
#   check_SCRIPTS += remotingTestRunner
#   check_SCRIPTS += red5testRunner
# endif

check_LTLIBRARIES = libgnashmingutils.la 
libgnashmingutils_la_SOURCES = \
	$(top_srcdir)/testsuite/misc-ming.all/ming_utils.h \
	$(top_srcdir)/testsuite/misc-ming.all/ming_utils.c

http.swf: $(srcdir)/http.as Dejagnu.swf Makefile ../actionscript.all/check.as ../actionscript.all/utils.as
	$(MAKESWF) -n network -r12 -o $@ -v7 -DRED5_HOST='\"$(RED5_HOST)\"' -DUSE_DEJAGNU_MODULE -DOUTPUT_VERSION=7 Dejagnu.swf $(srcdir)/http.as $(srcdir)/../actionscript.all/dejagnu_so_fini.as

httpTestRunner: $(srcdir)/../generic-testrunner.sh http.swf
	sh $< $(top_builddir) http.swf > $@
	chmod 755 $@

rtmp.swf: $(srcdir)/rtmp.as Dejagnu.swf Makefile ../actionscript.all/check.as ../actionscript.all/utils.as
	$(MAKESWF) -n network -r12 -o $@ -v7 -DRED5_HOST='\"$(RED5_HOST)\"' -DUSE_DEJAGNU_MODULE -DOUTPUT_VERSION=7 Dejagnu.swf $(srcdir)/rtmp.as $(srcdir)/../actionscript.all/dejagnu_so_fini.as

rtmpTestRunner: $(srcdir)/../generic-testrunner.sh rtmp.swf
	sh $< $(top_builddir) rtmp.swf > $@
	chmod 755 $@

netstream.swf: $(srcdir)/netstream.as Dejagnu.swf Makefile ../actionscript.all/check.as ../actionscript.all/utils.as
	$(MAKESWF) -n network -r12 -o $@ -v7 -DRED5_HOST='\"$(RED5_HOST)\"' -DUSE_DEJAGNU_MODULE -DOUTPUT_VERSION=7 Dejagnu.swf $(srcdir)/netstream.as $(srcdir)/../actionscript.all/dejagnu_so_fini.as

netstreamRunner: $(srcdir)/../generic-testrunner.sh netstream.swf
	sh $< $(top_builddir) netstream.swf > $@
	chmod 755 $@

Dejagnu_SOURCES = \
	Dejagnu.c	\
	$(NULL)
Dejagnu_LDADD = libgnashmingutils.la

Dejagnu.swf: Dejagnu
	./Dejagnu $(top_srcdir)/testsuite/media

test_ssl_SOURCES = test_ssl.cpp
test_ssl_LDADD = \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/libamf/libgnashamf.la \
	$(top_builddir)/libnet/libgnashnet.la

test_ssh_SOURCES = test_ssh.cpp
test_ssh_LDADD = \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/libamf/libgnashamf.la \
	$(top_builddir)/libnet/libgnashnet.la

test_ssl_DEPENDENCIES = site-update


test_allowDomain_SOURCES = test_allowDomain.cpp
test_allowDomain_LDADD = \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/libcore/libgnashcore.la

test_allowDomain_DEPENDENCIES = site-update

clean-local: 
	-rm *.swf media *.pp *unner

TEST_DRIVERS = network.exp
TEST_CASES = \
	http.swf \
	rtmp.swf \
	$(NULL)

if MAKESWF_SUPPORTS_PREBUILT_CLIPS
# if ENABLE_HTTP_TESTSUITE
# TEST_CASES += remotingTestRunner
# endif
# TEST_CASES += red5testRunner
endif

check-DEJAGNU: site-update $(check_PROGRAMS)
	@runtest=$(RUNTEST); \
	if $(SHELL) -c "$$runtest --version" > /dev/null 2>&1; then \
	    GNASH_GC_TRIGGER_THRESHOLD=0 GNASHRC=../gnashrc $$runtest $(RUNTESTFLAGS) $(TEST_DRIVERS); true; \
	else \
	  echo "WARNING: could not find \`runtest'" 1>&2; \
          for i in "$(TEST_CASES)"; do \
	    GNASH_GC_TRIGGER_THRESHOLD=0 GNASHRC=../gnashrc $(SHELL) $$i; \
	  done; \
	fi

site-update: site.exp
	@rm -fr site.exp.bak
	@cp site.exp site.exp.bak
	@sed -e '/testcases/d' site.exp.bak > site.exp
	@echo "# This is a list of the pre-compiled testcases" >> site.exp
	@echo "set testcases \"$(TEST_CASES)\"" >> site.exp

