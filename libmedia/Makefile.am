# 
#   Copyright (C) 2007 Free Software Foundation, Inc.
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

AUTOMAKE_OPTIONS = no-dependencies foreign

EXTRA_DIST = 

# this is where Gnash plugins get installed
pluginsdir = $(prefix)/lib/gnash/plugins

if INSTALL_LTDL
include_HEADERS =  $(top_srcdir)/libltdl/ltdl.h
LIBLTDLLIB = libltdl.la
LIBLTDLHEAD =  $(top_srcdir)/libltdl/ltdl.h
endif

# If we are using an installable libltdl, then it needs to be built
# before libgnash, as libgnash depends on it for extensions.
pkglib_LTLIBRARIES =  $(LIBLTDLLIB) libgnashmedia.la

libgnashmedia_la_CPPFLAGS = \
	-I.. -I$(srcdir) \
	-I$(top_srcdir)	\
	-I$(top_srcdir)/libamf	\
	-I$(top_srcdir)/backend	\
	-I$(top_srcdir)/libbase	\
	-I$(top_srcdir)/server	\
	$(PTHREAD_CFLAGS) \
	$(OPENGL_CFLAGS) \
	$(LIBXML_CFLAGS) \
	$(PNG_CFLAGS) \
	$(SDL_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GSTREAMER_CFLAGS) \
	$(CURL_CFLAGS) \
	$(Z_CFLAGS) \
	$(JPEG_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(INCLTDL) \
	$(NULL)

# These headers get installed
# include_HEADERS = log.h err.h proc.h serial.h xantrex.h outback.h

# Don't build Dmalloc support if it's not configured in
if HAVE_DMALLOC
DMALLOC_FILE = dlmalloc.c
endif

libgnashmedia_la_LIBADD = \
	$(JPEG_LIBS) \
	$(Z_LIBS) \
	$(CURL_LIBS) \
	$(OPENGL_LIBS) \
	$(LIBINTL) \
	$(LIBLTDL) \
	$(BOOST_LIBS) \
	$(PTHREAD_LIBS) \
	$(SDL_LIBS) \
	$(NULL)

libgnashmedia_la_SOURCES = \
	AudioDecoderNellymoser.cpp \
	AudioDecoderSimple.cpp \
	$(NULL)

noinst_HEADERS = \
	MediaBuffer.h \
	MediaDecoder.h \
	AudioDecoder.h \
	VideoDecoder.h \
	MediaParser.h \
	AudioDecoderNellymoser.h \
	AudioDecoderSimple.h \
	sound_handler.h \
	SoundInfo.h \
	$(NULL)


if USE_SOUND_GST
#libgnashmedia_la_SOURCES += gst/gstgnashsrc.c gst/VideoDecoderGst.cpp gst/gstappbuffer.c gst/gstappsink.c gst/gstappsrc.c gst/sound_handler_gst.cpp
libgnashmedia_la_SOURCES += gst/VideoDecoderGst.cpp gst/sound_handler_gst.cpp gst/gstgnashsrc.c
#noinst_HEADERS += gst/gstgnashsrc.h gst/gstappbuffer.h gst/VideoDecoderGst.h gst/gstappsink.h gst/gstappsrc.h gst/sound_handler_gst.h
noinst_HEADERS += gst/VideoDecoderGst.h gst/sound_handler_gst.h gst/gstgnashsrc.h
libgnashmedia_la_CPPFLAGS += $(GSTREAMER_CFLAGS)
libgnashmedia_la_LIBADD += $(GSTREAMER_LIBS) -lgstbase-0.10
endif

if USE_FFMPEG_ENGINE
#libgnashmedia_la_SOURCES += sdl/MediaDecoderSdl.cpp sdl/AudioDecoderFfmpeg.cpp sdl/VideoDecoderFfmpeg.cpp sdl/MediaParserFfmpeg.cpp sdl/sound_handler_sdl.cpp
libgnashmedia_la_SOURCES += sdl/VideoDecoderFfmpeg.cpp sdl/AudioDecoderFfmpeg.cpp sdl/sound_handler_sdl.cpp
#noinst_HEADERS += sdl/MediaDecoderSdl.h sdl/AudioDecoderFfmpeg.h sdl/VideoDecoderFfmpeg.h sdl/MediaParserFfmpeg.h sdl/sound_handler_sdl.h
noinst_HEADERS += sdl/AudioDecoderFfmpeg.h sdl/VideoDecoderFfmpeg.h sdl/sound_handler_sdl.h
libgnashmedia_la_LIBADD += $(FFMPEG_LIBS)
libgnashmedia_la_CPPFLAGS += $(FFMPEG_CFLAGS)
endif

if USE_MAD_ENGINE
#libgnashmedia_la_SOURCES += sdl/MediaDecoderSdl.cpp sdl/AudioDecoderMad.cpp sdl/sound_handler_sdl.cpp
libgnashmedia_la_SOURCES += sdl/AudioDecoderMad.cpp sdl/sound_handler_sdl.cpp
#noinst_HEADERS += sdl/MediaDecoderSdl.h sdl/AudioDecoderMad.h sdl/sound_handler_sdl.h
noinst_HEADERS += sdl/AudioDecoderMad.h sdl/sound_handler_sdl.h
libgnashmedia_la_LIBADD += $(MAD_LIBS)
libgnashmedia_la_CPPFLAGS += $(MAD_CFLAGS)
endif

libgnashmedia_la_LDFLAGS = -release $(VERSION)


# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.* *.obj

