## Process this fill with automake to generate Makefile.in
# 
# Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

# $Id: Makefile.am,v 1.50 2006/11/21 09:23:58 strk Exp $

AUTOMAKE_OPTIONS = dejagnu

check_SCRIPTS = \
	alltests-v5-Runner \
	alltests-v5-Runner \
	alltests-v7-Runner \
	alltests-v8-Runner
	

# We don't need  --tool anymore
#RUNTESTDEFAULTFLAGS = swf_exists.exp
TEST_DRIVERS = ../simple.exp
TEST_CASES = \
	alltests-v5-Runner \
	alltests-v5-Runner \
	alltests-v7-Runner \
	alltests-v8-Runner

dist_noinst_SCRIPTS = gen-test.sh gen-index.sh

INCLUDES = -I.. \
        -I$(top_srcdir)		\
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/server  \
        -I$(top_srcdir)/libgeometry \
	$(LIBXML_CFLAGS)        \
	$(OPENGL_CFLAGS)	\
	$(DMALLOC_CFLAGS)	\
	$(MP3_CFLAGS)		\
	$(OGG_CFLAGS)		\
        $(MING_CFLAGS)		\
        $(SDL_CFLAGS)		\
        $(NULL)

## These are needed to properly set dependencies
ASTESTS =			\
	array.as		\
	delete.as		\
	Boolean.as		\
	Camera.as		\
	Color.as		\
	ContextMenu.as		\
	CustomActions.as	\
	Date.as			\
	Error.as		\
	Global.as		\
	LoadVars.as		\
	Microphone.as		\
	Mouse.as		\
	MovieClip.as		\
	NetStream.as		\
	Number.as		\
	Selection.as		\
	SharedObject.as		\
	Stage.as		\
	String.as		\
	System.as		\
	TextSnapshot.as		\
	Video.as		\
	Object.as		\
	Inheritance.as		\
	NetConnection.as	\
	Function.as		\
	with.as			\
	XML.as			\
	rtmp.as 		\
	XMLNode.as		\
	LocalConnection.as	\
	$(NULL)

GENERATED = \
	alltests-v5.swf \
	alltests-v6.swf \
	alltests-v7.swf \
	alltests-v8.swf \
	$(NULL)

EXTRA_DIST = $(ASTESTS) \
	check.as \
	dejagnu.as \
	rtrace.as \
	utils.as \
	xtrace.as \
	moviecliploader_test.swf \
	movieclip_test.swf \
	text_formatting.swf \
	text_sizes.swf \
	visible_and_transparency.swf \
	swf_exists.exp \
	dejagnu_so_fini.as \
	dejagnu_so_init.as \
	$(NULL)

if ENABLE_MING
TARGET = $(GENERATED)
EXTRA_DIST += $(GENERATED)
else
TARGET = 
endif

CLEANFILES = gnash-dbg.log  \
	*.as.pp *.as.swf *.vswf *swf.frame*.pp \
	site.bak site.exp testrun.sum testrun.log out.swf

DISTCLEANFILES = index.wiki index.html embed.html

DEJAGNU_SO_PATH = ../misc-ming.all/Dejagnu.swf

# URL to Dejagnu.swf, relative to testcases run location
# This will be changed by 'online' test rules so to 
# reflect publication layout
DEJAGNU_SO_URL = $(DEJAGNU_SO_PATH)

# Default target version, can reset if you like
SWFVERSION = 6


MAINTAINERCLEANFILES = $(GENERATED) 

all:

check-local: $(TARGET)

swf: $(TARGET)

index-html: 
	sh $(srcdir)/gen-index.sh $(GENERATED)

ONLINETEST_RULES = v5-online-tests v6-online-tests v7-online-tests v8-online-tests online-tests vn-online-tests

vn-online-tests: index-html
	rm require-ming # force rebuild
	$(MAKE) $(GENERATED) DEJAGNU_SO_URL=../Dejagnu.swf 
	mkdir -p online-tests/v$(SWFVERSION);
	mv $(GENERATED) online-tests/v$(SWFVERSION);
	cp index.html embed.html online-tests/v$(SWFVERSION)
	cp $(DEJAGNU_SO_PATH) online-tests

v8-online-tests: index-html
	$(MAKE) vn-online-tests SWFVERSION="8"

v7-online-tests: index-html
	$(MAKE) vn-online-tests SWFVERSION="7"

v6-online-tests: index-html
	$(MAKE) vn-online-tests SWFVERSION="6"

v5-online-tests: index-html
	$(MAKE) vn-online-tests SWFVERSION="5"

.PHONY: online-tests
online-tests: v5-online-tests v6-online-tests v7-online-tests v8-online-tests

# List 'require-ming' as a dependency if the rule needs Ming to run
require-ming:
	@if test x"$(MAKESWF)" = x; then \
	  echo " - Ming is required for this rule -"; \
	  false; \
	else \
	  touch require-ming; \
	fi

# Dependencies for all generated SWF files
$(GENERATED) $(ONLINETEST_RULES) alltests.swf: require-ming dejagnu.as check.as dejagnu_so_init.as dejagnu_so_fini.as $(DEJAGNU_SO_PATH)

# Wrap a single .as tests in dejagnu_so importer.
# At play time, if the relative url '../ming-misc.all/Dejagnu.swf' 
# takes to the Dejagnu.swf shared library we'll get visual traces,
# otherwise normal traces will be used.
.as.swf: 
	@$(MAKESWF) \
		-i$(DEJAGNU_SO_URL):dejagnu \
		-DUSE_DEJAGNU_MODULE \
		-DOUTPUT_VERSION=$(SWFVERSION) -v$(SWFVERSION) \
		$(MAKESWF_FLAGS) \
		-o $@ \
		$(srcdir)/dejagnu_so_init.as \
		$(@:%.swf=$(srcdir)/%.as) \
		$(srcdir)/dejagnu_so_fini.as; 

# Wrap ALL .as tests in dejagnu_so importer.
# At play time, if the relative url '../ming-misc.all/Dejagnu.swf' 
# takes to the Dejagnu.swf shared library we'll get visual traces,
# otherwise normal traces will be used.
alltests.swf: $(ASTESTS)
	@$(MAKESWF) \
		-i$(DEJAGNU_SO_URL):dejagnu \
		-DUSE_DEJAGNU_MODULE \
		-DOUTPUT_VERSION=$(SWFVERSION) -v$(SWFVERSION) \
		$(MAKESWF_FLAGS) \
		-o $@ \
		$(srcdir)/dejagnu_so_init.as \
		$(ASTESTS:%=$(srcdir)/%) \
		$(srcdir)/dejagnu_so_fini.as; 

alltests-v5.swf: $(ASTESTS)
	@$(MAKE) alltests.swf SWFVERSION="5"
	@mv alltests.swf alltests-v5.swf

alltests-v6.swf: $(ASTESTS)
	@$(MAKE) alltests.swf SWFVERSION="6"
	@mv alltests.swf alltests-v6.swf

alltests-v7.swf: $(ASTESTS)
	@$(MAKE) alltests.swf SWFVERSION="7"
	@mv alltests.swf alltests-v7.swf

alltests-v8.swf: $(ASTESTS)
	@$(MAKE) alltests.swf SWFVERSION="8"
	@mv alltests.swf alltests-v8.swf

alltests-v5-Runner: $(srcdir)/../generic-testrunner.sh alltests-v5.swf
	sh $< $(top_builddir) alltests-v5.swf > $@
	chmod 755 $@

alltests-v6-Runner: $(srcdir)/../generic-testrunner.sh alltests-v6.swf
	sh $< $(top_builddir) alltests-v6.swf > $@
	chmod 755 $@

alltests-v7-Runner: $(srcdir)/../generic-testrunner.sh alltests-v7.swf
	sh $< $(top_builddir) alltests-v7.swf > $@
	chmod 755 $@

alltests-v8-Runner: $(srcdir)/../generic-testrunner.sh alltests-v8.swf
	sh $< $(top_builddir) alltests-v8.swf > $@
	chmod 755 $@

#clean-local:
#	rm -Rf online-tests require-ming embed.html \
#		index.html index.wiki

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

site-update: site.exp
	@rm -fr site.exp.bak
	@cp site.exp site.exp.bak
	@sed -e '/testcases/d' site.exp.bak > site.exp
	@echo "# This is a list of the pre-compiled testcases" >> site.exp
	@echo "set testcases \"$(TEST_CASES)\"" >> site.exp
