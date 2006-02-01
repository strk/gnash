## Process this fillocatee with automake to generate Makefile.in
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

if ENABLE_MING

LDFLAGS += $(MING_LIBS)

# moviecliploader_test_ming.swf definebitsjpeg2.swf 
GENERATED = xtrace.swf movieclip.swf array.swf

all: $(GENERATED)

swf: $(GENERATED)

test: $(GENERATED)
	gprocessor -v $(GENERATED)

.as.swf: 
	$(MAKESWF) -o $@ $<

moviecliploader_test: moviecliploader_test.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $<

moviecliploader_test_ming.swf: moviecliploader_test
	./moviecliploader_test

definebitsjpeg2: definebitsjpeg2.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $<

definebitsjpeg2.swf: definebitsjpeg2
	./definebitsjpeg2

clean:
	rm -f $(GENERATED)
	rm -f $(GENERATED) *.as.pp

endif # ENABLE_MING
