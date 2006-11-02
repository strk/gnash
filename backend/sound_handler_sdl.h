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

#ifndef SOUND_HANDLER_SDL_H
#define SOUND_HANDLER_SDL_H


#ifdef USE_FFMPEG
#include <ffmpeg/avcodec.h>
#elif defined(USE_MAD)
#include <mad.h>
#endif

#include "gnash.h"
#include <vector>

#include <SDL_audio.h>
#include <pthread.h>

// Used to hold the info about active sounds
typedef struct
{
#ifdef USE_FFMPEG
	// ffmpeg stuff
	AVCodec *codec;
	AVCodecContext *cc;
	AVCodecParserContext* parser;
#elif defined(USE_MAD)
	// mad stuff
	mad_stream	stream;
	mad_frame	frame;
	mad_synth 	synth;
#endif

	// data size
	unsigned long data_size;

	// position in the stream
	unsigned long position;

	// The compressed data
	uint8_t* data;

	// data size
	unsigned long raw_data_size;

	// position in the raw stream
	unsigned long raw_position;

	// The decompressed data
	uint8_t* raw_data;

	/// Numbers of loops: -1 means loop forever, 0 means play once
	long loop_count;

	// Offset, only used with mp3 streams
	unsigned int offset;

	// Envelopes
	std::vector<gnash::sound_handler::sound_envelope>* envelopes;

	// Current envelope
	uint32_t current_env;

	// Number if samples played
	unsigned long samples_played;
	
} active_sound;


// Used to hold the sounddata when doing on-demand-decoding
typedef struct
{
	// The (un)compressed data
	uint8_t* data;

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

	// Volume for AS-sounds, range: 0-100 
	// It's the SWF range that is represented here
	int volume;

	// active sounds being playes
	std::vector<active_sound*>	m_active_sounds;

} sound_data;


// Use SDL and ffmpeg/mad/nothing to handle sounds.
struct SDL_sound_handler : public gnash::sound_handler
{
	// NetStream audio callbacks
	gnash::hash< void* /* owner */, aux_streamer_ptr /* callback */> m_aux_streamer;	//vv

	// Sound data.
	std::vector<sound_data*>	m_sound_data;

	// Is sound device opened?
	bool soundOpened;

	// SDL_audio specs
	SDL_AudioSpec audioSpec;
	
	// Keeps track of numbers of playing sounds
	int soundsPlaying;

	// Is the audio muted?
	bool muted;
	
	// mutex for making sure threads doesn't mess things up
	pthread_mutex_t mutex;

	SDL_sound_handler();
	virtual ~SDL_sound_handler();

	// Called to create a sample.
	virtual int	create_sound(void* data, int data_bytes,
				     int sample_count, format_type format,
				     int sample_rate, bool stereo);

	// this gets called when a stream gets more data
	virtual long	fill_stream_data(void* data, int data_bytes,
					 int sample_count, int handle_id);

	// Play the index'd sample.
	virtual void	play_sound(int sound_handle, int loop_count, int offset,
				   long start_position, std::vector<sound_envelope>* envelopes);

	virtual void	stop_sound(int sound_handle);

	// this gets called when it's done with a sample.
	virtual void	delete_sound(int sound_handle);

	// this will stop all sounds playing.
	virtual void	stop_all_sounds();

	// returns the sound volume level as an integer from 0 to 100.
	virtual int	get_volume(int sound_handle);

	virtual void	set_volume(int sound_handle, int volume);
		
	virtual void	get_info(int sound_handle, int* format, bool* stereo);

	// gnash calls this to mute audio
	virtual void	mute();

	// gnash calls this to unmute audio
	virtual void	unmute();

	virtual bool	is_muted();

	virtual void	attach_aux_streamer(aux_streamer_ptr ptr, void* owner);	//vv
	virtual void	detach_aux_streamer(void* owner);	//vv

	// Converts input data to the SDL output format.
	virtual void	convert_raw_data(int16_t** adjusted_data,
			  int* adjusted_size, void* data, int sample_count,
			  int sample_size, int sample_rate, bool stereo);	//vv

};


#endif // SOUND_HANDLER_SDL_H
