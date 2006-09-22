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

# Linking Gnash statically or dynamically with other modules is making a
# combined work based on Gnash. Thus, the terms and conditions of the GNU
# General Public License cover the whole combination.
#
# As a special exception, the copyright holders of Gnash give you
# permission to combine Gnash with free software programs or libraries
# that are released under the GNU LGPL and with code included in any
# release of Talkback distributed by the Mozilla Foundation. You may
# copy and distribute such a system following the terms of the GNU GPL
# for all but the LGPL-covered parts and Talkback, and following the
# LGPL for the LGPL-covered parts.
#
# Note that people who make modified versions of Gnash are not obligated
# to grant this special exception for their modified versions; it is their
# choice whether to do so. The GNU General Public License gives permission
# to release a modified version without this exception; this exception
# also makes it possible to release a modified version which carries
# forward this exception.

#

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
        $(SDL_MIXER_CFLAGS)

# @@ Is this still needed ?
# @@ Should we add all generated files ?
## 
GENERATED =			\
	array.swf		\
	rtmp.swf 		\
	delete.swf		\
	Boolean.swf		\
	Camera.swf		\
	Color.swf		\
	ContextMenu.swf		\
	CustomActions.swf	\
	Date.swf		\
	Error.swf		\
	Global.swf		\
	LoadVars.swf		\
	Microphone.swf		\
	Mouse.swf		\
	MovieClip.swf		\
	NetStream.swf		\
	Number.swf		\
	Selection.swf		\
	SharedObject.swf	\
	Stage.swf		\
	String.swf		\
	System.swf		\
	TextSnapshot.swf	\
	Video.swf		\
	Object.swf		\
	LocalConnection.swf	\
	Inheritance.swf		\
	NetConnection.swf	\
	XML.swf			\
	Function.swf		\
	inheritance.swf

EXTRA_DIST = $(GENERATED:.swf=.as)

if ENABLE_MING
TARGET = $(GENERATED)
EXTRA_DIST += $(GENERATED)
else
TARGET = 
endif

CLEANFILES =                   \
      testrun.sum              \
      testrun.log

all: $(TARGET)

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

test: $(GENERATED)
	gnash -1 -r 0 -v $(GENERATED)

# To use XTRACE output:
#  MAKESWF_FLAGS=-DUSE_XTRACE make Whatever.swf
# or use the Whatever.vswf rule
.as.swf: 
	@if test x"$(MAKESWF)" != x; then \
	  if test x"$(SWFVERSION)" = x; then \
	   SWFVERSION="6" make $@; \
	  else \
	   $(MAKESWF) -DOUTPUT_VERSION=$(SWFVERSION) -v$(SWFVERSION) $(MAKESWF_FLAGS) -o $@ $< ; \
	  fi; \
	else \
	  echo "You need to install Ming to compile"; \
	fi

# Build with XTRACE output (visual swf)
.as.vswf: 
	@if test x"$(MAKESWF)" != x; then \
	  if test x"$(SWFVERSION)" = x; then \
	   SWFVERSION="6" make $@; \
	  else \
	   $(MAKESWF) -DUSE_XTRACE -DOUTPUT_VERSION=$(SWFVERSION) -v$(SWFVERSION) $(MAKESWF_FLAGS) -o $@ $< ; \
	  fi; \
	else \
	  echo "You need to install Ming to compile"; \
	fi

clean:
	rm -f $(GENERATED) *.as.pp *.as.swf *.vswf *swf.frame*.pp
	rm -Rf v5 v6 v7 v8


