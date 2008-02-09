// 
//   Copyright (C) 2008 Free Software Foundation, Inc.
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

// derives from code subject to the following license:

/* GStreamer
 * Copyright (C) 1999,2000 Erik Walthinsen <omega@cse.ogi.edu>
 *               2000 Wim Taymans <wtay@chello.be>
 *               2004 Thomas Vander Stichele <thomas@apestaart.org>
 *
 * gst-inspect.c: tool to inspect the GStreamer registry
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

// TODO:
//
// * Decode NellyMoser (easy)
// * Implement "envelopes" (not so easy)

#include "SoundGst.h"
#include <iostream>
#include "log.h"
#include "AudioDecoderNellymoser.h"

#include "gstbuffersrc.h"

namespace gnash {
namespace media {


SoundGst::SoundGst(std::auto_ptr<SoundInfo>& sinfo)
: _info(sinfo),
  _dataSize(0)
{
  if (!gstBuildPipeline()) {
    log_error("Failed to build an audio pipeline; no playback.");
  }  
}

SoundGst::SoundGst(void* data, unsigned int data_bytes,
           std::auto_ptr<SoundInfo>& sinfo)
: _info(sinfo),
  _dataSize(0),
  _loop_count(0)
{
  assert(data_bytes);
  
  boost::uint8_t* data_ptr = reinterpret_cast<boost::uint8_t*>(data);

  
  if (!gstBuildPipeline()) {
    log_error("Failed to build an audio pipeline; no playback.");
  }
  
  pushData(data_ptr, data_bytes, 0);
}

SoundGst::~SoundGst()
{  
  gst_element_set_state (_pipeline, GST_STATE_NULL);
  
  gst_element_get_state (_pipeline, NULL, NULL, 0);
  
  gst_object_unref(GST_OBJECT(_pipeline));
  
  std::for_each(_data_vec.begin(), _data_vec.end(),
                boost::checked_array_deleter<boost::uint8_t>());

}
 

long
SoundGst::pushData(unsigned char* data, unsigned int data_bytes,
                   unsigned int sample_count)
{
  assert(data);
  
  if (_info->getFormat() == AUDIO_CODEC_NELLYMOSER_8HZ_MONO ||
      _info->getFormat() == AUDIO_CODEC_NELLYMOSER) {

    AudioDecoderNellymoser decoder;
    boost::uint32_t decoded_size;
    float* decoded_data = decoder.decode(data, data_bytes, &decoded_size);
    
    delete [] data;
    
    // *whistles*
    data = reinterpret_cast<unsigned char*>(decoded_data);      
    data_bytes = decoded_size * sizeof(float);
  }  
  
  _data_vec.push_back(data);

  GstBuffer* buffer = gst_buffer_new();
  GST_BUFFER_DATA(buffer) = data;
  GST_BUFFER_SIZE(buffer) = data_bytes;
  GST_BUFFER_MALLOCDATA(buffer) = NULL;
  
  gst_buffer_src_push_buffer_unowned (GST_BUFFER_SRC(_buffersrc), buffer);
  
  long ret = _dataSize;
  _dataSize += data_bytes;

  return ret;
}

// The offset parameter is used for ActionScript calls, while start is
// used by the parser. The offset parameter is in seconds, while the start
// parameter is in samples.
void
SoundGst::play(int loop_count, int offset, long start,
            const std::vector<sound_handler::sound_envelope>* envelopes)
{
  GstState state = GST_STATE_NULL;
  gst_element_get_state (_pipeline, &state, NULL, 0);
  
  if (state == GST_STATE_PLAYING) {
    log_debug(_("Play request while we're already playing: repeat."));
    if (_loop_count <= 0) {
      _loop_count++;
    }
    return;
  }
  
  _loop_count = loop_count;

  gst_element_set_state (_pipeline, GST_STATE_PAUSED);
  
  gst_element_get_state (_pipeline, &state, NULL, 0);
  
  gst_element_seek(_pipeline, 1.0, GST_FORMAT_BYTES,
                       GstSeekFlags(GST_SEEK_FLAG_SEGMENT | GST_SEEK_FLAG_FLUSH),
                       GST_SEEK_TYPE_SET, 0, GST_SEEK_TYPE_SET, -1);

  gst_element_set_state (_pipeline, GST_STATE_PLAYING);
  
}


// FIXME: check (and verify) the semantics of this call.
void
SoundGst::stop()
{
  gst_element_set_state (_pipeline, GST_STATE_NULL);
}
  
unsigned int
SoundGst::getVolume()
{
  gdouble volume = 10;
  g_object_get(G_OBJECT(_volume), "volume", &volume, NULL);
  return unsigned(volume * 10);
}

void
SoundGst::setVolume(unsigned int new_volume)
{
  gdouble volume = new_volume / 10;
  g_object_set(G_OBJECT(_volume), "volume", volume, NULL);
}
  
void
SoundGst::mute()
{
  g_object_set(G_OBJECT(_volume), "mute", TRUE, NULL);
}

void
SoundGst::unmute()
{
  g_object_set(G_OBJECT(_volume), "mute", FALSE, NULL);
}

bool
SoundGst::muted()
{
  gboolean mute = FALSE;
  g_object_get(G_OBJECT(_volume), "mute", &mute, NULL);
  return mute;
}

unsigned int
SoundGst::duration()
{
  GstFormat format = GST_FORMAT_TIME;
  gint64 duration;
  bool success = gst_element_query_duration(_pipeline, &format, &duration);
  
  if (!success) {
    return 0;
  } 

  return unsigned(duration / GST_MSECOND);
}

unsigned int
SoundGst::position()
{
  GstFormat format = GST_FORMAT_TIME;
  gint64 position;
  bool success = gst_element_query_position(_pipeline, &format, &position);
  
  if (!success) {
    return 0;
  } 

  return unsigned(position / GST_MSECOND);
}
  
SoundInfo*
SoundGst::getSoundInfo()
{
  return _info.get();
}

/// Creates a GstCaps corresponding to the SoundInfo attached to this SoundGst.
/// @return a GstCaps pointer, or NULL if the codec type is invalid. The caller
///         owns the returned pointer.
GstCaps*
SoundGst::getCaps()
{
  GstCaps* caps = NULL;
  switch(_info->getFormat())
  {
    case AUDIO_CODEC_ADPCM:
    {
      caps = gst_caps_new_simple ("audio/x-adpcm",
                                  "rate", G_TYPE_INT, _info->getSampleRate(),
                                  "channels", G_TYPE_INT, _info->isStereo() ? 2 : 1,
                                  "layout", G_TYPE_STRING, "swf",
                                  NULL);
      break;
    }
    case AUDIO_CODEC_MP3:
    {
      caps = gst_caps_new_simple ("audio/mpeg",
                                  "mpegversion", G_TYPE_INT, 1,
                                  "layer", G_TYPE_INT, 3,
                                  "rate", G_TYPE_INT, _info->getSampleRate(),
                                  "channels", G_TYPE_INT, _info->isStereo() ? 2 : 1,
                                  NULL);
      break;
    }
    case AUDIO_CODEC_RAW:
    case AUDIO_CODEC_UNCOMPRESSED:
    {

      caps = gst_caps_new_simple ("audio/x-raw-int",
                                  "rate", G_TYPE_INT, _info->getSampleRate(),
                                  "channels", G_TYPE_INT, _info->isStereo() ? 2 : 1,
                                  "endianness", G_TYPE_INT, G_LITTLE_ENDIAN, // FIXME: how do we know?
                                  "width", G_TYPE_INT, (_info->is16bit() ? 16 : 8),
                                  "depth", G_TYPE_INT, (_info->is16bit() ? 16 : 7),  // 7 magic?
                                  "signed", G_TYPE_BOOLEAN, TRUE, // FIXME: how do we know?
                                  NULL);
      break;
    }
    case AUDIO_CODEC_NELLYMOSER_8HZ_MONO:
    case AUDIO_CODEC_NELLYMOSER:
    {      
      std::cout << "nellymoser found" << std::endl;
      caps = gst_caps_new_simple ("audio/x-raw-float",
                                  "rate", G_TYPE_INT, _info->getSampleRate(),
                                  "channels", G_TYPE_INT, _info->isStereo() ? 2 : 1,
                                  "endianness", G_TYPE_INT, G_LITTLE_ENDIAN, // FIXME: how do we know?
                                  "width", G_TYPE_INT, 32,
                                  NULL);
      break;
    }
    default:
    {
      break;
    }
  }
  
  return caps;
}


// This function is inspired by Gstreamer's gst-inspect.c.

/// This function searches the default Gstreamer registry for a decoder that
/// can sink the given caps.
/// @param caps The capabilities to search for.
/// @param name The name to create the new element with.
/// @return A GstElement with the highest autoplugging rank that can sink the
///         given caps. The caller owns the returned pointer. NULL if no
///         capable factory was found.
GstElement*
SoundGst::gstFindDecoder(const GstCaps* caps, const gchar* name)
{
  GList *features, *l;
  GstElementFactory *factory = NULL;

  features = gst_registry_get_feature_list (gst_registry_get_default (),
    GST_TYPE_ELEMENT_FACTORY);

  for (l = features; l != NULL; l = l->next) {
    GstPluginFeature *feature;

    feature = GST_PLUGIN_FEATURE (l->data);
    
    GstElementFactory* cur_factory;

    cur_factory = GST_ELEMENT_FACTORY (feature);
      
    if (gst_element_factory_can_sink_caps(cur_factory, caps)) {
          
      const gchar* klass = gst_element_factory_get_klass(cur_factory);      
      if (!g_strrstr(klass, "Codec/Decoder/Audio")) {
        continue;
      }
    
      if (factory) {
        guint oldrank = gst_plugin_feature_get_rank(GST_PLUGIN_FEATURE(factory));
        guint newrank = gst_plugin_feature_get_rank(feature);
        if (newrank < oldrank) {
          continue;
        }
      }
      
      factory = cur_factory;      
    }
  }
  
  GstElement* decoder = NULL;
  
  if (factory) {
    decoder = gst_element_factory_create(factory, NULL);
  } else {
    log_error(_("Gnash was unable to find an appropriate Gstreamer audio "
                "decoder. Please consider installing gstreamer-ffmpeg and/or "
                "gstreamer-plugins-bad."));
  }

  g_list_foreach (features, (GFunc) gst_object_unref, NULL);
  g_list_free (features);

  return decoder;
}

bool
SoundGst::needDecoder()
{
  switch(_info->getFormat())
  {
    case AUDIO_CODEC_ADPCM:
    case AUDIO_CODEC_MP3:
      return true;
    default:
      return false;
  }
}


bool
SoundGst::gstBuildPipeline()
{
  _pipeline = gst_pipeline_new(NULL);
  
  _buffersrc = gst_element_factory_make ("buffersrc", NULL);
  
  GstCaps* src_caps = getCaps();
  
  gst_buffer_src_set_caps (GST_BUFFER_SRC(_buffersrc), src_caps);
  
  GstElement* decoder = NULL;
  
  if (needDecoder()) {
    if (_info->getFormat() == AUDIO_CODEC_MP3) {
      GstElement* audioparse = gst_element_factory_make ("mp3parse", NULL);
      if (audioparse) {
    
        decoder = gst_bin_new(NULL); 
      
        GstElement* actual_decoder = gstFindDecoder(src_caps, NULL);
        gst_bin_add_many(GST_BIN(decoder), audioparse, actual_decoder, NULL);
        assert(gst_element_link(audioparse, actual_decoder));
        
        GstPad* sinkpad = gst_element_get_static_pad (audioparse, "sink");
        GstPad* srcpad = gst_element_get_static_pad (actual_decoder, "src");
        
        gst_element_add_pad (decoder, gst_ghost_pad_new ("sink", sinkpad));
        gst_element_add_pad (decoder, gst_ghost_pad_new ("src", srcpad));
        
        gst_object_unref (GST_OBJECT (srcpad));
        gst_object_unref (GST_OBJECT (sinkpad));
      
      } else {
        decoder = gstFindDecoder(src_caps, NULL);
      }
    } else {
    
      decoder = gstFindDecoder(src_caps, NULL);
    }

    
    // FIXME: if we fail to find a decoder, should we stop here?
  }

  GstElement* audioconvert = gst_element_factory_make ("audioconvert", NULL);

  GstElement* audioresample = gst_element_factory_make ("audioresample", NULL);

  _volume = gst_element_factory_make ("volume", NULL);

  GstElement* audiosink = gst_element_factory_make ("autoaudiosink", NULL);
  
  gboolean success;
  if (decoder) {
  
    gst_bin_add_many(GST_BIN(_pipeline), _buffersrc, decoder,
                     audioconvert, audioresample, _volume, audiosink, NULL);

    success = gst_element_link_many(_buffersrc, decoder, audioconvert,
                                    audioresample, _volume, audiosink, NULL);
  } else {    
    gst_bin_add_many(GST_BIN(_pipeline), _buffersrc,
                     audioconvert, audioresample, _volume, audiosink, NULL);

    success = gst_element_link_many(_buffersrc, audioconvert, audioresample,
                                    _volume, audiosink, NULL);

  }
  
  if (!success) {
    log_error(_("Failed to link Gstreamer elements."));
  }  
  
  gst_caps_unref(src_caps);
  
  if (!_volume || !audioconvert || !audioresample || !audiosink) {
    log_error("Couldn't load the necessary Gstreamer modules for playback. "
              "Please ensure a proper gstreamer-plugins-base installation.");
    return false;
  }
  
  return true;
}

void SoundGst::poll()
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
SoundGst::handleMessage (GstMessage *message)
{

  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR:
    {
      GError *err;
      gchar *debug;
      gst_message_parse_error (message, &err, &debug);
      
      log_error(_("Embedded audio playback halted; module %s reported: %s\n"),
                gst_element_get_name(GST_MESSAGE_SRC (message)), err->message);
      
      g_error_free (err);
      g_free (debug);
      
      // Clear any buffers.
      gst_element_set_state (_pipeline, GST_STATE_NULL);

      break;
    }
    case GST_MESSAGE_EOS:
      gst_element_set_state (_pipeline, GST_STATE_NULL);

      break;    
    case GST_MESSAGE_SEGMENT_DONE:
    {      
      if (_loop_count <= 0) {
        // This seek from 0 to 0 obviously doesn't do much seeking. However, it
        // makes sure that we will receive an EOS, since this is a non-flushing
        // seek which will wait until the segment being played is done.        
        gst_element_seek(_pipeline, 1.0, GST_FORMAT_BYTES,
                       GST_SEEK_FLAG_NONE,
                       GST_SEEK_TYPE_SET, 0, GST_SEEK_TYPE_SET, 0);
        break;
      }
      
      _loop_count--;
      
      gst_element_seek(_pipeline, 1.0, GST_FORMAT_BYTES,
                       GST_SEEK_FLAG_SEGMENT /* non-flushing: seamless seek */,
                       GST_SEEK_TYPE_SET, 0, GST_SEEK_TYPE_SET, -1);
      break;
    }    
    default:
    {
#if 0
      g_print("unhandled message\n");
#endif
    }
  }

}



} // namespace media
} // namespace gnash

