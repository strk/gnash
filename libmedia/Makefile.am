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

AM_CXXFLAGS = -DREGISTER_MEDIA_HANDLERS

# If we are using an installable libltdl, then it needs to be built
# before libgnash, as libgnash depends on it for extensions.
pkglib_LTLIBRARIES = libgnashmedia.la

libgnashmedia_la_SOURCES = \
	AudioDecoder.h \
	AudioDecoderSimple.cpp \
	AudioDecoderSimple.h \
	AudioResampler.cpp \
	AudioResampler.h \
	FLVParser.cpp \
	FLVParser.h \
	MediaHandler.cpp \
	MediaHandler.h \
	MediaParser.cpp \
	MediaParser.h \
	SoundInfo.h \
	VideoConverter.h \
	$(NULL)

if USE_GST_ENGINE
libgnashmedia_la_SOURCES += \
	AudioInput.h \
	gst/AudioDecoderGst.cpp \
	gst/AudioDecoderGst.h \
	gst/AudioInputGst.cpp \
	gst/AudioInputGst.h \
	gst/GstUtil.cpp \
	gst/GstUtil.h \
	gst/MediaHandlerGst.cpp \
	gst/MediaHandlerGst.h \
	gst/MediaParserGst.cpp \
	gst/MediaParserGst.h \
	gst/swfdec_codec_gst.c \
	gst/swfdec_codec_gst.h \
	gst/VideoConverterGst.cpp \
	gst/VideoConverterGst.h \
	gst/VideoDecoderGst.cpp \
	gst/VideoDecoderGst.h \
	gst/VideoInputGst.cpp \
	gst/VideoInputGst.h \
	$(NULL)
endif

if USE_FFMPEG_ENGINE
libgnashmedia_la_SOURCES += \
	AudioInput.h \
	ffmpeg/AudioDecoderFfmpeg.cpp \
	ffmpeg/AudioDecoderFfmpeg.h \
	ffmpeg/AudioInputFfmpeg.cpp \
	ffmpeg/AudioInputFfmpeg.h \
	ffmpeg/AudioResamplerFfmpeg.cpp \
	ffmpeg/AudioResamplerFfmpeg.h \
	ffmpeg/ffmpegHeaders.h \
	ffmpeg/MediaHandlerFfmpeg.cpp \
	ffmpeg/MediaHandlerFfmpeg.h \
	ffmpeg/MediaParserFfmpeg.cpp \
	ffmpeg/MediaParserFfmpeg.h \
	ffmpeg/VideoConverterFfmpeg.cpp \
	ffmpeg/VideoConverterFfmpeg.h \
	ffmpeg/VideoDecoderFfmpeg.cpp \
	ffmpeg/VideoDecoderFfmpeg.h \
	ffmpeg/VideoInputFfmpeg.cpp \
	ffmpeg/VideoInputFfmpeg.h \
	VideoDecoder.h \
	VideoInput.h \
	$(NULL)

if HAVE_VAAPI
libgnashmedia_la_SOURCES += \
	ffmpeg/VideoDecoderFfmpegVaapi.cpp \
	ffmpeg/VideoDecoderFfmpegVaapi.h \
	$(NULL)
endif
endif

if USE_HAIKU_ENGINE
libgnashmedia_la_SOURCES += \
	haiku/adipe.h \
	haiku/AudioDecoderHaiku.cpp \
	haiku/AudioDecoderHaiku.h \
	haiku/AudioInputHaiku.cpp \
	haiku/AudioInputHaiku.h \
	haiku/MediaHandlerHaiku.cpp \
	haiku/MediaHandlerHaiku.h \
	haiku/MediaParserHaiku.cpp \
	haiku/MediaParserHaiku.h \
	haiku/VideoConverterHaiku.cpp \
	haiku/VideoConverterHaiku.h \
	haiku/VideoDecoderHaiku.cpp \
	haiku/VideoDecoderHaiku.h \
	haiku/VideoInputHaiku.cpp \
	haiku/VideoInputHaiku.h \
	$(NULL)
endif

if HAVE_SPEEX
libgnashmedia_la_SOURCES += \
	AudioDecoderSpeex.cpp \
	AudioDecoderSpeex.h \
	$(NULL)
endif

libgnashmedia_la_CPPFLAGS = \
	-I$(top_srcdir)/libbase	\
	$(PTHREAD_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(NULL)

if ANDROID
libgnashmedia_la_CPPFLAGS += $(ANDROID_NDK)/usr/include
endif

if USE_GST_ENGINE
libgnashmedia_la_CPPFLAGS += \
	-I$(top_srcdir)/libmedia/gst \
	$(GSTREAMER_CFLAGS) \
	$(GSTAPP_CFLAGS) \
	$(GSTREAMER_PBUTILS_CFLAGS) \
	$(NULL)
endif

if USE_FFMPEG_ENGINE
libgnashmedia_la_CPPFLAGS += \
	$(FFMPEG_CFLAGS) \
	$(NULL)
endif

if USE_HAIKU_ENGINE
libgnashmedia_la_CPPFLAGS += \
	$(NULL)
endif

if HAVE_SPEEX
libgnashmedia_la_CPPFLAGS += \
	$(SPEEX_CFLAGS) \
	$(SPEEXDSP_CFLAGS) \
	$(NULL)
endif

if HAVE_VAAPI
libgnashmedia_la_CPPFLAGS += \
	-I$(top_srcdir)/libvaapi \
	$(NULL)
endif

libgnashmedia_la_LIBADD = \
	$(top_builddir)/libbase/libgnashbase.la \
	$(BOOST_LIBS) \
	$(NULL)

if USE_GST_ENGINE
libgnashmedia_la_LIBADD += \
	-lgstbase-0.10 \
	$(GSTREAMER_APP_LIBS) \
	$(GSTREAMER_PBUTILS_LIBS) \
	$(GSTREAMER_PLUGINS_BASE_LIBS) \
	$(GSTREAMER_LIBS) \
	$(NULL)
endif

if USE_FFMPEG_ENGINE
libgnashmedia_la_LIBADD += \
	$(FFMPEG_LIBS) \
	$(NULL)
endif

if USE_HAIKU_ENGINE
libgnashmedia_la_LIBADD += \
	-lmedia \
	$(NULL)
endif

if HAVE_SPEEX
libgnashmedia_la_LIBADD += \
	$(SPEEX_LIBS) \
	$(SPEEXDSP_LIBS) \
	$(NULL)
endif

if HAVE_VAAPI
libgnashmedia_la_LIBADD += \
	$(top_builddir)/libvaapi/libgnashvaapi.la \
	$(NULL)
endif

libgnashmedia_la_LDFLAGS = -release $(VERSION)

if WIN32
libgnashmedia_la_LDFLAGS += -no-undefined
libgnashmedia_la_LIBADD += -lintl
endif

if ENABLE_PCH
AM_CXXFLAGS += $(PCH_FLAGS)
endif

# Remove libtool .la files
install-exec-hook:
	$(RM) $(DESTDIR)$(libdir)/gnash/libgnashmedia.la

uninstall-local:
	$(RM) $(DESTDIR)$(libdir)/gnash/libgnashmedia-*.so
