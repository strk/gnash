// VideoDecoderFfmpeg.cpp: Video decoding using the FFMPEG library.
// 
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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


#include "VideoDecoderFfmpeg.h"
#include "MediaParserFfmpeg.h" // for ExtraVideoInfoFfmpeg 

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
#include <algorithm>

namespace gnash {
namespace media {

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
    
    SwsContext* getContext() { return _context; }

private:
    SwsContext* _context;

};
#endif

VideoDecoderFfmpeg::VideoDecoderFfmpeg(videoCodecType format, int width, int height)
  :
  _videoCodec(NULL),
  _videoCodecCtx(NULL)
{
  enum CodecID codec_id = flashToFfmpegCodec(format);

  init(codec_id, width, height);
}

VideoDecoderFfmpeg::VideoDecoderFfmpeg(VideoInfo& info)
  :
  _videoCodec(NULL),
  _videoCodecCtx(NULL)
{
  enum CodecID codec_id = CODEC_ID_NONE;
  if ( info.type == FLASH )
  {
    codec_id = flashToFfmpegCodec(static_cast<videoCodecType>(info.codec));
  }
  else codec_id = static_cast<enum CodecID>(info.codec);

  // This would cause nasty segfaults.
  assert(codec_id != CODEC_ID_NONE);

  boost::uint8_t* extradata=0;
  int extradataSize=0;
  if ( info.extra.get() )
  {
    assert(dynamic_cast<ExtraVideoInfoFfmpeg*>(info.extra.get()));
    const ExtraVideoInfoFfmpeg& ei = static_cast<ExtraVideoInfoFfmpeg&>(*info.extra);
    extradata = ei.data;
    extradataSize = ei.dataSize;
  }
  init(codec_id, info.width, info.height, extradata, extradataSize);
}

void
VideoDecoderFfmpeg::init(enum CodecID codecId, int width, int height, boost::uint8_t* extradata, int extradataSize)
{
  // Init the avdecoder-decoder
  avcodec_init();
  avcodec_register_all();// change this to only register need codec?

  _videoCodec = avcodec_find_decoder(codecId); 

  if (!_videoCodec) {
    log_error(_("libavcodec can't decode the current video format"));
    return;
  }

  _videoCodecCtx = avcodec_alloc_context();
  if (!_videoCodecCtx) {
    log_error(_("libavcodec couldn't allocate context"));
    return;
  }

  _videoCodecCtx->extradata = extradata;
  _videoCodecCtx->extradata_size = extradataSize;

  int ret = avcodec_open(_videoCodecCtx, _videoCodec);
  if (ret < 0) {
    log_error(_("VideoDecoderFfmpeg::init: avcodec_open: failed to initialize FFMPEG codec %s (%d)"),
		_videoCodec->name, (int)codecId);
    av_free(_videoCodecCtx);
    _videoCodecCtx=0;
    return;
  }
  _videoCodecCtx->width = width;
  _videoCodecCtx->height = height;

  log_debug(_("VideoDecoderFfmpeg::init: initialized FFMPEG codec %s (%d)"), 
		_videoCodec->name, (int)codecId);

  assert(_videoCodecCtx->width > 0);
  assert(_videoCodecCtx->height > 0);
}

VideoDecoderFfmpeg::~VideoDecoderFfmpeg()
{
  if (_videoCodecCtx)
  {
    avcodec_close(_videoCodecCtx);
    av_free(_videoCodecCtx);
  }
}

std::auto_ptr<image::ImageBase>
VideoDecoderFfmpeg::frameToImage(AVCodecContext* srcCtx,
                                 const AVFrame& srcFrame)
{

  const int width = srcCtx->width;
  const int height = srcCtx->height;

  PixelFormat pixFmt;
  std::auto_ptr<image::ImageBase> im;

#ifdef FFMPEG_VP6A
  if (srcCtx->codec->id == CODEC_ID_VP6A)
#else
  if (0)
#endif // def FFMPEG_VP6A
  {
    // Expect RGBA data
    //log_debug("alpha image");
    pixFmt = PIX_FMT_RGBA;
    im.reset(new image::ImageRGBA(width, height));    
  }
  else
  {
    // Expect RGB data
    pixFmt = PIX_FMT_RGB24;
    im.reset(new image::ImageRGB(width, height));
  }

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

  boost::uint8_t* buffer = new boost::uint8_t[bufsize];

  AVPicture picture;
  picture.data[0] = NULL;

  avpicture_fill(&picture, buffer, pixFmt, width, height);

#ifndef HAVE_SWSCALE_H
  img_convert(&picture, PIX_FMT_RGB24, (AVPicture*) &srcFrame,
      srcCtx->pix_fmt, width, height);
#else

  // Is it possible for the context to be reset
  // to NULL once it's been created?
  assert(_swsContext->getContext());

  int rv = sws_scale(_swsContext->getContext(), const_cast<uint8_t**>(srcFrame.data),
    const_cast<int*>(srcFrame.linesize), 0, height, picture.data,
    picture.linesize);

  if (rv == -1) {
    delete [] buffer;
    im.reset();
    return im;
  }
#endif

  im->update(picture.data[0]);
  return im;

}

std::auto_ptr<image::ImageBase>
VideoDecoderFfmpeg::decode(const boost::uint8_t* input, boost::uint32_t input_size)
{
  std::auto_ptr<image::ImageBase> ret;

  AVFrame* frame = avcodec_alloc_frame();
  if ( ! frame ) {
    log_error(_("Out of memory while allocating avcodec frame"));
    return ret;
  }

  int bytes = 0;  
  // no idea why avcodec_decode_video wants a non-const input...
  avcodec_decode_video(_videoCodecCtx, frame, &bytes, const_cast<boost::uint8_t*>(input), input_size);
  
  if (!bytes) {
    log_error("Decoding of a video frame failed");
    av_free(frame);
    return ret;
  }

  ret = frameToImage(_videoCodecCtx, *frame);

  // FIXME: av_free doesn't free frame->data!
  av_free(frame);
  return ret;
}


void
VideoDecoderFfmpeg::push(const EncodedVideoFrame& buffer)
{
  _video_frames.push_back(&buffer);

}

std::auto_ptr<image::ImageBase>
VideoDecoderFfmpeg::pop()
{
  std::auto_ptr<image::ImageBase> ret;

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
        case VIDEO_CODEC_H263:
             return CODEC_ID_FLV1; // why not CODEC_ID_H263I ?
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


} // gnash.media namespace 
} // gnash namespace
