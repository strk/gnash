//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// $Id: sound_handler_gst.h,v 1.9 2008/01/21 23:10:15 rsavoye Exp $

#ifndef SOUND_HANDLER_GST_H
#define SOUND_HANDLER_GST_H

//#include "gnash.h"
#include "sound_handler.h" // for inheritance

#include <vector>
#include <map>

#include <gst/gst.h>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>

#define BUFFER_SIZE 5000

namespace gnash {
namespace media {

// forward declaration
class GST_sound_handler;

// Used to hold the gstreamer when doing on-demand-decoding
class gst_elements
{
public:
	// gstreamer pipeline objects

	// the main bin containing the adder and output (sink)
	GstElement *pipeline;
	GstElement *audiosink;

	// gstreamer objects
	GstElement *input;
	GstElement *decoder;
	GstElement *capsfilter;
	GstElement *audioconvert;
	GstElement *audioresample;
	GstElement *volume;
	GstElement *bin;
	GstPad     *addersinkpad;
	
	// position in the stream
	unsigned long position;

	// data size
	unsigned long data_size;

	long loop_count;
	
	// signal id
	guint handoff_signal_id;

	// The sound handler. Used to get access to the GST_sound_handler->_mutex
	GST_sound_handler* handler;

	/// Returns the data pointer in the undecoded datastream
	/// for the given position. Boundaries are checked.
	const boost::uint8_t* get_data_ptr(unsigned long int pos);

	/// Set the undecoded data pointer
	void set_data(const boost::uint8_t*);

	/// The owner of the audio data this element will get data from.
	/// Only used when getting data from NetStream or Sound.
	void* owner;

private:
	// The (un)compressed data
	const guint8* data;

};


// Used to hold the sounddata when doing on-demand-decoding
class sound_data
{
	// The (un)compressed data
	Buffer _data;

public:

	sound_data()
	{}

	/// Append size bytes to this sound
	//
	/// @param data
	///	Data bytes, allocated with new[]. Ownership transferred.
	///
	/// @param size
	///	Size of the 'data' buffer.
	///
	void append(unsigned char* data, unsigned int size)
	{
		_data.append(data, size);
	}

	/// Return data size
	size_t dataSize() const { return _data.size(); }

	/// Return data buffer
	const boost::uint8_t* data() { return _data.data(); }

	// Object holding information about the sound
	std::auto_ptr<SoundInfo> soundinfo;

	// Volume, SWF range: 0-100, GST range 0-10 (we only use 0-1, the rest is amplified)
	// It's the SWF range that is represented here
	int volume;

	// gstreamer objects
	std::vector<gst_elements*>	m_gst_elements;

};

// Use gstreamer to handle sounds.
class GST_sound_handler : public sound_handler
{
private:
	/// AS classes (NetStream, Sound) audio callbacks
	typedef std::map< void* /* owner */, aux_streamer_ptr /* callback */> CallbacksMap;
	CallbacksMap m_aux_streamer;

	typedef std::map< void* /* owner */, gst_elements*> GstElementsMap;
	GstElementsMap m_aux_streamer_gstelements;

	/// Vector containing all the sounds
	std::vector<sound_data*>	m_sound_data;
	
	/// Is the loop running?
	bool looping;
	
	/// Is the audio muted?
	bool muted;

	/// Mutex for making sure threads doesn't mess things up
	boost::try_mutex _mutex;

	/// stop and delete all sounds
	void delete_all_sounds();

public:

	/// Gstreamer callback function
	static void callback_handoff (GstElement * /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data);

	/// Gstreamer callback function when using ActionScript audio source (NetStream, Sound)
	static void callback_as_handoff (GstElement * /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data);

	GST_sound_handler();
	virtual ~GST_sound_handler();

	/// Called to create a sound.
	virtual int	create_sound(void* data, unsigned int data_bytes,
				     std::auto_ptr<SoundInfo> sinfo);

	/// this gets called when a stream gets more data
	virtual long	fill_stream_data(unsigned char* data, unsigned int data_bytes,
					 unsigned int sample_count, int handle_id);

	/// Play the index'd sample.
	virtual void	play_sound(int sound_handle, int loop_count, int offset,
				   long start_position, const std::vector<sound_envelope>* envelopes);

	/// Stop the index'd sample.
	virtual void	stop_sound(int sound_handle);

	/// This gets called when it's done with a sample.
	virtual void	delete_sound(int sound_handle);

	/// This will stop all sounds playing.
	virtual void	stop_all_sounds();

	// See dox in sound_handler.h
	virtual void reset();

	/// Returns the sound volume level as an integer from 0 to 100. AS-script only.
	virtual int	get_volume(int sound_handle);

	/// Sets the sound volume level as an integer from 0 to 100. AS-script only.
	virtual void	set_volume(int sound_handle, int volume);
		
	/// Gnash uses this to get info about a sound. Used when a stream needs more data.
	virtual SoundInfo* get_sound_info(int sound_handle);

	/// Gnash calls this to mute audio.
	virtual void	mute();

	/// Gnash calls this to unmute audio.
	virtual void	unmute();

	/// Gnash calls this to get the mute state.
	virtual bool	is_muted();

	/// Gets the duration in milliseconds of an event sound connected to an AS Sound obejct.
	virtual unsigned int get_duration(int sound_handle);

	/// Gets the playhead position in milliseconds of an event sound connected to an AS Soound obejct.
	virtual unsigned int get_position(int sound_handle);

	virtual void	attach_aux_streamer(aux_streamer_ptr ptr, void* owner);
	virtual void	detach_aux_streamer(void* owner);
	
};

} // gnash.media namespace 
} // namespace gnash

#endif // SOUND_HANDLER_GST_H

