// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
//   Copyright (C) 1999-2008 the VideoLAN team
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA    02110-1301    USA

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "VideoConverterFfmpeg.h"
#include "GnashException.h"
#include "ffmpegHeaders.h"
#include "log.h"

namespace gnash {
namespace media {
namespace ffmpeg {

#ifdef HAVE_SWSCALE_H
/// A wrapper round an SwsContext that ensures it's
/// freed on destruction.
class SwsContextWrapper
{
public:

    SwsContextWrapper(SwsContext* context)
        :
        _context(context)
    {}

    ~SwsContextWrapper()
    {
         sws_freeContext(_context);
    }
    
    SwsContext* getContext() const { return _context; }

private:
    SwsContext* _context;

};
#endif

// The lookup table in this function is adapted from chroma.c from the VLC
// codebase; its license permits distribution under GPLv3 and later.
PixelFormat
fourcc_to_ffmpeg(ImgBuf::Type4CC code)
{

#define GNASH_FOURCC( a, b, c, d ) \
              ( ((uint32_t)a) | ( ((uint32_t)b) << 8 ) \
              | ( ((uint32_t)c) << 16 ) | ( ((uint32_t)d) << 24 ) )   

    static const struct
    {
        ImgBuf::Type4CC  fourcc;
        PixelFormat ffmpegcode;
    } pixfmt_table[] =
    {
        // Planar YUV formats
        {GNASH_FOURCC('I','4','4','4'), PIX_FMT_YUV444P},
        {GNASH_FOURCC('J','4','4','4'), PIX_FMT_YUVJ444P},

#if LIBAVUTIL_VERSION_INT >= ((49<<16)+(5<<8)+0)
        {GNASH_FOURCC('I','4','4','0'), PIX_FMT_YUV440P},
        {GNASH_FOURCC('J','4','4','0'), PIX_FMT_YUVJ440P},
#endif

        {GNASH_FOURCC('I','4','2','2'), PIX_FMT_YUV422P},
        {GNASH_FOURCC('J','4','2','2'), PIX_FMT_YUVJ422P},

        {GNASH_FOURCC('I','4','2','0'), PIX_FMT_YUV420P},
        {GNASH_FOURCC('Y','V','1','2'), PIX_FMT_YUV420P},
        {GNASH_FOURCC('I','Y','U','V'), PIX_FMT_YUV420P},
        {GNASH_FOURCC('J','4','2','0'), PIX_FMT_YUVJ420P},
        {GNASH_FOURCC('I','4','1','1'), PIX_FMT_YUV411P},
        {GNASH_FOURCC('I','4','1','0'), PIX_FMT_YUV410P},
        {GNASH_FOURCC('Y','V','U','9'), PIX_FMT_YUV410P},

#if LIBAVUTIL_VERSION_INT >= ((49<<16)+(0<<8)+1)
        {GNASH_FOURCC('N','V','1','2'), PIX_FMT_NV12},
        {GNASH_FOURCC('N','V','2','1'), PIX_FMT_NV21},
#endif

        {GNASH_FOURCC('Y','U','Y','2'), PIX_FMT_YUYV422},
        {GNASH_FOURCC('Y','U','Y','V'), PIX_FMT_YUYV422},
        {GNASH_FOURCC('U','Y','V','Y'), PIX_FMT_UYVY422},
        {GNASH_FOURCC('Y','4','1','1'), PIX_FMT_UYYVYY411},

        { 0, PIX_FMT_NONE}
    };
#undef GNASH_FOURCC

    for (int i = 0; pixfmt_table[i].fourcc != 0; i++ ) {
    
        if (pixfmt_table[i].fourcc == code) {
            return pixfmt_table[i].ffmpegcode;
        }
    }
   
    return PIX_FMT_NONE;
}

VideoConverterFfmpeg::VideoConverterFfmpeg(ImgBuf::Type4CC srcFormat, ImgBuf::Type4CC dstFormat)
    : VideoConverter(srcFormat, dstFormat)
{
     if(fourcc_to_ffmpeg(_dst_fmt) == PIX_FMT_NONE) {
         throw MediaException(_("VideoConverterFfmpeg cannot convert to the "
                              "requested format"));
     }
}

VideoConverterFfmpeg::~VideoConverterFfmpeg()
{
  
}


std::auto_ptr<ImgBuf>
VideoConverterFfmpeg::convert(const ImgBuf& src)
{
    std::auto_ptr<ImgBuf> ret;    
    
    const int width = src.width;
    const int height = src.height;

    PixelFormat dst_pixFmt = fourcc_to_ffmpeg(_dst_fmt);
    assert(dst_pixFmt != PIX_FMT_NONE);
    PixelFormat src_pixFmt = PIX_FMT_RGB24;
    
#ifdef HAVE_SWSCALE_H

    if (!_swsContext.get()) {

        _swsContext.reset(new SwsContextWrapper(sws_getContext(width, height,
            src_pixFmt, width, height, dst_pixFmt, SWS_BILINEAR, NULL, NULL,
            NULL)));

        if (!_swsContext->getContext()) {

            // This means we will try to assign the 
            // context again next time.
            _swsContext.reset();
            
            return ret;
        }
    }
#endif


    AVPicture srcpicture = {{src.data, 0, 0, 0}, {src.stride[0], 0, 0, 0}};
    
    
    int bufsize = avpicture_get_size(dst_pixFmt, width, height);
    if (bufsize == -1) {
        return ret;
    }

    boost::uint8_t* dstbuffer = new boost::uint8_t[bufsize];

    AVPicture dstpicture;
    avpicture_fill(&dstpicture, dstbuffer, dst_pixFmt, width, height);
    
 
#ifndef HAVE_SWSCALE_H
    img_convert(&dstpicture, dst_pixFmt, &srcpicture, src_pixFmt, width,
                height);
#else

    int rv = sws_scale(_swsContext->getContext(), srcpicture.data,
                       srcpicture.linesize, 0, height, dstpicture.data,
                       dstpicture.linesize);

    if (rv == -1) {
        return ret;
    }
#endif    
    ret.reset(new ImgBuf(_dst_fmt, dstbuffer, bufsize, src.width,
                         src.height));
    std::copy(dstpicture.linesize, dstpicture.linesize+4, ret->stride.begin()); 
 
    return ret;
}



} // gnash.media.ffmpeg namespace 
} // gnash.media namespace 
} // gnash namespace
