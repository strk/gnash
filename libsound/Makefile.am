# 
#   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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
	-I$(top_srcdir)/libmedia \
	$(DMALLOC_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(SDL_CFLAGS) \
	$(NULL)

libgnashsound_la_LIBADD = \
	$(top_builddir)/libmedia/libgnashmedia.la \
	$(top_builddir)/libbase/libgnashbase.la \
	$(BOOST_LIBS) \
	$(SDL_LIBS) \
	$(NULL)

libgnashsound_la_SOURCES = \
	sound_handler.cpp \
	EmbedSound.cpp \
	EmbedSoundInst.cpp \
	WAVWriter.cpp \
	$(NULL)

noinst_HEADERS = \
	NullSoundHandler.h \
	sound_handler.h \
	SoundEnvelope.h \
	InputStream.h \
	EmbedSound.h \
	EmbedSoundInst.h \
	AuxStream.h \
	WAVWriter.h \
	$(NULL)

if USE_SOUND_SDL
 libgnashsound_la_SOURCES += sdl/sound_handler_sdl.cpp 
 noinst_HEADERS += sdl/sound_handler_sdl.h
endif
if USE_SOUND_AHI
 libgnashsound_la_SOURCES += aos4/sound_handler_ahi.cpp 
 noinst_HEADERS += aos4/sound_handler_ahi.h
endif

if USE_SOUND_MKIT
 libgnashsound_la_SOURCES += mkit/sound_handler_mkit.cpp
 libgnashsound_la_LIBADD += -lmedia
 noinst_HEADERS += mkit/sound_handler_mkit.h
endif

if ENABLE_PCH
AM_CXXFLAGS = $(PCH_FLAGS)
endif

libgnashsound_la_LDFLAGS = -release $(VERSION)

if WIN32
  libgnashsound_la_LDFLAGS += -no-undefined
  libgnashsound_la_LIBADD += \
	-lintl
endif

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.* *.obj

# Remove libtool .la files
install-exec-hook:
	$(RM) $(DESTDIR)$(libdir)/gnash/libgnashsound.la

uninstall-local:
	$(RM) $(DESTDIR)$(libdir)/gnash/libgnashsound-*.so
