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

pkglib_LTLIBRARIES = libgnashsound.la

libgnashsound_la_SOURCES = \
	AuxStream.h \
	EmbedSound.cpp \
	EmbedSound.h \
	StreamingSoundData.cpp \
	StreamingSoundData.h \
	StreamingSound.cpp \
	StreamingSound.h \
	EmbedSoundInst.cpp \
	EmbedSoundInst.h \
	SoundUtils.h \
	InputStream.h \
	sound_handler.cpp \
	sound_handler.h \
	SoundEnvelope.h \
	WAVWriter.cpp \
	WAVWriter.h \
	$(NULL)

if USE_SOUND_SDL
libgnashsound_la_SOURCES += \
	sdl/sound_handler_sdl.cpp \
	sdl/sound_handler_sdl.h \
	$(NULL)
endif

if USE_SOUND_AHI
libgnashsound_la_SOURCES += \
	aos4/sound_handler_ahi.cpp \
	aos4/sound_handler_ahi.h \
	$(NULL)
endif

if USE_SOUND_MKIT
libgnashsound_la_SOURCES += \
	mkit/sound_handler_mkit.cpp \
	mkit/sound_handler_mkit.h \
	$(NULL)
endif

noinst_HEADERS = \
	NullSoundHandler.h \
	$(NULL)

libgnashsound_la_CPPFLAGS = \
	-I$(top_srcdir)/libbase	\
	-I$(top_srcdir)/libmedia \
	$(PTHREAD_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(SDL_CFLAGS) \
	$(NULL)

libgnashsound_la_LIBADD = \
	$(top_builddir)/libmedia/libgnashmedia.la \
	$(top_builddir)/libbase/libgnashbase.la \
	$(BOOST_LIBS) \
	$(SDL_LIBS) \
	$(NULL)

if USE_SOUND_MKIT
libgnashsound_la_LIBADD += -lmedia
endif

libgnashsound_la_LDFLAGS = -release $(VERSION)

if WIN32
libgnashsound_la_LDFLAGS += -no-undefined
libgnashsound_la_LIBADD += -lintl
endif

if ENABLE_PCH
AM_CXXFLAGS = $(PCH_FLAGS)
endif

# Remove libtool .la files
install-exec-hook:
	$(RM) $(DESTDIR)$(libdir)/gnash/libgnashsound.la

uninstall-local:
	$(RM) $(DESTDIR)$(libdir)/gnash/libgnashsound-*.so
