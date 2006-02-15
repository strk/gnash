## Process this fill with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#

AUTOMAKE_OPTIONS = dejagnu

# We don't need  --tool anymore
RUNTESTDEFAULTFLAGS = --srcdir $$srcdir
noinst_SCRIPTS = gen-test.sh

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

noinst_PROGRAMS = moviecliploader_test definebitsjpeg2 

definebitsjpeg2_SOURCES = definebitsjpeg2.c
definebitsjpeg2_LDADD = $(MING_LIBS)

moviecliploader_test_SOURCES =  moviecliploader_test.c
moviecliploader_test_LDADD = $(MING_LIBS)

# @@ Is this still needed ?
# @@ Should we add all generated files ?
## moviecliploader_test_ming.swf definebitsjpeg2.swf 
## 
GENERATED =			\
	array.swf		\
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
	Selection.swf		\
	SharedObject.swf	\
	Stage.swf		\
	System.swf		\
	TextSnapshot.swf	\
	Video.swf		\
	Object.swf		\
	LocalConnection.swf	\
	Inheritance.swf		\
	NetConnection.swf	\
	XML.swf			\
	Function.swf		

all: $(GENERATED)

swf: $(GENERATED)

index-html: 
	sh ./gen-index.sh $(GENERATED)

v8-online-tests: index-html
	MAKESWF_FLAGS="-DONLINE -DOUTPUT_VERSION=8 -v8" \
		make swf
	mkdir -p v8;
	cp index.html v8

v7-online-tests: index-html
	MAKESWF_FLAGS="-DONLINE -DOUTPUT_VERSION=7 -v7" \
		make swf
	mkdir -p v7;
	mv $(GENERATED) v7;
	cp index.html v7

v6-online-tests: index-html
	MAKESWF_FLAGS="-DONLINE -DOUTPUT_VERSION=6 -v6" \
		make swf
	mkdir -p v6;
	mv $(GENERATED) v6;
	cp index.html v6

v5-online-tests: index-html
	MAKESWF_FLAGS="-DONLINE -DOUTPUT_VERSION=5 -v5" \
		make swf
	mkdir -p v5;
	mv $(GENERATED) v5;
	cp index.html v5

online-tests: v5-online-tests v6-online-tests v7-online-tests v8-online-tests

test: $(GENERATED)
	gnash -1 -r 0 -v $(GENERATED)

# To use XTRACE output:
#  MAKESWF_FLAGS=-DUSE_XTRACE make Whatever.swf
.as.swf: 
	@if test x"$(MAKESWF)" != x; then \
	  $(MAKESWF) $(MAKESWF_FLAGS) -o $@ $< ; \
	else \
	  echo "You need to install Ming to compile"; \
	fi

.as.vswf: 
	@if test x"$(MAKESWF)" != x; then \
	  $(MAKESWF) -DUSE_XTRACE $(MAKESWF_FLAGS) -o $@ $< ; \
	else \
	  echo "You need to install Ming to compile"; \
	fi

moviecliploader_test_ming.swf: moviecliploader_test
	./moviecliploader_test

definebitsjpeg2.swf: definebitsjpeg2
	./definebitsjpeg2

clean:
	rm -f $(GENERATED) *.as.pp *.as.swf *.vswf
	rm -Rf v5 v6 v7 v8


