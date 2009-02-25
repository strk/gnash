# 
#   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = 

pkglib_LTLIBRARIES =  libgnashsound.la

libgnashsound_la_CPPFLAGS = \
	-I$(top_srcdir)/libbase	\
	-I$(top_srcdir)/libcore	\
    -I$(top_srcdir)/libmedia \
	$(PTHREAD_CFLAGS) \
	$(DMALLOC_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(INCLTDL) \
	$(SDL_CFLAGS) \
	$(NULL)

libgnashsound_la_LIBADD = \
	$(LIBLTDL) \
	$(LIBLTDLHEAD) \
	$(BOOST_LIBS) \
	$(PTHREAD_LIBS) \
	$(SDL_LIBS) \
	$(NULL)

libgnashsound_la_SOURCES = \
	sdl/sound_handler_sdl.cpp \
	sound_handler.cpp \
	EmbedSound.cpp \
	EmbedSoundInst.cpp \
	$(NULL)

noinst_HEADERS = \
	sdl/sound_handler_sdl.h \
	NullSoundHandler.h \
	sound_handler.h \
	SoundEnvelope.h \
	InputStream.h \
	EmbedSound.h \
	EmbedSoundInst.h \
    AuxStream.h
	$(NULL)


libgnashsound_la_LDFLAGS = -release $(VERSION)

if WIN32
  libgnashsound_la_LDFLAGS += -no-undefined
  libgnashsound_la_LIBADD += \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/libcore/libgnashcore.la \
	-lintl
endif

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.* *.obj

