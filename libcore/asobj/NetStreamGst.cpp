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
#include "VM.h" 
#include "string_table.h"
#include <boost/algorithm/string/case_conv.hpp> // for PROPNAME (shouldn't this include be in the header actualy defining PROPNAME, btw?)
#include <cmath> // std::floor

#include "Object.h"
#include "gstflvdemux.h"
#include <gst/gstelement.h>
#include <GstUtil.h>

#ifdef GST_HAS_MODERN_PBUTILS
#include <gst/pbutils/pbutils.h>
#include <gst/pbutils/missing-plugins.h>
#include <gst/pbutils/install-plugins.h>
#endif // GST_HAS_MODERN_PBUTILS


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
  _audiobin = gst_bin_new(NULL);
  _videobin = gst_bin_new(NULL);


  // Setup general decoders
  _dataqueue = gst_element_factory_make ("queue", "gnash_dataqueue");
  g_signal_connect (_dataqueue, "underrun", G_CALLBACK (NetStreamGst::queue_underrun_cb), this);
  g_signal_connect (_dataqueue, "running", G_CALLBACK (NetStreamGst::queue_running_cb), this);
    
  GstElement* decoder = gst_element_factory_make ("decodebin", NULL);  
  g_signal_connect (decoder, "new-decoded-pad", G_CALLBACK (NetStreamGst::decodebin_newpad_cb), this);
  g_signal_connect (decoder, "unknown-type", G_CALLBACK (NetStreamGst::decodebin_unknown_cb), this);   

  gst_bin_add_many (GST_BIN (_pipeline), _dataqueue, decoder, NULL);

  if (!_dataqueue || !decoder) {
    log_error(_("Couldn't create the \"queue\" and/or \"decoder\" elements. "
                "Please make sure Gstreamer and gstreamer-plugins-base are "
                "correctly installed. NetStream playback halted."));
    return;
  }

  bool rv = gst_element_link(_dataqueue, decoder);

  if (!rv) {
    log_error("Couldn't link \"queue\" and \"decoder\" elements. NetStream "
              "playback halted.");
    return;
  }

  // Setup video conversion and sink


  // setup the video colorspaceconverter converter
  GstElement* colorspace = gst_element_factory_make ("ffmpegcolorspace", "gnash_colorspace");

  GstElement* videocaps = gst_element_factory_make ("capsfilter", NULL);

  // Make sure we receive RGB
  GstCaps* videooutcaps = gst_caps_new_simple ("video/x-raw-rgb",
                                               "bpp", G_TYPE_INT, 24,
                                               "depth", G_TYPE_INT, 24,
                                               NULL);
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
  gst_bin_add_many (GST_BIN (_videobin), colorspace, videoscale, videocaps, videosink, NULL);
  
  if (!colorspace || !videoscale || !videocaps || !videosink) {
    log_error(_("Couldn't create the Gstreamer video conversion elements. "
                "Please make sure Gstreamer and gstreamer-plugins-base are "
                "correctly installed. Video playback will not be possible."));
  }
  
  rv = gst_element_link_many(colorspace, videoscale, videocaps, videosink, NULL);
  if (!rv) {
    log_error(_("Failed to link video conversion elements. Video playback will"
                " not be possible"));
  }

  // Setup audio sink
  GstElement* audioconvert = gst_element_factory_make ("audioconvert", NULL);	
  
  GstElement* audiosink;
  
  if (get_sound_handler()) {
    
    audiosink = gnash::media::GstUtil::get_audiosink_element();
  
    if(!audiosink) {
      log_error(_("Failed to make a valid audio sink."));
    }
  } else {
    audiosink = gst_element_factory_make ("fakesink", NULL);
  }

  gst_bin_add_many(GST_BIN(_audiobin), audioconvert, audiosink, NULL);

  if (!audioconvert || !audiosink) {
    log_error("Couldn't create Gstreamer audio elements. Audio playback will "
              "not be possible");
  }
  rv = gst_element_link(audioconvert, audiosink);
  if (!rv) {
    log_error("Couldn't link audio elements. There will be no audio playback.");
  }

  GstPad* target_audiopad = gst_element_get_static_pad (audioconvert, "sink");
  GstPad* target_videopad = gst_element_get_static_pad (colorspace, "sink");
  
  gst_element_add_pad(_videobin, gst_ghost_pad_new ("sink", target_videopad));
  gst_element_add_pad(_audiobin, gst_ghost_pad_new ("sink", target_audiopad));
  
  gst_object_unref(GST_OBJECT(target_videopad));
  gst_object_unref(GST_OBJECT(target_audiopad));


}

NetStreamGst::~NetStreamGst()
{
  gst_element_set_state (_pipeline, GST_STATE_NULL);
  
  
  gst_element_get_state(_pipeline, NULL, NULL, 0); // wait for a response

  gst_object_unref(GST_OBJECT(_pipeline));
  
#ifdef GST_HAS_MODERN_PBUTILS
  std::for_each(_missing_plugins.begin(), _missing_plugins.end(), g_free);
#endif
}

void
NetStreamGst::close()
{ 
  gst_element_set_state (_pipeline, GST_STATE_NULL);  

  setStatus(playStop);
  
  processStatusNotifications();

  boost::mutex::scoped_lock lock(image_mutex);

  m_imageframe.reset();

  stopAdvanceTimer();
}

void 
NetStreamGst::pause(PauseMode mode)
{
  GstState newstate = GST_STATE_VOID_PENDING;
  switch(mode) {
    case pauseModeToggle:
    {
      GstState cur_state;
      
      GstStateChangeReturn statereturn
        = gst_element_get_state(_pipeline, &cur_state, NULL, 1 * GST_MSECOND);

      if (statereturn == GST_STATE_CHANGE_ASYNC) {
        return;
      }
      
      if (cur_state == GST_STATE_PLAYING) {
        newstate = GST_STATE_PAUSED;
      } else {
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
  log_debug("%s: connecting to %s\n", __FUNCTION__, valid_url);
#endif
  if (valid_url.empty()) {
    log_error(_("%s: The provided URL could not be resolved (url: %s)"), 
              __FUNCTION__, valid_url.c_str());
    return;
  }
  
  if (_downloader) {
    gst_element_set_state (_pipeline, GST_STATE_NULL);
    
    gst_bin_remove(GST_BIN(_pipeline), _downloader); // will also unref
    
    // FIXME: we should probably disconnect the currently connected pads
    // and remove the video and audio bins from the pipeline.
  }
 
  _downloader = gst_element_make_from_uri(GST_URI_SRC, valid_url.c_str(),
                                          "gnash_uridownloader");
  if (!_downloader) {
    log_error(_("%s: No URI handler was found for the provided URL. NetStream "
              "playback will not be possible! (url: %s). Please make sure you "
              " have a URL handling gstreamer plugin, such as gnomevfssrc, "
              "neonhttpsrc or souphttpsrc."), __FUNCTION__,
              valid_url.c_str());
    return;  
  }

  GstElementFactory* urifactory = gst_element_get_factory(_downloader);
  const gchar* urifactoryname = gst_element_factory_get_longname(urifactory);

  log_debug(_("URI handler \"%s\" found for URI %s"), urifactoryname, 
            valid_url.c_str());

                                                         
  bool success = gst_bin_add(GST_BIN(_pipeline), _downloader);
  if (!success) {
    log_error(_("gst_bin_add failed. Aborting NetStream.play()."));
    gst_object_unref(GST_OBJECT(_downloader));
    _downloader = NULL;
    return;
  }

  success = gst_element_link(_downloader, _dataqueue);  
  if (!success) {
    log_error(_("gst_element_link failed. Aborting NetStream.play()."));
    gst_object_unref(GST_OBJECT(_downloader));
    _downloader = NULL;
    return;
  }



  // Pause the pipeline. This will give decodebin a chance to detect streams.
  gst_element_set_state (_pipeline, GST_STATE_PAUSED);
  
  // Wait for pause return; by this time, decodebin should be about finished.
  gst_element_get_state (_pipeline, NULL, NULL, 0);
  
  // Commence playback.
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
    log_debug(_("Seek failed. This is expected, but we tried it anyway."));
    setStatus(invalidTime);
  }
}

boost::int32_t
NetStreamGst::time()
{  
  if ( ! _pipeline ) return 0;

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
  
  handleMissingPlugins();
  
  processStatusNotifications();
  
  g_main_context_iteration (NULL, FALSE);
}

double
NetStreamGst::getCurrentFPS()
{
  GstElement*  colorspace = gst_bin_get_by_name (GST_BIN(_videobin), "gnash_colorspace");
    
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

  if ( ! _downloader ) return 0;

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
  if ( ! _downloader ) return _duration;

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

  // NOTE: gst provides more metadata elements then those provided by
  //       the reference player, we might want to pick only the standard ones.
  //       Won't do for now.

  // We want props of the metadata object to be:
  // 	- enumerable
  // 	- overridable
  // 	- deletable
  // This is tested in misc-ming.all/NetStream-SquareTest.{c,swf}
  //

  VM& vm = o->getVM();
  string_table& st = vm.getStringTable();
  string_table::key key = st.find(PROPNAME(nick));

  switch(gst_tag_get_type(tag)) {
    case G_TYPE_STRING:
    {
      gchar* value;

      gst_tag_list_get_string(list, tag, &value);
      
      o->set_member(key, value);
      
      g_free(value);

      break;
    }
    case G_TYPE_DOUBLE:
    {
      gdouble value;
      gst_tag_list_get_double(list, tag, &value);
      o->set_member(key, (double)value);
      
      break;
    }
    case G_TYPE_BOOLEAN:
    {
      gboolean value;
      gst_tag_list_get_boolean(list, tag, &value);
      o->set_member(key, (bool)value);
      break;
    }
    case G_TYPE_UINT64:
    {
      guint64 value;
      gst_tag_list_get_uint64(list, tag, &value);
      as_value val;
      if ( ! strcmp(nick, "duration") )
      {
         // duration is given in nanoseconds, we want that in seconds,
         // and rounded to the millisecond 
         val.set_double(std::floor( (value / 1000000.0) + 0.5) / 1000.0);
      }
      else
      {
         val.set_double(value);
      }
      o->set_member(key, val); 
      break;
    }
    case G_TYPE_UINT:
    {
      guint value;
      gst_tag_list_get_uint(list, tag, &value);
      o->set_member(key, value);
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
      GError *err = NULL;
      gchar *debug = NULL;
      gst_message_parse_error (message, &err, &debug);
      
      log_error(_("NetStream playback halted; module %s reported: %s\n"),
                gst_element_get_name(GST_MESSAGE_SRC (message)), err->message);
      
      g_error_free (err);
      if(debug)
      	g_free (debug);
      
      setStatus(streamNotFound);
      setStatus(playStop);
      
      // Clear any buffers.
      gst_element_set_state (_pipeline, GST_STATE_NULL);

      break;
    }
    case GST_MESSAGE_EOS:
      log_debug(_("NetStream has reached the end of the stream."));
      setStatus(playStop);
      break;
    case GST_MESSAGE_TAG:
    {
      GstTagList* taglist = NULL;

      gst_message_parse_tag(message, &taglist);
      
      gchar* value = NULL;
      if (!gst_tag_list_get_string(taglist, "___function_name___", &value)) {
        break;
      }
      
      std::string funcname(value);
      if(value)
      	g_free(value);
      
      gst_tag_list_remove_tag (taglist, "___function_name___");

        
      as_object* o = new as_object(getObjectInterface());

      gst_tag_list_foreach(taglist, metadata, o);

      processNotify(funcname, o);
      
      if(taglist)
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
      GstState oldstate, newstate, pending;

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
      break;
    }
    
    default:
    {
#ifdef DEBUG_MESSAGES
      g_print("unhandled message\n");
#endif
    }
  }
  
  missingPluginMsg(message);
}

void
NetStreamGst::missingPluginMsg(GstMessage* message)
{
#ifdef GST_HAS_MODERN_PBUTILS
  if (!gst_is_missing_plugin_message(message)) {
    return;
  }  
  
  gchar* plugin_name = gst_missing_plugin_message_get_description (message);

#ifdef HAVE_GST_INSTALL_PLUGINS_SUPPORTED
  if (!gst_install_plugins_supported()) {
    log_error(_("Missing Gstreamer plugin: %s. Please consider installing it."),
      plugin_name);
   	if(plugin_name)
    	g_free(plugin_name);
    return;
  }
#endif
      
  gchar* detail = gst_missing_plugin_message_get_installer_detail (message);
  
  _missing_plugins.push_back(detail);
  
  log_debug(_("Missing plugin: %s. Will attempt to start system installer"),
    plugin_name);
if(plugin_name)
  g_free(plugin_name);
#else
UNUSED(message);
#endif
}

#ifdef GST_HAS_MODERN_PBUTILS
static void
install_plugins_cb(GstInstallPluginsReturn result, gpointer /*user_data*/)
{
  switch(result) {
    case GST_INSTALL_PLUGINS_SUCCESS:
    case GST_INSTALL_PLUGINS_PARTIAL_SUCCESS:
      log_debug(_("Gstreamer plugin installation was at least partially "
                "successful. Will try to restart the pipeline."));
      break;    
    default:
      log_error(_("The request for system installation of missing plugins "
                  "has failed. Full playback will not be possible."));
  }
}
#endif

void
NetStreamGst::handleMissingPlugins()
{
#ifdef GST_HAS_MODERN_PBUTILS
  if (_missing_plugins.empty()) {
    return;
  }

  boost::scoped_array<gchar*> details( new char*[_missing_plugins.size()+1] );

  for (size_t i = 0; i < _missing_plugins.size(); ++i) {
    details[i] = _missing_plugins[i];
  }

  details[_missing_plugins.size()] = NULL;

  GstInstallPluginsReturn ret = gst_install_plugins_async(details.get(), NULL,
    install_plugins_cb, NULL);
  
  std::for_each(_missing_plugins.begin(), _missing_plugins.end(), g_free);
  _missing_plugins.clear();
  
  switch(ret) {
    case GST_INSTALL_PLUGINS_STARTED_OK:
    case GST_INSTALL_PLUGINS_INSTALL_IN_PROGRESS:
      break;
    default:
      log_error(_("Failed to start the system Gstreamer plugin installer."
                "Media playback will not work (fully)."));      
  }
#endif
}


// NOTE: callbacks will be called from the streaming thread!

void 
NetStreamGst::video_data_cb(GstElement* /*c*/, GstBuffer *buffer,
                            GstPad* /*pad*/, gpointer user_data)
{
  NetStreamGst* ns = reinterpret_cast<NetStreamGst*>(user_data);

  GstElement*  colorspace = gst_bin_get_by_name (GST_BIN(ns->_videobin),
                                                 "gnash_colorspace");
  
  GstPad* videopad = gst_element_get_static_pad (colorspace, "src");
  GstCaps* caps = gst_pad_get_negotiated_caps (videopad);
    
  gint height, width;

  GstStructure* str = gst_caps_get_structure (caps, 0);

  gst_structure_get_int (str, "width", &width);
  gst_structure_get_int (str, "height", &height);
  
  boost::mutex::scoped_lock lock(ns->image_mutex);
  
  if (!ns->m_imageframe.get() || unsigned(width) != ns->m_imageframe->width() ||
      unsigned(height) != ns->m_imageframe->height()) {
    ns->m_imageframe.reset( new image::ImageRGB(width, height) );
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

  GstElement* sink;
  
  if (g_strrstr (structure_name, "audio")) {
    sink = ns->_audiobin;
  } else if (g_strrstr (structure_name, "video")) {
    sink = ns->_videobin;
  } else {
    log_unimpl(_("Streams of type %s are not supported!"), structure_name);
    return;
  }
  
  log_debug("%s: linking %s stream.",  __FUNCTION__, structure_name);
  
  gst_caps_unref (caps);
  
  gst_bin_add (GST_BIN(ns->_pipeline), sink);
  
  gst_element_set_state (sink, GST_STATE_PAUSED);
  
  GstPad* sinkpad = gst_element_get_pad (sink, "sink");
  
  if (GST_PAD_IS_LINKED(sinkpad)) {
    // already linked
    gst_object_unref(G_OBJECT(sinkpad));
    return;
  }
  
  gst_pad_link(pad, sinkpad);
    
  gst_object_unref(G_OBJECT(sinkpad));  
}

void
NetStreamGst::decodebin_unknown_cb(GstElement* /*decodebin*/, GstPad* /*pad*/,
                                  GstCaps *caps, gpointer /*user_data*/)
{
  GstStructure* str = gst_caps_get_structure (caps, 0);
  const gchar* structure_name = gst_structure_get_name (str);
  
  log_error(_("Couldn't find a decoder for stream type %s!"), structure_name);
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

