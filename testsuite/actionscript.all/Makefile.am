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

# $Id: Makefile.am,v 1.46 2006/11/17 00:06:19 strk Exp $

AUTOMAKE_OPTIONS = dejagnu

# We don't need  --tool anymore
RUNTESTDEFAULTFLAGS = swf_exists.exp
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

# @@ Is this still needed ?
# @@ Should we add all generated files ?
#
# @@@ YES, we use this in the online-tests rule !
# @@@ --strk Tue Oct 17 18:49:30 CEST 2006
## 
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

GENERATED = $(ASTESTS:.as=.swf) \
	astests-v5.swf \
	astests-v6.swf \
	astests-v7.swf \
	astests-v8.swf \
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


MAINTAINERCLEANFILES = $(GENERATED) 

all:

check-local: $(TARGET)

swf: $(TARGET)

index-html: 
	sh $(srcdir)/gen-index.sh $(GENERATED)

v8-online-tests: index-html
	MAKESWF_FLAGS="-DONLINE" SWFVERSION="8" make swf
	mkdir -p v8;
	mv $(GENERATED) v8;
	cp index.html embed.html v8

v7-online-tests: index-html
	MAKESWF_FLAGS="-DONLINE" SWFVERSION="7" make swf
	mkdir -p v7;
	mv $(GENERATED) v7;
	cp index.html embed.html v7

v6-online-tests: index-html
	MAKESWF_FLAGS="-DONLINE" SWFVERSION="6" make swf
	mkdir -p v6;
	mv $(GENERATED) v6;
	cp index.html embed.html v6

v5-online-tests: index-html
	MAKESWF_FLAGS="-DONLINE" SWFVERSION="5" make swf
	mkdir -p v5;
	mv $(GENERATED) v5;
	cp index.html embed.html v5

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
$(GENERATED) astests.swf: require-ming dejagnu.as check.as dejagnu_so_init.as dejagnu_so_fini.as ../misc-ming.all/Dejagnu.swf

# Wrap a single .as tests in dejagnu_so importer.
# At play time, if the relative url '../ming-misc.all/Dejagnu.swf' 
# takes to the Dejagnu.swf shared library we'll get visual traces,
# otherwise normal traces will be used.
.as.swf: 
	@if test x"$(SWFVERSION)" = x; then \
	   SWFVERSION="6" make $@; \
	else \
	   $(MAKESWF) \
		-i../misc-ming.all/Dejagnu.swf:dejagnu \
		-DUSE_DEJAGNU_MODULE \
		-DOUTPUT_VERSION=$(SWFVERSION) -v$(SWFVERSION) \
		$(MAKESWF_FLAGS) \
		-o $@ \
		$(srcdir)/dejagnu_so_init.as \
		$(@:%.swf=$(srcdir)/%.as) \
		$(srcdir)/dejagnu_so_fini.as; \
	fi

# Wrap ALL .as tests in dejagnu_so importer.
# At play time, if the relative url '../ming-misc.all/Dejagnu.swf' 
# takes to the Dejagnu.swf shared library we'll get visual traces,
# otherwise normal traces will be used.
astests.swf: $(ASTESTS)
	@if test x"$(SWFVERSION)" = x; then \
	   SWFVERSION="6" make $@; \
	else \
	   $(MAKESWF) \
		-i../misc-ming.all/Dejagnu.swf:dejagnu \
		-DUSE_DEJAGNU_MODULE \
		-DOUTPUT_VERSION=$(SWFVERSION) -v$(SWFVERSION) \
		$(MAKESWF_FLAGS) \
		-o $@ \
		$(srcdir)/dejagnu_so_init.as \
		$(ASTESTS:%=$(srcdir)/%) \
		$(srcdir)/dejagnu_so_fini.as; \
	fi

astests-v5.swf: $(ASTESTS)
	SWFVERSION=5 $(MAKE) astests.swf
	mv astests.swf astests-v5.swf

astests-v6.swf: $(ASTESTS)
	SWFVERSION=6 $(MAKE) astests.swf
	mv astests.swf astests-v6.swf

astests-v7.swf: $(ASTESTS)
	SWFVERSION=7 $(MAKE) astests.swf
	mv astests.swf astests-v7.swf

astests-v8.swf: $(ASTESTS)
	SWFVERSION=8 $(MAKE) astests.swf
	mv astests.swf astests-v8.swf

clean-local:
	rm -Rf v5 v6 v7 v8 require-ming

