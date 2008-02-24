// VideoDecoderFfmpeg.h: Video decoding using the FFMPEG library.
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

// $Id: VideoDecoderFfmpeg.h,v 1.4 2008/02/24 19:21:12 bjacques Exp $

#ifndef __VIDEODECODERFFMPEG_H__
#define __VIDEODECODERFFMPEG_H__

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "log.h"
#include "VideoDecoder.h"

extern "C" {
#include <ffmpeg/avcodec.h>
}


namespace gnash {
namespace media {


class VideoDecoderFfmpeg : public VideoDecoder {
  
public:
  VideoDecoderFfmpeg(videoCodecType format, int width, int height);
  ~VideoDecoderFfmpeg();
  
  void push(const EncodedVideoFrame& buffer);

  std::auto_ptr<image::rgb> pop();
  
  bool peek();
  
  static boost::uint8_t* convertRGB24(AVCodecContext* srcCtx, AVFrame* srcFrame);

private:

  std::auto_ptr<image::rgb> decode(boost::uint8_t* input, boost::uint32_t input_size);
private:

  AVCodec* _videoCodec;
  AVCodecContext* _videoCodecCtx;
  std::vector<const EncodedVideoFrame*> _video_frames;
};
  
} // gnash.media namespace 
} // gnash namespace

#endif // __VIDEODECODERFFMPEG_H__
