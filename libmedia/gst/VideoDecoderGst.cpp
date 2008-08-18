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
#include "gstappsink.h"
#include "gstappsrc.h"


namespace gnash {
namespace media {

// gstappsrc -> decodebin -> (decoder) -> ffmpegcolorspace -> gstappsink (with rgb caps)

// TODO: implement proper seeking.

VideoDecoderGst::VideoDecoderGst(videoCodecType codec_type, int width, int height)
 : _appsink(NULL),
   _colorspace(NULL)
{
  gst_init (NULL, NULL);
  
  _pipeline = gst_pipeline_new (NULL);

  _appsrc = gst_element_factory_make ("appsrc", NULL);
  
  GstElement* decoder = NULL;
  
  GstCaps* caps;  
  switch (codec_type) {
    case VIDEO_CODEC_H263:
      decoder = gst_element_factory_make ("ffdec_flv", NULL);
      caps = gst_caps_new_simple ("video/x-flash-video",
                                      "width", G_TYPE_INT, width,
                                      "height", G_TYPE_INT, height,   
                                      NULL);
      break;
    case VIDEO_CODEC_VP6:
    case VIDEO_CODEC_VP6A:
      decoder = gst_element_factory_make ("ffdec_vp6f", NULL);
      caps = gst_caps_new_simple ("video/x-vp6-flash",
                                      "width", G_TYPE_INT, width,
                                      "height", G_TYPE_INT, height,     
                                      NULL);
      break;      
    case VIDEO_CODEC_SCREENVIDEO:
    case VIDEO_CODEC_SCREENVIDEO2:
      decoder = gst_element_factory_make ("ffdec_flashsv", NULL);
      caps = gst_caps_new_simple ("video/x-flash-screen",
                                      "width", G_TYPE_INT, width,
                                      "height", G_TYPE_INT, height,
                                      NULL);
      break;
    case 0:
      log_debug("Video codec is zero.  Streaming video expected later.");
      gst_object_unref (GST_OBJECT (_pipeline));
      _pipeline = NULL;
      break;
    default:
      log_error("No support for this video codec. %d", codec_type);
      gst_object_unref (GST_OBJECT (_pipeline));
      _pipeline = NULL;
      return;
  }
  
  if (!decoder) {
    log_error(_("failed to initialize the video decoder. Embedded video "
                "playback will not be available; consider installing "
                "gstreamer-ffmpeg."));
    gst_object_unref (GST_OBJECT (_pipeline));
    _pipeline = NULL;
    return;
  }
  
  gst_app_src_set_caps (GST_APP_SRC(_appsrc), caps);
  gst_caps_unref(caps);

  _colorspace = gst_element_factory_make ("ffmpegcolorspace", NULL);

  _appsink = gst_element_factory_make ("appsink", NULL);


  caps = gst_caps_new_simple ("video/x-raw-rgb", "bpp", G_TYPE_INT, 24,
                                                 "depth", G_TYPE_INT, 24, NULL);

  gst_app_sink_set_caps(GST_APP_SINK(_appsink), caps);

  gst_caps_unref (caps);


  gst_bin_add_many (GST_BIN (_pipeline), _appsrc, decoder, _colorspace, _appsink, NULL);

  gst_element_link_many(_appsrc, decoder, _colorspace, _appsink, NULL);
  
  gst_base_src_set_live(GST_BASE_SRC(_appsrc), TRUE);

  gst_element_set_state (GST_ELEMENT (_pipeline), GST_STATE_PLAYING);
}

VideoDecoderGst::~VideoDecoderGst()
{
  if (_pipeline) {
    gst_element_set_state (GST_ELEMENT (_pipeline), GST_STATE_NULL);
    gst_object_unref (GST_OBJECT (_pipeline));
  }
}

void
VideoDecoderGst::push(const EncodedVideoFrame& frame)
{
  if (!_pipeline) {
    return;
  }
  
  GstBuffer* buffer = gst_buffer_new();
  
  // dunno why gst needs non-const here
  GST_BUFFER_DATA(buffer) = const_cast<uint8_t*>(frame.data());
  GST_BUFFER_SIZE(buffer) = frame.dataSize();	
  GST_BUFFER_OFFSET(buffer) = frame.frameNum();
  GST_BUFFER_TIMESTAMP(buffer) = GST_CLOCK_TIME_NONE;
  GST_BUFFER_DURATION(buffer) = GST_CLOCK_TIME_NONE;
  
  gst_app_src_push_buffer (GST_APP_SRC(_appsrc), buffer);
  
  checkMessages();
}
  

std::auto_ptr<image::ImageRGB>
VideoDecoderGst::pop()
{
  if (!_pipeline) {
    return std::auto_ptr<image::ImageRGB>();
  }

  checkMessages();
  
  GstBuffer* buffer = gst_app_sink_pull_buffer_timed (GST_APP_SINK(_appsink));
  
  if (!buffer) {
    return std::auto_ptr<image::ImageRGB>();
  }
  
  GstCaps* caps = gst_buffer_get_caps(buffer);

  assert(gst_caps_get_size(caps) == 1);
  
  GstStructure* structure = gst_caps_get_structure (caps, 0);

  gint height, width;

  gst_structure_get_int (structure, "width", &width);
  gst_structure_get_int (structure, "height", &height);
  
  gst_caps_unref(caps);
  
  std::auto_ptr<image::ImageRGB> ret(new gnashGstBuffer(buffer, width, height));
  
  return ret;
}
  

bool
VideoDecoderGst::peek()
{
  if (!_pipeline) {
    return false;
  }

  return gst_app_sink_peek_buffer (GST_APP_SINK(_appsink));
}

void
VideoDecoderGst::checkMessages() // any messages for me?
{
  if (!_pipeline) {
    return;
  }

  GstBus* bus = gst_element_get_bus(_pipeline);

  while (gst_bus_have_pending(bus)) {
    GstMessage* msg = gst_bus_pop(bus);
    handleMessage(msg);

    gst_message_unref(msg);
  }

  gst_object_unref(GST_OBJECT(bus));
}

void
VideoDecoderGst::handleMessage (GstMessage *message)
{
#if 0
  g_print ("Got %s message\n", GST_MESSAGE_TYPE_NAME (message));
#endif

  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR:
    {
      GError *err;
      gchar *debug;
      gst_message_parse_error (message, &err, &debug);
      
      log_error(_("Embedded video playback halted; module %s reported: %s\n"),
                gst_element_get_name(GST_MESSAGE_SRC (message)), err->message);
      
      g_error_free (err);
      g_free (debug);
      
      // Clear any buffers.
      gst_element_set_state (_pipeline, GST_STATE_NULL);

      break;
    }
    case GST_MESSAGE_EOS:
      log_debug(_("NetStream has reached the end of the stream."));

      break;
    
    default:
    {
#if 0
      g_print("unhandled message\n");
#endif
    }
  }

}


} // namespace gnash::media
} // namespace gnash
