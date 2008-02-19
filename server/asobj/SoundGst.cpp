// SoundGst.cpp:  Produce sound for gnash, via Gstreamer library.
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
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "log.h"
#include "SoundGst.h"
#include "sound_definition.h" // for sound_sample
#include "movie_definition.h"
#include "sprite_instance.h"
#include "fn_call.h"
#include "GnashException.h"
#include "builtin_function.h"
#include "URL.h"

#include <string>

// TODO: implement loops
//       seeking

namespace gnash {

// Callback function used by Gstreamer to attach audio and video streams
// detected by decodebin to either the video out or audio out elements.
void
SoundGst::callback_newpad (GstElement* /*decodebin*/, GstPad *pad, gboolean /*last*/, gpointer data)
{
#if 0
  log_debug(_("%s: new pad found"), __FUNCTION__);
#endif
  SoundGst* so = static_cast<SoundGst*>(data);
  GstCaps *caps;
  GstStructure *str;
  GstPad *audiopad;

  audiopad = gst_element_get_static_pad (so->_audioconv, "sink");

  // check media type
  caps = gst_pad_get_caps (pad);
  str = gst_caps_get_structure (caps, 0);
  if (g_strrstr (gst_structure_get_name (str), "audio")) {
    // link'n'play
    gst_pad_link (pad, audiopad);
    log_debug(_("%s: new pad connected"), __FUNCTION__);
  } else {
    gst_object_unref (audiopad);
    log_debug(_("%s: Non-audio data found in Sound url"), __FUNCTION__);
  }
  gst_caps_unref (caps);
  gst_object_unref(GST_OBJECT(audiopad));
}

void
SoundGst::setupDecoder(const std::string& url)
{
  _inputPos = 0;

  // init GStreamer
  gst_init (NULL, NULL);

  // setup the pipeline
  _pipeline = gst_pipeline_new (NULL);

  if (!_pipeline) {
    log_error(_("Could not create gstreamer pipeline element"));
    return;
  }

#if !defined(__NetBSD__)
  _audiosink = gst_element_factory_make ("autoaudiosink", NULL);
  if (!_audiosink) _audiosink = gst_element_factory_make ("alsasink", NULL);
  if (!_audiosink) _audiosink = gst_element_factory_make ("osssink", NULL);
#endif
  if (!_audiosink) _audiosink = gst_element_factory_make ("esdsink", NULL);

  if (!_audiosink) {
    log_error(_("Could not create gstreamer audiosink element"));
                gst_object_unref(GST_OBJECT(_pipeline));
    return;
  }

  // setup the audio converter
  _audioconv = gst_element_factory_make ("audioconvert", NULL);

  // setup the volume controller
  _volume = gst_element_factory_make ("volume", NULL);

  // setup the decoder with callback
  _decoder = gst_element_factory_make ("decodebin", NULL);
  g_signal_connect (_decoder, "new-decoded-pad", G_CALLBACK (SoundGst::callback_newpad), this);


  if (!_audioconv || !_volume || !_decoder) {
    gnash::log_error(_("Could not create Gstreamer element(s) for movie handling"));
    return;
  }

  GstElement* downloader = gst_element_make_from_uri(GST_URI_SRC, url.c_str(),
                 "gnash_audiodownloader");

  GstElement* queue = gst_element_factory_make ("queue", "gnash_audioqueue");


  // put it all in the pipeline
  gst_bin_add_many (GST_BIN (_pipeline), downloader, queue, _decoder,
        _audiosink, _audioconv, _volume, NULL);

  // link the elements
  gst_element_link_many(_audioconv, _volume, _audiosink, NULL);
  gst_element_link_many(downloader, queue, _decoder, NULL);
  
  return;
}

SoundGst::~SoundGst()
{

  if (externalSound && _pipeline) {
    gst_element_set_state (_pipeline, GST_STATE_NULL);
    gst_object_unref (GST_OBJECT (_pipeline));
  }
}

void
SoundGst::loadSound(const std::string& url, bool streaming)
{
  connection = new NetConnection;

  std::string valid_url = connection->validateURL(url);

  log_debug("%s: loading URL %s from %s", __FUNCTION__, valid_url.c_str(),
          url.c_str());

  _remainingLoops = 0;

  if (_pipeline) {
    log_debug(_("%s: This sound already has a pipeline. Resetting for new URL connection. (%s)"), __FUNCTION__, valid_url.c_str());
          gst_element_set_state (_pipeline, GST_STATE_NULL); // FIXME: wait for state?

    GstElement* downloader = gst_bin_get_by_name(GST_BIN(_pipeline), "gnash_audiodownloader");
    gst_bin_remove(GST_BIN(_pipeline), downloader); 
    gst_object_unref(GST_OBJECT(downloader));

    downloader = gst_element_make_from_uri(GST_URI_SRC, valid_url.c_str(),
                                                       "gnash_audiodownloader");

    gst_bin_add(GST_BIN(_pipeline), downloader);

    GstElement* queue = gst_bin_get_by_name(GST_BIN(_pipeline), "gnash_audioqueue");

    gst_element_link(downloader, queue);
    gst_object_unref(GST_OBJECT(queue));
  } else {
    setupDecoder(valid_url);
  }

  externalSound = true;

  if (streaming) {
    start(0, 0);
  }
}

void
SoundGst::start(int offset, int loops)
{
  if (!externalSound) {
    Sound::start(offset, loops);
    return;
  }
  
  // Seek to offset position if necessary (Note: GST seems to report an error 
  // when trying to seek to the current position)
  gint64 oldcur = -1;
  gint64 newcur = GST_SECOND * static_cast<long>(offset);
  GstFormat fmt = GST_FORMAT_TIME;
  
  gst_element_query_position(_pipeline, &fmt, &oldcur); // ignore err.
  
  if (newcur != oldcur) {    
    if (!gst_element_seek (_pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
      GST_SEEK_TYPE_SET, newcur, GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) {
      log_error(_("%s: seeking to offset failed"), 
        __FUNCTION__);
    }
  }

  // Save how many loops to do
  if (loops > 0) {
    _remainingLoops = loops;
  }
  // start playing  
  gst_element_set_state (_pipeline, GST_STATE_PLAYING);
}

void
SoundGst::stop(int si)
{
  if (!externalSound) {
    Sound::stop(si);
    return;
  }

  gst_element_set_state (GST_ELEMENT (_pipeline), GST_STATE_NULL);
}

unsigned int
SoundGst::getDuration()
{
  // Return the duration of the file in milliseconds
  
  if (!externalSound) {
    return Sound::getDuration();
  }
  
  GstFormat fmt = GST_FORMAT_TIME;
  boost::int64_t len;

  if (_pipeline && gst_element_query_duration (_pipeline, &fmt, &len)) {
    return static_cast<unsigned int>(len / GST_MSECOND);
  } else {
    return 0;
  }
}

unsigned int
SoundGst::getPosition()
{
  // Return the position in the file in milliseconds
  
  if (!externalSound) {
        return Sound::getPosition();
  }
  
  if (!_pipeline) return 0;

  GstFormat fmt = GST_FORMAT_TIME;
  boost::int64_t pos;
  GstStateChangeReturn ret;
  GstState current, pending;

  ret = gst_element_get_state (GST_ELEMENT (_pipeline), &current, &pending, 0);

  if (current != GST_STATE_NULL && gst_element_query_position (_pipeline, &fmt, &pos)) {
    return static_cast<unsigned int>(pos / GST_MSECOND);
  } else {
    return 0;
  }
}

} // end of gnash namespace
