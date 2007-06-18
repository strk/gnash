// NetStreamGst.cpp:  Audio/video output via Gstreamer library, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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

/* $Id: NetStreamGst.cpp,v 1.56 2007/06/18 20:20:20 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef SOUND_GST

#include "log.h"
#include "NetStreamGst.h"
#include "fn_call.h"
#include "NetStream.h"
#include "URLAccessManager.h"
#include "render.h"	
#include "movie_root.h"
#include "NetConnection.h"
//#include "action.h"

#include "gstgnashsrc.h"

#ifdef GST_HAS_MODERN_PBUTILS
#include <gst/pbutils/missing-plugins.h>
#include <gst/pbutils/install-plugins.h>
#endif // GST_HAS_MODERN_PBUTILS

#include "URL.h"

// Define the following macro to enable debugging traces
//#define GNASH_DEBUG

namespace gnash {

static gboolean
register_elements (GstPlugin *plugin)
{
	return gst_element_register (plugin, "gnashsrc", GST_RANK_NONE, GST_TYPE_GNASH_SRC);
}

static GstPluginDesc gnash_plugin_desc = {
	0, // GST_VERSION_MAJOR
	10, // GST_VERSION_MINOR
	"gnashsrc",
	"Use gnash as source via callbacks",
	register_elements,
	"0.0.1",
	"LGPL",
	"gnash",
	"gnash",
	"http://www.gnu.org/software/gnash/",
	GST_PADDING_INIT
};

NetStreamGst::NetStreamGst():

	pipeline(NULL),
	audiosink(NULL),
	videosink(NULL),
	decoder(NULL),
	volume(NULL),
	colorspace(NULL),
	videorate(NULL),
	videocaps(NULL),
	videoflip(NULL),
	audioconv(NULL),

	audiosource(NULL),
	videosource(NULL),
	source(NULL),
	videodecoder(NULL),
	audiodecoder(NULL),
	videoinputcaps(NULL),
	audioinputcaps(NULL),
	_handoffVideoSigHandler(0),
	_handoffAudioSigHandler(0),

#ifndef DISABLE_START_THREAD
	startThread(NULL),
#endif
	videowidth(0),
	videoheight(0),
	m_clock_offset(0),
	m_pausePlayback(false)
{
	gst_init (NULL, NULL);
}

NetStreamGst::~NetStreamGst()
{
	close();
}

void NetStreamGst::pause(int mode)
{
	if (mode == -1)
	{
		m_pause = ! m_pause;
	}
	else
	{
		m_pause = (mode == 0) ? true : false;
	}

	if (pipeline)
	{
		if (m_pause)
		{ 
			log_msg("Pausing pipeline on user request");
			if ( ! pausePipeline(false) )
			{
				log_error("Could not pause pipeline");
			}
		}
		else
		{
			if ( ! playPipeline() )
			{
				log_error("Could not play pipeline");
			}
		}
	}

	if (!pipeline && !m_pause && !m_go) {
		setStatus(playStart);
		m_go = true;
		// To avoid blocking while connecting, we use a thread.
#ifndef DISABLE_START_THREAD
		startThread = new boost::thread(boost::bind(NetStreamGst::playbackStarter, this));
#else
		startPlayback(this);
#endif
	}
}

void NetStreamGst::close()
{
	if (m_go)
	{
		setStatus(playStop);
		m_go = false;
#ifndef DISABLE_START_THREAD
		startThread->join();
		delete startThread;
#endif
	}

	if ( ! disablePipeline() )
	{
		log_error("Can't reset pipeline on close");
	}

	// Should we keep the ref if the above failed ?
	// Unreffing the pipeline should also unref all elements in it.
	gst_object_unref (GST_OBJECT (pipeline));
	pipeline = NULL;

	boost::mutex::scoped_lock lock(image_mutex);

	delete m_imageframe;
	m_imageframe = NULL;

	_handoffVideoSigHandler = 0;
	_handoffAudioSigHandler = 0;

	videowidth = 0;
	videoheight = 0;
	m_clock_offset = 0;
	m_pausePlayback = false;
}


void
NetStreamGst::play(const std::string& c_url)
{

	// Does it have an associated NetConnection?
	if ( ! _netCon )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("No NetConnection associated with this NetStream, won't play"));
		);
		return;
	}

	// Is it already playing ?
	if (m_go)
	{
		if (m_pause)
		{
			playPipeline();
		}
		return;
	}

	if (url.size() == 0) url += c_url;
	// Remove any "mp3:" prefix. Maybe should use this to mark as audio-only
	if (url.compare(0, 4, std::string("mp3:")) == 0) {
		url = url.substr(4);
	}
	m_go = true;

	// To avoid blocking while connecting, we use a thread.
#ifndef DISABLE_START_THREAD
	startThread = new boost::thread(boost::bind(NetStreamGst::playbackStarter, this));
#else
	startPlayback(this);
#endif
	return;
}


// Callback function used by Gstreamer to to attached audio and video streams
// detected by decoderbin to either the video out or audio out elements.
// Only used when not playing FLV
void
NetStreamGst::callback_newpad (GstElement* /*decodebin*/, GstPad *pad, gboolean /*last*/, gpointer data)
{

	NetStreamGst* ns = static_cast<NetStreamGst*>(data);
	GstCaps *caps;
	GstStructure *str;
	GstPad *audiopad, *videopad;

	audiopad = gst_element_get_pad (ns->audioconv, "sink");
	videopad = gst_element_get_pad (ns->colorspace, "sink");

	// check media type
	caps = gst_pad_get_caps (pad);
	str = gst_caps_get_structure (caps, 0);
	if (g_strrstr (gst_structure_get_name (str), "audio")) {
		gst_object_unref (videopad);

		// link'n'play
		gst_pad_link (pad, audiopad);

	} else if (g_strrstr (gst_structure_get_name (str), "video")) {
		gst_object_unref (audiopad);
		// Link'n'play
		gst_pad_link (pad, videopad);
	} else {
		gst_object_unref (audiopad);
		gst_object_unref (videopad);
	}
	gst_caps_unref (caps);
	return;

}

// The callback function which unloads the decoded video frames unto the video
// output imageframe.
void 
NetStreamGst::callback_output (GstElement* /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data)
{
	NetStreamGst* ns = static_cast<NetStreamGst*>(user_data);

	boost::mutex::scoped_lock lock(ns->image_mutex);

	// If the video size has not yet been detected, detect them
	if (ns->videowidth == 0 && ns->videoheight == 0) {
		GstPad* videopad = gst_element_get_pad (ns->colorspace, "src");
		GstCaps* caps = gst_pad_get_caps (videopad);
		GstStructure* str = gst_caps_get_structure (caps, 0);

		int height, width, ret, framerate1, framerate2;
		ret = gst_structure_get_int (str, "width", &width);
		ret &= gst_structure_get_int (str, "height", &height);
		if (ret) {
			ns->videowidth = width;
			ns->videoheight = height;
		}
		ret = gst_structure_get_fraction (str, "framerate", &framerate1, &framerate2);
		
		// Setup the output videoframe
		if (ns->m_videoFrameFormat == render::YUV) {
			ns->m_imageframe = new image::yuv(width, height);
		} else if (ns->m_videoFrameFormat == render::RGB) {
			ns->m_imageframe = new image::rgb(width, height);
		}
	}

	if (ns->m_imageframe) {
//		ns->m_imageframe->update(GST_BUFFER_DATA(buffer));
		if (ns->m_videoFrameFormat == render::YUV) {
			assert(0);

		/*	image::yuv* yuvframe = static_cast<image::yuv*>(m_imageframe);
			int copied = 0;
			uint8_t* ptr = GST_BUFFER_DATA(buffer);
			for (int i = 0; i < 3 ; i++)
			{
				int shift = (i == 0 ? 0 : 1);
				uint8_t* yuv_factor = m_Frame->data[i];
				int h = ns->videoheight >> shift;
				int w = ns->videowidth >> shift;
				for (int j = 0; j < h; j++)
				{
					copied += w;
					assert(copied <= yuvframe->size());
					memcpy(ptr, yuv_factor, w);
					yuv_factor += m_Frame->linesize[i];
					ptr += w;
				}
			}
			video->m_size = copied;*/
		} else {
			ns->m_imageframe->update(GST_BUFFER_DATA(buffer));
			ns->m_newFrameReady = true;
		}

	}

}


// The callback function which refills the audio buffer with data
// Only used when playing FLV
void NetStreamGst::audio_callback_handoff (GstElement * /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data)
{
	NetStreamGst* ns = static_cast<NetStreamGst*>(user_data);

	FLVFrame* frame = ns->m_parser->nextAudioFrame();
	if (!frame) {
		ns->setStatus(bufferEmpty);
		ns->m_pausePlayback = true;
		return;
	}

//	if (GST_BUFFER_DATA(buffer)) delete [] GST_BUFFER_DATA(buffer);
	GST_BUFFER_SIZE(buffer) = frame->dataSize;
	GST_BUFFER_DATA(buffer) = frame->data;
	GST_BUFFER_TIMESTAMP(buffer) = (frame->timestamp + ns->m_clock_offset) * GST_MSECOND;
	delete frame;
	return;

}

// The callback function which refills the video buffer with data
// Only used when playing FLV
void
NetStreamGst::video_callback_handoff (GstElement * /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data)
{
	//GNASH_REPORT_FUNCTION;

	NetStreamGst* ns = static_cast<NetStreamGst*>(user_data);

	FLVFrame* frame = ns->m_parser->nextVideoFrame();
	if (!frame) {
		ns->setStatus(bufferEmpty);
		ns->m_pausePlayback = true;
		return;
	}

//	if (GST_BUFFER_DATA(buffer)) delete [] GST_BUFFER_DATA(buffer);
	GST_BUFFER_SIZE(buffer) = frame->dataSize;
	GST_BUFFER_DATA(buffer) = frame->data;
	GST_BUFFER_TIMESTAMP(buffer) = (frame->timestamp + ns->m_clock_offset) * GST_MSECOND;
	delete frame;
	return;
}

void
NetStreamGst::playbackStarter(NetStreamGst* ns)
{
	ns->startPlayback();
}

void
NetStreamGst::unrefElements()
{
#ifdef GNASH_DEBUG
	log_debug("unreffing elements");
#endif

	boost::mutex::scoped_lock lock(_pipelineMutex);

	// TODO: Define an GstElement class for storing all these elements,
	//       and have it's destructor take care of unreffing...
	//
	// TODO2: check if calling gst_object_unref is enough to release all
	//       resources allocated by gst_*_new or gst_element_factory_make

	if ( pipeline )
	{
		gst_object_unref (GST_OBJECT (pipeline));
		pipeline = NULL;
	}

	if ( audiosink )
	{
		gst_object_unref (GST_OBJECT (audiosink));
		audiosink = NULL;
	}

	if ( videosink )
	{
		gst_object_unref (GST_OBJECT (videosink));
		videosink = NULL;
	}

	if ( volume )
	{
		gst_object_unref (GST_OBJECT (volume));
		volume = NULL;
	}

	if ( colorspace )
	{
		gst_object_unref (GST_OBJECT (colorspace));
		colorspace = NULL;
	}

	if ( videorate )
	{
		gst_object_unref (GST_OBJECT (videorate));
		videorate = NULL;
	}

	if ( videocaps )
	{
		gst_object_unref (GST_OBJECT (videocaps));
		videocaps = NULL;
	}

	if ( videoflip )
	{
		gst_object_unref (GST_OBJECT (videoflip));
		videoflip = NULL;
	}

	if ( audioconv )
	{
		gst_object_unref (GST_OBJECT (audioconv));
		audioconv = NULL;
	}

	if (m_isFLV)
	{
		if ( audiosource )
		{
			gst_object_unref (GST_OBJECT (audiosource));
			audiosource = NULL;
		}

		if ( videosource )
		{
			gst_object_unref (GST_OBJECT (videosource));
			videosource = NULL;
		}

		if ( videodecoder )
		{
			gst_object_unref (GST_OBJECT (videodecoder));
			videodecoder = NULL;
		}

		if ( audiodecoder )
		{
			gst_object_unref (GST_OBJECT (audiodecoder));
			audiodecoder = NULL;
		}

		if ( videoinputcaps )
		{
			gst_object_unref (GST_OBJECT (videoinputcaps));
			videoinputcaps = NULL;
		}

		if ( audioinputcaps )
		{
			gst_object_unref (GST_OBJECT (audioinputcaps));
			audioinputcaps = NULL;
		}

		assert(source == NULL);
		assert(decoder == NULL);
	}
	else
	{
		if ( source )
		{
			gst_object_unref (GST_OBJECT (source));
			source = NULL;
		}

		if ( decoder )
		{
			gst_object_unref (GST_OBJECT (decoder));
			decoder = NULL;
		}

		assert( audiosource == NULL);
		assert( videosource == NULL);
		assert( videodecoder == NULL);
		assert( audiodecoder == NULL);
		assert( videoinputcaps == NULL);
		assert( audioinputcaps == NULL);
	}
}

bool
NetStreamGst::buildFLVPipeline(bool& video, bool& audio)
{
	boost::mutex::scoped_lock lock(_pipelineMutex);

	if ( ! buildFLVVideoPipeline(video) ) return false;
	if ( audio )
	{
		if ( ! buildFLVSoundPipeline(audio) ) return false;
	}

	return true;

}

#ifdef GST_HAS_MODERN_PBUTILS

static void
GstInstallPluginsResultCb (GstInstallPluginsReturn  result,
			   gpointer                 user_data)
{
  g_debug("JAU RESULTO MENDO");
}


static gboolean
NetStreamGst_install_missing_codecs(GList *missing_plugin_details)
{

  GstInstallPluginsReturn rv;
  int i,c;
  gchar **details = g_new0(gchar*, c+1);
  GstInstallPluginsContext *install_ctx = gst_install_plugins_context_new();

  c=g_list_length(missing_plugin_details);
  
  for(i=0; i < c; i++)
  {
    details[i] = (gchar*) g_list_nth_data(missing_plugin_details, i);
  }

  rv = gst_install_plugins_sync (details,
				 install_ctx);

  g_strfreev(details);

  switch(rv) {
  case GST_INSTALL_PLUGINS_SUCCESS:
    if(!gst_update_registry())
      g_warning("we failed to update gst registry for new codecs");
    else
      return true;
    break;
  case GST_INSTALL_PLUGINS_NOT_FOUND:
    g_debug("gst_install_plugins_sync -> GST_INSTALL_PLUGINS_NOT_FOUND");
    break;
  case GST_INSTALL_PLUGINS_ERROR:
    g_debug("gst_install_plugins_sync -> GST_INSTALL_PLUGINS_ERROR");
    break;
  case GST_INSTALL_PLUGINS_PARTIAL_SUCCESS:
    g_debug("gst_install_plugins_sync -> GST_INSTALL_PLUGINS_PARTIAL_SUCCESS");
    break;
  case GST_INSTALL_PLUGINS_USER_ABORT:
    g_debug("gst_install_plugins_sync -> GST_INSTALL_PLUGINS_USER_ABORT");
    break;
  case GST_INSTALL_PLUGINS_CRASHED:
    g_debug("gst_install_plugins_sync -> GST_INSTALL_PLUGINS_CRASHED");
    break;
  case GST_INSTALL_PLUGINS_INVALID:
    g_warning("gst_install_plugins_sync -> GST_INSTALL_PLUGINS_INVALID");
    break;
  default:
    g_warning("gst_install_plugins_sync -> UNEXPECTED RESULT (undocumented value)");
    break;				  
  };

  return false;
}

static GList*
NetStreamGst_append_missing_codec_to_details (GList *list,
					      GstElement *source,
					      const GstCaps* caps)
{
  GstMessage *missing_msg;
  missing_msg = gst_missing_decoder_message_new(source,
						caps);
  gchar* detail = gst_missing_plugin_message_get_installer_detail(missing_msg);  

  if(!detail)
  {
    g_warning("missing message details not found. No details added.");
    return list;
  } 

  return g_list_append(list, detail);
}

#endif // GST_HAS_MODERN_PBUTILS

bool
NetStreamGst::buildFLVVideoPipeline(bool &video)
{
#ifdef GNASH_DEBUG
	log_debug("Building FLV video decoding pipeline");
#endif

	FLVVideoInfo* videoInfo = m_parser->getVideoInfo();

	bool doVideo = video;

	GList *missing_plugin_details = NULL;
#ifdef GST_HAS_MODERN_PBUTILS
 retry:
#endif
	if (videoInfo) {
		doVideo = true;
		videosource = gst_element_factory_make ("fakesrc", NULL);
		if ( ! videosource )
		{
			log_error("Unable to create videosource 'fakesrc' element");
			return false;
		}
		
		// setup fake source
		g_object_set (G_OBJECT (videosource),
					"sizetype", 2, "can-activate-pull", FALSE, "signal-handoffs", TRUE, NULL);

		// Setup the callback
		if ( ! connectVideoHandoffSignal() )
		{
			log_error("Unable to connect the video 'handoff' signal handler");
			return false;
		}

		// Setup the input capsfilter
		videoinputcaps = gst_element_factory_make ("capsfilter", NULL);
		if ( ! videoinputcaps )
		{
			log_error("Unable to create videoinputcaps 'capsfilter' element");
			return false;
		}

		uint32_t fps = m_parser->videoFrameRate(); 

		GstCaps* videonincaps;
		if (videoInfo->codec == VIDEO_CODEC_H263) {
			videonincaps = gst_caps_new_simple ("video/x-flash-video",
				"width", G_TYPE_INT, videoInfo->width,
				"height", G_TYPE_INT, videoInfo->height,
				"framerate", GST_TYPE_FRACTION, fps, 1,
				"flvversion", G_TYPE_INT, 1,
				NULL);
			videodecoder = gst_element_factory_make ("ffdec_flv", NULL);
			if ( ! videodecoder )
			{
				log_error("Unable to create videodecoder 'ffdec_flv' element");

#ifdef GST_HAS_MODERN_PBUTILS
				missing_plugin_details = NetStreamGst_append_missing_codec_to_details
				  (missing_plugin_details,
				   videosource,
				   videonincaps);
#else // GST_HAS_MODERN_PBUTILS
				return false;
#endif // GST_HAS_MODERN_PBUTILS
			}

		} else if (videoInfo->codec == VIDEO_CODEC_VP6) {
			videonincaps = gst_caps_new_simple ("video/x-vp6-flash",
				"width", G_TYPE_INT, 320, // We don't yet have a size extract for this codec, so we guess...
				"height", G_TYPE_INT, 240,
				"framerate", GST_TYPE_FRACTION, fps, 1,
				NULL);
			videodecoder = gst_element_factory_make ("ffdec_vp6f", NULL);
			if ( ! videodecoder )
			{
				log_error("Unable to create videodecoder 'ffdec_vp6f' element");

#ifdef GST_HAS_MODERN_PBUTILS
				missing_plugin_details = NetStreamGst_append_missing_codec_to_details
				  (missing_plugin_details,
				   videosource,
				   videonincaps);
#else // GST_HAS_MODERN_PBUTILS
				return false;
#endif // GST_HAS_MODERN_PBUTILS
			}

		} else if (videoInfo->codec == VIDEO_CODEC_SCREENVIDEO) {
			videonincaps = gst_caps_new_simple ("video/x-flash-screen",
				"width", G_TYPE_INT, 320, // We don't yet have a size extract for this codec, so we guess...
				"height", G_TYPE_INT, 240,
				"framerate", GST_TYPE_FRACTION, fps, 1,
				NULL);
			videodecoder = gst_element_factory_make ("ffdec_flashsv", NULL);

			// Check if the element was correctly created
			if (!videodecoder) {
				log_error(_("A gstreamer flashvideo (ScreenVideo) decoder element could not be created! You probably need to install gst-ffmpeg."));

#ifdef GST_HAS_MODERN_PBUTILS
				missing_plugin_details = NetStreamGst_append_missing_codec_to_details
				  (missing_plugin_details,
				   videosource,
				   videonincaps);
#else // GST_HAS_MODERN_PBUTILS
				return false;
#endif // GST_HAS_MODERN_PBUTILS
			}

		} else {
			log_error(_("Unsupported video codec %d"), videoInfo->codec);
			return false;
		}

		if(g_list_length(missing_plugin_details) == 0)
		{
		  g_object_set (G_OBJECT (videoinputcaps), "caps", videonincaps, NULL);
		  gst_caps_unref (videonincaps);
		}
	}


#ifdef GST_HAS_MODERN_PBUTILS
	if(g_list_length(missing_plugin_details) == 0)
	{
	  g_debug("no missing plugins found");
	  video = doVideo;
	  return true;
	}

	g_debug("try to install missing plugins (count=%d)", g_list_length(missing_plugin_details));
	if(NetStreamGst_install_missing_codecs(missing_plugin_details))
	{
	  disconnectVideoHandoffSignal();
	  g_list_free(missing_plugin_details);
	  missing_plugin_details = NULL;
	  g_debug("gst_install_plugins_sync -> GST_INSTALL_PLUGINS_SUCCESS ... one more roundtrip");
	  goto retry;
	}
	g_list_free(missing_plugin_details);
	return false;
#else // GST_HAS_MODERN_PBUTILS
	return true;
#endif // GST_HAS_MODERN_PBUTILS
}

bool
NetStreamGst::buildFLVSoundPipeline(bool &sound)
{
	bool doSound = sound;

	FLVAudioInfo* audioInfo = m_parser->getAudioInfo();
	if (!audioInfo) doSound = false;

#ifdef GST_HAS_MODERN_PBUTILS
	GList *missing_plugin_details = NULL;
 retry:
#endif
	if (doSound) {

#ifdef GNASH_DEBUG
		log_debug("Building FLV video decoding pipeline");
#endif

		audiosource = gst_element_factory_make ("fakesrc", NULL);
		if ( ! audiosource )
		{
			log_error("Unable to create audiosource 'fakesrc' element");
			return false;
		}

		// setup fake source
		g_object_set (G_OBJECT (audiosource),
					"sizetype", 2, "can-activate-pull", FALSE, "signal-handoffs", TRUE, NULL);

		// Setup the callback
		if ( ! connectAudioHandoffSignal() )
		{
			log_error("Unable to connect the audio 'handoff' signal handler");
			// TODO: what to do in this case ?
		}


		if (audioInfo->codec == AUDIO_CODEC_MP3) { 

			audiodecoder = gst_element_factory_make ("mad", NULL);
			if ( ! audiodecoder )
			{
				audiodecoder = gst_element_factory_make ("flump3dec", NULL);
				// Check if the element was correctly created
				if (!audiodecoder)
				{
					log_error(_("A gstreamer mp3-decoder element could not be created! You probably need to install a mp3-decoder plugin like gstreamer0.10-mad or gstreamer0.10-fluendo-mp3."));
				}
			}


			// Set the info about the stream so that gstreamer knows what it is.
			audioinputcaps = gst_element_factory_make ("capsfilter", NULL);
			if (!audioinputcaps)
			{
				log_error("Unable to create audioinputcaps 'capsfilter' element");
				return false;
			}

			GstCaps* audioincaps = gst_caps_new_simple ("audio/mpeg",
				"mpegversion", G_TYPE_INT, 1,
				"layer", G_TYPE_INT, 3,
				"rate", G_TYPE_INT, audioInfo->sampleRate,
				"channels", G_TYPE_INT, audioInfo->stereo ? 2 : 1, NULL);

			if(!audiodecoder)
			{
#ifdef GST_HAS_MODERN_PBUTILS
			  missing_plugin_details = NetStreamGst_append_missing_codec_to_details
			    (missing_plugin_details,
			     audiosource,
			     audioincaps);

			  if(NetStreamGst_install_missing_codecs(missing_plugin_details))
			  {
			    disconnectAudioHandoffSignal();
			    g_list_free(missing_plugin_details);
			    missing_plugin_details = NULL;
			    g_debug("gst_install_plugins_sync -> GST_INSTALL_PLUGINS_SUCCESS ... one more roundtrip");
			    goto retry;
			  }

			  g_list_free(missing_plugin_details);
#endif // GST_HAS_MODERN_PBUTILS
			  return false;
			} 
			g_object_set (G_OBJECT (audioinputcaps), "caps", audioincaps, NULL);
			gst_caps_unref (audioincaps);
		} else {
			log_error(_("Unsupported audio codec %d"), audioInfo->codec);
			return false;
		}
	}

	sound = doSound;

	return true;
}

bool
NetStreamGst::buildPipeline()
{
#ifdef GNASH_DEBUG
	log_debug("Building non-FLV decoding pipeline");
#endif

	boost::mutex::scoped_lock lock(_pipelineMutex);

	// setup gnashnc source if we are not decoding FLV (our homegrown source element)
	source = gst_element_factory_make ("gnashsrc", NULL);
	if ( ! source )
	{
		log_error("Failed to create 'gnashrc' element");
		return false;
	}
	gnashsrc_callback* gc = new gnashsrc_callback; // TODO: who's going to delete this ?
	gc->read = NetStreamGst::readPacket;
	gc->seek = NetStreamGst::seekMedia;
	g_object_set (G_OBJECT (source), "data", this, "callbacks", gc, NULL);

	// setup the decoder with callback
	decoder = gst_element_factory_make ("decodebin", NULL);
	if (!decoder)
	{
		log_error("Unable to create decoder 'decodebin' element");
		return false;
	}
	g_signal_connect (decoder, "new-decoded-pad", G_CALLBACK (NetStreamGst::callback_newpad), this);

	return true;
}


void
NetStreamGst::startPlayback()
{
	// This should only happen if close() is called before this thread is ready
	if (!m_go) return;

	boost::intrusive_ptr<NetConnection> nc = _netCon;
	assert(nc);

	// Pass stuff from/to the NetConnection object.
	if ( !nc->openConnection(url) ) {
		setStatus(streamNotFound);
#ifdef GNASH_DEBUG
		log_debug(_("Gnash could not open movie: %s"), url.c_str());
#endif
		return;
	}

	inputPos = 0;

	uint8_t head[3];
	if (nc->read(head, 3) < 3) {
		setStatus(streamNotFound);
		return;
	}
	nc->seek(0);
	if (head[0] == 'F' && head[1] == 'L' && head[2] == 'V') { 
		m_isFLV = true;
		if (!m_parser.get()) {
			m_parser = nc->getConnectedParser(); 
			if (! m_parser.get() )
			{
				setStatus(streamNotFound);
				log_error(_("Gnash could not open FLV movie: %s"), url.c_str());
				return;
			}
		}

	}

	// setup the GnashNC plugin if we are not decoding FLV
	if (!m_isFLV) _gst_plugin_register_static (&gnash_plugin_desc);

	// setup the pipeline
	pipeline = gst_pipeline_new (NULL);

	// Check if the creation of the gstreamer pipeline was a succes
	if (!pipeline) {
		gnash::log_error(_("The gstreamer pipeline element could not be created"));
		return;
	}

	bool video = false;
	bool sound = false;

	// If sound is enabled we set it up
	if (get_sound_handler()) sound = true;
	
	// Setup the decoder and source
	// TODO: move the m_isFLV test into buildPipeline and just call that one...
	if (m_isFLV)
	{
		if ( ! buildFLVPipeline(video, sound) )
		{
			unrefElements();
			return;
		}
	}
	else
	{
		if (!buildPipeline())
		{
			unrefElements();
			return;
		}
	}


	if (sound) {
		// create an audio sink - use oss, alsa or...? make a commandline option?
		// we first try autodetect, then alsa, then oss, then esd, then...?
		// If the gstreamer adder ever gets fixed this should be connected to the
		// adder in the soundhandler.
#if !defined(__NetBSD__)
		audiosink = gst_element_factory_make ("autoaudiosink", NULL);
		if (!audiosink) audiosink = gst_element_factory_make ("alsasink", NULL);
		if (!audiosink) audiosink = gst_element_factory_make ("osssink", NULL);
#endif
		if (!audiosink) audiosink = gst_element_factory_make ("esdsink", NULL);

		if (!audiosink) {
			log_error(_("The gstreamer audiosink element could not be created"));
			unrefElements();
			return;
		}
		// setup the audio converter
		audioconv = gst_element_factory_make ("audioconvert", NULL);
		if (!audioconv) {
			log_error(_("The gstreamer audioconvert element could not be created"));
			unrefElements();
			return;
		}

		// setup the volume controller
		volume = gst_element_factory_make ("volume", NULL);
		if (!volume) {
			log_error(_("The gstreamer volume element could not be created"));
			unrefElements();
			return;
		}

	} else  {
		audiosink = gst_element_factory_make ("fakesink", NULL);
		if (!audiosink) {
			log_error(_("The gstreamer fakesink element could not be created"));
			unrefElements();
			return;
		}
	}

	if (video) {
		// setup the video colorspaceconverter converter
		colorspace = gst_element_factory_make ("ffmpegcolorspace", NULL);
		if (!colorspace)
		{
			log_error("Unable to create colorspace 'ffmpegcolorspace' element");
			unrefElements();
			return;
		}

		// Setup the capsfilter which demands either YUV or RGB videoframe format
		videocaps = gst_element_factory_make ("capsfilter", NULL);
		if (!videocaps)
		{
			log_error("Unable to create videocaps 'capsfilter' element");
			unrefElements();
			return;
		}

		GstCaps* videooutcaps;
		if (m_videoFrameFormat == render::YUV) {
			videooutcaps = gst_caps_new_simple ("video/x-raw-yuv", NULL);
		} else {
			videooutcaps = gst_caps_new_simple ("video/x-raw-rgb", NULL);
		}
		g_object_set (G_OBJECT (videocaps), "caps", videooutcaps, NULL);
		gst_caps_unref (videooutcaps);

		// Setup the videorate element which makes sure the frames are delivered on time.
		videorate = gst_element_factory_make ("videorate", NULL);
		if (!videorate)
		{
			log_error("Unable to create videorate 'videorate' element");
			unrefElements();
			return;
		}

		// setup the videosink with callback
		videosink = gst_element_factory_make ("fakesink", NULL);
		if (!videosink)
		{
			log_error("Unable to create videosink 'fakesink' element");
			unrefElements();
			return;
		}

		g_object_set (G_OBJECT (videosink), "signal-handoffs", TRUE, "sync", TRUE, NULL);
		// TODO: use connectVideoSincCallback()
		g_signal_connect (videosink, "handoff", G_CALLBACK (NetStreamGst::callback_output), this);
	}

	if (video && (!colorspace || !videocaps || !videorate || !videosink)) {
		log_error(_("Gstreamer element(s) for video movie handling could not be created, you probably need to install gstreamer0.10-base for ffmpegcolorspace and videorate support."));
		unrefElements();
		return;
	}

	// put it all in the pipeline and link the elements
	if (!m_isFLV) { 
		if (sound) gst_bin_add_many (GST_BIN (pipeline),audiosink, audioconv, volume, NULL);
		if (video) gst_bin_add_many (GST_BIN (pipeline), source, decoder, colorspace, 
					videosink, videorate, videocaps, NULL);

		if (video || sound) gst_element_link(source, decoder);
		if (video) gst_element_link_many(colorspace, videocaps, videorate, videosink, NULL);
		if (sound) gst_element_link_many(audioconv, volume, audiosink, NULL);

	} else {
		if (video) gst_bin_add_many (GST_BIN (pipeline), videosource, videoinputcaps, videodecoder, colorspace, videocaps, videorate, videosink, NULL);
		if (sound) gst_bin_add_many (GST_BIN (pipeline), audiosource, audioinputcaps, audiodecoder, audioconv, volume, audiosink, NULL);

		if (sound) gst_element_link_many(audiosource, audioinputcaps, audiodecoder, audioconv, volume, audiosink, NULL);
		if (video) gst_element_link_many(videosource, videoinputcaps, videodecoder, colorspace, videocaps, videorate, videosink, NULL);

	}

	// start playing	
	if (!m_isFLV)
	{
		if (video || sound)
		{
			// TODO: should call playPipeline() ?
			gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);
		}
	}
	else
	{
		if (video || sound)
		{
			log_msg("Pausing pipeline on startPlayback");
			if ( ! pausePipeline(true) )
			{
				log_error("Could not pause pipeline");
			}
		}
	}

	setStatus(playStart);
	return;
}

void
NetStreamGst::seek(uint32_t pos)
{
	if (!pipeline) {
		if (m_parser.get())  {
			m_parser->seek(pos);
			m_clock_offset = 0;
		}
		return;
	}

	if (m_isFLV) {
		assert(m_parser.get()); // why assumed here and not above ?
		uint32_t newpos = m_parser->seek(pos);
		GstClock* clock = GST_ELEMENT_CLOCK(pipeline);
		uint64_t currenttime = gst_clock_get_time (clock);
		gst_object_unref(clock);
		
		m_clock_offset = (currenttime / GST_MSECOND) - newpos;

	} else {
		if (!gst_element_seek (pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
			GST_SEEK_TYPE_SET, GST_MSECOND * pos,
			GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) {
			log_error("Gstreamer seek failed");
			setStatus(invalidTime);
			return;
		}
	}
	setStatus(seekNotify);
}

void
NetStreamGst::advance()
{
	// Check if we should start the playback when a certain amount is buffered
	// This can happen in 2 cases: 
	// 1) When playback has just started and we've been waiting for the buffer 
	//    to be filled (buffersize set by setBufferTime() and default is 100
	//    miliseconds).
	// 2) The buffer has be "starved" (not being filled as quickly as needed),
	//    and we then wait until the buffer contains some data (1 sec) again.
	if (m_isFLV && m_pause && m_go && m_start_onbuffer && m_parser.get() && m_parser->isTimeLoaded(m_bufferTime))
	{
		if ( ! playPipeline() )
		{
			log_error("Could not enable pipeline");
			return;
		}
	}

	// If we're out of data, but still not done loading, pause playback,
	// or stop if loading is complete
	if (m_pausePlayback)
	{
#ifdef GNASH_DEBUG
		log_debug("Playback paused (out of data?)");
#endif

		m_pausePlayback = false;
		if (_netCon->loadCompleted())
		{
#ifdef GNASH_DEBUG
			log_debug("Load completed, setting playStop status and shutting down pipeline");
#endif
			setStatus(playStop);

			// Drop gstreamer pipeline so callbacks are not called again
			if ( ! disablePipeline() )
			{
				// the state change failed
				log_error("Could not interrupt pipeline!");

				// @@ eh.. what to do then ?
			}

			m_go = false;
			m_clock_offset = 0;
		}
		else
		{
			//log_debug("Pausing pipeline on ::advance() [ loadCompleted returned false ]");
			if ( !pausePipeline(true) )
			{
				log_error("Could not pause pipeline");
			}

			int64_t pos;
			GstState current, pending;
			GstStateChangeReturn ret;
			GstFormat fmt = GST_FORMAT_TIME;

			ret = gst_element_get_state (GST_ELEMENT (pipeline), &current, &pending, 0);
			if (ret == GST_STATE_CHANGE_SUCCESS) {
				if (current != GST_STATE_NULL && gst_element_query_position (pipeline, &fmt, &pos)) {
					pos = pos / 1000000;
				} else {
					pos = 0;
				}
				// Buffer a second before continuing
				m_bufferTime = pos + 1000;
			} else {
				// the pipeline failed state change
				log_error("Pipeline failed to complete state change!");

				// @@ eh.. what to do then 
			}
		}
	}

	// Check if there are any new status messages, and if we should
	// pass them to a event handler
	processStatusNotifications();
}

int32_t
NetStreamGst::time()
{

	if (!pipeline) return 0;

	GstFormat fmt = GST_FORMAT_TIME;
	int64_t pos;
	GstStateChangeReturn ret;
	GstState current, pending;

	ret = gst_element_get_state (GST_ELEMENT (pipeline), &current, &pending, 0);

	if (current != GST_STATE_NULL && gst_element_query_position (pipeline, &fmt, &pos)) {
		pos = pos / 1000000;

		return pos - m_clock_offset;
	} else {
		return 0;
	}
}

// Gstreamer callback function
int 
NetStreamGst::readPacket(void* opaque, char* buf, int buf_size){

	NetStreamGst* ns = static_cast<NetStreamGst*>(opaque);

	boost::intrusive_ptr<NetConnection> nc = ns->_netCon;
	size_t ret = nc->read(static_cast<void*>(buf), buf_size);
	ns->inputPos += ret;

	return ret;

}

// Gstreamer callback function
int 
NetStreamGst::seekMedia(void *opaque, int offset, int whence){

	NetStreamGst* ns = static_cast<NetStreamGst*>(opaque);
	boost::intrusive_ptr<NetConnection> nc = ns->_netCon;

	bool ret;

	// Offset is absolute new position in the file
	if (whence == SEEK_SET) {
		ret = nc->seek(offset);
		if (!ret) return -1;
		ns->inputPos = offset;

	// New position is offset + old position
	} else if (whence == SEEK_CUR) {
		ret = nc->seek(ns->inputPos + offset);
		if (!ret) return -1;
		ns->inputPos = ns->inputPos + offset;

	// 	// New position is offset + end of file
	} else if (whence == SEEK_END) {
		// This is (most likely) a streamed file, so we can't seek to the end!
		// Instead we seek to 50.000 bytes... seems to work fine...
		ret = nc->seek(50000);
		ns->inputPos = 50000;
	}
	return ns->inputPos;
}

/*private*/
bool
NetStreamGst::disconnectVideoHandoffSignal()
{
	if (videosource && _handoffVideoSigHandler )
	{
#ifdef GNASH_DEBUG
		log_debug("Disconnecting video handoff signal %lu", _handoffVideoSigHandler);
#endif
		g_signal_handler_disconnect(videosource, _handoffVideoSigHandler);
		_handoffVideoSigHandler = 0;
	}

	// TODO: check return code from previous call !
	return true;
}

/*private*/
bool
NetStreamGst::disconnectAudioHandoffSignal()
{
	if ( audiosource && _handoffAudioSigHandler )
	{
#ifdef GNASH_DEBUG
		log_debug("Disconnecting audio handoff signal %lu", _handoffAudioSigHandler);
#endif
		g_signal_handler_disconnect(audiosource, _handoffAudioSigHandler);
		_handoffAudioSigHandler = 0;
	}

	// TODO: check return code from previous call !
	return true;
}

/*private*/
bool
NetStreamGst::connectVideoHandoffSignal()
{
#ifdef GNASH_DEBUG
	log_debug("Connecting video handoff signal");
#endif

	assert(_handoffVideoSigHandler == 0);

	_handoffVideoSigHandler = g_signal_connect (videosource, "handoff",
			G_CALLBACK (NetStreamGst::video_callback_handoff), this);
#ifdef GNASH_DEBUG
	log_debug("New _handoffVideoSigHandler id : %lu", _handoffVideoSigHandler);
#endif

	assert(_handoffVideoSigHandler != 0);

	// TODO: check return code from previous call !
	return true;
}

/*private*/
bool
NetStreamGst::connectAudioHandoffSignal()
{
#ifdef GNASH_DEBUG
	log_debug("Connecting audio handoff signal");
#endif

	assert(_handoffAudioSigHandler == 0);

	_handoffAudioSigHandler = g_signal_connect (audiosource, "handoff",
			G_CALLBACK (NetStreamGst::audio_callback_handoff), this);

#ifdef GNASH_DEBUG
	log_debug("New _handoffAudioSigHandler id : %lu", _handoffAudioSigHandler);
#endif

	assert(_handoffAudioSigHandler != 0);

	// TODO: check return code from previous call !
	return true;
}

/*private*/
bool
NetStreamGst::disablePipeline()
{
	boost::mutex::scoped_lock lock(_pipelineMutex);

	// Disconnect the handoff handler
	// TODO: VERIFY THE SIGNAL WILL BE RESTORED WHEN NEEDED !!
	if ( videosource ) disconnectVideoHandoffSignal();
	if ( audiosource ) disconnectAudioHandoffSignal();

	// Drop gstreamer pipeline so callbacks are not called again
	GstStateChangeReturn ret =  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);
	if ( ret == GST_STATE_CHANGE_FAILURE )
	{
		// the state change failed
		log_error("Could not interrupt pipeline!");
		return false;

		// @@ eh.. what to do then ?
	}
	else if ( ret == GST_STATE_CHANGE_SUCCESS )
	{
		// the state change succeeded
#ifdef GNASH_DEBUG
		log_debug("State change to NULL successful");
#endif

		// just make sure
		GstState current, pending;
		ret = gst_element_get_state (GST_ELEMENT (pipeline), &current, &pending, 0);
		if (current != GST_STATE_NULL )
		{
			log_error("State change to NULL NOT confirmed !");
			return false;
		}
	}
	else if ( ret == GST_STATE_CHANGE_ASYNC )
	{
		// The element will perform the remainder of the state change
		// asynchronously in another thread
		// We'll wait for it...

#ifdef GNASH_DEBUG
		log_debug("State change to NULL will be asynchronous.. waiting for it");
#endif

		GstState current, pending;
		do {
			ret = gst_element_get_state (GST_ELEMENT (pipeline), &current, &pending, GST_SECOND*1); 

#ifdef GNASH_DEBUG
			log_debug(" NULL state change still not completed after X seconds");
#endif

		} while ( ret == GST_STATE_CHANGE_ASYNC && current != GST_STATE_NULL );

		if ( ret == GST_STATE_CHANGE_SUCCESS )
		{
			assert ( current == GST_STATE_NULL );
#ifdef GNASH_DEBUG
			log_debug(" Async NULL completed successfully");
#endif
		}
		else if ( ret == GST_STATE_CHANGE_FAILURE )
		{
			assert ( current != GST_STATE_NULL );
#ifdef GNASH_DEBUG
			log_debug(" Async NULL completed failing.");
#endif
			return false;
		}
		else abort();


	}
	else if ( ret == GST_STATE_CHANGE_NO_PREROLL )
	{
		// the state change succeeded but the element
		// cannot produce data in PAUSED.
		// This typically happens with live sources.
#ifdef GNASH_DEBUG
		log_debug("State change succeeded but the element cannot produce data in PAUSED");
#endif

		// @@ what to do in this case ?
	}
	else
	{
		log_error("Unknown return code from gst_element_set_state");
		return false;
	}

	return true;

}

/*private*/
bool
NetStreamGst::playPipeline()
{
	boost::mutex::scoped_lock lock(_pipelineMutex);

#ifdef GNASH_DEBUG
	log_debug("Setting status to bufferFull and enabling pipeline");
#endif

	if ( videosource && ! _handoffVideoSigHandler )
	{
		connectVideoHandoffSignal();
	}

	if ( audiosource && ! _handoffAudioSigHandler )
	{
		connectAudioHandoffSignal();
	}

	if (!m_go) { 
		setStatus(playStart);
		m_go = true;
	}
	m_pause = false;
	m_start_onbuffer = false;


	// Set pipeline to PLAYING state
	GstStateChangeReturn ret =  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);
	if ( ret == GST_STATE_CHANGE_FAILURE )
	{
		// the state change failed
		log_error("Could not set pipeline state to PLAYING!");
		return false;

		// @@ eh.. what to do then ?
	}
	else if ( ret == GST_STATE_CHANGE_SUCCESS )
	{
		// the state change succeeded
#ifdef GNASH_DEBUG
		log_debug("State change to PLAYING successful");
#endif

		// just make sure
		GstState current, pending;
		ret = gst_element_get_state (GST_ELEMENT (pipeline), &current, &pending, 0);
		if (current != GST_STATE_PLAYING )
		{
			log_error("State change to PLAYING NOT confirmed !");
			return false;
		}
	}
	else if ( ret == GST_STATE_CHANGE_ASYNC )
	{
		// The element will perform the remainder of the state change
		// asynchronously in another thread
		// We'll wait for it...

#ifdef GNASH_DEBUG
		log_debug("State change to play will be asynchronous.. waiting for it");
#endif

		GstState current, pending;
		do {
			ret = gst_element_get_state (GST_ELEMENT (pipeline), &current, &pending, GST_SECOND*1); 

#ifdef GNASH_DEBUG
			log_debug(" Play still not completed after X seconds");
#endif

		} while ( ret == GST_STATE_CHANGE_ASYNC && current != GST_STATE_PLAYING );

		if ( ret == GST_STATE_CHANGE_SUCCESS )
		{
			assert ( current == GST_STATE_PLAYING );
#ifdef GNASH_DEBUG
			log_debug(" Async play completed successfully");
#endif
		}
		else if ( ret == GST_STATE_CHANGE_FAILURE )
		{
			assert ( current != GST_STATE_PLAYING );
#ifdef GNASH_DEBUG
			log_debug(" Async play completed failing.");
#endif
			return false;
		}
		else abort();

	}
	else if ( ret == GST_STATE_CHANGE_NO_PREROLL )
	{
		// the state change succeeded but the element
		// cannot produce data in PAUSED.
		// This typically happens with live sources.
#ifdef GNASH_DEBUG
		log_debug("State change succeeded but the element cannot produce data in PAUSED");
#endif

		// @@ what to do in this case ?
	}
	else
	{
		log_error("Unknown return code from gst_element_set_state");
		return false;
	}

	return true;

}

/*private*/
bool
NetStreamGst::pausePipeline(bool startOnBuffer)
{
	boost::mutex::scoped_lock lock(_pipelineMutex);

#ifdef GNASH_DEBUG
	log_debug("Setting pipeline state to PAUSE");
#endif

	if ( ! m_go )
	{
#ifdef GNASH_DEBUG
		log_debug("Won't set the pipeline to PAUSE state if m_go is false");
#endif
		return false;
	}


	if ( videosource && ! _handoffVideoSigHandler )
	{
		connectVideoHandoffSignal();
	}

	if ( audiosource && ! _handoffAudioSigHandler )
	{
		connectAudioHandoffSignal();
	}

	m_pause = true;
	m_start_onbuffer = startOnBuffer;

	// Set pipeline to PAUSE state
	GstStateChangeReturn ret =  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PAUSED);
	if ( ret == GST_STATE_CHANGE_FAILURE )
	{
		// the state change failed
		log_error("Could not interrupt pipeline!");
		return false;
	}
	else if ( ret == GST_STATE_CHANGE_SUCCESS )
	{
		// the state change succeeded
#ifdef GNASH_DEBUG
		log_debug("State change to PAUSE successful");
#endif

		// just make sure
		GstState current, pending;
		ret = gst_element_get_state (GST_ELEMENT (pipeline), &current, &pending, 0);
		if (current != GST_STATE_PAUSED )
		{
			log_error("State change to PLAYING NOT confirmed !");
			return false;
		}
	}
	else if ( ret == GST_STATE_CHANGE_ASYNC )
	{
		// The element will perform the remainder of the state change
		// asynchronously in another thread
		// We'll wait for it...

#ifdef GNASH_DEBUG
		log_debug("State change to paused will be asynchronous.. waiting for it");
#endif

		GstState current, pending;
		do {
			ret = gst_element_get_state (GST_ELEMENT (pipeline), &current, &pending, GST_SECOND*1); 

#ifdef GNASH_DEBUG
			log_debug(" Pause still not completed after X seconds");
#endif

		} while ( ret == GST_STATE_CHANGE_ASYNC && current != GST_STATE_PAUSED );

		if ( ret == GST_STATE_CHANGE_SUCCESS )
		{
			assert ( current == GST_STATE_PAUSED );
#ifdef GNASH_DEBUG
			log_debug(" Async pause completed successfully");
#endif
		}
		else if ( ret == GST_STATE_CHANGE_FAILURE )
		{
			assert ( current != GST_STATE_PAUSED );
#ifdef GNASH_DEBUG
			log_debug(" Async pause completed failing.");
#endif
			return false;
		}
		else abort();

	}
	else if ( ret == GST_STATE_CHANGE_NO_PREROLL )
	{
		// the state change succeeded but the element
		// cannot produce data in PAUSED.
		// This typically happens with live sources.
#ifdef GNASH_DEBUG
		log_debug("State change succeeded but the element cannot produce data in PAUSED");
#endif

		// @@ what to do in this case ?
	}
	else
	{
		log_error("Unknown return code from gst_element_set_state");
		return false;
	}

	return true;

}


} // gnash namespcae

#endif // SOUND_GST
