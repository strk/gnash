// VideoDecoderGst.h: Video decoding using Gstreamer.
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


#ifndef GNASH_VIDEODECODERGST_H
#define GNASH_VIDEODECODERGST_H


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "image.h"
#include "log.h"
#include "VideoDecoder.h"
#include "dsodefs.h"
#include "MediaParser.h" // for videoCodecType enum

#include <gst/gst.h>


#include "swfdec_codec_gst.h"


namespace gnash {
namespace media {

// Convenience wrapper for GstBuffer. Intended to be wrapped in an auto_ptr.
class gnashGstBuffer : public image::ImageRGB
{
public:
  gnashGstBuffer(GstBuffer* buf, int width, int height)
  : ImageRGB(NULL, width, height, (width * 3 + 3) & ~3),
    _buffer(buf)
  {}
  
  ~gnashGstBuffer()
  {
    gst_buffer_unref(_buffer);
  }
  
  boost::uint8_t* data()
  {
    return GST_BUFFER_DATA(_buffer);
  }

  const boost::uint8_t* data() const
  {
    return GST_BUFFER_DATA(_buffer);
  }
  std::auto_ptr<image::ImageBase> clone() const
  {
    return std::auto_ptr<ImageBase>(new ImageRGB(*this));
  }

private:
  GstBuffer* _buffer;
};


class DSOEXPORT VideoDecoderGst : public VideoDecoder
{
public:
    VideoDecoderGst(videoCodecType codec_type, int width, int height);
    VideoDecoderGst(GstCaps* caps);
    ~VideoDecoderGst();

    void push(const EncodedVideoFrame& buffer);

    std::auto_ptr<image::ImageBase> pop();
  
    bool peek();

private:
    void setup(GstCaps* caps);

    VideoDecoderGst();
    VideoDecoderGst(const gnash::media::VideoDecoderGst&);

    SwfdecGstDecoder _decoder;
};


} // namespace media
} // namespace gnash
#endif // __VIDEODECODERGST_H__
