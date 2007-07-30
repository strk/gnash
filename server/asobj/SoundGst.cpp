// SoundGst.cpp:  Produce sound for gnash, via Gstreamer library.
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
#include "config.h"
#endif

#include "log.h"
#include "SoundGst.h"
#include "sound_definition.h" // for sound_sample
#include "movie_definition.h"
#include "sprite_instance.h"
#include "fn_call.h"
#include "GnashException.h"
#include "builtin_function.h"

#include "gstgnashsrc.h"

#include <string>

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

// Gstreamer callback function
int 
SoundGst::readPacket(void* opaque, char* buf, int buf_size)
{

	SoundGst* so = static_cast<SoundGst*>(opaque);
	boost::intrusive_ptr<NetConnection> nc = so->connection;

	size_t ret = nc->read(static_cast<void*>(buf), buf_size);
	so->inputPos += ret;
	return ret;

}

// Gstreamer callback function
int 
SoundGst::seekMedia(void *opaque, int offset, int whence){

	SoundGst* so = static_cast<SoundGst*>(opaque);
	boost::intrusive_ptr<NetConnection> nc = so->connection;


	// Offset is absolute new position in the file
	if (whence == SEEK_SET) {
		nc->seek(offset);
		so->inputPos = offset;

	// New position is offset + old position
	} else if (whence == SEEK_CUR) {
		nc->seek(so->inputPos + offset);
		so->inputPos = so->inputPos + offset;

	// 	// New position is offset + end of file
	} else if (whence == SEEK_END) {
		// This is (most likely) a streamed file, so we can't seek to the end!
		// Instead we seek to 50.000 bytes... seems to work fine...
		nc->seek(50000);
		so->inputPos = 50000;
		
	}

	return so->inputPos;
}

// Callback function used by Gstreamer to to attached audio and video streams
// detected by decoderbin to either the video out or audio out elements.
void
SoundGst::callback_newpad (GstElement* /*decodebin*/, GstPad *pad, gboolean /*last*/, gpointer data)
{
	log_msg(_("%s: new pad found"), __FUNCTION__);
	SoundGst* so = static_cast<SoundGst*>(data);
	GstCaps *caps;
	GstStructure *str;
	GstPad *audiopad;

	audiopad = gst_element_get_pad (so->audioconv, "sink");

	// check media type
	caps = gst_pad_get_caps (pad);
	str = gst_caps_get_structure (caps, 0);
	if (g_strrstr (gst_structure_get_name (str), "audio")) {
		// link'n'play
		gst_pad_link (pad, audiopad);
		log_msg(_("%s: new pad connected"), __FUNCTION__);
	} else {
		gst_object_unref (audiopad);
		log_error(_("%s: Non-audio data found in file %s"), __FUNCTION__,
				so->externalURL.c_str());
	}
	gst_caps_unref (caps);
	return;
}

void
SoundGst::setupDecoder(SoundGst* so)
{

	boost::intrusive_ptr<NetConnection> nc = so->connection;
	assert(nc);

	// Pass stuff from/to the NetConnection object.
	assert(so);
	if ( !nc->openConnection(so->externalURL) ) {
		log_error(_("could not open audio url: %s"), so->externalURL.c_str());
		delete so->lock;
		return;
	}

	so->inputPos = 0;

	// init GStreamer
	gst_init (NULL, NULL);

	// setup the GnashNC plugin
	_gst_plugin_register_static (&gnash_plugin_desc);

	// setup the pipeline
	so->pipeline = gst_pipeline_new (NULL);

	// create an audio sink - use oss, alsa or...? make a commandline option?
	// we first try atudetect, then alsa, then oss, then esd, then...?
	// If the gstreamer adder ever gets fixed this should be connected to the
	// adder in the soundhandler.
#if !defined(__NetBSD__)
	so->audiosink = gst_element_factory_make ("autoaudiosink", NULL);
	if (!so->audiosink) so->audiosink = gst_element_factory_make ("alsasink", NULL);
	if (!so->audiosink) so->audiosink = gst_element_factory_make ("osssink", NULL);
#endif
	if (!so->audiosink) so->audiosink = gst_element_factory_make ("esdsink", NULL);

	// Check if the creation of the gstreamer pipeline and audiosink was a succes
	if (!so->pipeline) {
		gnash::log_error(_("Could not create gstreamer pipeline element"));
		return;
	}
	if (!so->audiosink) {
		gnash::log_error(_("Could not create gstreamer audiosink element"));
		return;
	}

	// setup gnashnc source (our homegrown source element)
	so->source = gst_element_factory_make ("gnashsrc", NULL);
	gnashsrc_callback* gc = new gnashsrc_callback;
	gc->read = SoundGst::readPacket;
	gc->seek = SoundGst::seekMedia;
	g_object_set (G_OBJECT (so->source), "data", so, "callbacks", gc, NULL);

	// setup the audio converter
	so->audioconv = gst_element_factory_make ("audioconvert", NULL);

	// setup the volume controller
	so->volume = gst_element_factory_make ("volume", NULL);

	// setup the decoder with callback
	so->decoder = gst_element_factory_make ("decodebin", NULL);
	g_signal_connect (so->decoder, "new-decoded-pad", G_CALLBACK (SoundGst::callback_newpad), so);


	if (!so->source || !so->audioconv || !so->volume || !so->decoder) {
		gnash::log_error(_("Could not create Gstreamer element(s) for movie handling"));
		return;
	}

	// put it all in the pipeline
	gst_bin_add_many (GST_BIN (so->pipeline), so->source, so->decoder, so->audiosink, so->audioconv, so->volume, NULL);

	// link the elements
	gst_element_link(so->source, so->decoder);
	gst_element_link_many(so->audioconv, so->volume, so->audiosink, NULL);
	
	// By deleting this lock we allow start() to start playback
	delete so->lock;
	return;
}

SoundGst::~SoundGst() {

	if (externalSound && pipeline) {
		gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);
		gst_object_unref (GST_OBJECT (pipeline));
	}
}

void
SoundGst::loadSound(std::string file, bool streaming)
{
	pipeline = NULL;
	remainingLoops = 0;

	if (connection) {
		log_error(_("%s: This sound already has a connection?  (We try to handle this by overriding the old one...)"), __FUNCTION__);
	}
	externalURL = file;

	connection = new NetConnection();

	externalSound = true;
	isStreaming = streaming;

	lock = new boost::mutex::scoped_lock(setupMutex);

	// To avoid blocking while connecting, we use a thread.
	setupThread = new boost::thread(boost::bind(SoundGst::setupDecoder, this));

}

void
SoundGst::start(int offset, int loops)
{
	boost::mutex::scoped_lock lock(setupMutex);

	if (externalSound) {
		if (offset > 0) {
			// Seek to offset position
			if (!gst_element_seek (pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
				GST_SEEK_TYPE_SET, GST_SECOND * static_cast<long>(offset),
				GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) {
				log_error(_("%s: seeking to offset failed"), 
					__FUNCTION__);
			}

		}
		// Save how many loops to do
		if (loops > 0) {
			remainingLoops = loops;
		}
		// start playing	
		gst_element_set_state (pipeline, GST_STATE_PLAYING);

	}


	// Start sound
	sound_handler* s = get_sound_handler();
	if (s) {
		if (!externalSound) {
	    	s->play_sound(soundId, loops, offset, 0, NULL);
	    }
	}
}

void
SoundGst::stop(int si)
{
	// stop the sound
	sound_handler* s = get_sound_handler();
	if (s != NULL)
	{
	    if (si < 0) {
	    	if (externalSound) {
				gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);
	    	} else {
				s->stop_sound(soundId);
			}
		} else {
			s->stop_sound(si);
		}
	}
}

unsigned int
SoundGst::getDuration()
{
	// Return the duration of the file in milliseconds
	
	// If this is a event sound get the info from the soundhandler
	if (!externalSound) {
		sound_handler* s = get_sound_handler();
		if (s) {		
	    	return (s->get_duration(soundId));
	    } else {
	    	return 0; // just in case
		}
	}
	
	GstFormat fmt = GST_FORMAT_TIME;
	int64_t len;

	if (pipeline && gst_element_query_duration (pipeline, &fmt, &len)) {
		return static_cast<unsigned int>(len / GST_MSECOND);
	} else {
		return 0;
	}
}

unsigned int
SoundGst::getPosition()
{
	// Return the position in the file in milliseconds
	
	// If this is a event sound get the info from the soundhandler
	if (!externalSound) {
		sound_handler* s = get_sound_handler();
		if (s) {
			return s->get_position(soundId);	
	    } else {
	    	return 0; // just in case
		}
	}
	
	if (!pipeline) return 0;

	GstFormat fmt = GST_FORMAT_TIME;
	int64_t pos;
	GstStateChangeReturn ret;
	GstState current, pending;

	ret = gst_element_get_state (GST_ELEMENT (pipeline), &current, &pending, 0);

	if (current != GST_STATE_NULL && gst_element_query_position (pipeline, &fmt, &pos)) {
		return static_cast<unsigned int>(pos / GST_MSECOND);
	} else {
		return 0;
	}
}

} // end of gnash namespace
