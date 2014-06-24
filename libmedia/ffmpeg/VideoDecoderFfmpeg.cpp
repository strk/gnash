// VideoDecoderFfmpeg.cpp: Video decoding using the FFMPEG library.
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
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "VideoDecoderFfmpeg.h"

#include <boost/format.hpp>
#include <algorithm>

#include "ffmpegHeaders.h"
#include "MediaParserFfmpeg.h" // for ExtraVideoInfoFfmpeg 
#include "GnashException.h" // for MediaException
#include "utility.h"
#include "FLVParser.h"

#ifdef HAVE_VA_VA_H
#  include "vaapi_utils.h"
#  include "VideoDecoderFfmpegVaapi.h"
#  include "GnashVaapiImage.h"
#endif

namespace gnash {
namespace media {
namespace ffmpeg {
 
class VaapiContextFfmpeg;

// Forward declarations of VAAPI functions.
namespace {
    VaapiContextFfmpeg* get_vaapi_context(AVCodecContext* avctx);
    void set_vaapi_context(AVCodecContext* avctx, VaapiContextFfmpeg* vactx);
    void clear_vaapi_context(AVCodecContext* avctx);
    void reset_context(AVCodecContext* avctx, VaapiContextFfmpeg* vactx = nullptr);
    PixelFormat get_format(AVCodecContext* avctx, const PixelFormat* fmt);
#if LIBAVCODEC_VERSION_MAJOR >= 55
    int get_buffer(AVCodecContext* avctx, AVFrame* pic, int flags);
#else
    int get_buffer(AVCodecContext* avctx, AVFrame* pic);
    int reget_buffer(AVCodecContext* avctx, AVFrame* pic);
    void release_buffer(AVCodecContext* avctx, AVFrame* pic);
#endif
}

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


// A Wrapper ensuring an AVCodecContext is closed and freed
// on destruction.
class CodecContextWrapper
{
public:
    CodecContextWrapper(AVCodecContext* context)
        :
        _codecCtx(context)
    {}

    ~CodecContextWrapper()
    {
        if (_codecCtx)
        {
            avcodec_close(_codecCtx);
            clear_vaapi_context(_codecCtx);
            av_free(_codecCtx);
        }
    }

    AVCodecContext* getContext() const { return _codecCtx; }

private:
    AVCodecContext* _codecCtx;
};


VideoDecoderFfmpeg::VideoDecoderFfmpeg(videoCodecType format, int width, int height)
    :
    _videoCodec(nullptr)
{

    CODECID codec_id = flashToFfmpegCodec(format);
    init(codec_id, width, height);

}

VideoDecoderFfmpeg::VideoDecoderFfmpeg(const VideoInfo& info)
    :
    _videoCodec(nullptr)
{

    CODECID codec_id = AV_CODEC_ID_NONE;

    if ( info.type == CODEC_TYPE_FLASH )
    {
        codec_id = flashToFfmpegCodec(static_cast<videoCodecType>(info.codec));
    }
    else codec_id = static_cast<CODECID>(info.codec);

    // This would cause nasty segfaults.
    if (codec_id == AV_CODEC_ID_NONE)
    {
        boost::format msg = boost::format(_("Cannot find suitable "
                "decoder for flash codec %d")) % info.codec;
        throw MediaException(msg.str());
    }

    std::uint8_t* extradata=nullptr;
    int extradataSize=0;
    if (info.extra.get())
    {
        if (dynamic_cast<ExtraVideoInfoFfmpeg*>(info.extra.get())) {
            const ExtraVideoInfoFfmpeg& ei = 
                static_cast<ExtraVideoInfoFfmpeg&>(*info.extra);
            extradata = ei.data;
            extradataSize = ei.dataSize;
        }
        else if (dynamic_cast<ExtraVideoInfoFlv*>(info.extra.get())) {
            const ExtraVideoInfoFlv& ei = 
                static_cast<ExtraVideoInfoFlv&>(*info.extra);
            extradata = ei.data.get();
            extradataSize = ei.size;
        }
        else {
            std::abort();
        }
    }
    init(codec_id, info.width, info.height, extradata, extradataSize);
}

void
VideoDecoderFfmpeg::init(enum CODECID codecId, int /*width*/, int /*height*/,
        std::uint8_t* extradata, int extradataSize)
{
    // Init the avdecoder-decoder
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52,6,2)
    // Starting from this version avcodec_register calls avcodec_init
    avcodec_init();
#endif
    avcodec_register_all();// change this to only register need codec?

    _videoCodec = avcodec_find_decoder(codecId); 

    if (!_videoCodec) {
        throw MediaException(_("libavcodec can't decode this video format"));
    }

    _videoCodecCtx.reset(new CodecContextWrapper(avcodec_alloc_context3(_videoCodec)));
    if (!_videoCodecCtx->getContext()) {
        throw MediaException(_("libavcodec couldn't allocate context"));
    }

    AVCodecContext* const ctx = _videoCodecCtx->getContext();

    ctx->extradata = extradata;
    ctx->extradata_size = extradataSize;

    ctx->get_format     = get_format;
#if LIBAVCODEC_VERSION_MAJOR >= 55
    ctx->get_buffer2    = get_buffer;
#else
    ctx->get_buffer     = get_buffer;
    ctx->reget_buffer   = reget_buffer;
    ctx->release_buffer = release_buffer;
#endif

#ifdef HAVE_VA_VA_H
    if (vaapi_is_enabled()) {
        VaapiContextFfmpeg *vactx = VaapiContextFfmpeg::create(codecId);
        if (vactx)
            reset_context(ctx, vactx);
    }
#endif

    int ret = avcodec_open2(ctx, _videoCodec, nullptr);
    if (ret < 0) {
        boost::format msg = boost::format(_("libavcodec "
                            "failed to initialize FFMPEG "
                            "codec %s (%d)")) % 
                            _videoCodec->name % (int)codecId;

        throw MediaException(msg.str());
    }
    
    log_debug(_("VideoDecoder: initialized FFMPEG codec %s (%d)"), 
                _videoCodec->name, (int)codecId);

}

VideoDecoderFfmpeg::~VideoDecoderFfmpeg()
{
}

int
VideoDecoderFfmpeg::width() const
{
    if (!_videoCodecCtx.get()) return 0;
    return _videoCodecCtx->getContext()->width;
}

int
VideoDecoderFfmpeg::height() const
{
    if (!_videoCodecCtx.get()) return 0;
    return _videoCodecCtx->getContext()->height;
}

std::unique_ptr<image::GnashImage>
VideoDecoderFfmpeg::frameToImage(AVCodecContext* srcCtx,
                                 const AVFrame& srcFrameRef)
{
    const AVFrame *srcFrame = &srcFrameRef;
    PixelFormat srcPixFmt = srcCtx->pix_fmt;

    const int width = srcCtx->width;
    const int height = srcCtx->height;

#ifdef FFMPEG_VP6A
    PixelFormat pixFmt = (srcCtx->codec->id == AV_CODEC_ID_VP6A) ?
        PIX_FMT_RGBA : PIX_FMT_RGB24;
#else 
    PixelFormat pixFmt = PIX_FMT_RGB24;
#endif 

    std::unique_ptr<image::GnashImage> im;

#ifdef HAVE_VA_VA_H
    VaapiContextFfmpeg * const vactx = get_vaapi_context(srcCtx);
    if (vactx) {
        VaapiSurfaceFfmpeg * const vaSurface = vaapi_get_surface(&srcFrameRef);
        if (!vaSurface) {
            im.reset();
            return im;
        }
        im.reset(new GnashVaapiImage(vaSurface->get(), image::TYPE_RGBA));
        return im;
    }
#endif

#ifdef HAVE_SWSCALE_H
    // Check whether the context wrapper exists
    // already.
    if (!_swsContext.get()) {

        _swsContext.reset(new SwsContextWrapper(
            sws_getContext(width, height, srcPixFmt, width, height,
                pixFmt, SWS_BILINEAR, nullptr, nullptr, nullptr)
        ));
        
        // Check that the context was assigned.
        if (!_swsContext->getContext()) {

            // This means we will try to assign the 
            // context again next time.
            _swsContext.reset();
            
            // Can't do anything now, though.
            return im;
        }
    }
#endif

    int bufsize = avpicture_get_size(pixFmt, width, height);
    if (bufsize == -1) return im;

    switch (pixFmt)
    {
        case PIX_FMT_RGBA:
            im.reset(new image::ImageRGBA(width, height));
            break;
        case PIX_FMT_RGB24:
            im.reset(new image::ImageRGB(width, height));
            break;
        default:
            log_error(_("Pixel format not handled"));
            return im;
    }

    AVPicture picture;

    // Let ffmpeg write directly to the GnashImage data. It is an uninitialized
    // buffer here, so do not return the image if there is any error in
    // conversion.
    avpicture_fill(&picture, im->begin(), pixFmt, width, height);

#ifndef HAVE_SWSCALE_H
    img_convert(&picture, PIX_FMT_RGB24, (AVPicture*)srcFrame,
            srcPixFmt, width, height);
#else

    // Is it possible for the context to be reset
    // to NULL once it's been created?
    assert(_swsContext->getContext());

    int rv = sws_scale(_swsContext->getContext(), 
            const_cast<uint8_t**>(srcFrame->data),
            const_cast<int*>(srcFrame->linesize), 0, height, picture.data,
            picture.linesize);

    if (rv == -1) {
        im.reset();
        return im;
    }

#endif

    return im;

}

std::unique_ptr<image::GnashImage>
VideoDecoderFfmpeg::decode(const std::uint8_t* input,
        std::uint32_t input_size)
{
    // This object shouldn't exist if there's no codec, as it can'
    // do anything anyway.
    assert(_videoCodecCtx.get());

    std::unique_ptr<image::GnashImage> ret;

    std::unique_ptr<AVFrame, decltype(av_free)*> frame(FRAMEALLOC(), av_free);
    if ( ! frame ) {
        log_error(_("Out of memory while allocating avcodec frame"));
        return ret;
    }

    int got_frame = 0;
    // no idea why avcodec_decode_video wants a non-const input...
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = const_cast<uint8_t*>(input);
    pkt.size = input_size;
    int bytesConsumed = avcodec_decode_video2(_videoCodecCtx->getContext(),
                                              frame.get(), &got_frame, &pkt);
    
    if (bytesConsumed < 0) {
        log_error(_("Decoding of a video frame failed: %1%", bytesConsumed));
        return ret;
    }
    if (bytesConsumed < input_size) {
        log_error("only %1% of %2% bytes consumed", bytesConsumed, input_size);
    }
    if (!got_frame) {
        log_debug("Decoding succeeded, but no frame is available yet.");
        return ret;
    }

    ret = frameToImage(_videoCodecCtx->getContext(), *frame);

    return ret;
}


void
VideoDecoderFfmpeg::push(const EncodedVideoFrame& buffer)
{
    _video_frames.push_back(&buffer);
}

std::unique_ptr<image::GnashImage>
VideoDecoderFfmpeg::pop()
{
    std::unique_ptr<image::GnashImage> ret;

    for (const EncodedVideoFrame* frame : _video_frames) {
         ret = decode(frame->data(), frame->dataSize());
    }

    _video_frames.clear();

    return ret;
}
    
bool
VideoDecoderFfmpeg::peek()
{
    return (!_video_frames.empty());
}

/* public static */
enum CODECID
VideoDecoderFfmpeg::flashToFfmpegCodec(videoCodecType format)
{
        // Find the decoder and init the parser
        switch(format) {
                case VIDEO_CODEC_H264:
                         return AV_CODEC_ID_H264;
                case VIDEO_CODEC_H263:
			 // AV_CODEC_ID_H263I didn't work with Lavc51.50.0
			 // and NetStream-SquareTest.swf
                         return AV_CODEC_ID_FLV1;
                case VIDEO_CODEC_VP6:
                        return AV_CODEC_ID_VP6F;
#ifdef FFMPEG_VP6A
                case VIDEO_CODEC_VP6A:
	                return AV_CODEC_ID_VP6A;
#endif
                case VIDEO_CODEC_SCREENVIDEO:
                        return AV_CODEC_ID_FLASHSV;
                default:
                        log_error(_("Unsupported video codec %d"),
                                static_cast<int>(format));
                        return AV_CODEC_ID_NONE;
        }
}

namespace {

inline VaapiContextFfmpeg*
get_vaapi_context(AVCodecContext* avctx)
{
#ifdef HAVE_VA_VA_H
    return static_cast<VaapiContextFfmpeg *>(avctx->hwaccel_context);
#else
    UNUSED(avctx);
	return nullptr;
#endif
}

inline void
set_vaapi_context(AVCodecContext* avctx, VaapiContextFfmpeg* vactx)
{
#ifdef HAVE_VA_VA_H
    avctx->hwaccel_context = vactx;
#else
    UNUSED(avctx), UNUSED(vactx);
#endif
    
}

inline void
clear_vaapi_context(AVCodecContext* avctx)
{
#ifdef HAVE_VA_VA_H
    VaapiContextFfmpeg* const vactx = get_vaapi_context(avctx);
    if (!vactx) return;

    delete vactx;
    set_vaapi_context(avctx, NULL);
#else
    UNUSED(avctx);
#endif
}

/// (Re)set AVCodecContext to sane values 
void
reset_context(AVCodecContext* avctx, VaapiContextFfmpeg* vactx)
{
    clear_vaapi_context(avctx);
    set_vaapi_context(avctx, vactx);

    avctx->thread_count = 1;
    avctx->draw_horiz_band = nullptr;
    if (vactx) {
        avctx->slice_flags = SLICE_FLAG_CODED_ORDER|SLICE_FLAG_ALLOW_FIELD;
    }
    else avctx->slice_flags = 0;
}

/// AVCodecContext.get_format() implementation
PixelFormat
get_format(AVCodecContext* avctx, const PixelFormat* fmt)
{
#ifdef HAVE_VA_VA_H
    VaapiContextFfmpeg* const vactx = get_vaapi_context(avctx);

    if (vactx) {
        for (int i = 0; fmt[i] != PIX_FMT_NONE; i++) {
            if (fmt[i] != PIX_FMT_VAAPI_VLD) continue;

            if (vactx->initDecoder(avctx->width, avctx->height)) {
                return fmt[i];
            }
        }
    }
#endif

    reset_context(avctx);
    return avcodec_default_get_format(avctx, fmt);
}

/// AVCodecContext.get_buffer() implementation
int
#if LIBAVCODEC_VERSION_MAJOR >= 55
get_buffer(AVCodecContext* avctx, AVFrame* pic, int flags)
#else
get_buffer(AVCodecContext* avctx, AVFrame* pic)
#endif
{
    VaapiContextFfmpeg* const vactx = get_vaapi_context(avctx);
#if LIBAVCODEC_VERSION_MAJOR >= 55
    if (!vactx) return avcodec_default_get_buffer2(avctx, pic, flags);
#else
    if (!vactx) return avcodec_default_get_buffer(avctx, pic);
#endif

#ifdef HAVE_VA_VA_H
    if (!vactx->initDecoder(avctx->width, avctx->height)) return -1;

    VaapiSurfaceFfmpeg * const surface = vactx->getSurface();
    if (!surface) return -1;

    vaapi_set_surface(pic, surface);

    static unsigned int pic_num = 0;
    pic->type = FF_BUFFER_TYPE_USER;
#if LIBAVCODEC_VERSION_MAJOR < 54
    // This field has been unused for longer but has been removed with
    // libavcodec 54.
    pic->age  = ++pic_num - surface->getPicNum();
#endif
    surface->setPicNum(pic_num);
    return 0;
#endif
    return -1;
}

#if LIBAVCODEC_VERSION_MAJOR < 55
/// AVCodecContext.reget_buffer() implementation
int
reget_buffer(AVCodecContext* avctx, AVFrame* pic)
{
    VaapiContextFfmpeg* const vactx = get_vaapi_context(avctx);

    if (!vactx) return avcodec_default_reget_buffer(avctx, pic);

    return get_buffer(avctx, pic);
}

/// AVCodecContext.release_buffer() implementation
void
release_buffer(AVCodecContext *avctx, AVFrame *pic)
{
    VaapiContextFfmpeg* const vactx = get_vaapi_context(avctx);
    if (!vactx) {
        avcodec_default_release_buffer(avctx, pic);
        return;
    }

#ifdef HAVE_VA_VA_H
    VaapiSurfaceFfmpeg* const surface = vaapi_get_surface(pic);
    delete surface;

    pic->data[0] = NULL;
    pic->data[1] = NULL;
    pic->data[2] = NULL;
    pic->data[3] = NULL;
#endif
}
#endif

}

} // gnash.media.ffmpeg namespace 
} // gnash.media namespace 
} // gnash namespace
