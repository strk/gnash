// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/* $id$ */

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
	source(NULL),
	decoder(NULL),
	volume(NULL),
	colorspace(NULL),
	videorate(NULL),
	videocaps(NULL),
	videoflip(NULL),
	audioconv(NULL),
	m_go(false),
	m_imageframe(NULL),
	startThread(NULL),
	m_pause(false),
	inputPos(0),
	videowidth(0),
	videoheight(0)

{
}

NetStreamGst::~NetStreamGst()
{
	close();
}

// called from avstreamer thread
void NetStreamGst::set_status(const char* /*code*/)
{
	//m_netstream_object->init_member("onStatus_Code", code);
	//push_video_event(this);
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
	if (m_pause) gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PAUSED);
	else  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);
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

	// Does it have an associated NetConnectoin ?
	if ( ! _netCon )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("No NetConnection associated with this NetStream, won't play");
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
		}

	}

}


void
NetStreamGst::startPlayback(NetStreamGst* ns)
{
	NetConnection* nc = ns->_netCon;
	assert(nc);

	// Pass stuff from/to the NetConnection object.
	assert(ns);
	if ( !nc->openConnection(ns->url.c_str(), ns) ) {
		log_warning("Gnash could not open movie url: %s", ns->url.c_str());
		return;
	}

	ns->inputPos = 0;

	// init GStreamer
	gst_init (NULL, NULL);

	// setup the GnashNC plugin
	_gst_plugin_register_static (&gnash_plugin_desc);

	// setup the pipeline
	ns->pipeline = gst_pipeline_new (NULL);

	// create an audio sink - use oss, alsa or...? make a commandline option?
	// we first try atudetect, then alsa, then oss, then esd, then...?
	// If the gstreamer adder ever gets fixed this should be connected to the
	// adder in the soundhandler.
#if !defined(__NetBSD__)
	ns->audiosink = gst_element_factory_make ("autoaudiosink", NULL);
	if (!ns->audiosink) ns->audiosink = gst_element_factory_make ("alsasink", NULL);
	if (!ns->audiosink) ns->audiosink = gst_element_factory_make ("osssink", NULL);
#endif
	if (!ns->audiosink) ns->audiosink = gst_element_factory_make ("esdsink", NULL);

	// Check if the creation of the gstreamer pipeline and audiosink was a succes
	if (!ns->pipeline) {
		gnash::log_error("The gstreamer pipeline element could not be created\n");
		return;
	}
	if (!ns->audiosink) {
		gnash::log_error("The gstreamer audiosink element could not be created\n");
		return;
	}

	// setup gnashnc source (our homegrown source element)
	ns->source = gst_element_factory_make ("gnashsrc", NULL);
	gnashsrc_callback* gc = new gnashsrc_callback;
	gc->read = NetStreamGst::readPacket;
	gc->seek = NetStreamGst::seekMedia;
	g_object_set (G_OBJECT (ns->source), "data", ns, "callbacks", gc, NULL);

	// setup the audio converter
	ns->audioconv = gst_element_factory_make ("audioconvert", NULL);

	// setup the volume controller
	ns->volume = gst_element_factory_make ("volume", NULL);

	// setup the decoder with callback
	ns->decoder = gst_element_factory_make ("decodebin", NULL);
	g_signal_connect (ns->decoder, "new-decoded-pad", G_CALLBACK (NetStreamGst::callback_newpad), ns);

	// setup the video colorspaceconverter converter
	ns->colorspace = gst_element_factory_make ("ffmpegcolorspace", NULL);

	// Setup the capsfilter which demands either YUV or RGB videoframe format
	ns->videocaps = gst_element_factory_make ("capsfilter", NULL);
	GstCaps* caps;
	if (gnash::render::videoFrameFormat() == render::YUV) {
		caps = gst_caps_new_simple ("video/x-raw-yuv", NULL);
	} else {
		caps = gst_caps_new_simple ("video/x-raw-rgb", NULL);
	}
	g_object_set (G_OBJECT (ns->videocaps), "caps", caps, NULL);
	gst_caps_unref (caps);

	// Setup the videorate element which makes sure the frames are delivered on time.
	ns->videorate = gst_element_factory_make ("videorate", NULL);

	// setup the videosink with callback
	ns->videosink = gst_element_factory_make ("fakesink", NULL);
	g_object_set (G_OBJECT (ns->videosink), "signal-handoffs", TRUE, "sync", TRUE, NULL);
	g_signal_connect (ns->videosink, "handoff", G_CALLBACK (NetStreamGst::callback_output), ns);

	if (!ns->source || !ns->audioconv || !ns->volume || !ns->decoder || !ns->colorspace || !ns->videocaps || !ns->videorate || !ns->videosink) {
		gnash::log_error("Gstreamer element(s) for movie handling could not be created\n");
		return;
	}

	// put it all in the pipeline
	gst_bin_add_many (GST_BIN (ns->pipeline), ns->source, ns->decoder, ns->audiosink, ns->audioconv, ns->colorspace, ns->videosink, ns->videorate, ns->videocaps, ns->volume, NULL);

	// link the elements
	gst_element_link(ns->source, ns->decoder);
	gst_element_link_many(ns->audioconv, ns->volume, ns->audiosink, NULL);
	gst_element_link_many(ns->colorspace, ns->videocaps, ns->videorate, ns->videosink, NULL);
	
	// start playing	
	gst_element_set_state (ns->pipeline, GST_STATE_PLAYING);
	return;
}

image::image_base* NetStreamGst::get_video()
{
	return m_imageframe;
}

void
NetStreamGst::seek(double pos)
{

	if (!gst_element_seek (pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
		GST_SEEK_TYPE_SET, GST_SECOND * static_cast<long>(pos),
		GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) {
		log_warning("Seek failed");
	}
}

void
NetStreamGst::setBufferTime()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
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

// Gstreamer callback function
int 
NetStreamGst::readPacket(void* opaque, char* buf, int buf_size){

	NetStreamGst* ns = static_cast<NetStreamGst*>(opaque);

	NetConnection* nc = ns->_netCon;
	size_t ret = nc->read(static_cast<void*>(buf), buf_size);
	ns->inputPos += ret;

	return ret;

}

// Gstreamer callback function
int 
NetStreamGst::seekMedia(void *opaque, int offset, int whence){

	NetStreamGst* ns = static_cast<NetStreamGst*>(opaque);
	NetConnection* nc = ns->_netCon;

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
