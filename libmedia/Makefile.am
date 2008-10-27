# 
#   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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

# this is where Gnash plugins get installed
pluginsdir = $(prefix)/lib/gnash/plugins

#if INSTALL_LTDL
#LIBLTDLHEAD =  $(top_srcdir)/libltdl/ltdl.h
#endif

# If we are using an installable libltdl, then it needs to be built
# before libgnash, as libgnash depends on it for extensions.
pkglib_LTLIBRARIES =  libgnashmedia.la

libgnashmedia_la_CPPFLAGS = \
	-I$(top_srcdir)/libamf	\
	-I$(top_srcdir)/libnet \
	-I$(top_srcdir)/backend	\
	-I$(top_srcdir)/libbase	\
	-I$(top_srcdir)/libcore	\
	-I$(top_srcdir)/libcore/vm \
	-I$(top_srcdir)/libcore/parser \
	$(PTHREAD_CFLAGS) \
	$(DMALLOC_CFLAGS) \
	$(OPENGL_CFLAGS) \
	$(LIBXML_CFLAGS) \
	$(LIBPNG_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(CURL_CFLAGS) \
	$(Z_CFLAGS) \
	$(JPEG_CFLAGS) \
	$(GIF_CFLAGS) \
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
	$(PNG_LIBS) \
	$(JPEG_LIBS) \
	$(GIF_LIBS) \
	$(Z_LIBS) \
	$(CURL_LIBS) \
	$(OPENGL_LIBS) \
	$(LIBLTDL) \
	$(LIBLTDLHEAD) \
	$(BOOST_LIBS) \
	$(PTHREAD_LIBS) \
	$(NULL)

libgnashmedia_la_SOURCES = \
	MediaHandler.cpp \
	AudioDecoderNellymoser.cpp \
	AudioDecoderSimple.cpp \
	MediaParser.cpp \
	FLVParser.cpp \
	Util.cpp \
	$(NULL)

noinst_HEADERS = \
	AudioDecoder.h \
	VideoDecoder.h \
	MediaParser.h \
	MediaHandler.h \
	FLVParser.h \
	AudioDecoderNellymoser.h \
	AudioDecoderSimple.h \
	SoundInfo.h \
	Util.h \
	$(NULL)


if USE_GST_ENGINE
   libgnashmedia_la_SOURCES += \
		gst/MediaHandlerGst.cpp \
		gst/VideoDecoderGst.cpp \
		gst/AudioDecoderGst.cpp \
		gst/MediaParserGst.cpp \
		gst/GstUtil.cpp \
		gst/swfdec_codec_gst.c \
		$(NULL)
	
   noinst_HEADERS += \
		gst/MediaHandlerGst.h \
		gst/AudioDecoderGst.h \
		gst/VideoDecoderGst.h \
		gst/MediaParserGst.h \
		gst/GstUtil.h \
		gst/swfdec_codec_gst.h \
		gst/gnash_gst_version.h \
		$(NULL)

   libgnashmedia_la_CPPFLAGS += \
		-I$(top_srcdir)/libmedia/gst \
		$(GSTREAMER_CFLAGS) \
		$(NULL)

   libgnashmedia_la_LIBADD += \
		$(GSTREAMER_LIBS) -lgstbase-0.10 \
		$(NULL)

endif

if USE_FFMPEG_ENGINE
   libgnashmedia_la_SOURCES += \
		ffmpeg/MediaHandlerFfmpeg.cpp \
		ffmpeg/MediaParserFfmpeg.cpp \
		ffmpeg/AudioDecoderFfmpeg.cpp \
		ffmpeg/VideoDecoderFfmpeg.cpp \
		ffmpeg/AudioResamplerFfmpeg.cpp \
		$(NULL)

   noinst_HEADERS += \
		ffmpeg/MediaHandlerFfmpeg.h \
		ffmpeg/MediaParserFfmpeg.h \
		ffmpeg/AudioDecoderFfmpeg.h \
		ffmpeg/VideoDecoderFfmpeg.h \
		ffmpeg/AudioResamplerFfmpeg.h \
		ffmpeg/ffmpegHeaders.h \
		$(NULL)

   libgnashmedia_la_LIBADD += \
		$(FFMPEG_LIBS) \
		$(NULL)

   libgnashmedia_la_CPPFLAGS += \
		$(FFMPEG_CFLAGS) \
		$(NULL)
endif

if HAVE_SPEEX
   libgnashmedia_la_LIBADD += \
	$(SPEEX_LIBS) \
	$(SPEEXDSP_LIBS) \
	$(NULL)
   libgnashmedia_la_CPPFLAGS += \
	$(SPEEX_CFLAGS) \
	$(SPEEXDSP_CFLAGS) \
	$(NULL)
   libgnashmedia_la_SOURCES += AudioDecoderSpeex.cpp
   noinst_HEADERS += AudioDecoderSpeex.h
endif

libgnashmedia_la_LDFLAGS = -release $(VERSION)

if WIN32
  libgnashmedia_la_LDFLAGS += -no-undefined
  libgnashmedia_la_LIBADD += \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/libcore/libgnashcore.la \
	-lintl
endif

#libltdlc_la_CPPFLAGS = $(LTDLINCL)
#if CONVENIENCE_LTDL
#noinst_LTLIBRARIES = libltdlc.la
#libltdlc_la_SOURCES = $(top_srcdir)/libltdl/ltdl.c
#endif
#CLEANFILES = libltdl.la libltdlc.la

#libltdl_la_SOURCES = $(top_srcdir)/libltdl/ltdl.c
#libltdl_la_LDFLAGS = -no-undefined -version-info 4:4:1
#libltdl_la_LIBADD = $(LIBADD_DL)

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.* *.obj

