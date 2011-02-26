# 
#   Copyright (C) 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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

AM_CXXFLAGS = -DREGISTER_MEDIA_HANDLERS

#if INSTALL_LTDL
#LIBLTDLHEAD =  $(top_srcdir)/libltdl/ltdl.h
#endif

# If we are using an installable libltdl, then it needs to be built
# before libgnash, as libgnash depends on it for extensions.
pkglib_LTLIBRARIES =  libgnashmedia.la

libgnashmedia_la_CPPFLAGS = \
	-I$(top_srcdir)/libbase	\
	$(PTHREAD_CFLAGS) \
	$(OPENGL_CFLAGS) \
	$(PNG_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(CURL_CFLAGS) \
	$(Z_CFLAGS) \
	$(JPEG_CFLAGS) \
	$(GIF_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(NULL)

if ANDROID
  libgnashmedia_la_CPPFLAGS += $(ANDROID_NDK)/usr/include
endif


# These headers get installed
# include_HEADERS = log.h err.h

libgnashmedia_la_LIBADD = \
	$(top_builddir)/libbase/libgnashbase.la \
	$(FFMPEG_LIBS) \
	$(PNG_LIBS) \
	$(JPEG_LIBS) \
	$(GIF_LIBS) \
	$(Z_LIBS) \
	$(CURL_LIBS) \
	$(OPENGL_LIBS) \
	$(BOOST_LIBS) \
	$(PTHREAD_LIBS) \
	$(NULL)

libgnashmedia_la_SOURCES = \
	MediaHandler.cpp \
	AudioDecoderSimple.cpp \
	MediaParser.cpp \
	FLVParser.cpp \
	AudioResampler.cpp \
	$(NULL)

noinst_HEADERS = \
	AudioDecoder.h \
	VideoDecoder.h \
	VideoConverter.h \
	MediaParser.h \
	MediaHandler.h \
	FLVParser.h \
	AudioDecoderSimple.h \
	SoundInfo.h \
	AudioResampler.h \
	AudioInput.h \
	VideoInput.h \
	$(NULL)

if USE_GST_ENGINE
   libgnashmedia_la_SOURCES += \
	gst/MediaHandlerGst.cpp \
	gst/VideoDecoderGst.cpp \
	gst/AudioDecoderGst.cpp \
	gst/MediaParserGst.cpp \
	gst/VideoConverterGst.cpp \
	gst/GstUtil.cpp \
	gst/swfdec_codec_gst.c \
	gst/VideoInputGst.cpp \
	gst/AudioInputGst.cpp \
	$(NULL)

   noinst_HEADERS += \
	gst/MediaHandlerGst.h \
	gst/AudioDecoderGst.h \
	gst/VideoDecoderGst.h \
	gst/MediaParserGst.h \
	gst/VideoConverterGst.h \
	gst/GstUtil.h \
	gst/swfdec_codec_gst.h \
	gst/gnash_gst_version.h \
	gst/VideoInputGst.h \
	gst/AudioInputGst.h \
	$(NULL)

   libgnashmedia_la_CPPFLAGS += \
	-I$(top_srcdir)/libmedia/gst \
	$(GSTREAMER_CFLAGS) \
	$(GSTAPP_CFLAGS) \
	$(GSTREAMER_PBUTILS_CFLAGS) \
	$(NULL)

   libgnashmedia_la_LIBADD += \
	-lgstbase-0.10 \
	$(GSTREAMER_APP_LIBS) \
	$(GSTREAMER_PBUTILS_LIBS) \
	$(GSTREAMER_PLUGINS_BASE_LIBS) \
	$(GSTREAMER_LIBS) \
	$(NULL)

endif

if USE_FFMPEG_ENGINE
   libgnashmedia_la_SOURCES += \
	ffmpeg/MediaHandlerFfmpeg.cpp \
	ffmpeg/MediaParserFfmpeg.cpp \
	ffmpeg/AudioDecoderFfmpeg.cpp \
	ffmpeg/VideoDecoderFfmpeg.cpp \
	ffmpeg/AudioResamplerFfmpeg.cpp \
	ffmpeg/VideoConverterFfmpeg.cpp \
	ffmpeg/VideoInputFfmpeg.cpp \
	ffmpeg/AudioInputFfmpeg.cpp \
	$(NULL)

   noinst_HEADERS += \
	ffmpeg/MediaHandlerFfmpeg.h \
	ffmpeg/MediaParserFfmpeg.h \
	ffmpeg/AudioDecoderFfmpeg.h \
	ffmpeg/VideoDecoderFfmpeg.h \
	ffmpeg/AudioResamplerFfmpeg.h \
	ffmpeg/ffmpegHeaders.h \
	ffmpeg/VideoConverterFfmpeg.h \
	ffmpeg/VideoInputFfmpeg.h \
	ffmpeg/AudioInputFfmpeg.h \
	$(NULL)

   libgnashmedia_la_LIBADD += \
	$(FFMPEG_LIBS) \
	$(NULL)

   libgnashmedia_la_CPPFLAGS += \
	$(FFMPEG_CFLAGS) \
	$(NULL)

if HAVE_VAAPI
   libgnashmedia_la_SOURCES += \
		ffmpeg/VideoDecoderFfmpegVaapi.cpp \
		$(NULL)

   noinst_HEADERS += \
		ffmpeg/VideoDecoderFfmpegVaapi.h \
		$(NULL)
endif
endif

if USE_HAIKU_ENGINE
   libgnashmedia_la_SOURCES += \
	haiku/MediaHandlerHaiku.cpp \
	haiku/MediaParserHaiku.cpp \
	haiku/AudioDecoderHaiku.cpp \
	haiku/VideoDecoderHaiku.cpp \
	haiku/VideoConverterHaiku.cpp \
	haiku/VideoInputHaiku.cpp \
	haiku/AudioInputHaiku.cpp \
	$(NULL)

   noinst_HEADERS += \
	haiku/MediaHandlerHaiku.h \
	haiku/MediaParserHaiku.h \
	haiku/AudioDecoderHaiku.h \
	haiku/VideoDecoderHaiku.h \
	haiku/VideoConverterHaiku.h \
	haiku/VideoInputHaiku.h \
	haiku/AudioInputHaiku.h \
	$(NULL)

   libgnashmedia_la_LIBADD += \
	-lmedia \
	$(NULL)

   libgnashmedia_la_CPPFLAGS += \
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

   libgnashmedia_la_CPPFLAGS += \
	-I$(top_srcdir)/libvaapi \
	$(NULL)

if HAVE_VAAPI
   libgnashmedia_la_LIBADD += \
	$(top_builddir)/libvaapi/libgnashvaapi.la \
	$(NULL)
endif

if WIN32
  libgnashmedia_la_LDFLAGS += -no-undefined
  libgnashmedia_la_LIBADD += \
	-lintl
endif

if ENABLE_PCH
AM_CXXFLAGS += $(PCH_FLAGS)
endif

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.* *.obj

# Remove libtool .la files
install-exec-hook:
	$(RM) $(DESTDIR)$(libdir)/gnash/libgnashmedia.la
