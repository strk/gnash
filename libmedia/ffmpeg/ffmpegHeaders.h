// ffmpegHeaders.h - hide braindamage required to support ffmpeg includes in a single file
// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef GNASH_MEDIA_FFMPEG_HEADERS_H
#define GNASH_MEDIA_FFMPEG_HEADERS_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

/// Allows complilation of 64-bit constants on a 32-bit machine.
#ifndef __STDC_CONSTANT_MACROS
# define __STDC_CONSTANT_MACROS
#endif

// This is for compatibility with braindamaged versions of ffmpeg
#if !defined INT64_C
#if defined __WORDSIZE && __WORDSIZE == 64
#define INT64_C(c) c ## L
#else
#define INT64_C(c) c ## LL
#endif
#endif

#ifdef HAVE_FFMPEG_AVCODEC_H
extern "C" {
# include <ffmpeg/avcodec.h>
}
#endif

#ifdef HAVE_LIBAVCODEC_AVCODEC_H
extern "C" {
# include <libavcodec/avcodec.h>
}
#endif

#ifdef HAVE_FFMPEG_AVFORMAT_H
extern "C" {
#include <ffmpeg/avformat.h>
}
#endif

#ifdef HAVE_LIBAVFORMAT_AVFORMAT_H
extern "C" {
#include <libavformat/avformat.h>
}
#endif


#ifdef HAVE_SWSCALE_H
extern "C" {
#include <swscale.h>
}
#endif

#ifdef HAVE_FFMPEG_SWSCALE_H
extern "C" {
#include <ffmpeg/swscale.h>
}
#define HAVE_SWSCALE_H 1
#endif

#ifdef HAVE_LIBSWSCALE_SWSCALE_H
extern "C" {
#include <libswscale/swscale.h>
}
#define HAVE_SWSCALE_H 1
#endif

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52,94,1)
#define AV_SAMPLE_FMT_NONE SAMPLE_FMT_NONE
#define AV_SAMPLE_FMT_U8   SAMPLE_FMT_U8
#define AV_SAMPLE_FMT_S16  SAMPLE_FMT_S16
#define AV_SAMPLE_FMT_S32  SAMPLE_FMT_S32
#define AV_SAMPLE_FMT_FLT  SAMPLE_FMT_FLT
#define AV_SAMPLE_FMT_DBL  SAMPLE_FMT_DBL

#define AVSampleFormat SampleFormat
#endif

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55,18,102)
#define CODECID AVCodecID
#else
#define CODECID CodecID
#endif

#endif // GNASH_MEDIA_FFMPEG_HEADERS_H
