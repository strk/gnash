// sound_handler_gst.cpp: Audio output via GStreamer, for Gnash.
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

// Based on sound_handler_sdl.cpp by Thatcher Ulrich http://tulrich.com 2003
// which has been donated to the Public Domain.

/* $Id: sound_handler_gst.cpp,v 1.5 2007/12/01 21:07:20 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "utility.h" // for convert_raw_data

#include <utility> // for std::make_pair

// Assume people running --enable-media=gst know what they are doing
// (HAVE_GST_GST_H seems broken atm, specifically when an older glib
//  install is around)
//

#ifdef SOUND_GST

#include "sound_handler_gst.h"
#include "gnash.h"
#include "log.h"
#include "types.h"	// for IF_VERBOSE_* macros
#include <cmath>
#include <vector>

#include <gst/gst.h>

#define BUFFER_SIZE 5000

using namespace boost;

namespace gnash {
namespace media {

GST_sound_handler::GST_sound_handler()
	: looping(false),
	  muted(false)
{
		// init gstreamer
	gst_init(NULL, NULL);
}

GST_sound_handler::~GST_sound_handler()
{

	for (size_t i=0, e=m_sound_data.size(); i < e; ++i)  {
		stop_sound(i);
		delete_sound(i);
	}
}


int	GST_sound_handler::create_sound(
	void* data_,
	unsigned int data_bytes,
	std::auto_ptr<SoundInfo> sinfo)
// Called to create a sample.  We'll return a sample ID that
// can be use for playing it.
{

	try_mutex::scoped_lock lock(_mutex);

	unsigned char* data = static_cast<unsigned char*>(data_);

	assert(sinfo.get());
	sound_data *sounddata = new sound_data;
	if (!sounddata) {
		log_error(_("could not allocate memory for sound data"));
		return -1;
	}

	sounddata->volume = 100;
	sounddata->soundinfo = sinfo;

	switch (sounddata->soundinfo->getFormat())
	{
	case AUDIO_CODEC_MP3:
	case AUDIO_CODEC_RAW:
	case AUDIO_CODEC_ADPCM:
	case AUDIO_CODEC_UNCOMPRESSED:
	case AUDIO_CODEC_NELLYMOSER:
	case AUDIO_CODEC_NELLYMOSER_8HZ_MONO:
		if ( data ) sounddata->append(data, data_bytes);
		break;


	default:
		// Unhandled format.
		log_error(_("Unknown sound format %d requested; gnash does not handle it"), (int)sounddata->soundinfo->getFormat());
		return -1; // Unhandled format, set to NULL.
	}

	m_sound_data.push_back(sounddata);

	return m_sound_data.size()-1;
}


// this gets called when a stream gets more data
long	GST_sound_handler::fill_stream_data(unsigned char* data, unsigned int data_bytes, unsigned int /*sample_count*/, int handle_id)
{
	try_mutex::scoped_lock lock(_mutex);

	// @@ does a negative handle_id have any meaning ?
	//    should we change it to unsigned instead ?
	if (handle_id >= 0 && (unsigned int) handle_id < m_sound_data.size())
	{
		sound_data* sounddata = m_sound_data[handle_id];

		long startSize = sounddata->dataSize();

		sounddata->append(data, data_bytes);

		// If playback has already started, we also update the active sounds
		for (size_t i=0, e=sounddata->m_gst_elements.size(); i < e; ++i) {
			gst_elements* sound = sounddata->m_gst_elements[i];
			sound->data_size = sounddata->dataSize();
			sound->set_data(sounddata->data());
		}

		return startSize;
	}
	else
	{
		delete [] data;
		return 0;
	}
}

// This stops sounds when they are done playing
static gboolean sound_killer (gpointer user_data)
{
	gst_elements *gstelements = static_cast<gst_elements*>(user_data);
	gst_element_set_state (GST_ELEMENT (gstelements->pipeline), GST_STATE_NULL);
	return false;
}

// The callback function which refills the buffer with data
void GST_sound_handler::callback_handoff (GstElement * /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data)
{
	gst_elements *gstelements = static_cast<gst_elements*>(user_data);

	try_mutex::scoped_try_lock lock(gstelements->handler->_mutex);

	// If we couldn't obtain a lock return to avoid a deadlock
	if (!lock.locked()) {

		// We return nothing in this case to avoid noise being decoded and played
		if (GST_BUFFER_SIZE(buffer) != 0 && GST_BUFFER_DATA(buffer)) {
			GST_BUFFER_DATA(buffer) = 0;
			GST_BUFFER_SIZE(buffer) = 0;
		}
		return;
	}

	// First callback or after a couldn't-get-lock-return 
	if (GST_BUFFER_SIZE(buffer) == 0) {
		if (gstelements->data_size > BUFFER_SIZE) {
			GST_BUFFER_SIZE(buffer) = BUFFER_SIZE;
		} else {
			GST_BUFFER_SIZE(buffer) = gstelements->data_size;
		}

		// Reallocate the required memory.
		guint8* tmp_buf = new guint8[GST_BUFFER_SIZE(buffer)];
		memcpy(tmp_buf, GST_BUFFER_DATA(buffer), sizeof(buffer));

		delete [] GST_BUFFER_DATA(buffer);
		GST_BUFFER_DATA(buffer) = tmp_buf;
	}

	// All the data has been given to the pipeline, so now we need to stop
	// the pipeline. g_idle_add() makes sure sound_killer is called soon.
	if (gstelements->position > gstelements->data_size) {
		g_idle_add(sound_killer, user_data);
		GST_BUFFER_SIZE(buffer) = 0;
		GST_BUFFER_DATA(buffer) = 0;
		return;
	}

	const guint8* data_pos = gstelements->get_data_ptr(gstelements->position);

	// Last callback - the last re-fill
	if (gstelements->position+BUFFER_SIZE > gstelements->data_size) {
	
		unsigned int chunk_size = gstelements->data_size-gstelements->position;
		// Check if we should loop. If loop_count is 0 we have we just
		// played the sound for the last (and perhaps first) time.
		// If loop_count is anything else we continue to loop.
		if (gstelements->loop_count == 0) {
			GST_BUFFER_SIZE(buffer) = chunk_size;
			memcpy(GST_BUFFER_DATA(buffer), data_pos, chunk_size);
			gstelements->position += BUFFER_SIZE;

			gst_element_set_state (GST_ELEMENT (gstelements->input), GST_STATE_PAUSED);

		} else {
			// Copy what's left of the data, and then fill the rest with "new" data.
			memcpy(GST_BUFFER_DATA(buffer), data_pos,  chunk_size);
			memcpy(GST_BUFFER_DATA(buffer) + chunk_size, gstelements->get_data_ptr(0), GST_BUFFER_SIZE(buffer)- chunk_size);
			gstelements->position = GST_BUFFER_SIZE(buffer) - chunk_size;
			gstelements->loop_count--;

		}

		return;

	}

	// Standard re-fill
	memcpy(GST_BUFFER_DATA(buffer), data_pos, BUFFER_SIZE);
	gstelements->position += BUFFER_SIZE;

}


void	GST_sound_handler::play_sound(int sound_handle, int loop_count, int /*offset*/, long start_position, const std::vector<sound_envelope>* /*envelopes*/)
// Play the index'd sample.
{
	try_mutex::scoped_lock lock(_mutex);

	// Check if the sound exists, or if audio is muted
	if (sound_handle < 0 || (unsigned int) sound_handle >= m_sound_data.size() || muted)
	{
		// Invalid handle, or audio is muted.
		return;
	}

	sound_data* sounddata = m_sound_data[sound_handle];

	// If this is called from a streamsoundblocktag, we only start if this
	// sound isn't already playing.
	if (start_position > 0 && sounddata->m_gst_elements.size() > 0) {
		return;
	}
	// Make sure sound actually got some data
	if (sounddata->dataSize() < 1) {
		IF_VERBOSE_MALFORMED_SWF(
			log_swferror(_("Trying to play sound with size 0"));
		);
		return;
	}

	// Make a "gst_elements" for this sound which is latter placed on the vector of instances of this sound being played
	gst_elements* gst_element = new gst_elements;
	if (gst_element == NULL) {
		log_error (_("Could not allocate memory for gst_element"));
		return;
	}

	// Set the handler
	gst_element->handler = this;

	// Copy data-info to the "gst_elements"
	gst_element->data_size = sounddata->dataSize();
	gst_element->set_data(sounddata->data());
	gst_element->position = start_position;

	// Set number of loop we should do. -1 is infinte loop, 0 plays it once, 1 twice etc.
	gst_element->loop_count = loop_count;

	// create main pipeline
	gst_element->pipeline = gst_pipeline_new (NULL);

	// create an audio sink - use oss, alsa or...? make a commandline option?
	// we first try atudetect, then alsa, then oss, then esd, then...?
#if !defined(__NetBSD__)
	gst_element->audiosink = gst_element_factory_make ("autoaudiosink", NULL);
	if (!gst_element->audiosink) gst_element->audiosink = gst_element_factory_make ("alsasink", NULL);
	if (!gst_element->audiosink) gst_element->audiosink = gst_element_factory_make ("osssink", NULL);
#endif
	if (!gst_element->audiosink) gst_element->audiosink = gst_element_factory_make ("esdsink", NULL);

	// Check if the creation of the gstreamer pipeline, adder and audiosink was a succes
	if (!gst_element->pipeline) {
		log_error(_("The gstreamer pipeline element could not be created"));
	}
	if (!gst_element->audiosink) {
		log_error(_("The gstreamer audiosink element could not be created"));
	}

	// link adder and output to bin
	gst_bin_add (GST_BIN (gst_element->pipeline), gst_element->audiosink);

	gst_element->bin = gst_bin_new(NULL);
	gst_element->input = gst_element_factory_make ("fakesrc", NULL);
	gst_element->capsfilter = gst_element_factory_make ("capsfilter", NULL);
	gst_element->audioconvert = gst_element_factory_make ("audioconvert", NULL);
	gst_element->audioresample = gst_element_factory_make ("audioresample", NULL);
	gst_element->volume = gst_element_factory_make ("volume", NULL);

	// Put the gstreamer elements in the pipeline
	gst_bin_add_many (GST_BIN (gst_element->bin), gst_element->input,
					gst_element->capsfilter,
					gst_element->audioconvert,
					gst_element->audioresample, 
					gst_element->volume, NULL);

	// Test if the fakesrc, typefind and audio* elements was correctly created
	if (!gst_element->input
		|| !gst_element->capsfilter
		|| !gst_element->audioconvert
		|| !gst_element->audioresample) {

		log_error(_("Gstreamer element for audio handling could not be created"));
		return;
	}

	// Create a gstreamer decoder for the chosen sound.

	// Temp variables to make the code simpler and easier to read
	audioCodecType soundFormat = sounddata->soundinfo->getFormat();
	bool soundStereo = sounddata->soundinfo->isStereo();
	uint32_t soundSampleRate = sounddata->soundinfo->getSampleRate();

	if (soundFormat == AUDIO_CODEC_MP3) {

		gst_element->decoder = gst_element_factory_make ("mad", NULL);
		if (gst_element->decoder == NULL) {
			gst_element->decoder = gst_element_factory_make ("flump3dec", NULL);
			if (gst_element->decoder != NULL && !gst_default_registry_check_feature_version("flump3dec", 0, 10, 4))
			{
				static bool warned=false;
				if ( ! warned ) 
				{
					// I keep getting these messages even if I hear sound... too much paranoia ?
					log_debug(_("This version of fluendos mp3 plugin does not support flash streaming sounds, please upgrade to version 0.10.4 or higher"));
					warned=true;
				}
			}
		}
		// Check if the element was correctly created
		if (!gst_element->decoder) {
			log_error(_("A gstreamer mp3-decoder element could not be created.  You probably need to install a mp3-decoder plugin like gstreamer0.10-mad or gstreamer0.10-fluendo-mp3."));
			return;
		}
		gst_bin_add (GST_BIN (gst_element->bin), gst_element->decoder);

		// Set the info about the stream so that gstreamer knows what it is.
		GstCaps *caps = gst_caps_new_simple ("audio/mpeg",
			"mpegversion", G_TYPE_INT, 1,
			"layer", G_TYPE_INT, 3,
			"rate", G_TYPE_INT, soundSampleRate,
			"channels", G_TYPE_INT, soundStereo ? 2 : 1, NULL);
		g_object_set (G_OBJECT (gst_element->capsfilter), "caps", caps, NULL);
		gst_caps_unref (caps);

		// setup fake source
		g_object_set (G_OBJECT (gst_element->input),
					"sizetype", 2, "can-activate-pull", FALSE, "signal-handoffs", TRUE,
					"sizemax", BUFFER_SIZE, NULL);
		// Setup the callback
		gst_element->handoff_signal_id = g_signal_connect (gst_element->input, "handoff", G_CALLBACK (callback_handoff), gst_element);

		// link data, decoder, audio* and adder
		gst_element_link_many (gst_element->input,
						gst_element->capsfilter,
						gst_element->decoder,
						gst_element->audioconvert,
						gst_element->audioresample, 
						gst_element->volume, NULL);

	} else if (soundFormat == AUDIO_CODEC_ADPCM) {
		gst_element->decoder = gst_element_factory_make ("ffdec_adpcm_swf", NULL);

		// Check if the element was correctly created
		if (!gst_element->decoder) {
			log_error(_("A gstreamer adpcm-decoder element could not be created.  You probably need to install gst-ffmpeg."));
			return;
		}
		gst_bin_add (GST_BIN (gst_element->bin), gst_element->decoder);

		// Set the info about the stream so that gstreamer knows what it is.
		GstCaps *caps = gst_caps_new_simple ("audio/x-adpcm",
			"rate", G_TYPE_INT, soundSampleRate,
			"channels", G_TYPE_INT, soundStereo ? 2 : 1, NULL);
		g_object_set (G_OBJECT (gst_element->capsfilter), "caps", caps, NULL);
		gst_caps_unref (caps);

		// setup fake source
		g_object_set (G_OBJECT (gst_element->input),
					"sizetype", 2, "can-activate-pull", FALSE, "signal-handoffs", TRUE,
					"sizemax", BUFFER_SIZE, NULL);
		// Setup the callback
		gst_element->handoff_signal_id = g_signal_connect (gst_element->input, "handoff", G_CALLBACK (callback_handoff), gst_element);

		// link data, decoder, audio* and adder
		gst_element_link_many (gst_element->input,
						gst_element->capsfilter,
						gst_element->decoder,
						gst_element->audioconvert,
						gst_element->audioresample, 
						gst_element->volume, NULL);

	} else if (soundFormat == AUDIO_CODEC_NELLYMOSER_8HZ_MONO || soundFormat == AUDIO_CODEC_NELLYMOSER) {
		return;
	} else {

		// Set the info about the stream so that gstreamer knows what it is.
		GstCaps *caps = gst_caps_new_simple ("audio/x-raw-int",
			"rate", G_TYPE_INT, soundSampleRate,
			"channels", G_TYPE_INT, soundStereo ? 2 : 1,
			"endianness", G_TYPE_INT, G_BIG_ENDIAN,
			"width", G_TYPE_INT, (sounddata->soundinfo->is16bit() ? 16 : 8),
			"depth", G_TYPE_INT, 16,
			//"signed", G_TYPE_INT, 1,
			 NULL);
		g_object_set (G_OBJECT (gst_element->capsfilter), "caps", caps, NULL);
		gst_caps_unref (caps);

		// setup fake source
		g_object_set (G_OBJECT (gst_element->input),
					"sizetype", 2, "can-activate-pull", FALSE, "signal-handoffs", TRUE,
					"sizemax", BUFFER_SIZE, NULL);
		// Setup the callback
		gst_element->handoff_signal_id = g_signal_connect (gst_element->input, "handoff", G_CALLBACK (callback_handoff), gst_element);

		// Raw native sound-data, output directly
		gst_element_link_many (gst_element->input, 
					gst_element->capsfilter, 
					gst_element->audioconvert,
					gst_element->audioresample,
					gst_element->volume, NULL);
	}
	// Add ghostpad
	GstPad *pad = gst_element_get_pad (gst_element->volume, "src");
	gst_element_add_pad (gst_element->bin, gst_ghost_pad_new ("src", pad));
	gst_object_unref (GST_OBJECT (pad));
	
	// Add the bin to the main pipeline
	gst_bin_add(GST_BIN (gst_element->pipeline), gst_element->bin);
	// Link to the adder sink pad
	GstPad *sinkpad = gst_element_get_pad (gst_element->audiosink, "sink");
	GstPad *srcpad = gst_element_get_pad (gst_element->bin, "src");
	gst_pad_link (srcpad, sinkpad);
	gst_object_unref (GST_OBJECT (srcpad));
	gst_object_unref (GST_OBJECT (sinkpad));

	// Set the volume
	g_object_set (G_OBJECT (gst_element->volume), "volume", static_cast<double>(sounddata->volume) / 100.0, NULL);

	//gst_pad_add_event_probe(pad, G_CALLBACK(event_callback), sounddata);

	// Put the gst_element on the vector
	sounddata->m_gst_elements.push_back(gst_element);

	// If not already playing, start doing it
	gst_element_set_state (GST_ELEMENT (gst_element->pipeline), GST_STATE_PLAYING);

	++_soundsStarted;

}


void	GST_sound_handler::stop_sound(int sound_handle)
{
	try_mutex::scoped_lock lock(_mutex);

	// Check if the sound exists.
	if (sound_handle < 0 || (unsigned int) sound_handle >= m_sound_data.size())
	{
		// Invalid handle.
		return;
	}

	sound_data* sounddata = m_sound_data[sound_handle];

	// Stop all the instances of this sound.
	// TODO: fix the loop to use size_t instead of i
	for (int i = sounddata->m_gst_elements.size()-1; i >= 0 ; i--)
	{
		gst_elements* elements = sounddata->m_gst_elements[i];

		// Check if we can succesfully stop the elements
		// playback - if not we skip cleaning this for now
		// FIXME: what if it ain't possible to stop an element when this is called from ~GST_sound_handler

		// Disconnect signals
		g_signal_handler_disconnect (elements->input, elements->handoff_signal_id);

		gst_element_set_state (GST_ELEMENT (elements->pipeline), GST_STATE_NULL);
		gst_object_unref (GST_OBJECT (elements->pipeline));

		// Delete the gst_element struct
		// @@ we're deleting the elements from the start, so half-way of the loop we will be referring to undefined elements. Is this intended ? --strk;
		delete elements;
		sounddata->m_gst_elements.erase(sounddata->m_gst_elements.begin() + i);
	}

	++_soundsStopped;
}


void	GST_sound_handler::delete_sound(int sound_handle)
// this gets called when it's done with a sample.
{
	try_mutex::scoped_lock lock(_mutex);

	if (sound_handle >= 0 && (unsigned int) sound_handle < m_sound_data.size())
	{
		delete m_sound_data[sound_handle];
		m_sound_data.erase (m_sound_data.begin() + sound_handle);
	}

}

// This will stop all sounds playing. Will cause problems if the soundhandler is made static
// and supplys sound_handling for many SWF's, since it will stop all sounds with no regard
// for what sounds is associated with what SWF.
void	GST_sound_handler::stop_all_sounds()
{
	for (size_t i=0, e=m_sound_data.size(); i < e; ++i) 
		stop_sound(i);
}


//	returns the sound volume level as an integer from 0 to 100,
//	where 0 is off and 100 is full volume. The default setting is 100.
int	GST_sound_handler::get_volume(int sound_handle) {

	try_mutex::scoped_lock lock(_mutex);

	// Check if the sound exists.
	if (sound_handle >= 0 && (unsigned int) sound_handle < m_sound_data.size())
	{
		return m_sound_data[sound_handle]->volume;
	} else {
		return 0; // Invalid handle
	}
}


//	A number from 0 to 100 representing a volume level. 
//	100 is full volume and 0 is no volume. The default setting is 100.
void	GST_sound_handler::set_volume(int sound_handle, int volume) {

	try_mutex::scoped_lock lock(_mutex);

	// Check if the sound exists.
	if (sound_handle < 0 || (unsigned int) sound_handle >= m_sound_data.size())
	{
		// Invalid handle.
		return;
	}

	sound_data* sd = m_sound_data[sound_handle];

	// Set volume for this sound. Should this only apply to the active sounds?

	sd->volume = volume;
	
	for (size_t i=0, n=sd->m_gst_elements.size(); i<n; ++i)
	{
		g_object_set (
			G_OBJECT (sd->m_gst_elements[i]->volume),
			"volume",
			static_cast<double>(volume/100.0),
			NULL);
	}

}

SoundInfo* GST_sound_handler::get_sound_info(int sound_handle) {

	try_mutex::scoped_lock lock(_mutex);

	// Check if the sound exists.
	if (sound_handle >= 0 && static_cast<unsigned int>(sound_handle) < m_sound_data.size())
	{
		return m_sound_data[sound_handle]->soundinfo.get();
	} else {
		return NULL;
	}

}

// gnash calls this to mute audio
void GST_sound_handler::mute() {
	stop_all_sounds();
	muted = true;
}

// gnash calls this to unmute audio
void GST_sound_handler::unmute() {
	muted = false;
}

bool GST_sound_handler::is_muted() {
	return muted;
}



// The callback function which refills the buffer with data
void GST_sound_handler::callback_as_handoff (GstElement * /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data)
{
	gst_elements *gstelements = static_cast<gst_elements*>(user_data);

	try_mutex::scoped_try_lock lock(gstelements->handler->_mutex);

	// If we couldn't obtain a lock return to avoid a deadlock
	if (!lock.locked()) {

		// We return nothing in this case to avoid noise being decoded and played
		if (GST_BUFFER_SIZE(buffer) != 0 && GST_BUFFER_DATA(buffer)) {
			GST_BUFFER_DATA(buffer) = 0;
			GST_BUFFER_SIZE(buffer) = 0;
		}
		return;
	}

	// First callback or after a couldn't-get-lock-return 
	if (GST_BUFFER_SIZE(buffer) == 0) {
		if (gstelements->data_size > BUFFER_SIZE) {
			GST_BUFFER_SIZE(buffer) = BUFFER_SIZE;
		} else {
			GST_BUFFER_SIZE(buffer) = gstelements->data_size;
		}

		// Reallocate the required memory.
		guint8* tmp_buf = new guint8[GST_BUFFER_SIZE(buffer)];
		memcpy(tmp_buf, GST_BUFFER_DATA(buffer), sizeof(buffer));

		delete [] GST_BUFFER_DATA(buffer);
		GST_BUFFER_DATA(buffer) = tmp_buf;
	}

	aux_streamer_ptr aux_streamer = gstelements->handler->m_aux_streamer[gstelements->owner];
	
	// If false is returned the sound doesn't want to be attached anymore
	bool ret = (aux_streamer)(gstelements->owner, GST_BUFFER_DATA(buffer), GST_BUFFER_SIZE(buffer));

}

void GST_sound_handler::attach_aux_streamer(aux_streamer_ptr ptr, void* owner)
{
	try_mutex::scoped_lock lock(_mutex);
	assert(owner);
	assert(ptr);

	if ( m_aux_streamer.insert(std::make_pair(owner, ptr)).second )
	{
		// Already in the hash.
		return;
	}

	// Make a pipeline that can play the raw data

	// Make a "gst_elements" for this sound which is latter placed on the vector of instances of this sound being played
	gst_elements* gst_element = new gst_elements;
	if (gst_element == NULL) {
		log_error (_("Could not allocate memory for gst_element"));
		return;
	}

	// Set the handler
	gst_element->handler = this;

	// create main pipeline
	gst_element->pipeline = gst_pipeline_new (NULL);

	// create an audio sink - use oss, alsa or...? make a commandline option?
	// we first try atudetect, then alsa, then oss, then esd, then...?
#if !defined(__NetBSD__)
	gst_element->audiosink = gst_element_factory_make ("autoaudiosink", NULL);
	if (!gst_element->audiosink) gst_element->audiosink = gst_element_factory_make ("alsasink", NULL);
	if (!gst_element->audiosink) gst_element->audiosink = gst_element_factory_make ("osssink", NULL);
#endif
	if (!gst_element->audiosink) gst_element->audiosink = gst_element_factory_make ("esdsink", NULL);

	// Check if the creation of the gstreamer pipeline, adder and audiosink was a succes
	if (!gst_element->pipeline) {
		log_error(_("The gstreamer pipeline element could not be created"));
	}
	if (!gst_element->audiosink) {
		log_error(_("The gstreamer audiosink element could not be created"));
	}

	// link adder and output to bin
	gst_bin_add (GST_BIN (gst_element->pipeline), gst_element->audiosink);

	gst_element->bin = gst_bin_new(NULL);
	gst_element->input = gst_element_factory_make ("fakesrc", NULL);
	gst_element->capsfilter = gst_element_factory_make ("capsfilter", NULL);
	gst_element->audioconvert = gst_element_factory_make ("audioconvert", NULL);
	gst_element->audioresample = gst_element_factory_make ("audioresample", NULL);
	gst_element->volume = gst_element_factory_make ("volume", NULL);

	// Put the gstreamer elements in the pipeline
	gst_bin_add_many (GST_BIN (gst_element->bin), gst_element->input,
					gst_element->capsfilter,
					gst_element->audioconvert,
					gst_element->audioresample, 
					gst_element->volume, NULL);

	// Test if the fakesrc, typefind and audio* elements was correctly created
	if (!gst_element->input
		|| !gst_element->capsfilter
		|| !gst_element->audioconvert
		|| !gst_element->audioresample) {

		log_error(_("Gstreamer element for audio handling could not be created"));
		return;
	}

	// Create a gstreamer decoder for the chosen sound.

	// Set the info about the stream so that gstreamer knows what it is.
	// We know what the pre-decoded data format is.
	GstCaps *caps = gst_caps_new_simple ("audio/x-raw-int",
		"rate", G_TYPE_INT, 44100,
		"channels", G_TYPE_INT, 2,
		"endianness", G_TYPE_INT, G_BIG_ENDIAN,
		"width", G_TYPE_INT, 16,
		"depth", G_TYPE_INT, 16,
		//"signed", G_TYPE_INT, 1,
		 NULL);
	g_object_set (G_OBJECT (gst_element->capsfilter), "caps", caps, NULL);
	gst_caps_unref (caps);

	// setup fake source
	g_object_set (G_OBJECT (gst_element->input),
				"sizetype", 2, "can-activate-pull", FALSE, "signal-handoffs", TRUE,
				"sizemax", BUFFER_SIZE, NULL);
	// Setup the callback
	gst_element->handoff_signal_id = g_signal_connect (gst_element->input, "handoff", G_CALLBACK (callback_as_handoff), gst_element);

	// we receive raw native sound-data, output directly
	gst_element_link_many (gst_element->input, 
				gst_element->capsfilter, 
				gst_element->audioconvert,
				gst_element->audioresample,
				gst_element->volume, NULL);

	// Add ghostpad
	GstPad *pad = gst_element_get_pad (gst_element->volume, "src");
	gst_element_add_pad (gst_element->bin, gst_ghost_pad_new ("src", pad));
	gst_object_unref (GST_OBJECT (pad));
	
	// Add the bin to the main pipeline
	gst_bin_add(GST_BIN (gst_element->pipeline), gst_element->bin);
	// Link to the adder sink pad
	GstPad *sinkpad = gst_element_get_pad (gst_element->audiosink, "sink");
	GstPad *srcpad = gst_element_get_pad (gst_element->bin, "src");
	gst_pad_link (srcpad, sinkpad);
	gst_object_unref (GST_OBJECT (srcpad));
	gst_object_unref (GST_OBJECT (sinkpad));

	gst_element->owner = owner;

	// Put the gst_element in the map
	m_aux_streamer_gstelements[owner] = gst_element;
	
	// If not already playing, start doing it
	gst_element_set_state (GST_ELEMENT (gst_element->pipeline), GST_STATE_PLAYING);
printf("pipeline stated playing\n");
}

void GST_sound_handler::detach_aux_streamer(void* owner)
{
	try_mutex::scoped_lock lock(_mutex);

	GstElementsMap::iterator it=m_aux_streamer_gstelements.find(owner);
	if ( it == m_aux_streamer_gstelements.end() ) return; // not found

	delete it->second;
	// WARNING: erasing would break any iteration in the map
	m_aux_streamer_gstelements.erase(it);
}

unsigned int GST_sound_handler::get_duration(int sound_handle)
{
	try_mutex::scoped_lock lock(_mutex);

	// Check if the sound exists.
	if (sound_handle < 0 || (unsigned int) sound_handle >= m_sound_data.size())
	{
		// Invalid handle.
		return 0;
	}

	sound_data* sounddata = m_sound_data[sound_handle];

	uint32_t sampleCount = sounddata->soundinfo->getSampleCount();
	uint32_t sampleRate = sounddata->soundinfo->getSampleRate();

	// Return the sound duration in milliseconds
	if (sampleCount > 0 && sampleRate > 0) {
		unsigned int ret = sampleCount / sampleRate * 1000;
		ret += ((sampleCount % sampleRate) * 1000) / sampleRate;
		if (sounddata->soundinfo->isStereo()) ret = ret / 2;
		return ret;
	} else {
		return 0;
	}
}

unsigned int GST_sound_handler::get_position(int sound_handle)
{
	try_mutex::scoped_lock lock(_mutex);

	// Check if the sound exists.
	if (sound_handle < 0 || (unsigned int) sound_handle >= m_sound_data.size())
	{
		// Invalid handle.
		return 0;
	}

	sound_data* sounddata = m_sound_data[sound_handle];

	// If there is no active sounds, return 0
	if (sounddata->m_gst_elements.size() == 0) {
		return 0;
	}

	// return the position of the last element added
	GstElement *pipeline,*audioconvert;
	GstStateChangeReturn ret;
	GstState current, pending;
	int64_t pos;
	GstFormat fmt = GST_FORMAT_TIME;
	
	pipeline = sounddata->m_gst_elements[sounddata->m_gst_elements.size()-1]->pipeline;
	
	ret = gst_element_get_state (GST_ELEMENT (pipeline), &current, &pending, 0);

	if (current != GST_STATE_NULL) {
		audioconvert = sounddata->m_gst_elements[sounddata->m_gst_elements.size()-1]->audioconvert;
		if (gst_element_query_position (audioconvert, &fmt, &pos)) {
			return static_cast<unsigned int>(pos / GST_MSECOND);
		} else {
			return 0;
		}
	}
	return 0;
}

// Pointer handling and checking functions
const uint8_t* gst_elements::get_data_ptr(unsigned long int pos)
{
	assert(data_size > pos);
	return data + pos;
}

void gst_elements::set_data(const uint8_t* idata) {
	data = idata;
}

sound_handler*	create_sound_handler_gst()
// Factory.
{
	return new GST_sound_handler;
}

} // gnash.media namespace 
} // namespace gnash

#endif // SOUND_GST

// Local Variables:
// mode: C++
// End:

