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

/* $Id: NetStreamGst.cpp,v 1.24 2007/04/18 11:00:30 jgilmore Exp $ */

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
#include "action.h"

#include "gstgnashsrc.h"

#include "URL.h"

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

	m_go(false),
	m_imageframe(NULL),
	startThread(NULL),
	m_pause(false),
	inputPos(0),
	videowidth(0),
	videoheight(0),
	m_newFrameReady(false),
	m_parser(NULL),
	m_env(NULL),
	m_pausePlayback(false),
	m_start_onbuffer(false)
{
}

NetStreamGst::~NetStreamGst()
{
	close();
}

void NetStreamGst::set_status(const char* status)
{
	std::string std_status = status;
	if (!(m_status_messages.size() > 0 && m_status_messages.back().compare(std_status) == 0)) {
		m_status_messages.push_back(std_status);
		m_statusChanged = true;
	}
}

void NetStreamGst::setEnvironment(as_environment* env)
{
	m_env = env;
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
	if (pipeline) {
		if (m_pause) gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PAUSED);
		else  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);
	}
}

void NetStreamGst::close()
{
	if (m_go)
	{
		m_go = false;
		startThread->join();
		delete startThread;
	}

	gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);
	gst_object_unref (GST_OBJECT (pipeline));

	if (m_imageframe) delete m_imageframe;

}


int
NetStreamGst::play(const char* c_url)
{

	// Is it already playing ?
	if (m_go)
	{
		if (m_pause) {
			m_pause = false;
			gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);
		}
		return 0;
	}

	// Does it have an associated NetConnection?
	if ( ! _netCon )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("No NetConnection associated with this NetStream, won't play"));
		);
		return 0;
	}

	url += c_url;
	// Remove any "mp3:" prefix. Maybe should use this to mark as audio-only
	if (url.compare(0, 4, std::string("mp3:")) == 0) {
		url = url.substr(4);
	}
	m_go = true;

	// To avoid blocking while connecting, we use a thread.
	startThread = new boost::thread(boost::bind(NetStreamGst::startPlayback, this));
	return 0;
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
		int videoFrameFormat = gnash::render::videoFrameFormat();
		if (videoFrameFormat == render::YUV) {
			ns->m_imageframe = new image::yuv(width, height);
		} else if (videoFrameFormat == render::RGB) {
			ns->m_imageframe = new image::rgb(width, height);
		}
	}

	if (ns->m_imageframe) {
//		ns->m_imageframe->update(GST_BUFFER_DATA(buffer));
		if (gnash::render::videoFrameFormat() == render::YUV) {
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
		ns->set_status("NetStream.Buffer.Empty");
		ns->m_pausePlayback = true;
		return;
	}

//	if (GST_BUFFER_DATA(buffer)) delete [] GST_BUFFER_DATA(buffer);
	GST_BUFFER_SIZE(buffer) = frame->dataSize;
	GST_BUFFER_DATA(buffer) = frame->data;
	GST_BUFFER_TIMESTAMP(buffer) = frame->timestamp * 1000000;
	delete frame;
	return;

}

// The callback function which refills the video buffer with data
// Only used when playing FLV
void NetStreamGst::video_callback_handoff (GstElement * /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data)
{
	NetStreamGst* ns = static_cast<NetStreamGst*>(user_data);

	FLVFrame* frame = ns->m_parser->nextVideoFrame();
	if (!frame) {
		ns->set_status("NetStream.Buffer.Empty");
		ns->m_pausePlayback = true;
		return;
	}

//	if (GST_BUFFER_DATA(buffer)) delete [] GST_BUFFER_DATA(buffer);
	GST_BUFFER_SIZE(buffer) = frame->dataSize;
	GST_BUFFER_DATA(buffer) = frame->data;
	GST_BUFFER_TIMESTAMP(buffer) = frame->timestamp * 1000000;
	delete frame;
	return;
}

void
NetStreamGst::startPlayback(NetStreamGst* ns)
{
	boost::intrusive_ptr<NetConnection> nc = ns->_netCon;
	assert(nc);

	// Pass stuff from/to the NetConnection object.
	assert(ns);
	if ( !nc->openConnection(ns->url.c_str(), ns) ) {
		ns->set_status("NetStream.Play.StreamNotFound");
		log_warning(_("Gnash could not open movie: %s"), ns->url.c_str());
		return;
	}

	ns->inputPos = 0;

	uint8_t head[3];
	if (nc->read(head, 3) < 3) {
		ns->set_status("NetStream.Buffer.StreamNotFound");
		return;
	}
	nc->seek(0);
	if (head[0] == 'F'|| head[1] == 'L' || head[2] == 'V') { 
		ns->m_isFLV = true;
		ns->m_parser = new FLVParser();
		if (!nc->connectParser(ns->m_parser)) {
			ns->set_status("NetStream.Play.StreamNotFound");
			log_error(_("Gnash could not open movie: %s"), ns->url.c_str());
			return;
			
		}
	}

	// init GStreamer
	gst_init (NULL, NULL);

	// setup the GnashNC plugin if we are not decoding FLV
	if (!ns->m_isFLV) _gst_plugin_register_static (&gnash_plugin_desc);

	// setup the pipeline
	ns->pipeline = gst_pipeline_new (NULL);

	// Check if the creation of the gstreamer pipeline and audiosink was a succes
	if (!ns->pipeline) {
		gnash::log_error(_("The gstreamer pipeline element could not be created"));
		return;
	}

	// If sound is enabled we set it up
	sound_handler* sound = get_sound_handler();
	if (sound) {
		// create an audio sink - use oss, alsa or...? make a commandline option?
		// we first try autodetect, then alsa, then oss, then esd, then...?
		// If the gstreamer adder ever gets fixed this should be connected to the
		// adder in the soundhandler.
#if !defined(__NetBSD__)
		ns->audiosink = gst_element_factory_make ("autoaudiosink", NULL);
		if (!ns->audiosink) ns->audiosink = gst_element_factory_make ("alsasink", NULL);
		if (!ns->audiosink) ns->audiosink = gst_element_factory_make ("osssink", NULL);
#endif
		if (!ns->audiosink) ns->audiosink = gst_element_factory_make ("esdsink", NULL);

		if (!ns->audiosink) {
			log_error(_("The gstreamer audiosink element could not be created"));
			return;
		}
	} else {
		ns->audiosink = gst_element_factory_make ("fakesink", NULL);
	}

	// setup the audio converter
	ns->audioconv = gst_element_factory_make ("audioconvert", NULL);

	// setup the volume controller
	ns->volume = gst_element_factory_make ("volume", NULL);

	if (!ns->audioconv || !ns->volume) {
		log_error(_("Gstreamer audio element(s) for movie handling could not be created"));
		return;
	}

	// setup gnashnc source if we are not decoding FLV (our homegrown source element)
	if (!ns->m_isFLV) {
		ns->source = gst_element_factory_make ("gnashsrc", NULL);
		gnashsrc_callback* gc = new gnashsrc_callback;
		gc->read = NetStreamGst::readPacket;
		gc->seek = NetStreamGst::seekMedia;
		g_object_set (G_OBJECT (ns->source), "data", ns, "callbacks", gc, NULL);
	} else {

		FLVVideoInfo* videoInfo = ns->m_parser->getVideoInfo();
		FLVAudioInfo* audioInfo = ns->m_parser->getAudioInfo();

		ns->audiosource = gst_element_factory_make ("fakesrc", NULL);
		ns->videosource = gst_element_factory_make ("fakesrc", NULL);
		
		// setup fake sources
		g_object_set (G_OBJECT (ns->audiosource),
					"sizetype", 2, "can-activate-pull", FALSE, "signal-handoffs", TRUE, NULL);
		g_object_set (G_OBJECT (ns->videosource),
					"sizetype", 2, "can-activate-pull", FALSE, "signal-handoffs", TRUE, NULL);

		// Setup the callbacks
		g_signal_connect (ns->audiosource, "handoff", G_CALLBACK (NetStreamGst::audio_callback_handoff), ns);
		g_signal_connect (ns->videosource, "handoff", G_CALLBACK (NetStreamGst::video_callback_handoff), ns);

		// Setup the input capsfilter
		ns->videoinputcaps = gst_element_factory_make ("capsfilter", NULL);
		uint32_t fps = ns->m_parser->videoFrameRate(); 

		GstCaps* videonincaps;
		if (videoInfo->codec == VIDEO_CODEC_H263) {
			videonincaps = gst_caps_new_simple ("video/x-flash-video",
				"width", G_TYPE_INT, videoInfo->width,
				"height", G_TYPE_INT, videoInfo->height,
				"framerate", GST_TYPE_FRACTION, fps, 1,
				"flvversion", G_TYPE_INT, 1,
				NULL);
			ns->videodecoder = gst_element_factory_make ("ffdec_flv", NULL);

			// Check if the element was correctly created
			if (!ns->videodecoder) {
				log_error(_("A gstreamer flashvideo (h.263) decoder element could not be created.  You probably need to install gst-ffmpeg."));
				return;
			}

		} else if (videoInfo->codec == VIDEO_CODEC_VP6) {
			videonincaps = gst_caps_new_simple ("video/x-vp6-flash",
				"width", G_TYPE_INT, 320, // We don't yet have a size extract for this codec, so we guess...
				"height", G_TYPE_INT, 240,
				"framerate", GST_TYPE_FRACTION, fps, 1,
				NULL);
			ns->videodecoder = gst_element_factory_make ("ffdec_vp6f", NULL);

			// Check if the element was correctly created
			if (!ns->videodecoder) {
				log_error(_("A gstreamer flashvideo (VP6) decoder element could not be created! You probably need to install gst-ffmpeg."));
				return;
			}

		} else if (videoInfo->codec == VIDEO_CODEC_SCREENVIDEO) {
			videonincaps = gst_caps_new_simple ("video/x-flash-screen",
				"width", G_TYPE_INT, 320, // We don't yet have a size extract for this codec, so we guess...
				"height", G_TYPE_INT, 240,
				"framerate", GST_TYPE_FRACTION, fps, 1,
				NULL);
			ns->videodecoder = gst_element_factory_make ("ffdec_flashsv", NULL);

			// Check if the element was correctly created
			if (!ns->videodecoder) {
				log_error(_("A gstreamer flashvideo (ScreenVideo) decoder element could not be created! You probably need to install gst-ffmpeg."));
				return;
			}

		} else {
			log_error(_("Unsupported video codec %d"),
				  videoInfo->codec);
			return;
		}

		g_object_set (G_OBJECT (ns->videoinputcaps), "caps", videonincaps, NULL);
		gst_caps_unref (videonincaps);

		if (audioInfo->codec == AUDIO_CODEC_MP3) { 

			ns->audiodecoder = gst_element_factory_make ("mad", NULL);
			if (ns->audiodecoder == NULL) {
				ns->audiodecoder = gst_element_factory_make ("flump3dec", NULL);
				if (ns->audiodecoder != NULL && !gst_default_registry_check_feature_version("flump3dec", 0, 10, 4))
				{
					static bool warned = false;
					if ( ! warned )
					{
					log_error(_("This version of Fluendo's mp3 plugin does not support flash streaming sounds, please upgrade to version 0.10.4 or higher."));
					warned=true;
					}
				}
			}
			// Check if the element was correctly created
			if (!ns->audiodecoder) {
				log_error(_("A gstreamer mp3-decoder element could not be created! You probably need to install a mp3-decoder plugin like gstreamer0.10-mad or gstreamer0.10-fluendo-mp3."));
				return;
			}

			// Set the info about the stream so that gstreamer knows what it is.
			ns->audioinputcaps = gst_element_factory_make ("capsfilter", NULL);
			GstCaps* audioincaps = gst_caps_new_simple ("audio/mpeg",
				"mpegversion", G_TYPE_INT, 1,
				"layer", G_TYPE_INT, 3,
				"rate", G_TYPE_INT, audioInfo->sampleRate,
				"channels", G_TYPE_INT, audioInfo->stereo ? 2 : 1, NULL);
			g_object_set (G_OBJECT (ns->audioinputcaps), "caps", audioincaps, NULL);
			gst_caps_unref (audioincaps);
		} else {
			log_error(_("Unsupported audio codec %d"),
				  audioInfo->codec);
			return;
		}
	}

	// setup the decoder with callback, but only if we are not decoding a FLV
	if (!ns->m_isFLV) {
		ns->decoder = gst_element_factory_make ("decodebin", NULL);
		g_signal_connect (ns->decoder, "new-decoded-pad", G_CALLBACK (NetStreamGst::callback_newpad), ns);
	}

	// setup the video colorspaceconverter converter
	ns->colorspace = gst_element_factory_make ("ffmpegcolorspace", NULL);

	// Setup the capsfilter which demands either YUV or RGB videoframe format
	ns->videocaps = gst_element_factory_make ("capsfilter", NULL);
	GstCaps* videooutcaps;
	if (gnash::render::videoFrameFormat() == render::YUV) {
		videooutcaps = gst_caps_new_simple ("video/x-raw-yuv", NULL);
	} else {
		videooutcaps = gst_caps_new_simple ("video/x-raw-rgb", NULL);
	}
	g_object_set (G_OBJECT (ns->videocaps), "caps", videooutcaps, NULL);
	gst_caps_unref (videooutcaps);

	// Setup the videorate element which makes sure the frames are delivered on time.
	ns->videorate = gst_element_factory_make ("videorate", NULL);

	// setup the videosink with callback
	ns->videosink = gst_element_factory_make ("fakesink", NULL);
	g_object_set (G_OBJECT (ns->videosink), "signal-handoffs", TRUE, "sync", TRUE, NULL);
	g_signal_connect (ns->videosink, "handoff", G_CALLBACK (NetStreamGst::callback_output), ns);

	if (ns->m_isFLV) {
		if (!ns->videosource || !ns->audiosource || !ns->videoinputcaps ||  !ns->audioinputcaps) {
			log_error(_("Gstreamer source element(s) for video movie handling could not be created, you probably need to install gstreamer0.10-core for fakesrc and capsfilter support."));
			return;
		}
	} else {
		if (!ns->decoder || !ns->source) {
			log_error(_("Gstreamer element(s) for video movie handling could not be created, you probably need to install gstreamer0.10-base for decodebin support."));
			return;
		}
	}

	if (!ns->colorspace || !ns->videocaps || !ns->videorate || !ns->videosink) {
		log_error(_("Gstreamer element(s) for video movie handling could not be created, you probably need to install gstreamer0.10-base for ffmpegcolorspace and videorate support."));
		return;
	}

	// put it all in the pipeline and link the elements
	if (!ns->m_isFLV) { 
		if (sound) gst_bin_add_many (GST_BIN (ns->pipeline),ns->audiosink, ns->audioconv, NULL);
		gst_bin_add_many (GST_BIN (ns->pipeline), ns->source, ns->decoder, ns->colorspace, 
			ns->videosink, ns->videorate, ns->videocaps, ns->volume, NULL);

		gst_element_link(ns->source, ns->decoder);
		gst_element_link_many(ns->colorspace, ns->videocaps, ns->videorate, ns->videosink, NULL);
		if (sound) gst_element_link_many(ns->audioconv, ns->volume, ns->audiosink, NULL);

	} else {
		gst_bin_add_many (GST_BIN (ns->pipeline), ns->videosource, ns->videoinputcaps, ns->videodecoder, ns->colorspace, ns->videocaps, ns->videorate, ns->videosink, NULL);
		if (sound) gst_bin_add_many (GST_BIN (ns->pipeline), ns->audiosource, ns->audioinputcaps, ns->audiodecoder, ns->audioconv, ns->volume, ns->audiosink, NULL);

		if (sound) gst_element_link_many(ns->audiosource, ns->audioinputcaps, ns->audiodecoder, ns->audioconv, ns->volume, ns->audiosink, NULL);
		gst_element_link_many(ns->videosource, ns->videoinputcaps, ns->videodecoder, ns->colorspace, ns->videocaps, ns->videorate, ns->videosink, NULL);

	}

	// start playing	
	if (!ns->m_isFLV) {
		gst_element_set_state (GST_ELEMENT (ns->pipeline), GST_STATE_PLAYING);
	} else {
		gst_element_set_state (GST_ELEMENT (ns->pipeline), GST_STATE_PAUSED);
		ns->m_pause = true;
		ns->m_start_onbuffer = true;
	}

	ns->set_status("NetStream.Play.Start");
	return;
}

image::image_base* NetStreamGst::get_video()
{
	boost::mutex::scoped_lock lock(image_mutex);

	if (!m_imageframe) return NULL;

	image::image_base* ret_image;
	int videoFrameFormat = gnash::render::videoFrameFormat();
	if (videoFrameFormat == render::YUV) {
		ret_image = new image::yuv(m_imageframe->m_width, m_imageframe->m_height);
	} else if (videoFrameFormat == render::RGB) {
		ret_image = new image::rgb(m_imageframe->m_width, m_imageframe->m_height);
	} else {
		return NULL;
	}

	ret_image->update(m_imageframe->m_data);
	return ret_image;
}

void
NetStreamGst::seek(double pos)
{

	if (!pipeline) return;

	if (m_isFLV) {
		/*uint32_t newpos =*/ m_parser->seek(static_cast<uint32_t>(pos*1000))/1000;
		/*if (!gst_element_seek (pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
			GST_SEEK_TYPE_SET, GST_SECOND * static_cast<long>(newpos),
			GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) {
			log_error("Gstreamer seek failed");
			set_status("NetStream.Seek.InvalidTime");
			return;
		}*/
	} else {
		if (!gst_element_seek (pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
			GST_SEEK_TYPE_SET, GST_SECOND * static_cast<long>(pos),
			GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) {
			log_error("Gstreamer seek failed");
			set_status("NetStream.Seek.InvalidTime");
			return;
		}
	}
	set_status("NetStream.Seek.Notify");
}

void
NetStreamGst::setBufferTime(double time)
{
	// The argument is in seconds, but we store in milliseconds
    m_bufferTime = static_cast<uint32_t>(time*1000);
}

void
NetStreamGst::advance()
{
	// Check if we should start the playback when a certain amount is buffered
	if (m_isFLV && m_pause && m_go && m_start_onbuffer && m_parser && m_parser->isTimeLoaded(m_bufferTime)) {
		set_status("NetStream.Buffer.Full");
		m_pause = false;
		gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);
	}

	// If we're out of data, but still not done loading, pause playback,
	// or stop if loading is complete
	if (m_pausePlayback) {
		m_pausePlayback = false;

		if (_netCon->loadCompleted()) {
			set_status("NetStream.Play.Stop");
			gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);
			m_go = false;
		} else {
			gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PAUSED);
			GstFormat fmt = GST_FORMAT_TIME;
			int64_t pos;
			GstStateChangeReturn ret;
			GstState current, pending;

			ret = gst_element_get_state (GST_ELEMENT (pipeline), &current, &pending, 0);

			if (current != GST_STATE_NULL && gst_element_query_position (pipeline, &fmt, &pos)) {
				pos = pos / 1000000;
			} else {
				pos = 0;
			}
			// Buffer a second before continuing
			m_bufferTime = pos + 1000;
			m_start_onbuffer = true;
			m_pause = true;
		}
	}

	// Check if there are any new status messages, and if we should
	// pass them to a event handler
	as_value status;
	if (m_statusChanged && get_member(std::string("onStatus"), &status) && status.is_function()) {

		int size = m_status_messages.size();
		for (int i = 0; i < size; ++i) {
			boost::intrusive_ptr<as_object> o = new as_object();
			o->init_member(std::string("code"), as_value(m_status_messages[i]), 1);

			if (m_status_messages[i].find("StreamNotFound") == string::npos && m_status_messages[i].find("InvalidTime") == string::npos) {
				o->init_member(std::string("level"), as_value("status"), as_prop_flags::dontDelete|as_prop_flags::dontEnum);
			} else {
				o->init_member(std::string("level"), as_value("error"), as_prop_flags::dontDelete|as_prop_flags::dontEnum);
			}
			m_env->push_val(as_value(o.get()));

			call_method(status, m_env, this, 1, m_env->get_top_index() );


		}
		m_status_messages.clear();
		m_statusChanged = false;
	} else if (m_statusChanged) {
		m_status_messages.clear();
		m_statusChanged = false;
	}
}

int64_t
NetStreamGst::time()
{

	if (!pipeline) return 0;

	GstFormat fmt = GST_FORMAT_TIME;
	int64_t pos;
	GstStateChangeReturn ret;
	GstState current, pending;

	ret = gst_element_get_state (GST_ELEMENT (pipeline), &current, &pending, 0);

	if (current != GST_STATE_NULL && gst_element_query_position (pipeline, &fmt, &pos)) {
		pos = pos / 1000000000;

		return pos;
	} else {
		return 0;
	}
}

long
NetStreamGst::bytesLoaded()
{
	return _netCon->getBytesLoaded();
}

long
NetStreamGst::bytesTotal()
{
	return _netCon->getBytesTotal();
}

bool
NetStreamGst::newFrameReady()
{
	if (m_newFrameReady) {
		m_newFrameReady = false;
		return true;
	} else {
		return false;
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

} // gnash namespcae

#endif // SOUND_GST
