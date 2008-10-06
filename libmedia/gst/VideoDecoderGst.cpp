// VideoDecoderGst.cpp: Video decoding using Gstreamer.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "VideoDecoderGst.h"
#include "MediaParserGst.h"


namespace gnash {
namespace media {

// TODO: implement proper seeking.

VideoDecoderGst::VideoDecoderGst(GstCaps* caps)
{
    // init GStreamer. TODO: what about doing this in MediaHandlerGst ctor?
    gst_init (NULL, NULL);

    setup(caps);
}


VideoDecoderGst::VideoDecoderGst(videoCodecType codec_type, int width, int height)
{
    // init GStreamer. TODO: what about doing this in MediaHandlerGst ctor?
    gst_init (NULL, NULL);

  GstCaps* caps;  
  switch (codec_type) {
    case VIDEO_CODEC_H263:
      caps = gst_caps_new_simple ("video/x-flash-video",
                                      "width", G_TYPE_INT, width,
                                      "height", G_TYPE_INT, height,   
                                      NULL);
      break;
    case VIDEO_CODEC_VP6:
      caps = gst_caps_new_simple ("video/x-vp6-flash",
                                      "width", G_TYPE_INT, width,
                                      "height", G_TYPE_INT, height,     
                                      NULL);
      break;
    case VIDEO_CODEC_VP6A:
      caps = gst_caps_new_simple ("video/x-vp6-alpha",
                                      "width", G_TYPE_INT, width,
                                      "height", G_TYPE_INT, height,     
                                      NULL);
      break;      
    case VIDEO_CODEC_SCREENVIDEO:
    case VIDEO_CODEC_SCREENVIDEO2:
      caps = gst_caps_new_simple ("video/x-flash-screen",
                                      "width", G_TYPE_INT, width,
                                      "height", G_TYPE_INT, height,
                                      NULL);
      break;
    case 0:
      throw MediaException(_("Video codec is zero.  Streaming video expected later."));
      break;
    default:
      boost::format msg = boost::format(_("No support for video codec %d.")) % (int)codec_type;
      throw MediaException(msg.str());

      return;
  }
  
  setup(caps);
  
}


VideoDecoderGst::~VideoDecoderGst()
{
    swfdec_gst_decoder_push_eos(&_decoder);
    swfdec_gst_decoder_finish(&_decoder);
}

void
VideoDecoderGst::setup(GstCaps* srccaps)
{
    if (!srccaps) {
        throw MediaException(_("VideoDecoderGst: internal error (caps creation failed)"));      
    }

    GstCaps* sinkcaps = gst_caps_new_simple ("video/x-raw-rgb", "bpp", G_TYPE_INT, 24,
                                             "depth", G_TYPE_INT, 24,
                                             NULL);
    if (!sinkcaps) {
        throw MediaException(_("VideoDecoderGst: internal error (caps creation failed)"));      
    }

    bool rv = swfdec_gst_decoder_init (&_decoder, srccaps, sinkcaps, "ffmpegcolorspace", NULL);
    if (!rv) {
        throw MediaException(_("VideoDecoderGst: initialisation failed."));      
    }

    gst_caps_unref (srccaps);
    gst_caps_unref (sinkcaps);
}

void
VideoDecoderGst::push(const EncodedVideoFrame& frame)
{
    GstBuffer* buffer;
    
    EncodedExtraGstData* extradata = dynamic_cast<EncodedExtraGstData*>(frame.extradata.get());
    
    if (extradata) {
        buffer = extradata->buffer;
    } else {
        buffer = gst_buffer_new();

        GST_BUFFER_DATA(buffer) = const_cast<uint8_t*>(frame.data());
        GST_BUFFER_SIZE(buffer) = frame.dataSize();
        GST_BUFFER_OFFSET(buffer) = frame.frameNum();
        GST_BUFFER_TIMESTAMP(buffer) = GST_CLOCK_TIME_NONE;
        GST_BUFFER_DURATION(buffer) = GST_CLOCK_TIME_NONE;
    }
  
    bool success = swfdec_gst_decoder_push(&_decoder, buffer);
    if (!success) {
        log_error(_("VideoDecoderGst: buffer push failed."));
    }
}
  

std::auto_ptr<image::ImageBase>
VideoDecoderGst::pop()
{
    GstBuffer * buffer = swfdec_gst_decoder_pull (&_decoder);

    if (!buffer) {
        return std::auto_ptr<image::ImageBase>();
    }
  
    GstCaps* caps = gst_buffer_get_caps(buffer);

    assert(gst_caps_get_size(caps) == 1);
  
    GstStructure* structure = gst_caps_get_structure (caps, 0);

    gint height, width;

    gst_structure_get_int (structure, "width", &width);
    gst_structure_get_int (structure, "height", &height);
  
    gst_caps_unref(caps);
  
    std::auto_ptr<image::ImageBase> ret(new gnashGstBuffer(buffer, width, height));
  
    return ret;
}
  

bool
VideoDecoderGst::peek()
{
  return !g_queue_is_empty (_decoder.queue);
}


} // namespace gnash::media
} // namespace gnash
