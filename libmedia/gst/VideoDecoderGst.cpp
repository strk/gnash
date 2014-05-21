// VideoDecoderGst.cpp: Video decoding using Gstreamer.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#include "VideoDecoderGst.h"
#include "MediaParserGst.h"
#include "GstUtil.h"

namespace gnash {
namespace media {
namespace gst {

// TODO: implement proper seeking.

VideoDecoderGst::VideoDecoderGst(GstCaps* caps)
    :
    _width(0),
    _height(0)
{
    // init GStreamer. TODO: what about doing this in MediaHandlerGst ctor?
    gst_init (nullptr, nullptr);

    setup(caps);
}

int
VideoDecoderGst::width() const
{
    return _width;
}

int
VideoDecoderGst::height() const
{
    return _height;
}

// TODO: either use width and height or remove them!
VideoDecoderGst::VideoDecoderGst(videoCodecType codec_type,
        int /*width*/, int /*height*/,
        const std::uint8_t* extradata, size_t extradatasize)
    :
    _width(0),
    _height(0)
{
    // init GStreamer. TODO: what about doing this in MediaHandlerGst ctor?
    gst_init (nullptr, nullptr);

  GstCaps* caps;  
  switch (codec_type) {
    case VIDEO_CODEC_H264:
    {
      caps = gst_caps_new_simple ("video/x-h264",
                                      nullptr);

      if (extradata && extradatasize) {

          GstBuffer* buf = gst_buffer_new_and_alloc(extradatasize);
          memcpy(GST_BUFFER_DATA(buf), extradata, extradatasize);
          gst_caps_set_simple (caps, "codec_data", GST_TYPE_BUFFER, buf, NULL);
      }
      break;
    } 
    case VIDEO_CODEC_H263:
      caps = gst_caps_new_simple ("video/x-flash-video",
                                      nullptr);
      break;
    case VIDEO_CODEC_VP6:
      caps = gst_caps_new_simple ("video/x-vp6-flash",
                                      nullptr);
      break;
    case VIDEO_CODEC_VP6A:
      caps = gst_caps_new_simple ("video/x-vp6-alpha",
                                      nullptr);
      break;      
    case VIDEO_CODEC_SCREENVIDEO:
    case VIDEO_CODEC_SCREENVIDEO2:
      caps = gst_caps_new_simple ("video/x-flash-screen",
                                      nullptr);
      break;
    case NO_VIDEO_CODEC:
      throw MediaException(_("Video codec is zero.  Streaming video expected later."));
      break;
    default:
      boost::format msg = boost::format(_("No support for video codec %s.")) %
          codec_type;
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
        throw MediaException(_("VideoDecoderGst: internal error "
                    "(caps creation failed)"));      
    }

    bool success = GstUtil::check_missing_plugins(srccaps);
    if (!success) {
        GstStructure* sct = gst_caps_get_structure(srccaps, 0);
        std::string type(gst_structure_get_name(sct));
        std::string msg = (boost::format(_("Couldn't find a plugin for "
                    "video type %s!")) % type).str();

        if (type == "video/x-flash-video" || type == "video/x-h264") {
            msg += _(" Please make sure you have gstreamer-ffmpeg installed.");
        }

        gst_caps_unref(srccaps);

        throw MediaException(msg);
    }

    GstCaps* sinkcaps = gst_caps_new_simple("video/x-raw-rgb", "bpp",
            G_TYPE_INT, 24,
            "depth", G_TYPE_INT, 24,
            NULL);

    if (!sinkcaps) {
        throw MediaException(_("VideoDecoderGst: internal error "
                    "(caps creation failed)"));      
    }

    bool rv = swfdec_gst_decoder_init (&_decoder, srccaps, sinkcaps,
            "ffmpegcolorspace", NULL);
    if (!rv) {
        GstStructure* sct = gst_caps_get_structure(srccaps, 0);
        std::string type(gst_structure_get_name(sct));
        std::string msg = (boost::format(
            _("VideoDecoderGst: initialisation failed for video type %s!"))
            % type).str();
        throw MediaException(msg);
    }

    gst_caps_unref (srccaps);
    gst_caps_unref (sinkcaps);
}

void
VideoDecoderGst::push(const EncodedVideoFrame& frame)
{
    GstBuffer* buffer;
    
    EncodedExtraGstData* extradata = 
        dynamic_cast<EncodedExtraGstData*>(frame.extradata.get());
    
    if (extradata) {
        buffer = extradata->buffer;
    } else {
        buffer = gst_buffer_new();

        GST_BUFFER_DATA(buffer) = const_cast<std::uint8_t*>(frame.data());
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
  

std::unique_ptr<image::GnashImage>
VideoDecoderGst::pop()
{
    GstBuffer * buffer = swfdec_gst_decoder_pull (&_decoder);

    if (!buffer) {
        return std::unique_ptr<image::GnashImage>();
    }
  
    GstCaps* caps = gst_buffer_get_caps(buffer);

    assert(gst_caps_get_size(caps) == 1);
  
    GstStructure* structure = gst_caps_get_structure (caps, 0);

    gst_structure_get_int (structure, "width", &_width);
    gst_structure_get_int (structure, "height", &_height);

    gst_caps_unref(caps);
  
    std::unique_ptr<image::GnashImage> ret(new gnashGstBuffer(buffer, _width, _height));
  
    return ret;
}
  

bool
VideoDecoderGst::peek()
{
  return !g_queue_is_empty (_decoder.queue);
}


} // namespace gnash::media::gst
} // namespace gnash::media
} // namespace gnash
