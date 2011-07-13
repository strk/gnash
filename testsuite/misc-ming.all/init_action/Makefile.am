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
abs_builddir=$(shell cd $(top_builddir)/testsuite/misc-ming.all/init_action; pwd)
abs_mediadir = $(shell cd $(srcdir)/../../media; pwd)

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

if MING_SUPPORTS_INIT_ACTIONS

check_PROGRAMS = \
	InitActionTest \
	InitActionTest2 \
	$(NULL)

check_SCRIPTS = \
	InitActionTestRunner \
	InitActionTest2Runner \
	$(NULL)

InitActionTest_SOURCES =	\
	InitActionTest.c	\
	$(NULL)
InitActionTest_LDADD = ../libgnashmingutils.la

InitActionTest.swf: InitActionTest
	./InitActionTest $(abs_mediadir)

InitActionTestRunner: $(srcdir)/../../generic-testrunner.sh InitActionTest.swf
	sh $(srcdir)/../../generic-testrunner.sh $(top_builddir) InitActionTest.swf > $@
	chmod 755 $@

InitActionTest2_SOURCES =	\
	InitActionTest2.c	\
	$(NULL)
InitActionTest2_LDADD = ../libgnashmingutils.la

InitActionTest2.swf: InitActionTest2
	./InitActionTest2 $(abs_mediadir)

InitActionTest2Runner: $(srcdir)/../../generic-testrunner.sh InitActionTest2.swf
	sh $(srcdir)/../../generic-testrunner.sh $(top_builddir) InitActionTest2.swf > $@
	chmod 755 $@


TEST_CASES = \
	InitActionTestRunner \
	InitActionTest2Runner \
	$(NULL)


clean-local: 
	-rm *.swf *Runner

CLEANFILES =  \
	gnash-dbg.log \
	site.exp.bak \
	testrun.sum \
	testrun.log


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

endif
# MING_SUPPORTS_INIT_ACTIONS
