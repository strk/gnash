//   Copyright (C) 2007 Free Software Foundation, Inc.
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

// $Id: sound_handler_test.h,v 1.1 2007/02/20 16:37:19 tgc Exp $

#ifndef SOUND_HANDLER_TEST_H
#define SOUND_HANDLER_TEST_H

#include "gnash.h"
#include "hash_wrapper.h"

#include <vector>

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>

/// Used to hold the info about active sounds
class active_sound
{
public:

	/// The size of the undecoded data
	unsigned long data_size;

	/// Current decoding position in the stream
	unsigned long position;

	/// Numbers of loops: -1 means loop forever, 0 means play once.
	/// For every loop completed, it is decremented.
	long loop_count;

	/// Offset to make playback start in-sync, only used with mp3 streams.
	unsigned int offset;

	/// Sound envelopes for the current sound, which determine the volume level
	/// from a given position. Only used with sound events.
	std::vector<gnash::sound_handler::sound_envelope>* envelopes;

	/// Index of current envelope.
	uint32_t current_env;

	/// Number of samples played so far.
	unsigned long samples_played;

	/// Set the undecoded data pointer
	void set_data(uint8_t*);

private:
	/// The undecoded data
	uint8_t* data;

};


/// Used to hold the sounddata when doing on-demand-decoding
class sound_data
{
public:
	/// The undecoded data
	uint8_t* data;

	/// Format of the sound (MP3, raw, etc).
	int format;

	/// The size of the undecoded data
	long data_size;

	/// Stereo or not
	bool stereo;

	/// Number of samples
	int sample_count;

	/// Sample rate
	int sample_rate;

	/// Volume for AS-sounds, range: 0-100.
	/// It's the SWF range that is represented here.
	int volume;

	/// Vector containing the active instances of this sounds being played
	std::vector<active_sound*>	m_active_sounds;

	/// Test information
	int started;
	int stopped;

};


// Handle sounds for test purposes
class TEST_sound_handler : public gnash::sound_handler
{
public:
	/// NetStream audio callbacks
	hash_wrapper< void* /* owner */, aux_streamer_ptr /* callback */> m_aux_streamer;	//vv

	/// Vector containing all sounds.
	std::vector<sound_data*>	m_sound_data;

	/// Keeps track of numbers of playing sounds
	int soundsPlaying;

	/// Is the audio muted?
	bool muted;
	
	/// Mutex for making sure threads doesn't mess things up
	boost::mutex _mutex;

	/// Test information
	int started_all;
	int stopped_all;

	TEST_sound_handler();
	virtual ~TEST_sound_handler();

	/// Called to create a sound.
	virtual int	create_sound(void* data, int data_bytes,
				     int sample_count, format_type format,
				     int sample_rate, bool stereo);

	/// this gets called when a stream gets more data
	virtual long	fill_stream_data(void* data, int data_bytes,
					 int sample_count, int handle_id);

	/// Play the index'd sample.
	virtual void	play_sound(int sound_handle, int loop_count, int offset,
				   long start_position, std::vector<sound_envelope>* envelopes);

	/// Stop the index'd sample.
	virtual void	stop_sound(int sound_handle);

	/// This gets called when it's done with a sample.
	virtual void	delete_sound(int sound_handle);

	/// This will stop all sounds playing.
	virtual void	stop_all_sounds();

	/// Returns the sound volume level as an integer from 0 to 100. AS-script only.
	virtual int	get_volume(int sound_handle);

	/// Sets the sound volume level as an integer from 0 to 100. AS-script only.
	virtual void	set_volume(int sound_handle, int volume);
		
	/// Gnash uses this to get info about a sound. Used when a stream needs more data.
	virtual void	get_info(int sound_handle, int* format, bool* stereo);

	/// Gnash calls this to mute audio.
	virtual void	mute();

	/// Gnash calls this to unmute audio.
	virtual void	unmute();

	/// Gnash calls this to get the mute state.
	virtual bool	is_muted();

	virtual void	attach_aux_streamer(aux_streamer_ptr ptr, void* owner);	//vv
	virtual void	detach_aux_streamer(void* owner);	//vv

	/// Converts input data to the SDL output format.
	virtual void	convert_raw_data(int16_t** adjusted_data,
			  int* adjusted_size, void* data, int sample_count,
			  int sample_size, int sample_rate, bool stereo);	//vv

	/// Special test-fuction. Reports how many times this sound has been started
	int test_times_started(int sound_handle);

	/// Special test-fuction. Reports how many times this sound has been stopped
	int test_times_stopped(int sound_handle);

	/// Special test-fuction. Reports how many times a sound has been started
	int test_times_started_all();

	/// Special test-fuction. Reports how many times a sound has been stopped
	int test_times_stopped_all();

};


#endif // SOUND_HANDLER_TEST_H
