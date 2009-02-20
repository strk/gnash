// VideoDecoderFfmpeg.cpp: Video decoding using the FFMPEG library.
// 
//     Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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
//


#include "VideoDecoderFfmpeg.h"
#include "MediaParserFfmpeg.h" // for ExtraVideoInfoFfmpeg 
#include "GnashException.h" // for MediaException

#ifdef HAVE_FFMPEG_SWSCALE_H
#define HAVE_SWSCALE_H 1
extern "C" {
#include <ffmpeg/swscale.h>
}
#endif

#ifdef HAVE_LIBSWSCALE_SWSCALE_H
#define HAVE_SWSCALE_H 1
extern "C" {
#include <libswscale/swscale.h>
}
#endif

#include <boost/scoped_array.hpp>
#include <boost/format.hpp>
#include <algorithm>

#include "FLVParser.h"

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
            av_free(_codecCtx);
        }
    }

    AVCodecContext* getContext() const { return _codecCtx; }

private:
    AVCodecContext* _codecCtx;
};


VideoDecoderFfmpeg::VideoDecoderFfmpeg(videoCodecType format, int width, int height)
    :
    _videoCodec(NULL)
{

    CodecID codec_id = flashToFfmpegCodec(format);
    init(codec_id, width, height);

}

VideoDecoderFfmpeg::VideoDecoderFfmpeg(const VideoInfo& info)
    :
    _videoCodec(NULL)
{

    CodecID codec_id = CODEC_ID_NONE;

    if ( info.type == FLASH )
    {
        codec_id = flashToFfmpegCodec(static_cast<videoCodecType>(info.codec));
    }
    else codec_id = static_cast<CodecID>(info.codec);

    // This would cause nasty segfaults.
    if (codec_id == CODEC_ID_NONE)
    {
        boost::format msg = boost::format(_("Cannot find suitable "
                "decoder for flash codec %d")) % info.codec;
        throw MediaException(msg.str());
    }

    boost::uint8_t* extradata=0;
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
VideoDecoderFfmpeg::init(enum CodecID codecId, int /*width*/, int /*height*/,
        boost::uint8_t* extradata, int extradataSize)
{
    // Init the avdecoder-decoder
    avcodec_init();
    avcodec_register_all();// change this to only register need codec?

    _videoCodec = avcodec_find_decoder(codecId); 

    if (!_videoCodec) {
        throw MediaException(_("libavcodec can't decode this video format"));
    }

    _videoCodecCtx.reset(new CodecContextWrapper(avcodec_alloc_context()));
    if (!_videoCodecCtx->getContext()) {
        throw MediaException(_("libavcodec couldn't allocate context"));
    }

    AVCodecContext* const ctx = _videoCodecCtx->getContext();

    ctx->extradata = extradata;
    ctx->extradata_size = extradataSize;

    int ret = avcodec_open(ctx, _videoCodec);
    if (ret < 0) {
        boost::format msg = boost::format(_("libavcodec"
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

std::auto_ptr<GnashImage>
VideoDecoderFfmpeg::frameToImage(AVCodecContext* srcCtx,
                                 const AVFrame& srcFrame)
{

    // Adjust to next highest 4-pixel value.
    const int width = srcCtx->width;
    const int height = srcCtx->height;

    PixelFormat pixFmt;
    std::auto_ptr<GnashImage> im;

#ifdef FFMPEG_VP6A
    if (srcCtx->codec->id == CODEC_ID_VP6A) {
        // Expect RGBA data
        //log_debug("alpha image");
        pixFmt = PIX_FMT_RGBA;
        im.reset(new ImageRGBA(width, height));        
    } else {
        // Expect RGB data
        pixFmt = PIX_FMT_RGB24;
        im.reset(new ImageRGB(width, height));
    }
#else // ndef FFMPEG_VPA6
    // Expect RGB data
    pixFmt = PIX_FMT_RGB24;
    im.reset(new ImageRGB(width, height));
#endif // def FFMPEG_VP6A

#ifdef HAVE_SWSCALE_H
    // Check whether the context wrapper exists
    // already.
    if (!_swsContext.get()) {

        _swsContext.reset(
                        new SwsContextWrapper(
                                sws_getContext(width, height, srcCtx->pix_fmt,
                                width, height, pixFmt,
                                SWS_BILINEAR, NULL, NULL, NULL)
                        ));
        
        // Check that the context was assigned.
        if (!_swsContext->getContext()) {

            // This means we will try to assign the 
            // context again next time.
            _swsContext.reset();
            
            // Can't do anything now, though.
            im.reset();
            return im;
        }
    }
#endif

    int bufsize = avpicture_get_size(pixFmt, width, height);
            if (bufsize == -1) {
                im.reset();
                return im;
            }

    boost::scoped_array<boost::uint8_t> buffer ( new boost::uint8_t[bufsize] );

    AVPicture picture;
    picture.data[0] = NULL;

    avpicture_fill(&picture, buffer.get(), pixFmt, width, height);

#ifndef HAVE_SWSCALE_H
    img_convert(&picture, PIX_FMT_RGB24, (AVPicture*) &srcFrame,
            srcCtx->pix_fmt, width, height);
#else

    // Is it possible for the context to be reset
    // to NULL once it's been created?
    assert(_swsContext->getContext());

    int rv = sws_scale(_swsContext->getContext(), 
            const_cast<uint8_t**>(srcFrame.data),
            const_cast<int*>(srcFrame.linesize), 0, height, picture.data,
            picture.linesize);

    if (rv == -1) {
        im.reset();
        return im;
    }
#endif

    im->update(picture.data[0]);
    return im;

}

std::auto_ptr<GnashImage>
VideoDecoderFfmpeg::decode(const boost::uint8_t* input,
        boost::uint32_t input_size)
{
    // This object shouldn't exist if there's no codec, as it can'
    // do anything anyway.
    assert(_videoCodecCtx.get());

    std::auto_ptr<GnashImage> ret;

    AVFrame* frame = avcodec_alloc_frame();
    if ( ! frame ) {
        log_error(_("Out of memory while allocating avcodec frame"));
        return ret;
    }

    int bytes = 0;    
    // no idea why avcodec_decode_video wants a non-const input...
    avcodec_decode_video(_videoCodecCtx->getContext(), frame, &bytes,
            const_cast<boost::uint8_t*>(input), input_size);
    
    if (!bytes) {
        log_error("Decoding of a video frame failed");
        av_free(frame);
        return ret;
    }

    ret = frameToImage(_videoCodecCtx->getContext(), *frame);

    // FIXME: av_free doesn't free frame->data!
    av_free(frame);
    return ret;
}


void
VideoDecoderFfmpeg::push(const EncodedVideoFrame& buffer)
{
    _video_frames.push_back(&buffer);
}

std::auto_ptr<GnashImage>
VideoDecoderFfmpeg::pop()
{
    std::auto_ptr<GnashImage> ret;

    for (std::vector<const EncodedVideoFrame*>::iterator it =
             _video_frames.begin(), end = _video_frames.end(); it != end; ++it) {
         ret = decode((*it)->data(), (*it)->dataSize());
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
enum CodecID
VideoDecoderFfmpeg::flashToFfmpegCodec(videoCodecType format)
{
        // Find the decoder and init the parser
        switch(format) {
                case VIDEO_CODEC_H264:
                         return CODEC_ID_H264;
                case VIDEO_CODEC_H263:
			 // CODEC_ID_H263I didn't work with Lavc51.50.0
			 // and NetStream-SquareTest.swf
                         return CODEC_ID_FLV1;
#ifdef FFMPEG_VP6
                case VIDEO_CODEC_VP6:
                        return CODEC_ID_VP6F;
#endif
#ifdef FFMPEG_VP6A
                case VIDEO_CODEC_VP6A:
	                return CODEC_ID_VP6A;
#endif
                case VIDEO_CODEC_SCREENVIDEO:
                        return CODEC_ID_FLASHSV;
                default:
                        log_error(_("Unsupported video codec %d"),
                                static_cast<int>(format));
                        return CODEC_ID_NONE;
        }
}


} // gnash.media.ffmpeg namespace 
} // gnash.media namespace 
} // gnash namespace
