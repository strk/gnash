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


/*  TODO:
	* dealloker elementer efter brug (sinkXX stiger: 1go_menu.swf) brug bus_callback'en? event_probe!
	* hvordan fikser vi traaden? stopper vi loop, eller pauser vi pipelinen? Begge dele! :)
	* streams
	* Not really a todo... ATM Gstreamer can't handle multiple elements trying to cennect at the same time.
*/


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_GST_GST_H
#include "gnash.h"
#include "container.h"
#include "log.h"
#include "types.h"	// for IF_VERBOSE_* macros
#include <pthread.h>
#include <math.h>

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
struct GST_sound_handler : gnash::sound_handler
{
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
	
	// latest sound stream we've created - we expect some data to arrive
	int currentStream;
	
	GST_sound_handler()
	{
		// init gstreamer
		gst_init(NULL, NULL);

		// create main pipeline
		pipeline = gst_pipeline_new (NULL);

		// create adder
		adder = gst_element_factory_make ("adder", NULL);

		// create an audio sink - use oss, alsa or...? make a commandline option?
		// we first try alsa, then oss, then esd, then...?
		audiosink = gst_element_factory_make ("alsasink", NULL);
		if (!audiosink) audiosink = gst_element_factory_make ("osssink", NULL);
		if (!audiosink) audiosink = gst_element_factory_make ("esdsink", NULL);

		// Check if the creation of the gstreamer pipeline, adder and audiosink was a succes
		if (!pipeline || !adder || !audiosink) {
			gnash::log_error("One gstreamer element could not be created\n");
		}

		// link adder and output to bin
		gst_bin_add (GST_BIN (pipeline), adder);
		gst_bin_add (GST_BIN (pipeline), audiosink);

		// link adder and audiosink
		gst_element_link (adder, audiosink);

		soundsPlaying = 0;
		
		looping = false;
		
	}

	~GST_sound_handler()
	{

		for (int i = 0; i < m_sound_data.size(); i++) {
			stop_sound(i);
			delete_sound(i);
		}
		m_sound_data.clear();

		gst_object_unref (GST_OBJECT (pipeline));

	}


	virtual int	create_sound(
		void* data,
		int data_bytes,
		int sample_count,
		format_type format,
		int sample_rate,
		bool stereo,
		bool stream)
	// Called to create a sample.  We'll return a sample ID that
	// can be use for playing it.
	{
		// Add something similar... check gst elements?
		/*if (m_opened == false)
		{
			return 0;
		}*/

		int16_t* adjusted_data = 0;
		int	adjusted_size = 0;

		sound_data *sounddata = new sound_data;
		if (sounddata == NULL) {
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
			/*convert_raw_data(&adjusted_data, &adjusted_size, data, sample_count, 1, sample_rate, stereo);
			sounddata->data = (guint8*) malloc(adjusted_size);
			memcpy(sounddata->data, adjusted_data, adjusted_size);*/

			sounddata->data = (guint8*) malloc(data_bytes);
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
			/*convert_raw_data(&adjusted_data, &adjusted_size, data, sample_count, 2, sample_rate, stereo);
			sounddata->data = (guint8*) malloc(adjusted_size);
			memcpy(sounddata->data, adjusted_data, adjusted_size);*/
			
			sounddata->data = (guint8*) malloc(data_bytes);
			memcpy(sounddata->data, data, data_bytes);
			break;

		case FORMAT_MP3:
		//case FORMAT_VORBIS:
			sounddata->data = (guint8*) malloc(data_bytes);
			memcpy(sounddata->data, data, data_bytes);

			break;
		default:
			// Unhandled format.
			gnash::log_error("unknown format sound requested; this demo does not handle it\n");
			break;
		}

		m_sound_data.push_back(sounddata);

		
		if (stream) currentStream = m_sound_data.size()-1;

		return m_sound_data.size()-1;
	}


	// this gets called when a stream gets more data
	virtual void	fill_stream_data(void* data, int data_bytes)
	{
		
		if (currentStream >= 0 && currentStream < m_sound_data.size())
		{
			m_sound_data[currentStream]->data = (guint8*) realloc(m_sound_data[currentStream]->data, data_bytes + m_sound_data[currentStream]->data_size);
			memcpy(m_sound_data[currentStream]->data + m_sound_data[currentStream]->data_size, data, data_bytes);
			m_sound_data[currentStream]->data_size += data_bytes;
		}
		// FIXME: if the playback of the stream has already started we'll need to update the struct


	}



	// The callback function which refills the buffer with data
	static void callback_handoff (GstElement *c, GstBuffer *buffer, GstPad  *pad, gpointer user_data)
	{
		gst_elements *gstelements = (gst_elements*) user_data;

		// First callback
		if (GST_BUFFER_SIZE(buffer) == 0) {
			if (gstelements->data_size > BUFFER_SIZE) {
				GST_BUFFER_SIZE(buffer) = BUFFER_SIZE;
			} else {
				GST_BUFFER_SIZE(buffer) = gstelements->data_size;
			}
			GST_BUFFER_DATA(buffer) = (guint8*) realloc(GST_BUFFER_DATA(buffer),GST_BUFFER_SIZE(buffer));
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
				memcpy(GST_BUFFER_DATA(buffer), (guint8*) gstelements->data+gstelements->position, gstelements->data_size-gstelements->position);
				gstelements->position += BUFFER_SIZE;

				gst_element_set_state (GST_ELEMENT (gstelements->input), GST_STATE_PAUSED);

			} else {
				// Copy what's left of the data, and then fill the rest with "new" data.
				//int chunck_size = (gstelements->data_size-gstelements->position);
				memcpy(GST_BUFFER_DATA(buffer), (guint8*) gstelements->data+gstelements->position,  (gstelements->data_size-gstelements->position));
				memcpy(GST_BUFFER_DATA(buffer)+ (gstelements->data_size-gstelements->position), (guint8*) gstelements->data, GST_BUFFER_SIZE(buffer)- (gstelements->data_size-gstelements->position));
				gstelements->position = GST_BUFFER_SIZE(buffer)- (gstelements->data_size-gstelements->position);
				gstelements->loop_count--;

			}

			return;

		}

		// Standard re-fill
		memcpy(GST_BUFFER_DATA(buffer), (guint8*)gstelements->data+gstelements->position, BUFFER_SIZE);
		gstelements->position += BUFFER_SIZE;

	}


	/*static void event_callback (GstPad*, GstMiniObject *o, gpointer user_data)
	{

		GstEvent *event = (GstEvent*) o;
		if (GST_EVENT_TYPE (event) == GST_EVENT_EOS) {
			// Find the instance of this sound which needs to be deleted
			sound_data *sounddata = (sound_data*) user_data;
			for (int i = 0; i < m_gst_elements.size(); i++) {
				if (m_gst_elements.at(i)->position > m_gst_elements.at(i)->data_size)
			}
			printf("EOS detected! :D\n");
			gst_object_unref (GST_OBJECT (
		}

	}*/


	virtual void	play_sound(int sound_handle, int loop_count, int offset)
	// Play the index'd sample.
	{

		// Check if the sound exists.
		if (sound_handle < 0 || sound_handle >= m_sound_data.size())
		{
			// Invalid handle.
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
		gst_element->position = 0;

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

			gnash::log_error("One gstreamer element could not be created\n");
			return;
		}

		// Create a gstreamer decoder for the chosen sound.

		if (m_sound_data[sound_handle]->format == FORMAT_MP3) { // || sound_data[m_sound_handle]->format == FORMAT_VORBIS) {
			gst_element->decoder = gst_element_factory_make ("mad", NULL);
			if (!gst_element->decoder) gst_element_factory_make ("ffdec_mp3", NULL);
			//if (!gst_element->decoder) gst_element_factory_make ("ffdec_mp3", NULL); what is the fluendo decoder called?

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
			int numBuf = (int)ceil((float)m_sound_data[sound_handle]->data_size / (float)BUFFER_SIZE);
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
			int numBuf = (int)ceil((float)m_sound_data[sound_handle]->data_size / (float)BUFFER_SIZE);
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
		g_object_set (G_OBJECT (gst_element->volume), "volume", (double)m_sound_data[sound_handle]->volume / 100.0, NULL);

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
		if (sound_handle < 0 || sound_handle >= m_sound_data.size())
		{
			// Invalid handle.
			return;
		}

		// This variable is used to asure that we don't try to pause
		// if nothing is playing, which would mess things up
		bool stopped = false;

		// Stop all the instances of this sound.
		for (int i = m_sound_data[sound_handle]->m_gst_elements.size()-1; i >= 0 ; i--) {
			// Check if we can succesfully stop the elements playback - if not we skip cleaning this for now
			// FIXME: what if it ain't possible to stop an element when this is called from ~GST_sound_handler

			// Unlink the elements
			gst_element_unlink_many (m_sound_data[sound_handle]->m_gst_elements[i]->bin, adder, NULL);

			// FIXME: This stops ALL sounds, not just the current.
			if (gst_element_set_state (GST_ELEMENT (m_sound_data[sound_handle]->m_gst_elements[i]->bin), GST_STATE_NULL) != 1) continue;


			// Unref/delete the elements
			gst_object_unref (GST_OBJECT (m_sound_data[sound_handle]->m_gst_elements[i]->bin));


			// Delete the gst_element struct
			m_sound_data[sound_handle]->m_gst_elements.erase(m_sound_data[sound_handle]->m_gst_elements.begin() + i);
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
		
		if (sound_handle >= 0 && sound_handle < m_sound_data.size())
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
		for (int i = 0; i < m_sound_data.size(); i++)
			stop_sound(i);


	}


	//	returns the sound volume level as an integer from 0 to 100,
	//	where 0 is off and 100 is full volume. The default setting is 100.
	virtual int	get_volume(int sound_handle) {
	
		// Check if the sound exists.
		if (sound_handle >= 0 && sound_handle < m_sound_data.size())
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
		if (sound_handle < 0 || sound_handle >= m_sound_data.size())
		{
			// Invalid handle.
			return;
		}

		// Set volume for this sound. Should this only apply to the active sounds?
		m_sound_data[sound_handle]->volume = volume;
		
		for (int i = 0; i < m_sound_data[sound_handle]->m_gst_elements.size(); i++) {
			g_object_set (G_OBJECT (m_sound_data[sound_handle]->m_gst_elements[i]->volume),
						"volume", volume/100, NULL);
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

