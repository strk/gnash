## Process this fill with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
#   Free Software Foundation, Inc.
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

# Temporarly always disable MING, distribution
# of the generated SWF should work, but it's currently
# broken and needs to be used in misc-ming.all too
# so I take responsibility of dropping that for the moment...

abs_mediadir = $(shell cd $(srcdir)/../media; pwd)
swfdir = $(shell cd $(top_builddir)/testsuite/actionscript.all; pwd)

COM_MAKESWF_FLAGS=-DMING_VERSION_CODE=$(MING_VERSION_CODE) -I"$(top_builddir)"
DEF_MAKESWF_FLAGS=$(COM_MAKESWF_FLAGS) \
	-DMEDIADIR='\"$(abs_mediadir)\"' \
	-DSWFDIR='\"$(swfdir)\"' 

quickcheck_RUNNERS = \
	alltests-v5-Runner \
	alltests-v6-Runner \
	alltests-v7-Runner \
	alltests-v8-Runner


base_RUNNERS = \
	astests-v5-Runner \
	astests-v6-Runner \
	astests-v7-Runner \
	astests-v8-Runner
#	astests-v9-Runner

check_SCRIPTS = \
	$(base_RUNNERS) \
	$(NULL)

# We don't need  --tool anymore
#RUNTESTDEFAULTFLAGS = swf_exists.exp
TEST_DRIVERS = ../simple.exp
TEST_CASES = \
	$(base_RUNNERS)

dist_noinst_SCRIPTS = gen-test.sh gen-index.sh

AM_CPPFLAGS = \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/libcore  \
	-I$(top_srcdir)/libcore/vm  \
	-I$(top_srcdir)/libcore/asobj \
	-I$(top_srcdir)/testsuite \
	$(OPENGL_CFLAGS)	\
	$(MP3_CFLAGS)		\
	$(MING_CFLAGS)		\
	$(SDL_CFLAGS)		\
	$(NULL)

## These are needed to properly set dependencies
ASTESTS =			\
	AsBroadcaster.as	\
	ASnative.as		\
	Accessibility.as \
	array.as		\
	BitmapData.as \
	delete.as		\
	getvariable.as		\
	swap.as			\
	setProperty.as 		\
	targetPath.as		\
	Boolean.as		\
	Camera.as		\
	Color.as		\
	ColorTransform.as 	\
	ContextMenu.as		\
	Date.as			\
	Error.as		\
	ExternalInterface.as	\
	Function.as		\
	flash.as		\
	Global.as		\
	HitTest.as		\
	Inheritance.as		\
	Instance.as \
	Key.as			\
	LoadVars.as		\
	Math.as			\
	Microphone.as		\
	Mouse.as		\
	MovieClip.as		\
	MovieClipLoader.as	\
	NetConnection.as	\
	NetStream.as		\
	Number.as		\
	Random.as		\
	Selection.as		\
	SharedObject.as		\
	Stage.as		\
	String.as		\
	System.as		\
	TextField.as		\
	TextFieldHTML.as		\
	TextFormat.as		\
	TextSnapshot.as		\
	Transform.as		\
	Try.as \
	Video.as		\
	Matrix.as \
	Object.as		\
	Point.as		\
	with.as			\
	XML.as			\
	XMLSocket.as	\
	rtmp.as 		\
	XMLNode.as		\
	enumerate.as	\
	case.as			\
	caseconv.as			\
	ops.as			\
	toString_valueOf.as	\
	Rectangle.as		\
	argstest.as		\
	$(NULL)

if BUILD_LIBSOUND
ASTESTS += Sound.as
endif

if HAVE_SEMGET
ASTESTS += LocalConnection.as
endif

if HAVE_PERL
if ENABLE_HAXE
HAXETESTS = \
	String.as \
	$(NULL)
endif
endif

ASTESTS_OUT = $(ASTESTS:.as=.swf)

ASTESTS_V5_OUT = $(ASTESTS:.as=-v5.swf)
ASTESTS_V6_OUT = $(ASTESTS:.as=-v6.swf)
ASTESTS_V7_OUT = $(ASTESTS:.as=-v7.swf)
ASTESTS_V8_OUT = $(ASTESTS:.as=-v8.swf)

ASTESTS_VERSIONED_OUT = \
	$(ASTESTS_V5_OUT) \
	$(ASTESTS_V6_OUT) \
	$(ASTESTS_V7_OUT) \
	$(ASTESTS_V8_OUT) \
	$(NULL)

ALLTESTS_VERSIONED_OUT = \
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
	dejagnu_so_fini.as \
	dejagnu_so_init.as \
	$(NULL)

CLEANFILES = gnash-dbg.log  \
	*.as.pp *.as.swf *.vswf *swf.frame*.pp \
	site.bak site.exp testrun.sum testrun.log out.swf

DISTCLEANFILES = index.wiki index.html embed.html

DEJAGNU_SO_PATH = ../misc-ming.all/Dejagnu.swf

# URL to Dejagnu.swf, relative to testcases run location
# This will be changed by 'online' test rules so to 
# reflect publication layout
DEJAGNU_SO_URL = Dejagnu.swf

# Default target version, can reset if you like
SWFVERSION = 6


all:

#check-local: $(TARGET)

#swf: $(TARGET)

ONLINETEST_RULES = v5-online-tests v6-online-tests v7-online-tests v8-online-tests online-tests vn-online-tests

#
# 'online' rules are needed to override the path to Dejagnu.swf and to MEDIADIR
#

v8-online-tests: 
	rm -f $(ASTESTS_V8_OUT) # force rebuild
	$(MAKE) $(ASTESTS_V8_OUT) alltests-v8.swf DEF_MAKESWF_FLAGS="$(COM_MAKESWF_FLAGS) -DMEDIADIR='\\\"../media\\\"'" DEJAGNU_SO_URL=../Dejagnu.swf
	mkdir -p online-tests/v8
	mv $(ASTESTS_V8_OUT) alltests-v8.swf online-tests/v8
	sh $(srcdir)/gen-index.sh alltests-v8.swf $(ASTESTS_V8_OUT)
	mv index.html embed.html online-tests/v8

v7-online-tests: 
	rm -f $(ASTESTS_V7_OUT) # force rebuild
	$(MAKE) $(ASTESTS_V7_OUT) alltests-v7.swf DEF_MAKESWF_FLAGS="$(COM_MAKESWF_FLAGS) -DMEDIADIR='\\\"../media\\\"'" DEJAGNU_SO_URL=../Dejagnu.swf
	mkdir -p online-tests/v7
	mv $(ASTESTS_V7_OUT) alltests-v7.swf online-tests/v7
	sh $(srcdir)/gen-index.sh alltests-v7.swf $(ASTESTS_V7_OUT)
	mv index.html embed.html online-tests/v7

v6-online-tests: 
	rm -f $(ASTESTS_V6_OUT) # force rebuild
	$(MAKE) $(ASTESTS_V6_OUT) alltests-v6.swf DEF_MAKESWF_FLAGS="$(COM_MAKESWF_FLAGS) -DMEDIADIR='\\\"../media\\\"'" DEJAGNU_SO_URL=../Dejagnu.swf
	mkdir -p online-tests/v6
	mv $(ASTESTS_V6_OUT) alltests-v6.swf online-tests/v6
	sh $(srcdir)/gen-index.sh alltests-v6.swf $(ASTESTS_V6_OUT)
	mv index.html embed.html online-tests/v6

v5-online-tests: 
	rm -f $(ASTESTS_V5_OUT) # force rebuild
	$(MAKE) $(ASTESTS_V5_OUT) alltests-v5.swf DEF_MAKESWF_FLAGS="$(COM_MAKESWF_FLAGS) -DMEDIADIR='\\\"../media\\\"'" DEJAGNU_SO_URL=../Dejagnu.swf
	mkdir -p online-tests/v5
	mv $(ASTESTS_V5_OUT) alltests-v5.swf online-tests/v5
	sh $(srcdir)/gen-index.sh alltests-v5.swf $(ASTESTS_V5_OUT)
	mv index.html embed.html online-tests/v5

index.wiki: gen-index-wiki.sh
	sh $(srcdir)/gen-index-wiki.sh $(ASTESTS_OUT)

online-tests: v5-online-tests v6-online-tests v7-online-tests v8-online-tests index.wiki
	cp -a $(abs_mediadir) online-tests/media
	cp $(DEJAGNU_SO_PATH) online-tests/

# List 'require-ming' as a dependency if the rule needs Ming to run
require-ming:
	@if test x"$(MAKESWF)" = x; then \
	  echo " - Ming is required for this rule -"; \
	  false; \
	else \
	  touch require-ming; \
	fi

# Dependencies for all generated SWF files
$(ASTESTS_OUT) $(ASTESTS_VERSIONED_OUT) $(ONLINETEST_RULES) $(ALLTESTS_VERSIONED_OUT): require-ming dejagnu.as check.as dejagnu_so_init.as dejagnu_so_fini.as Dejagnu.swf

# Every SWF depends on check.as
*.swf: check.as dejagnu_so_init.as dejagnu_so_fini.as

# Wrap a single .as tests in dejagnu_so importer.
# At play time, if the relative url '../ming-misc.all/Dejagnu.swf' 
# takes to the Dejagnu.swf shared library we'll get visual traces,
# otherwise normal traces will be used.
.as.swf: 
	$(MAKESWF) \
		-i$(DEJAGNU_SO_URL):dejagnu \
		-i$(DEJAGNU_SO_URL):dejafont \
		-DUSE_DEJAGNU_MODULE \
		-DOUTPUT_VERSION=$(SWFVERSION) -v$(SWFVERSION) \
		$(DEF_MAKESWF_FLAGS) \
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
		$(DEF_MAKESWF_FLAGS) \
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

astests-v5-Runner: $(srcdir)/../generic-testrunner.sh $(ASTESTS_V5_OUT)
	GNASHRC="$(top_builddir)/testsuite/gnashrc" sh $(srcdir)/../generic-testrunner.sh $(top_builddir) $(ASTESTS_V5_OUT) > $@
	chmod 755 $@

astests-v6-Runner: $(srcdir)/../generic-testrunner.sh $(ASTESTS_V6_OUT)
	GNASHRC="$(top_builddir)/testsuite/gnashrc" sh $(srcdir)/../generic-testrunner.sh $(top_builddir) $(ASTESTS_V6_OUT) > $@
	chmod 755 $@

astests-v7-Runner: $(srcdir)/../generic-testrunner.sh $(ASTESTS_V7_OUT)
	GNASHRC="$(top_builddir)/testsuite/gnashrc" sh $(srcdir)/../generic-testrunner.sh $(top_builddir) $(ASTESTS_V7_OUT) > $@
	chmod 755 $@

astests-v8-Runner: $(srcdir)/../generic-testrunner.sh $(ASTESTS_V8_OUT)
	GNASHRC="$(top_builddir)/testsuite/gnashrc" sh $(srcdir)/../generic-testrunner.sh $(top_builddir) $(ASTESTS_V8_OUT) > $@
	chmod 755 $@

astests-v9-Runner: $(srcdir)/../generic-testrunner.sh $(ASTESTS_V9_OUT)
	GNASHRC="$(top_builddir)/testsuite/gnashrc" sh $(srcdir)/../generic-testrunner.sh $(top_builddir) $(HAXE_DIR)/$(ASTESTS_V9_OUT) > $@
	chmod 755 $@

# This version runs all testcases in a single SWF targetted at player 5
alltests-v5-Runner: $(srcdir)/../generic-testrunner.sh alltests-v5.swf
	GNASHRC="$(top_builddir)/testsuite/gnashrc" sh $(srcdir)/../generic-testrunner.sh $(top_builddir) alltests-v5.swf > $@
	chmod 755 $@

# This version runs all testcases in a single SWF targetted at player 6
alltests-v6-Runner: $(srcdir)/../generic-testrunner.sh alltests-v6.swf
	GNASHRC="$(top_builddir)/testsuite/gnashrc" sh $(srcdir)/../generic-testrunner.sh $(top_builddir) alltests-v6.swf > $@
	chmod 755 $@

# This version runs all testcases in a single SWF targetted at player 7
alltests-v7-Runner: $(srcdir)/../generic-testrunner.sh alltests-v7.swf
	GNASHRC="$(top_builddir)/testsuite/gnashrc" sh $(srcdir)/../generic-testrunner.sh $(top_builddir) alltests-v7.swf > $@
	chmod 755 $@

# This version runs all testcases in a single SWF targetted at player 8
alltests-v8-Runner: $(srcdir)/../generic-testrunner.sh alltests-v8.swf
	GNASHRC="$(top_builddir)/testsuite/gnashrc" sh $(srcdir)/../generic-testrunner.sh $(top_builddir) alltests-v8.swf > $@
	chmod 755 $@

$(ASTESTS_V5_OUT) : $(ASTESTS) Dejagnu.swf
	@$(MAKE) ${@:-v5.swf=.swf} SWFVERSION="5"
	@mv ${@:-v5.swf=.swf} $@

$(ASTESTS_V6_OUT) : $(ASTESTS) Dejagnu.swf
	@$(MAKE) ${@:-v6.swf=.swf} SWFVERSION="6"
	@mv ${@:-v6.swf=.swf} $@

$(ASTESTS_V7_OUT) : $(ASTESTS) Dejagnu.swf
	@$(MAKE) ${@:-v7.swf=.swf} SWFVERSION="7"
	@mv ${@:-v7.swf=.swf} $@

$(ASTESTS_V8_OUT) : $(ASTESTS) Dejagnu.swf
	@$(MAKE) ${@:-v8.swf=.swf} SWFVERSION="8"
	@mv ${@:-v8.swf=.swf} $@

$(ASTESTS_V9_OUT) : $(ASTESTS) Dejagnu.swf
	$(HAXE_CPP) $(srcdir)/${@:-v9.swf=.as} | $(PERL) $(HAXE_DIR)/as_to_hx.pl ${@:-v9.swf=} > $(HAXE_DIR)/${@:-v9.swf=_as.hx}
	$(HAXE) $(HAXE_FLAGS) -main $(@:-v9.swf=_as) -swf-version 9 -swf $(HAXE_DIR)/$@
#	@$(MAKE) ${@:-v9.swf=.swf} SWFVERSION="9"
#	@mv ${@:-v9.swf=.swf} $@

$(DEJAGNU_SO_PATH):
	$(MAKE) -C ../misc-ming.all Dejagnu.swf

Dejagnu.swf: $(DEJAGNU_SO_PATH)
	cp ../misc-ming.all/Dejagnu.swf .

clean-local:
	rm -Rf online-tests require-ming embed.html \
		index.html index.wiki \
		$(ASTESTS_VERSIONED_OUT) \
		$(ASTESTS_OUT) \
		$(ALLTESTS_VERSIONED_OUT) \
		alltests.swf \
		site.exp site.exp.bak \
		$(check_SCRIPTS) \
		Dejagnu.swf

check-DEJAGNU: site-update $(TEST_CASES)
	@runtest=$(RUNTEST); \
	if $(SHELL) -c "$$runtest --version" > /dev/null 2>&1; then \
	    GNASH_GC_TRIGGER_THRESHOLD=0 GNASHRC=../gnashrc $$runtest $(RUNTESTFLAGS) $(TEST_DRIVERS); true; \
	else \
	  echo "WARNING: could not find \`runtest'" 1>&2; \
          for i in "$(TEST_CASES)"; do \
	    GNASH_GC_TRIGGER_THRESHOLD=0 GNASHRC=../gnashrc $(SHELL) $$i; \
	  done; \
	fi

quickcheck: quicksite-update $(quickcheck_RUNNERS)
	@runtest=$(RUNTEST); \
	if $(SHELL) -c "$$runtest --version" > /dev/null 2>&1; then \
	    $$runtest $(RUNTESTFLAGS) $(TEST_DRIVERS); \
	else \
	  echo "WARNING: could not find \`runtest'" 1>&2; \
          for i in "$(quickcheck_RUNNERS)"; do \
	    $(SHELL) $$i; \
	  done; \
	fi

quicksite-update: site.exp
	@rm -fr site.exp.bak
	@cp site.exp site.exp.bak
	@sed -e '/testcases/d' site.exp.bak > site.exp
	@echo "# This is a list of the pre-compiled testcases" >> site.exp
	@echo "set testcases \"$(quickcheck_RUNNERS)\"" >> site.exp

site-update: site.exp
	@rm -fr site.exp.bak
	@cp site.exp site.exp.bak
	@sed -e '/testcases/d' site.exp.bak > site.exp
	@echo "# This is a list of the pre-compiled testcases" >> site.exp
	@echo "set testcases \"$(TEST_CASES)\"" >> site.exp

