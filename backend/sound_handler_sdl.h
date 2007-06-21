//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// $Id: sound_handler_sdl.h,v 1.26 2007/06/21 09:02:06 tgc Exp $

#ifndef SOUND_HANDLER_SDL_H
#define SOUND_HANDLER_SDL_H


#include "sound_handler.h" // for inheritance
#include "hash_wrapper.h"

#include "log.h"

#ifdef USE_FFMPEG
#include <ffmpeg/avcodec.h>
#elif defined(USE_MAD)
#include <mad.h>
#endif

#include <vector>

#include <SDL_audio.h>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>

/// Used to hold the info about active sounds
class active_sound
{
public:
#ifdef USE_FFMPEG
	/// ffmpeg stuff
	AVCodec *codec;
	AVCodecContext *cc;
	AVCodecParserContext* parser;
#elif defined(USE_MAD)
	/// mad stuff
	mad_stream	stream;
	mad_frame	frame;
	mad_synth 	synth;
#endif

	/// The size of the undecoded data
	unsigned long data_size;

	/// Current decoding position in the stream
	unsigned long position;

	/// Size of the decoded data
	unsigned long raw_data_size;

	/// Current playing position in the decoded stream
	unsigned long raw_position;

	/// Numbers of loops: -1 means loop forever, 0 means play once.
	/// For every loop completed, it is decremented.
	long loop_count;

	/// Offset to make playback start in-sync, only used with mp3 streams.
	unsigned int offset;

	/// Sound envelopes for the current sound, which determine the volume level
	/// from a given position. Only used with sound events.
	const std::vector<gnash::sound_handler::sound_envelope>* envelopes;

	/// Index of current envelope.
	uint32_t current_env;

	/// Number of samples played so far.
	unsigned long samples_played;

	/// Returns the data pointer in the undecoded datastream
	/// for the given position. Boundaries are checked.
	uint8_t* get_data_ptr(unsigned long int pos);

	/// Returns the data pointer in the decoded datastream
	/// for the given position. Boundaries are checked.
	uint8_t* get_raw_data_ptr(unsigned long int pos);

	/// Set the undecoded data pointer
	void set_data(uint8_t*);

	/// Set the decoded data pointer
	void set_raw_data(uint8_t*);

	/// Deletes the decoded data
	void delete_raw_data();

private:
	/// The undecoded data
	uint8_t* data;

	/// The decoded data
	uint8_t* raw_data;

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

};


// Use SDL and ffmpeg/mad/nothing to handle sounds.
class SDL_sound_handler : public gnash::sound_handler
{
private:
	/// NetStream audio callbacks
	hash_wrapper< void* /* owner */, aux_streamer_ptr /* callback */> m_aux_streamer;	//vv

	/// Vector containing all sounds.
	std::vector<sound_data*>	m_sound_data;

	/// Is sound device opened?
	bool soundOpened;

	/// The SDL_audio specs
	SDL_AudioSpec audioSpec;
	
	/// Keeps track of numbers of playing sounds
	int soundsPlaying;

	/// Is the audio muted?
	bool muted;
	
	/// Mutex for making sure threads doesn't mess things up
	boost::mutex _mutex;

public:
	SDL_sound_handler();
	virtual ~SDL_sound_handler();

	/// Called to create a sound.
	virtual int	create_sound(void* data, int data_bytes,
				     int sample_count, format_type format,
				     int sample_rate, bool stereo);

	/// this gets called when a stream gets more data
	virtual long	fill_stream_data(void* data, int data_bytes,
					 int sample_count, int handle_id);

	/// Play the index'd sample.
	virtual void	play_sound(int sound_handle, int loop_count, int offset,
				   long start_position, const std::vector<sound_envelope>* envelopes);

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

	/// Callback invoked by the SDL audio thread.
	//
	/// Refills the output stream/buffer with data.
	///
	/// We run trough all the attached auxiliary streamers fetching decoded
	/// audio blocks and mixing them into the given output stream.
	///
	/// If sound is compresssed (mp3) a mp3-frame is decoded into a buffer,
	/// and resampled if needed. When the buffer has been sampled, another
	/// frame is decoded until all frames has been decoded.
	/// If a sound is looping it will be decoded from the beginning again.
	///
	/// TODO: make a static method of the SDL_sound_handler class
	///
	/// @param udata
	///	User data pointer (SDL_sound_handler instance in our case).
	///	We'll lock the SDL_sound_handler::_mutex during operations.
	///
	/// @param stream
	/// 	The output stream/buffer to fill
	///
	/// @param buffer_length_in
	///	Length of the buffer.
	///	If zero or negative we log an error and return
	///	(negative is probably an SDL bug, zero dunno yet).
	///
	static void sdl_audio_callback (void *udata, Uint8 *stream, int buffer_length_in);
};


#endif // SOUND_HANDLER_SDL_H
