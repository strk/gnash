## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2011 Free Software Foundation, Inc.
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
abs_builddir=$(shell cd $(top_builddir)/testsuite/misc-ming.all/RegisterClass; pwd)
abs_mediadir = $(shell cd $(srcdir)/../../media; pwd)

CLEANFILES =  \
	gnash-dbg.log \
	site.exp.bak \
	testrun.sum \
	testrun.log

EXTRA_DIST = \
	$(NULL)

AM_CPPFLAGS = \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libsound \
	-I$(top_srcdir)/librender \
	-I$(top_srcdir)/libcore  \
	-I$(top_srcdir)/libcore/swf  \
	-I$(top_srcdir)/libcore/parser  \
	-I$(top_srcdir)/libcore/vm \
	-I$(top_srcdir)/libcore/asobj \
	-I$(top_srcdir)/testsuite \
	-I$(top_srcdir)/testsuite/misc-ming.all \
	$(MING_CFLAGS) \
	$(OPENGL_CFLAGS) \
	$(BOOST_CFLAGS) \
	-DMING_VERSION_CODE=$(MING_VERSION_CODE) \
	$(NULL)

AM_LDFLAGS = \
	$(MING_LIBS) \
	$(OPENGL_LIBS) \
	$(GIF_LIBS) \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/librender/libgnashrender.la \
	$(top_builddir)/libcore/libgnashcore.la \
	$(NULL)

check_PROGRAMS = \
	registerClassTest \
	registerClassTestRunner \
	$(NULL)

if MING_SUPPORTS_INIT_ACTIONS
check_PROGRAMS += \
	RegisterClassTest4 \
	RegisterClassTest3 \
	registerClassTest2 \
	$(NULL)
endif

check_SCRIPTS = 

if MING_SUPPORTS_INIT_ACTIONS
check_SCRIPTS += \
	RegisterClassTest4Runner \
	RegisterClassTest3Runner \
	registerClassTest2runner \
	$(NULL)
endif

# This is so check.as finds revno.h
MAKESWF_FLAGS = -I$(top_builddir)

#----------------------------------

registerClassTest_SOURCES =	\
	registerClassTest.c	\
	$(NULL)
registerClassTest_LDADD = ../libgnashmingutils.la

registerClassTest.swf: registerClassTest
	./registerClassTest $(abs_mediadir)

registerClassTestRunner_SOURCES = \
	registerClassTestRunner.cpp \
	$(NULL)
registerClassTestRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
registerClassTestRunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
registerClassTestRunner_DEPENDENCIES = \
	registerClassTest.swf \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)

#----------------------------------

registerClassTest2_SOURCES =	\
	registerClassTest2.c	\
	$(NULL)
registerClassTest2_LDADD = ../libgnashmingutils.la

registerClassTest2.swf: registerClassTest2
	./registerClassTest2 $(abs_mediadir)

registerClassTest2runner: $(srcdir)/../../generic-testrunner.sh registerClassTest2.swf
	sh $(srcdir)/../../generic-testrunner.sh $(top_builddir) registerClassTest2.swf > $@
	chmod 755 $@

#----------------------------------

RegisterClassTest3_SOURCES =	\
	RegisterClassTest3.c	\
	$(NULL)
RegisterClassTest3_LDADD = ../libgnashmingutils.la

RegisterClassTest3.swf: RegisterClassTest3
	./RegisterClassTest3 $(abs_mediadir)

RegisterClassTest3Runner: $(srcdir)/../../generic-testrunner.sh RegisterClassTest3.swf
	sh $(srcdir)/../../generic-testrunner.sh $(top_builddir) RegisterClassTest3.swf > $@
	chmod 755 $@

#----------------------------------

RegisterClassTest4_SOURCES =	\
	RegisterClassTest4.c	\
	$(NULL)
RegisterClassTest4_LDADD = ../libgnashmingutils.la

RegisterClassTest4.swf: RegisterClassTest4
	./RegisterClassTest4 $(abs_mediadir)

RegisterClassTest4Runner: $(srcdir)/../../generic-testrunner.sh RegisterClassTest4.swf
	sh $(srcdir)/../../generic-testrunner.sh $(top_builddir) RegisterClassTest4.swf > $@
	chmod 755 $@

#----------------------------------

clean-local: 
	-rm *.swf *.pp *unner

TEST_CASES = \
	registerClassTestRunner \
	$(NULL)

if MING_SUPPORTS_INIT_ACTIONS
TEST_CASES += \
	registerClassTest2runner \
	RegisterClassTest3Runner \
	RegisterClassTest4Runner \
	$(NULL)
endif

TEST_DRIVERS = ../../simple.exp

TEST_ENV = GNASH_GC_TRIGGER_THRESHOLD=0
TEST_ENV += GNASHRC=../../gnashrc
TEST_ENV += GST_INSTALL_PLUGINS_HELPER=$(top_srcdir)/testsuite/gst_install_plugins_helper.sh


check-DEJAGNU: site-update $(check_PROGRAMS)
	runtest=$(RUNTEST); \
	if $(SHELL) -c "$$runtest --version" > /dev/null 2>&1; then \
	    $(TEST_ENV) $$runtest $(RUNTESTFLAGS) $(TEST_DRIVERS); true; \
	else \
	  echo "WARNING: could not find \`runtest'" 1>&2; \
          for i in "$(TEST_CASES)"; do \
	    $(TEST_ENV) $(SHELL) $$i; \
	  done; \
	fi

site-update: site.exp
	@rm -fr site.exp.bak
	@cp site.exp site.exp.bak
	@sed -e '/testcases/d' site.exp.bak > site.exp
	@echo "# This is a list of the pre-compiled testcases" >> site.exp
	@echo "set testcases \"$(TEST_CASES)\"" >> site.exp

