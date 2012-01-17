// VideoDecoderGst.h: Video decoding using Gstreamer.
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

#ifndef GNASH_VIDEODECODERGST_H
#define GNASH_VIDEODECODERGST_H

#include "GnashImage.h"
#include "log.h"
#include "VideoDecoder.h"
#include "dsodefs.h"
#include "MediaParser.h" // for videoCodecType enum

#include <gst/gst.h>


#include "swfdec_codec_gst.h"


namespace gnash {
namespace media {
namespace gst {

// Convenience wrapper for GstBuffer. Intended to be wrapped in an auto_ptr.
class gnashGstBuffer : public image::ImageRGB
{
public:
  gnashGstBuffer(GstBuffer* buf, int width, int height)
  : image::ImageRGB(NULL, width, height),
    _buffer(buf)
  {}
  
  ~gnashGstBuffer()
  {
    gst_buffer_unref(_buffer);
  }
  
  virtual size_t stride() const {
      return (width() * channels() + 3) &~ 3;
  }

  virtual iterator begin()
  {
    return GST_BUFFER_DATA(_buffer);
  }

  virtual const_iterator begin() const
  {
    return GST_BUFFER_DATA(_buffer);
  }

private:
  GstBuffer* _buffer;
};


/// GST based VideoDecoder
class DSOEXPORT VideoDecoderGst : public VideoDecoder
{
public:
    VideoDecoderGst(videoCodecType codec_type, int width, int height,
                    const boost::uint8_t* extradata, size_t extradatasize);
    VideoDecoderGst(GstCaps* caps);
    ~VideoDecoderGst();

    void push(const EncodedVideoFrame& buffer);

    std::auto_ptr<image::GnashImage> pop();
  
    bool peek();

    /// Get the width of the video
    //
    /// @return The width of the video in pixels or 0 if unknown.
    int width() const;

    /// Get the height of the video
    //
    /// @return The height of the video in pixels or 0 if unknown.
    int height() const;

private:

    int _width;
    int _height;

    void setup(GstCaps* caps);

    VideoDecoderGst();
    VideoDecoderGst(const VideoDecoderGst&);

    SwfdecGstDecoder _decoder;
};


} // gnash.media.gst namespace
} // namespace media
} // namespace gnash
#endif // __VIDEODECODERGST_H__
