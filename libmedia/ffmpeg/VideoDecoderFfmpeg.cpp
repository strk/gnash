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

#ifdef HAVE_SWSCALE_H
extern "C" {
#include <ffmpeg/swscale.h>
}
#endif
#include <boost/scoped_array.hpp>
#include <boost/foreach.hpp>

namespace gnash {
namespace media {
  
VideoDecoderFfmpeg::VideoDecoderFfmpeg(videoCodecType format, int width,int height)
  :
  _videoCodec(NULL),
  _videoCodecCtx(NULL)
{
  // Init the avdecoder-decoder
  avcodec_init();
  avcodec_register_all();// change this to only register need codec?

  enum CodecID codec_id;

  // Find the decoder and init the parser
  switch(format) {
    case VIDEO_CODEC_H263:
      codec_id = CODEC_ID_FLV1;
      break;
#ifdef FFMPEG_VP6
    case VIDEO_CODEC_VP6:
      codec_id = CODEC_ID_VP6F;
      break;
#endif
    case VIDEO_CODEC_SCREENVIDEO:
      codec_id = CODEC_ID_FLASHSV;
      break;
    default:
      log_error(_("Unsupported video codec %d"),
            static_cast<int>(format));
      return;
  }

  _videoCodec = avcodec_find_decoder(static_cast<CodecID>(codec_id)); // WTF?

  if (!_videoCodec) {
    log_error(_("libavcodec can't decode the current video format"));
    return;
  }

  _videoCodecCtx = avcodec_alloc_context();
  if (!_videoCodecCtx) {
    log_error(_("libavcodec couldn't allocate context"));
    return;
  }

  int ret = avcodec_open(_videoCodecCtx, _videoCodec);
  if (ret < 0) {
    log_error(_("libavcodec failed to initialize codec"));
    return;
  }
  _videoCodecCtx->width = width;
  _videoCodecCtx->height = height;

  assert(_videoCodecCtx->width > 0);
  assert(_videoCodecCtx->height > 0);
  return;
}

VideoDecoderFfmpeg::~VideoDecoderFfmpeg()
{
  if (_videoCodecCtx)
  {
    avcodec_close(_videoCodecCtx);
    av_free(_videoCodecCtx);
  }
}

AVPicture /*static*/
VideoDecoderFfmpeg::convertRGB24(AVCodecContext* srcCtx,
                                 const AVFrame& srcFrame)
{
  AVPicture picture;
  int width = srcCtx->width, height = srcCtx->height;
  
  picture.data[0] = NULL;
  
  int bufsize = avpicture_get_size(PIX_FMT_RGB24, width, height);
  if (bufsize == -1) {
    return picture;
  }

  boost::uint8_t* buffer = new boost::uint8_t[bufsize];

  avpicture_fill(&picture, buffer, PIX_FMT_RGB24, width, height);

#ifndef HAVE_SWSCALE_H
  img_convert(&picture, PIX_FMT_RGB24, (AVPicture*) &srcFrame,
      srcCtx->pix_fmt, width, height);
#else
  // FIXME: this will live forever ...
  static struct SwsContext* context = NULL;

  if (!context) {
    // FIXME: this leads to wrong results (read: segfaults) if this method
    //        is called from two unrelated video contexts, for example from
    //        a NetStreamFfmpeg and an embedded video context. Or two
    //        separate instances of one of the former two.    
    context = sws_getContext(width, height, srcCtx->pix_fmt,
           width, height, PIX_FMT_RGB24,
           SWS_FAST_BILINEAR, NULL, NULL, NULL);
    
    if (!context) {
      delete [] buffer;
      return picture;
    }
  }

  int rv = sws_scale(context, const_cast<uint8_t**>(srcFrame.data),
    const_cast<int*>(srcFrame.linesize), 0, height, picture.data,
    picture.linesize);

  if (rv == -1) {
    delete [] buffer;
  }

#endif // HAVE_SWSCALE_H
  return picture;
}

std::auto_ptr<image::rgb>
VideoDecoderFfmpeg::decode(boost::uint8_t* input, boost::uint32_t input_size)
{
  std::auto_ptr<image::rgb> ret;

  AVFrame* frame = avcodec_alloc_frame();
  if ( ! frame ) {
    log_error(_("Out of memory while allocating avcodec frame"));
    return ret;
  }

  int bytes = 0;  
  avcodec_decode_video(_videoCodecCtx, frame, &bytes, input, input_size);
  
  if (!bytes) {
    log_error("Decoding of a video frame failed");
    av_free(frame);
    return ret;
  }

  AVPicture rgbpicture = convertRGB24(_videoCodecCtx, *frame);
  
  ret.reset(new image::rgb(rgbpicture.data[0], _videoCodecCtx->width,
                           _videoCodecCtx->height, rgbpicture.linesize[0]));

  // FIXME: av_free doesn't free frame->data!
  av_free(frame);
  return ret;
}


void
VideoDecoderFfmpeg::push(const EncodedVideoFrame& buffer)
{
  _video_frames.push_back(&buffer);

}

std::auto_ptr<image::rgb>
VideoDecoderFfmpeg::pop()
{
  std::auto_ptr<image::rgb> ret;

  BOOST_FOREACH(const EncodedVideoFrame* frame, _video_frames) {
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

} // gnash.media namespace 
} // gnash namespace
