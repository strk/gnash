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

#include "NetStreamGst.h"

#include "gstgnashsrc.h"
#include "Object.h"
#include "gstflvdemux.h"


//                                        video -> ffmpegcolorspace -> capsfilter -> fakesink
//                                       /
// (GstUriHandler) -> queue -> decodebin
//                                       |
//                                        audio -> audioconvert -> autoaudiosink


namespace gnash {

NetStreamGst::NetStreamGst()
 : _downloader(NULL),
   _duration(0)
{
  gst_init(NULL, NULL);

  _pipeline = gst_pipeline_new ("gnash_pipeline");

  // Figure out if flvdemux is present on the system. If not load the one from
  // the Gnash tree.
  GstElementFactory* factory = gst_element_factory_find ("flvdemux");
  if (!factory) {
    if (!gst_element_register (NULL, "flvdemux", GST_RANK_PRIMARY,
          gst_flv_demux_get_type ())) {
      log_error("Failed to register our own FLV demuxer. FLV playback may not "
                "work.");            
    }    
  } else {
    gst_object_unref(GST_OBJECT(factory));
  }

  // Setup general decoders
  _dataqueue = gst_element_factory_make ("queue", "gnash_dataqueue");
  g_signal_connect (_dataqueue, "underrun", G_CALLBACK (NetStreamGst::queue_underrun_cb), this);
  g_signal_connect (_dataqueue, "running", G_CALLBACK (NetStreamGst::queue_running_cb), this);
    
  GstElement* decoder = gst_element_factory_make ("decodebin", NULL);  
  g_signal_connect (decoder, "new-decoded-pad", G_CALLBACK (NetStreamGst::decodebin_newpad_cb), this);   

  gst_bin_add_many (GST_BIN (_pipeline), _dataqueue, decoder, NULL);
  gst_element_link(_dataqueue, decoder);

  // Setup video conversion and sink


  // setup the video colorspaceconverter converter
  GstElement* colorspace = gst_element_factory_make ("ffmpegcolorspace", "gnash_colorspace");

  GstElement* videocaps = gst_element_factory_make ("capsfilter", NULL);

  // Make sure we receive RGB
  GstCaps* videooutcaps = gst_caps_new_simple ("video/x-raw-rgb", NULL);
  g_object_set (G_OBJECT (videocaps), "caps", videooutcaps, NULL);
  gst_caps_unref (videooutcaps);

  // Videoscale isn't needed currently, but it'll just pass the video through.
  // At some point we might make this feature available to the renderer, for
  // example.
  GstElement* videoscale = gst_element_factory_make ("videoscale", NULL);

  // setup the videosink with callback
  GstElement* videosink = gst_element_factory_make ("fakesink", NULL);

  g_object_set (G_OBJECT (videosink), "signal-handoffs", TRUE, "sync", TRUE, NULL);
  g_signal_connect (videosink, "handoff", G_CALLBACK (NetStreamGst::video_data_cb), this);


  // Create the video pipeline and link the elements. The pipeline will
  // dereference the elements when they are destroyed.
  gst_bin_add_many (GST_BIN (_pipeline), colorspace, videoscale, videocaps, videosink, NULL);
  gst_element_link_many(colorspace, videoscale, videocaps, videosink, NULL);	

  // Setup audio sink
  GstElement* audioconvert = gst_element_factory_make ("audioconvert", NULL);	

  GstElement* audiosink = gst_element_factory_make ("autoaudiosink", NULL);

  gst_bin_add_many(GST_BIN(_pipeline), audioconvert, audiosink, NULL);
  gst_element_link(audioconvert, audiosink);

  _audiopad = gst_element_get_static_pad (audioconvert, "sink");
  _videopad = gst_element_get_static_pad (colorspace, "sink");
}

NetStreamGst::~NetStreamGst()
{
  gst_element_set_state (_pipeline, GST_STATE_NULL);
  
  
  gst_element_get_state(_pipeline, NULL, NULL, 0); // wait for a response

  gst_object_unref(GST_OBJECT(_pipeline));
  
  gst_object_unref(GST_OBJECT(_videopad));
  gst_object_unref(GST_OBJECT(_audiopad));
}

void
NetStreamGst::close()
{ 
  gst_element_set_state (_pipeline, GST_STATE_NULL);  

  setStatus(playStop);
  
  processStatusNotifications();

  boost::mutex::scoped_lock lock(image_mutex);

  delete m_imageframe;
  m_imageframe = NULL;
}

void 
NetStreamGst::pause(PauseMode mode)
{
  GstState newstate;
  switch(mode) {
    case pauseModeToggle:
    {
      GstState cur_state;
      
      GstStateChangeReturn statereturn = gst_element_get_state(_pipeline,
                                          &cur_state, NULL,
                                          1000000 /* wait 1 ms */);
      if (statereturn != GST_STATE_CHANGE_SUCCESS) {
        return;
      }
      
      if (cur_state == GST_STATE_PLAYING) {
        newstate = GST_STATE_PAUSED;
      } else {
        gst_element_set_base_time(_pipeline, 0);
        newstate = GST_STATE_PLAYING;
      }
      
      break;
    }
    case pauseModePause:
      newstate = GST_STATE_PAUSED;
      break;
    case pauseModeUnPause:
      
      newstate = GST_STATE_PLAYING;

      break;
  }
  
  gst_element_set_state (_pipeline, newstate);

}

void
NetStreamGst::play(const std::string& url)
{
  std::string valid_url = _netCon->validateURL(url);
#if 0
  log_msg("%s: connecting to %s\n", __FUNCTION__, valid_url);
#endif
  if (valid_url.empty()) {
    // error; TODO: nofiy user
    return;
  }
  
  if (_downloader) {
    gst_element_set_state (_pipeline, GST_STATE_NULL);
    
    gst_bin_remove(GST_BIN(_pipeline), _downloader); // will also unref
  }
 
  _downloader = gst_element_make_from_uri(GST_URI_SRC, valid_url.c_str(),
                                          "gnash_uridownloader");
                                                         
  bool success = gst_bin_add(GST_BIN(_pipeline), _downloader);
  assert(success);

  gst_element_link(_downloader, _dataqueue);  


  // if everything went well, start playback
  gst_element_set_state (_pipeline, GST_STATE_PLAYING);

}


// FIXME: this does not work for HTTP streams.
void
NetStreamGst::seek(boost::uint32_t pos)
{
  bool success = gst_element_seek_simple(_pipeline, GST_FORMAT_TIME,
                   GstSeekFlags(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT),
                   GST_MSECOND * pos);
 
  if (success) {
    setStatus(seekNotify);
  } else {
    log_msg(_("Seek failed. This is expected, but we tried it anyway."));
    setStatus(invalidTime);
  }
}

boost::int32_t
NetStreamGst::time()
{  
  GstFormat fmt = GST_FORMAT_TIME;
  
  gint64 pos = 0;
  
  bool rv = gst_element_query_position (_pipeline, &fmt, &pos);  
  
  if (!rv) {
    return 0;
  }
  
  return pos / GST_MSECOND;
}

void
NetStreamGst::advance()
{
  GstBus* bus = gst_element_get_bus(_pipeline);

  while (gst_bus_have_pending(bus)) {
    GstMessage* msg = gst_bus_pop(bus);
    handleMessage(msg);
    
    gst_message_unref(msg); 
  }
  
  gst_object_unref(GST_OBJECT(bus));
  
  processStatusNotifications();
}

double
NetStreamGst::getCurrentFPS()
{
  GstElement*  colorspace = gst_bin_get_by_name (GST_BIN(_pipeline), "gnash_colorspace");
    
  GstPad* videopad = gst_element_get_static_pad (colorspace, "src");
  
  gst_object_unref(GST_OBJECT(colorspace));
  
  GstCaps* caps = gst_pad_get_negotiated_caps (videopad);
  if (!caps) {
    return 0;
  }
  
  gst_object_unref(GST_OBJECT(videopad));
	
  // must not be freed
  GstStructure* structure = gst_caps_get_structure (caps, 0);
  
  gst_caps_unref(caps);

  gint framerate[2] = {0, 0};	

  gst_structure_get_fraction (structure, "framerate", &framerate[0],
                              &framerate[1]);
  if (framerate[1] == 0) {
    return 0;
  }

  return double(framerate[0]) / double(framerate[1]);
}

long
NetStreamGst::bytesLoaded()
{

  gint64 pos = 0;
  GstFormat format = GST_FORMAT_BYTES;
  gst_element_query_position(_downloader, &format, &pos);
  
  guint buffer_size = 0;
  g_object_get(G_OBJECT(_dataqueue), "current-level-bytes", &buffer_size, NULL);
  
  guint64 bytesloaded = pos + buffer_size;
  
  // Sanity check; did we exceed the total data size?
  guint64 total_bytes = bytesTotal();
  
  if (total_bytes && bytesloaded > total_bytes) {
    return total_bytes;
  }

  return bytesloaded;
}

long
NetStreamGst::bytesTotal()
{  
  gint64 duration = 0;
  GstFormat format = GST_FORMAT_BYTES;
  
  gst_element_query_duration (_downloader, &format, &duration);
  
  if (!duration) {
    return _duration;
  }
  
  return duration;
}


void
metadata(const GstTagList *list, const gchar *tag, gpointer user_data)
{
  const gchar* nick = gst_tag_get_nick(tag);
  as_object* o = static_cast<as_object*>(user_data);

#ifdef DEBUG_METADATA
  const gchar* descr = gst_tag_get_description(tag);
  g_print("tag name: %s,description: %s, type: %s.\n", nick, descr, g_type_name(gst_tag_get_type(tag)));
#endif


  switch(gst_tag_get_type(tag)) {
    case G_TYPE_STRING:
    {
      gchar* value;

      gst_tag_list_get_string(list, tag, &value);
      
      o->init_member(nick, value);
      
      g_free(value);

      break;
    }
    case G_TYPE_DOUBLE:
    {
      gdouble value;
      gst_tag_list_get_double(list, tag, &value);
      o->init_member(nick, value);
      
      break;
    }
    case G_TYPE_BOOLEAN:
    {
      gboolean value;
      gst_tag_list_get_boolean(list, tag, &value);
      o->init_member(nick, value);
      break;
    }
    case G_TYPE_UINT64:
    {
      guint64 value;
      gst_tag_list_get_uint64(list, tag, &value);
      o->init_member(nick, (unsigned long) value); // FIXME: actually, fix as_value().
      break;
    }
    case G_TYPE_UINT:
    {
      guint value;
      gst_tag_list_get_uint(list, tag, &value);
      o->init_member(nick, value);
      break;
    }
    default:
    {}
  } // switch

}


// TODO: apparently the onStatus message.details propery can be set with some
//       actually useful error description. Investigate and implement.
void
NetStreamGst::handleMessage (GstMessage *message)
{
#ifdef DEBUG_MESSAGES
  g_print ("Got %s message\n", GST_MESSAGE_TYPE_NAME (message));
#endif

  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR:
    {
      GError *err;
      gchar *debug;
      gst_message_parse_error (message, &err, &debug);
      
      log_error(_("NetStream playback halted because: %s\n"), err->message);
      
      g_error_free (err);
      g_free (debug);
      
      setStatus(streamNotFound);
      
      // Clear any buffers.
      gst_element_set_state (_pipeline, GST_STATE_NULL);

      break;
    }
    case GST_MESSAGE_EOS:
      log_msg(_("NetStream has reached the end of the stream."));
      setStatus(playStop);
      break;
    case GST_MESSAGE_TAG:
    {
      GstTagList* taglist;

      gst_message_parse_tag(message, &taglist);      
        
      boost::intrusive_ptr<as_object> o = new as_object(getObjectInterface());

      gst_tag_list_foreach(taglist, metadata, o.get());

      processMetaData(o);
      
      g_free(taglist);
      break;
    }    
    case GST_MESSAGE_BUFFERING:
    {
#if 0
      // This is most likely not necessary since we have a signal handler
      // that achieves the same functionality.
      gint percent_buffered;
      gst_message_parse_buffering(message, &percent_buffered);
      
      if (percent_buffered == 100) {
        setStatus(bufferFull);      
      }
#endif
      break;
    }
    case GST_MESSAGE_STATE_CHANGED:
    {
      GstState oldstate;
      GstState newstate;
      GstState pending;

      gst_message_parse_state_changed(message, &oldstate, &newstate, &pending);
    
      if (oldstate == GST_STATE_READY && (newstate == GST_STATE_PAUSED || newstate == GST_STATE_PLAYING)) {
      
        setStatus(playStart);
      }
      break;
    }
    case GST_MESSAGE_DURATION:
    {
      // Sometimes the pipeline fails to use this number in queries.
      GstFormat format = GST_FORMAT_BYTES;
      gst_message_parse_duration(message, &format, &_duration);
    }
    
    default:
    {
#ifdef DEBUG_MESSAGES
      g_print("unhandled message\n");
#endif
    }
  }

}

// NOTE: callbacks will be called from the streaming thread!

void 
NetStreamGst::video_data_cb(GstElement* /*c*/, GstBuffer *buffer,
                            GstPad* /*pad*/, gpointer user_data)
{
  NetStreamGst* ns = reinterpret_cast<NetStreamGst*>(user_data);

  GstElement*  colorspace = gst_bin_get_by_name (GST_BIN(ns->_pipeline),
                                                 "gnash_colorspace");
  
  GstPad* videopad = gst_element_get_static_pad (colorspace, "src");
  GstCaps* caps = gst_pad_get_negotiated_caps (videopad);
    
  gint height, width;

  GstStructure* str = gst_caps_get_structure (caps, 0);

  gst_structure_get_int (str, "width", &width);
  gst_structure_get_int (str, "height", &height);
  
  boost::mutex::scoped_lock lock(ns->image_mutex);
  
  if (!ns->m_imageframe || unsigned(width) != ns->m_imageframe->width() ||
      unsigned(height) != ns->m_imageframe->height()) {
    delete ns->m_imageframe;
    ns->m_imageframe = new image::rgb(width, height);
  }    
  
  ns->m_imageframe->update(GST_BUFFER_DATA(buffer));
  
  ns->m_newFrameReady = true;
  	
  gst_object_unref(GST_OBJECT (colorspace));
  gst_object_unref(GST_OBJECT(videopad));
  gst_caps_unref(caps);
}


void
NetStreamGst::decodebin_newpad_cb(GstElement* /*decodebin*/, GstPad* pad,
                                  gboolean /*last*/, gpointer user_data)
{
  NetStreamGst* ns = static_cast<NetStreamGst*>(user_data);  
  
  GstCaps* caps = gst_pad_get_caps (pad);
  GstStructure* str = gst_caps_get_structure (caps, 0);
  const gchar* structure_name = gst_structure_get_name (str);

  gst_caps_unref (caps);

  if (g_strrstr (structure_name, "audio")) {
    if (GST_PAD_IS_LINKED (ns->_audiopad)) {
      return;
    }

    gst_pad_link (pad, ns->_audiopad);

  } else if (g_strrstr (structure_name, "video")) {

    if (GST_PAD_IS_LINKED (ns->_videopad)) {
      return;
    }

    gst_pad_link (pad, ns->_videopad);
	
  } else {
    log_unimpl(_("Streams of type %s are not expected!"), structure_name);
  }
}

void
NetStreamGst::queue_underrun_cb(GstElement* /*queue*/, gpointer user_data)
{
  NetStreamGst* ns = static_cast<NetStreamGst*>(user_data);

  ns->setStatus(bufferEmpty);
}

void
NetStreamGst::queue_running_cb(GstElement* /*queue*/, gpointer  user_data)
{
  NetStreamGst* ns = static_cast<NetStreamGst*>(user_data);  
  
  ns->setStatus(bufferFull);
}

} // end of gnash namespace

