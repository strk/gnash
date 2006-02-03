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

definebitsjpeg2_SOURCES = moviecliploader_test.c
definebitsjpeg2_LDADD = $(MING_LIBS)

moviecliploader_test_SOURCES = definebitsjpeg2.c
moviecliploader_test_LDADD = $(MING_LIBS)

# moviecliploader_test_ming.swf definebitsjpeg2.swf 
GENERATED = xtrace.swf movieclip.swf array.swf

all: $(GENERATED)

swf: $(GENERATED)

test: $(GENERATED)
	gprocessor -v $(GENERATED)

# To use XTRACE output:
#  MAKESWF_FLAGS=-DUSE_XTRACE make Whatever.swf
.as.swf: 
	@if test x"$(MAKESWF)" != x; then \
	  $(MAKESWF) $(MAKESWF_FLAGS) -o $@ $< ; \
	else \
	  echo "You need to install Ming to compile"; \
	fi

moviecliploader_test_ming.swf: moviecliploader_test
	./moviecliploader_test

definebitsjpeg2.swf: definebitsjpeg2
	./definebitsjpeg2

clean:
	rm -f $(GENERATED) *.as.pp


