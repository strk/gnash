//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

// Based on sound_handler_sdl.cpp by Thatcher Ulrich http://tulrich.com 2003
// which has been donated to the Public Domain.


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// Assume people running --enable-sound=gst know what they are doing
// (HAVE_GST_GST_H seems broken atm, specifically when an older glib
//  install is around)
//
//#ifdef HAVE_GST_GST_H
#ifdef SOUND_GST
#include "gnash.h"
#include "container.h"
#include "log.h"
#include "types.h"	// for IF_VERBOSE_* macros
#include <pthread.h>
#include <cmath>
#include <vector>

#include <gst/gst.h>

#define BUFFER_SIZE 5000

// Used to hold the gstreamer when doing on-demand-decoding
typedef struct
{
	// gstreamer objects
	GstElement *input;
	GstElement *decoder;
	GstElement *capsfilter;
	GstElement *audioconvert;
	GstElement *audioresample;
	GstElement *volume;
	GstElement *bin;
	
	// position in the stream
	long position;

	// The (un)compressed data
	guint8* data;

	// data size
	long data_size;

	long loop_count;
	
} gst_elements;


// Used to hold the sounddata when doing on-demand-decoding
typedef struct
{
	// The (un)compressed data
	guint8* data;

	// data format
	int format;

	// data size
	long data_size;

	// stereo or not
	bool stereo;

	// number of samples
	int sample_count;

	// sample rate
	int sample_rate;

	// Volume, SWF range: 0-100, GST range 0-10 (we only use 0-1, the rest is amplified)
	// It's the SWF range that is represented here
	int volume;

	// gstreamer objects
	std::vector<gst_elements*>	m_gst_elements;

} sound_data;

// Use gstreamer to handle sounds.
class GST_sound_handler : public gnash::sound_handler
{
public:
	// gstreamer pipeline objects

	// the main bin containing the adder and output (sink)
	GstElement *pipeline;

	GstElement *adder;
	GstElement *audiosink;

	// Sound data.
	std::vector<sound_data*>	m_sound_data;

	// Keeps track of numbers of playing sounds
	int soundsPlaying;
	
	// Is the loop running?
	bool looping;
	
	GST_sound_handler()
		: soundsPlaying(0),
		  looping(false)
	{
  		// init gstreamer
		gst_init(NULL, NULL);

		// create main pipeline
		pipeline = gst_pipeline_new (NULL);

		// create adder
		adder = gst_element_factory_make ("adder", NULL);

		// create an audio sink - use oss, alsa or...? make a commandline option?
		// we first try atudetect, then alsa, then oss, then esd, then...?
		audiosink = gst_element_factory_make ("autoaudiosink", NULL);
		if (!audiosink) audiosink = gst_element_factory_make ("alsasink", NULL);
		if (!audiosink) audiosink = gst_element_factory_make ("osssink", NULL);
		if (!audiosink) audiosink = gst_element_factory_make ("esdsink", NULL);

		// Check if the creation of the gstreamer pipeline, adder and audiosink was a succes
		if (!pipeline) {
			gnash::log_error("The gstreamer pipeline element could not be created\n");
		}
		if (!adder) {
			gnash::log_error("The gstreamer adder element could not be created\n");
		}
		if (!audiosink) {
			gnash::log_error("The gstreamer audiosink element could not be created\n");
		}

		// link adder and output to bin
		gst_bin_add (GST_BIN (pipeline), adder);
		gst_bin_add (GST_BIN (pipeline), audiosink);

		// link adder and audiosink
		gst_element_link (adder, audiosink);
		
	}

	~GST_sound_handler()
	{

		for (size_t i= m_sound_data.size(); i > 0; i--) { //Optimized
			stop_sound(i);
			delete_sound(i);
		}

		gst_object_unref (GST_OBJECT (pipeline));

	}


	virtual int	create_sound(
		void* data,
		int data_bytes,
		int sample_count,
		format_type format,
		int sample_rate,
		bool stereo)
	// Called to create a sample.  We'll return a sample ID that
	// can be use for playing it.
	{

		sound_data *sounddata = new sound_data;
		if (!sounddata) {
			gnash::log_error("could not allocate memory for sounddata !\n");
			return -1;
		}

		sounddata->format = format;
		sounddata->data_size = data_bytes;
		sounddata->stereo = stereo;
		sounddata->sample_count = sample_count;
		sounddata->sample_rate = sample_rate;
		sounddata->volume = 100;

		switch (format)
		{
		// TODO: Do we need to do the raw-data-convert? Can't Gstreamer handle it?
		case FORMAT_RAW:
/*	caps info:
      audio/x-raw-int
                   rate: [ 1, 2147483647 ]
               channels: [ 1, 8 ]
             endianness: { 1234, 4321 }
                  width: 8
                  depth: [ 1, 8 ]
                 signed: { true, false }*/
			sounddata->data = new guint8[data_bytes];
			if (!sounddata->data) { 
				gnash::log_error("could not allocate space for data in soundhandler\n");
				return -1;
			}
			memcpy(sounddata->data, data, data_bytes);
			break;

		case FORMAT_NATIVE16:
/*	caps info:
      audio/x-raw-int
                   rate: [ 1, 2147483647 ]
               channels: [ 1, 8 ]
             endianness: { 1234, 4321 }
                  width: 16
                  depth: [ 1, 16 ]
                 signed: { true, false }*/
			sounddata->data = new guint8[data_bytes];
			if (!sounddata->data) { 
				gnash::log_error("could not allocate space for data in soundhandler\n");
				return -1;
			}
			memcpy(sounddata->data, data, data_bytes);
			break;

		case FORMAT_MP3:
		//case FORMAT_VORBIS:
			sounddata->data = new guint8[data_bytes];
			if (!sounddata->data) { 
				gnash::log_error("could not allocate space for data in soundhandler\n");
				return -1;
			}
			memcpy(sounddata->data, data, data_bytes);

			break;
		default:
			// Unhandled format.
			gnash::log_error("unknown format sound requested; this demo does not handle it\n");
			break;
		}

		m_sound_data.push_back(sounddata);

		return m_sound_data.size()-1;
	}


	// this gets called when a stream gets more data
	virtual long	fill_stream_data(void* data, int data_bytes, int handle_id)
	{
		
		// @@ does a negative handle_id have any meaning ?
		//    should we change it to unsigned instead ?
		if (handle_id >= 0 && (unsigned int) handle_id < m_sound_data.size())
		{

			// Reallocate the required memory.
			guint8* tmp_data = new guint8[data_bytes + m_sound_data[handle_id]->data_size];
			memcpy(tmp_data, m_sound_data[handle_id]->data, m_sound_data[handle_id]->data_size);
			memcpy(tmp_data + m_sound_data[handle_id]->data_size, data, data_bytes);
			delete [] m_sound_data[handle_id]->data;
			m_sound_data[handle_id]->data = tmp_data;
			
			m_sound_data[handle_id]->data_size += data_bytes;

			return m_sound_data[handle_id]->data_size - data_bytes;
		}
		return 0;
		// FIXME: if the playback of the stream has already started we'll need to update the struct


	}



	// The callback function which refills the buffer with data
	static void callback_handoff (GstElement * /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data)
	{
		gst_elements *gstelements = static_cast<gst_elements*>(user_data);

		// First callback
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

		// This shouldn't happen
		if (gstelements->position > gstelements->data_size) {
			return;
		}

		// Last callback - the last re-fill
		if (gstelements->position+BUFFER_SIZE > gstelements->data_size) {
			// Check if we should loop. If loop_count is 0 we have we just
			// played the sound for the last (and perhaps first) time.
			// If loop_count is anything else we continue to loop.
			if (gstelements->loop_count == 0) {
				GST_BUFFER_SIZE(buffer) = gstelements->data_size-gstelements->position;
				memcpy(GST_BUFFER_DATA(buffer), static_cast<guint8*>(gstelements->data+gstelements->position), gstelements->data_size-gstelements->position);
				gstelements->position += BUFFER_SIZE;

				gst_element_set_state (GST_ELEMENT (gstelements->input), GST_STATE_PAUSED);

			} else {
				// Copy what's left of the data, and then fill the rest with "new" data.
				//int chunck_size = (gstelements->data_size-gstelements->position);
				memcpy(GST_BUFFER_DATA(buffer), static_cast<guint8*>(gstelements->data+gstelements->position),  (gstelements->data_size-gstelements->position));
				memcpy(GST_BUFFER_DATA(buffer) + (gstelements->data_size-gstelements->position), static_cast<guint8*>(gstelements->data), GST_BUFFER_SIZE(buffer)- (gstelements->data_size-gstelements->position));
				gstelements->position = GST_BUFFER_SIZE(buffer)- (gstelements->data_size-gstelements->position);
				gstelements->loop_count--;

			}

			return;

		}

		// Standard re-fill
		memcpy(GST_BUFFER_DATA(buffer), static_cast<guint8*>(gstelements->data+gstelements->position), BUFFER_SIZE);
		gstelements->position += BUFFER_SIZE;

	}


	/*static void event_callback (GstPad*, GstMiniObject *o, gpointer user_data)
	{

		GstEvent *event = (GstEvent*) o;
		if (GST_EVENT_TYPE (event) == GST_EVENT_EOS)
		{
			// Find the instance of this sound which needs to be deleted
			sound_data *sounddata = (sound_data*) user_data;

#if 0 // this loop does nothing, right ?
			for (size_t i=0, n=m_gst_elements.size(); i<n; ++i)
			{
				if (m_gst_elements.at(i)->position > m_gst_elements.at(i)->data_size)
			}
#endif
			printf("EOS detected! :D\n");
			gst_object_unref (GST_OBJECT (
		}

	}*/


	virtual void	play_sound(int sound_handle, int loop_count, int /*offset*/, long start_position)
	// Play the index'd sample.
	{

		// Check if the sound exists.
		if (sound_handle < 0 || (unsigned int) sound_handle >= m_sound_data.size())
		{
			// Invalid handle.
			return;
		}
		
		// If this is called from a streamsoundblocktag, we only start if this
		// sound isn't already playing. If a gst_element-struct is existing we
		// assume it is also playing.
		if (start_position > 0 && m_sound_data[sound_handle]->m_gst_elements.size() > 0) {
			return;
		}

		// Make a "gst_elements" for this sound which is latter placed on the vector of instances of this sound being played
		gst_elements* gst_element = new gst_elements;
		if (gst_element == NULL) {
			gnash::log_error ("could not allocate memory for gst_element !\n");
			return;
		}
		// Copy data-info to the "gst_elements"
		gst_element->data_size = m_sound_data[sound_handle]->data_size;
		gst_element->data = m_sound_data[sound_handle]->data;
		gst_element->position = start_position;

		// Set number of loop we should do. -1 is infinte loop, 0 plays it once, 1 twice etc.
		gst_element->loop_count = loop_count;

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

			gnash::log_error("Gstreamer element for audio handling could not be created\n");
			return;
		}

		// Create a gstreamer decoder for the chosen sound.

		if (m_sound_data[sound_handle]->format == FORMAT_MP3) { // || sound_data[m_sound_handle]->format == FORMAT_VORBIS) {

			gst_element->decoder = gst_element_factory_make ("mad", NULL);
			if (gst_element->decoder == NULL) gst_element->decoder = gst_element_factory_make ("ffdec_mp3", NULL);
			if (gst_element->decoder == NULL) gst_element->decoder = gst_element_factory_make ("flump3dec", NULL);

			// Check if the element was correctly created
			if (!gst_element->decoder) {
				gnash::log_error("A gstreamer mp3-decoder element could not be created\n");
				return;
			}
			gst_bin_add (GST_BIN (gst_element->bin), gst_element->decoder);

			// Set the info about the stream so that gstreamer knows what it is.
			GstCaps *caps = gst_caps_new_simple ("audio/mpeg",
				"mpegversion", G_TYPE_INT, 1,
				"layer", G_TYPE_INT, 3,
				"rate", G_TYPE_INT, m_sound_data[sound_handle]->sample_rate,
				"channels", G_TYPE_INT, m_sound_data[sound_handle]->stereo ? 2 : 1, NULL);
			g_object_set (G_OBJECT (gst_element->capsfilter), "caps", caps, NULL);
			gst_caps_unref (caps);

			// number of buffers to send
			int numBuf = static_cast<int>(ceil(static_cast<float>(m_sound_data[sound_handle]->data_size) / static_cast<float>(BUFFER_SIZE)));
			if (loop_count == -1) {
				numBuf = -1;
			} else if (loop_count > 0) {
				numBuf = numBuf * (loop_count+1) -1;
			}

			// setup fake source
			g_object_set (G_OBJECT (gst_element->input),
						"sizetype", 2, "can-activate-pull", FALSE, "signal-handoffs", TRUE,
						"sizemax", BUFFER_SIZE, "num-buffers", numBuf, NULL);
			// Setup the callback
			g_signal_connect (gst_element->input, "handoff", G_CALLBACK (callback_handoff), gst_element);

			// link data, decoder, audio* and adder
			gst_element_link_many (gst_element->input,
							gst_element->capsfilter,
							gst_element->decoder,
							gst_element->audioconvert,
							gst_element->audioresample, 
							gst_element->volume, NULL);

		} else if (m_sound_data[sound_handle]->format == FORMAT_NATIVE16) {

			// Set the info about the stream so that gstreamer knows what it is.
			GstCaps *caps = gst_caps_new_simple ("audio/x-raw-int",
				"rate", G_TYPE_INT, m_sound_data[sound_handle]->sample_rate,
				"channels", G_TYPE_INT, m_sound_data[sound_handle]->stereo ? 2 : 1,
				"endianness", G_TYPE_INT, G_BIG_ENDIAN,
				"width", G_TYPE_INT, 16,
				/*"signed", G_TYPE_INT, 1,*/ NULL);
			g_object_set (G_OBJECT (gst_element->capsfilter), "caps", caps, NULL);
			gst_caps_unref (caps);

			// number of buffers to send
			int numBuf = static_cast<int>(ceil(static_cast<float>(m_sound_data[sound_handle]->data_size) / static_cast<float>(BUFFER_SIZE)));
			if (loop_count == -1) {
				numBuf = -1;
			} else if (loop_count > 0) {
				numBuf = numBuf * (loop_count+1) -1;
			}
			// setup fake source
			g_object_set (G_OBJECT (gst_element->input),
						"sizetype", 2, "can-activate-pull", FALSE, "signal-handoffs", TRUE,
						"sizemax", BUFFER_SIZE, "num-buffers", numBuf, NULL);
			// Setup the callback
			g_signal_connect (gst_element->input, "handoff", G_CALLBACK (callback_handoff), gst_element);

/*	caps info:
      audio/x-raw-int
                   rate: [ 1, 2147483647 ]
               channels: [ 1, 8 ]
             endianness: { 1234, 4321 }
                  width: 16
                  depth: [ 1, 16 ]
                 signed: { true, false }*/
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
		gst_bin_add(GST_BIN (pipeline), gst_element->bin);
		gst_element_link(gst_element->bin,adder);
		
		// Set the volume
		g_object_set (G_OBJECT (gst_element->volume), "volume", static_cast<double>(m_sound_data[sound_handle]->volume) / 100.0, NULL);

		//gst_pad_add_event_probe(pad, G_CALLBACK(event_callback), m_sound_data[sound_handle]);

		// Put the gst_element on the vector
		m_sound_data[sound_handle]->m_gst_elements.push_back(gst_element);

		// If not already playing, start doing it
		gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);

		++soundsPlaying;
	}


	virtual void	stop_sound(int sound_handle)
	{
		
		// Check if the sound exists.
		if (sound_handle < 0 || (unsigned int) sound_handle >= m_sound_data.size())
		{
			// Invalid handle.
			return;
		}

		sound_data* sounddata = m_sound_data[sound_handle];

		// This variable is used to asure that we don't try to pause
		// if nothing is playing, which would mess things up
		bool stopped = false;

		// Stop all the instances of this sound.
		// TODO: fix the loop to use size_t instead of i
		for (int i = sounddata->m_gst_elements.size()-1; i >= 0 ; i--)
		{
			gst_elements* elements = sounddata->m_gst_elements[i];

			// Check if we can succesfully stop the elements
			// playback - if not we skip cleaning this for now
			// FIXME: what if it ain't possible to stop an element when this is called from ~GST_sound_handler

			// Unlink the elements
			gst_element_unlink_many (elements->bin, adder, NULL);

			// FIXME: This stops ALL sounds, not just the current.
			if (gst_element_set_state (GST_ELEMENT (elements->bin), GST_STATE_NULL) != 1) continue;


			// Unref/delete the elements
			gst_object_unref (GST_OBJECT (elements->bin));


			// Delete the gst_element struct
			// @@ we're deleting the elements from the start, so half-way of the loop we will be referring to undefined elements. Is this intended ? --strk;
			sounddata->m_gst_elements.erase(sounddata->m_gst_elements.begin() + i);
			--soundsPlaying;
			stopped = true;
		}
		
			
		// If no sounds, set pipeline to paused. Else the pipeline thinks it's still playing,
		// and will fastforward through new sounds until it reach the "correct posistion".
		if (soundsPlaying == 0 && stopped) {
			gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PAUSED);
		}
		
	}


	virtual void	delete_sound(int sound_handle)
	// this gets called when it's done with a sample.
	{
		
		if (sound_handle >= 0 && (unsigned int) sound_handle < m_sound_data.size())
		{
			free (m_sound_data[sound_handle]->data);
			m_sound_data[sound_handle]->data = 0;
		}

	}

	// This will stop all sounds playing. Will cause problems if the soundhandler is made static
	// and supplys sound_handling for many SWF's, since it will stop all sounds with no regard
	// for what sounds is associated with what SWF.
	virtual void	stop_all_sounds()
	{
		for (size_t i = m_sound_data.size(); i > 0; i--) //Optimized
			stop_sound(i);
	}


	//	returns the sound volume level as an integer from 0 to 100,
	//	where 0 is off and 100 is full volume. The default setting is 100.
	virtual int	get_volume(int sound_handle) {
	
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
	virtual void	set_volume(int sound_handle, int volume) {

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


};


gnash::sound_handler*	gnash::create_sound_handler_gst()
// Factory.
{
	return new GST_sound_handler;
}


#endif

// Local Variables:
// mode: C++
// End:

