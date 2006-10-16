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

#include "sound_handler_sdl.h"

#include "container.h"
#include "log.h"
#include <pthread.h>
#include <cmath>
#include <vector>

#include <SDL/SDL.h>


void sdl_audio_callback(void *udata, Uint8 *stream, int len); // SDL C audio handler



SDL_sound_handler::SDL_sound_handler()
	: soundOpened(false),
		soundsPlaying(0)
{
	// Init mutex
	pthread_mutex_init(&mutex , NULL);

	// This is our sound settings
	audioSpec.freq = 44100;
	audioSpec.format = AUDIO_S16SYS; // AUDIO_S8 AUDIO_U8;
	audioSpec.channels = 2;
	audioSpec.callback = sdl_audio_callback;
	audioSpec.userdata = this;
	audioSpec.samples = 512;
}

SDL_sound_handler::~SDL_sound_handler()
{
	for (size_t i= m_sound_data.size(); i > 0; i--) {
		stop_sound(i);
		delete_sound(i);
	}
	if (soundOpened) SDL_CloseAudio();
	pthread_mutex_destroy(&mutex);
}


int	SDL_sound_handler::create_sound(
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

	int16_t*	adjusted_data = 0;
	int	adjusted_size = 0;

	pthread_mutex_lock(&mutex);

	switch (format)
	{
	case FORMAT_RAW:

		if (data_bytes > 0) {
			convert_raw_data(&adjusted_data, &adjusted_size, data, sample_count, 1, sample_rate, stereo);
			if (!adjusted_data) {
				gnash::log_error("Some kind of error with raw sound data\n");
				pthread_mutex_unlock(&mutex);
				return -1;
			}
			sounddata->data_size = adjusted_size;
			sounddata->data = (Uint8*) adjusted_data;
		}
		break;

	case FORMAT_NATIVE16:

		if (data_bytes > 0) {
			convert_raw_data(&adjusted_data, &adjusted_size, data, sample_count, 2, sample_rate, stereo);
			if (!adjusted_data) {
				gnash::log_error("Some kind of error with adpcm sound data\n");
				pthread_mutex_unlock(&mutex);
				return -1;
			}
			sounddata->data_size = adjusted_size;
			sounddata->data = (Uint8*) adjusted_data;
		}
		break;

	case FORMAT_MP3:
	//case FORMAT_VORBIS:
#ifndef USE_FFMPEG
#ifndef USE_MAD
		gnash::log_warning("gnash has not been compiled to handle mp3 audio\n");
		pthread_mutex_unlock(&mutex);
		return -1;
#endif
#endif
		sounddata->data = new Uint8[data_bytes];
		if (!sounddata->data) {
			gnash::log_error("could not allocate space for data in soundhandler\n");
			pthread_mutex_unlock(&mutex);
			return -1;
		}
		memcpy(sounddata->data, data, data_bytes);

		break;
	default:
		// Unhandled format.
		gnash::log_error("unknown format sound requested; gnash does not handle it\n");
		pthread_mutex_unlock(&mutex);
		return -1; // Unhandled format, set to NULL.
	}

	m_sound_data.push_back(sounddata);
	int sound_id = m_sound_data.size()-1;

	pthread_mutex_unlock(&mutex);

	return sound_id;

}

// this gets called when a stream gets more data
long	SDL_sound_handler::fill_stream_data(void* data, int data_bytes, int sample_count, int handle_id)
{

	pthread_mutex_lock(&mutex);
	// @@ does a negative handle_id have any meaning ?
	//    should we change it to unsigned instead ?
	if (handle_id < 0 || (unsigned int) handle_id+1 > m_sound_data.size()) {
		pthread_mutex_unlock(&mutex);
		return 1;
	}
	int start_size = 0;

	// Handling of the sound data
	if (m_sound_data[handle_id]->format == FORMAT_NATIVE16)
	{
		int16_t*	adjusted_data = 0;
		int	adjusted_size = 0;

		convert_raw_data(&adjusted_data, &adjusted_size, data, sample_count, 2, m_sound_data[handle_id]->sample_rate, m_sound_data[handle_id]->stereo);
		if (!adjusted_data || adjusted_size < 1) {
			gnash::log_error("Some kind of error with re-formating sound data\n");
			pthread_mutex_unlock(&mutex);
			return -1;
		}
		adjusted_data = (int16_t*)data;
		adjusted_size = data_bytes;

		// Reallocate the required memory.
		Uint8* tmp_data = new Uint8[adjusted_size + m_sound_data[handle_id]->data_size];
		memcpy(tmp_data, m_sound_data[handle_id]->data, m_sound_data[handle_id]->data_size);
		memcpy(tmp_data + m_sound_data[handle_id]->data_size, adjusted_data, adjusted_size);
		if (m_sound_data[handle_id]->data_size > 0) delete [] m_sound_data[handle_id]->data;
		m_sound_data[handle_id]->data = tmp_data;

		start_size = m_sound_data[handle_id]->data_size;
		m_sound_data[handle_id]->data_size += adjusted_size;

		for(uint32_t i=0; i < m_sound_data[handle_id]->m_active_sounds.size(); i++) {
			m_sound_data[handle_id]->m_active_sounds[i]->data = m_sound_data[handle_id]->data;
			m_sound_data[handle_id]->m_active_sounds[i]->data_size = m_sound_data[handle_id]->data_size;
			m_sound_data[handle_id]->m_active_sounds[i]->position = m_sound_data[handle_id]->data_size;
			m_sound_data[handle_id]->m_active_sounds[i]->raw_data = m_sound_data[handle_id]->data;
		}
	} else if (m_sound_data[handle_id]->format == FORMAT_MP3) {

		// Reallocate the required memory.
		Uint8* tmp_data = new Uint8[data_bytes + m_sound_data[handle_id]->data_size];
		memcpy(tmp_data, m_sound_data[handle_id]->data, m_sound_data[handle_id]->data_size);
		memcpy(tmp_data + m_sound_data[handle_id]->data_size, data, data_bytes);
		if (m_sound_data[handle_id]->data_size > 0) delete [] m_sound_data[handle_id]->data;
		m_sound_data[handle_id]->data = tmp_data;

		start_size = m_sound_data[handle_id]->data_size;
		m_sound_data[handle_id]->data_size += data_bytes;

		// If playback has already started, we also update the active sounds
		for(uint32_t i=0; i < m_sound_data[handle_id]->m_active_sounds.size(); i++) {
			m_sound_data[handle_id]->m_active_sounds[i]->data = m_sound_data[handle_id]->data;
			m_sound_data[handle_id]->m_active_sounds[i]->data_size = m_sound_data[handle_id]->data_size;
#ifdef USE_MAD
			uint64_t this_frame = (const Uint8*)m_sound_data[handle_id]->m_active_sounds[i]->stream.this_frame - m_sound_data[handle_id]->m_active_sounds[i]->data;
			uint64_t next_frame = (const Uint8*)m_sound_data[handle_id]->m_active_sounds[i]->stream.next_frame - m_sound_data[handle_id]->m_active_sounds[i]->data;
			mad_stream_buffer(&m_sound_data[handle_id]->m_active_sounds[i]->stream, m_sound_data[handle_id]->data, m_sound_data[handle_id]->data_size);
			m_sound_data[handle_id]->m_active_sounds[i]->stream.this_frame += this_frame;
			m_sound_data[handle_id]->m_active_sounds[i]->stream.next_frame += next_frame;
#endif
		}

	} else {
		gnash::log_error("Behavior for this codec is unknown. Please send this SWF to the developers\n");
	}

	pthread_mutex_unlock(&mutex);
	return start_size;


}


void	SDL_sound_handler::play_sound(int sound_handle, int loop_count, int offset, long start_position, std::vector<sound_envelope>* envelopes)
//uint8_t env_count, uint32_t* envelopes)
// Play the index'd sample.
{
	pthread_mutex_lock(&mutex);

	// Check if the sound exists.
	if (sound_handle < 0 || (unsigned int) sound_handle >= m_sound_data.size())
	{
		// Invalid handle.
		pthread_mutex_unlock(&mutex);
		return;
	}

	// If this is called from a streamsoundblocktag, we only start if this
	// sound isn't already playing. If a active_sound-struct is existing we
	// assume it is also playing.
	if (start_position > 0 && m_sound_data[sound_handle]->m_active_sounds.size() > 0) {
		pthread_mutex_unlock(&mutex);
		return;
	}

	// Make a "active_sound" for this sound which is later placed on the vector of instances of this sound being played
	active_sound* sound = new active_sound;

	// Copy data-info to the active_sound
	sound->data_size = m_sound_data[sound_handle]->data_size;
	sound->data = m_sound_data[sound_handle]->data;

	// Set the given options of the sound
	sound->position = start_position;
	if (start_position < 0) sound->position = 0;
	sound->offset = (m_sound_data[sound_handle]->stereo ? offset : offset*2); // offset is stored as stereo
	if (offset < 0) sound->offset = 0;
	sound->envelopes = envelopes;
	sound->current_env = 0;
	sound->samples_played = 0;

	// Set number of loop we should do. -1 is infinte loop, 0 plays it once, 1 twice etc.
	sound->loop_count = loop_count;

	if (m_sound_data[sound_handle]->format == FORMAT_MP3) {

#ifdef USE_FFMPEG
		// Init the avdecoder-decoder
		avcodec_init();
		avcodec_register_all();// change this to only register mp3?
		sound->codec = avcodec_find_decoder(CODEC_ID_MP3);

		// Init the parser
		sound->parser = av_parser_init(CODEC_ID_MP3);

		if (!sound->codec) {
			gnash::log_error("Your FFMPEG can't decode MP3?!\n");
			pthread_mutex_unlock(&mutex);
			return;
		}

		sound->raw_data = new uint8[AVCODEC_MAX_AUDIO_FRAME_SIZE];
		memset((void*)sound->raw_data, 0, AVCODEC_MAX_AUDIO_FRAME_SIZE);
		sound->raw_position = AVCODEC_MAX_AUDIO_FRAME_SIZE;
		sound->raw_data_size = AVCODEC_MAX_AUDIO_FRAME_SIZE;
		sound->cc = avcodec_alloc_context();
		avcodec_open(sound->cc, sound->codec);
#elif defined(USE_MAD)
		// Init the mad decoder
		mad_stream_init(&sound->stream);
		mad_frame_init(&sound->frame);
		mad_synth_init(&sound->synth);
		mad_stream_buffer(&sound->stream, sound->data, sound->data_size);

		sound->raw_data = 0;
		sound->raw_data_size = 0;
		sound->raw_position = 0;
#endif
	} else {
		sound->raw_data_size = m_sound_data[sound_handle]->data_size;
		sound->raw_data = m_sound_data[sound_handle]->data;
		sound->raw_position = 0;
		sound->position = m_sound_data[sound_handle]->data_size;

	}

	if (!soundOpened) {
		if (SDL_OpenAudio(&audioSpec, NULL) < 0 ) {
			gnash::log_error("Unable to START SOUND: %s\n", SDL_GetError());
			pthread_mutex_unlock(&mutex);
			return;
		}
		soundOpened = true;


	}

	++soundsPlaying;
	m_sound_data[sound_handle]->m_active_sounds.push_back(sound);

	if (soundsPlaying == 1) {
		SDL_PauseAudio(0);
	}

	pthread_mutex_unlock(&mutex);

}


void	SDL_sound_handler::stop_sound(int sound_handle)
{
	pthread_mutex_lock(&mutex);

	// Check if the sound exists.
	if (sound_handle < 0 || (unsigned int) sound_handle >= m_sound_data.size())
	{
		// Invalid handle.
	} else {

		for (int32_t i = (int32_t)m_sound_data[sound_handle]->m_active_sounds.size()-1; i >-1; i--) {

			// Stop sound, remove it from the active list (mp3)
			if (m_sound_data[sound_handle]->format == 2) {
#ifdef USE_FFMPEG
				avcodec_close(m_sound_data[sound_handle]->m_active_sounds[i]->cc);
				av_parser_close(m_sound_data[sound_handle]->m_active_sounds[i]->parser);
#elif defined(USE_MAD)
				mad_synth_finish(&m_sound_data[sound_handle]->m_active_sounds[i]->synth);
				mad_frame_finish(&m_sound_data[sound_handle]->m_active_sounds[i]->frame);
				mad_stream_finish(&m_sound_data[sound_handle]->m_active_sounds[i]->stream);
#endif
				delete[] m_sound_data[sound_handle]->m_active_sounds[i]->raw_data;
				m_sound_data[sound_handle]->m_active_sounds.erase(m_sound_data[sound_handle]->m_active_sounds.begin() + i);
				soundsPlaying--;

			// Stop sound, remove it from the active list (adpcm/native16)
			} else {
				m_sound_data[i]->m_active_sounds.erase(m_sound_data[sound_handle]->m_active_sounds.begin() + i);
				soundsPlaying--;
			}
		}
	}
	pthread_mutex_unlock(&mutex);

}


void	SDL_sound_handler::delete_sound(int sound_handle)
// this gets called when it's done with a sample.
{
	pthread_mutex_lock(&mutex);

	if (sound_handle >= 0 && (unsigned int) sound_handle < m_sound_data.size())
	{
		delete[] m_sound_data[sound_handle]->data;
	}
	pthread_mutex_unlock(&mutex);

}

// This will stop all sounds playing. Will cause problems if the soundhandler is made static
// and supplys sound_handling for many SWF's, since it will stop all sounds with no regard
// for what sounds is associated with what SWF.
void	SDL_sound_handler::stop_all_sounds()
{
	int32_t num_sounds = (int32_t) m_sound_data.size()-1;
	for (int32_t i = num_sounds; i > -1; i--) //Optimized
		stop_sound(i);
}


//	returns the sound volume level as an integer from 0 to 100,
//	where 0 is off and 100 is full volume. The default setting is 100.
int	SDL_sound_handler::get_volume(int sound_handle) {

	pthread_mutex_lock(&mutex);

	int ret;
	// Check if the sound exists.
	if (sound_handle >= 0 && (unsigned int) sound_handle < m_sound_data.size())
	{
		ret = m_sound_data[sound_handle]->volume;
	} else {
		ret = 0; // Invalid handle
	}
	pthread_mutex_unlock(&mutex);
	return ret;
}


//	A number from 0 to 100 representing a volume level.
//	100 is full volume and 0 is no volume. The default setting is 100.
void	SDL_sound_handler::set_volume(int sound_handle, int volume) {

	pthread_mutex_lock(&mutex);

	// Check if the sound exists.
	if (sound_handle < 0 || (unsigned int) sound_handle >= m_sound_data.size())
	{
		// Invalid handle.
	} else {

		// Set volume for this sound. Should this only apply to the active sounds?
		m_sound_data[sound_handle]->volume = volume;
	}
	pthread_mutex_unlock(&mutex);


}
	
void SDL_sound_handler::get_info(int sound_handle, int* format, bool* stereo) {

	pthread_mutex_lock(&mutex);

	// Check if the sound exists.
	if (sound_handle >= 0 && (unsigned int) sound_handle < m_sound_data.size())
	{
		*format = m_sound_data[sound_handle]->format;
		*stereo = m_sound_data[sound_handle]->stereo;
	}

	pthread_mutex_unlock(&mutex);
}


void SDL_sound_handler::convert_raw_data(
	int16_t** adjusted_data,
	int* adjusted_size,
	void* data,
	int sample_count,
	int sample_size,
	int sample_rate,
	bool stereo)
// VERY crude sample-rate & sample-size conversion.  Converts
// input data to the SDL output format (SAMPLE_RATE,
// stereo, 16-bit native endianness)
{
// 		// xxxxx debug pass-thru
// 		{
// 			int	output_sample_count = sample_count * (stereo ? 2 : 1);
// 			int16_t*	out_data = new int16_t[output_sample_count];
// 			*adjusted_data = out_data;
// 			*adjusted_size = output_sample_count * 2;	// 2 bytes per sample
// 			memcpy(out_data, data, *adjusted_size);
// 			return;
// 		}
// 		// xxxxx

	bool m_stereo = (audioSpec.channels == 2 ? true : false);
	int m_sample_rate = audioSpec.freq;

	// simple hack to handle dup'ing mono to stereo
	if ( !stereo && m_stereo)
	{
		sample_rate >>= 1;
	}

		// simple hack to lose half the samples to get mono from stereo
	if ( stereo && !m_stereo)
	{
		sample_rate <<= 1;
	}

	// Brain-dead sample-rate conversion: duplicate or
	// skip input samples an integral number of times.
	int	inc = 1;	// increment
	int	dup = 1;	// duplicate
	if (sample_rate > m_sample_rate)
	{
		inc = sample_rate / m_sample_rate;
	}
	else if (sample_rate < m_sample_rate)
	{
		dup = m_sample_rate / sample_rate;
	}

	int	output_sample_count = (sample_count * dup) / inc;
	int16_t*	out_data = new int16_t[output_sample_count];
	*adjusted_data = out_data;
	*adjusted_size = output_sample_count * 2;	// 2 bytes per sample

	if (sample_size == 1)
	{
		// Expand from 8 bit to 16 bit.
		uint8_t*	in = (uint8_t*) data;
		for (int i = 0; i < output_sample_count; i++)
		{
			uint8_t	val = *in;
			for (int j = 0; j < dup; j++)
			{
				*out_data++ = (int(val) - 128);
			}
			in += inc;
		}
	}
	else
	{
		// 16-bit to 16-bit conversion.
		int16_t*	in = (int16_t*) data;
		for (int i = 0; i < output_sample_count; i += dup)
		{
			int16_t	val = *in;
			for (int j = 0; j < dup; j++)
			{
				*out_data++ = val;
			}
			in += inc;
		}
	}
}


gnash::sound_handler*	gnash::create_sound_handler_sdl()
// Factory.
{
	return new SDL_sound_handler;
}

// AS-volume adjustment
void adjust_volume(int16_t* data, int size, int volume)
{
	for (int i=0; i < size*0.5; i++) {
		data[i] = data[i] * volume/100;
	}
}

// envelope-volume adjustment
void use_envelopes(active_sound* sound, int length)
{
	// Check if this is the time to use envelopes yet
	if (sound->current_env == 0 && sound->envelopes->at(0).m_mark44 > sound->samples_played+length/2) {
		return;

	// switch to the next envelope if needed and possible
	} else if (sound->current_env < sound->envelopes->size()-1 && sound->envelopes->at(sound->current_env+1).m_mark44 >= sound->samples_played) {
		sound->current_env++;
	}

	// Current envelope position
	int32_t cur_env_pos = sound->envelopes->at(sound->current_env).m_mark44;

	// Next envelope position
	int32_t next_env_pos = 0;
	if (sound->current_env == (sound->envelopes->size()-1)) {
		// If there is no "next envelope" then set the next envelope start point to be unreachable
		next_env_pos = cur_env_pos + length;
	} else {
		next_env_pos = sound->envelopes->at(sound->current_env+1).m_mark44;
	}

	int startpos = 0;
	// Make sure we start adjusting at the right sample
	if (sound->current_env == 0 && sound->envelopes->at(sound->current_env).m_mark44 > sound->samples_played) {
		startpos = sound->raw_position + (sound->envelopes->at(sound->current_env).m_mark44 - sound->samples_played)*2;
	} else {
		startpos = sound->raw_position;
	}
	int16_t* data = (int16_t*) (sound->raw_data + startpos);

	for (int i=0; i < length/2; i+=2) {
		float left = (float)sound->envelopes->at(sound->current_env).m_level0 / 32768.0;
		float right = (float)sound->envelopes->at(sound->current_env).m_level1 / 32768.0;

		data[i] = (int16_t)(data[i] * left); // Left
		data[i+1] = (int16_t)(data[i+1] * right); // Right

		if ((sound->samples_played+(length/2-i)) >= next_env_pos && sound->current_env != (sound->envelopes->size()-1)) {
			sound->current_env++;
			// Next envelope position
			if (sound->current_env == (sound->envelopes->size()-1)) {
				// If there is no "next envelope" then set the next envelope start point to be unreachable
				next_env_pos = cur_env_pos + length;
			} else {
				next_env_pos = sound->envelopes->at(sound->current_env+1).m_mark44;
			}
		}
	}
}


// The callback function which refills the buffer with data
void sdl_audio_callback (void *udata, Uint8 *stream, int buffer_length)
{
	// We run through all of the sounds, and mix all of the active sounds 
	// into the stream given by the callback.
	// If a sound is looping it will be decoded from the beginning again.
	// When a sound is finished the empty space in the stream will be set to 0.

	SDL_sound_handler* handler = static_cast<SDL_sound_handler*>(udata);

	// If nothing to play there is no reason to play
	// Is this a potential deadlock problem?
	if (handler->soundsPlaying == 0) {
		SDL_PauseAudio(1);
		return;
	}

	pthread_mutex_lock(&handler->mutex);
	
	// Mixed sounddata buffer
	Uint8* buffer = stream;  //new Uint8[len];
	memset(buffer, 0, buffer_length);

	for(uint32_t i=0; i < handler->m_sound_data.size(); i++) {
		for(uint32_t j = 0; j < handler->m_sound_data[i]->m_active_sounds.size(); j++) {

			active_sound* sound = handler->m_sound_data[i]->m_active_sounds[j];

			// When the current sound dont have enough decoded data to fill the buffer, 
			// we first mix what is alreadt decoded, then decode some more data, and
			// mix some more until the buffer is full. If a sound loops the magic
			// happens here ;)
			if (sound->raw_data_size - sound->raw_position < buffer_length 
				&& (sound->position < sound->data_size || sound->loop_count != 0)) {

				// First we mix what is decoded
				int index = 0;
				if (sound->raw_data_size - sound->raw_position > 0) {
					// If the volume needs adjustments we call a function to do that
					if (handler->m_sound_data[i]->volume != 100) {
						adjust_volume((int16_t*)(sound->raw_data + sound->raw_position), 
							sound->raw_data_size - sound->raw_position,
							handler->m_sound_data[i]->volume);
					} else if (sound->envelopes != NULL) {
						use_envelopes(sound, sound->raw_data_size - sound->raw_position);
					}
					SDL_MixAudio(stream, (const Uint8*)(sound->raw_data + sound->raw_position), 
						sound->raw_data_size - sound->raw_position,
						SDL_MIX_MAXVOLUME);
					index = sound->raw_data_size - sound->raw_position;
				}
				sound->raw_position += index;
				sound->samples_played += index;

				// Then we decode some data
				int outsize = 0;	
#ifdef USE_FFMPEG
				// If we need to loop, we reset the data pointer
				if (sound->data_size == sound->position && sound->loop_count != 0) {
					sound->loop_count--;
					sound->position = 0;
				}
				
				if (sound->raw_data_size > 0) memset(sound->raw_data, 0, sound->raw_data_size);

				uint8_t* frame;
				int framesize;

				long bytes_decoded = av_parser_parse(sound->parser, sound->cc, &frame, &framesize,
							(uint8_t *)(sound->data + sound->position), sound->data_size - sound->position,
							0 ,0);	//pts, dts

				int tmp = 0;
				tmp = avcodec_decode_audio(sound->cc, 
						(int16_t *)sound->raw_data, 
						&outsize, 
						frame, 
						framesize);

				if (bytes_decoded < 0) {
					gnash::log_error("Error while decoding MP3-stream\n");
					// TODO: Remove the sound from the active list
					sound->position = sound->data_size;
					continue;
				}

				sound->position += bytes_decoded;

#elif defined(USE_MAD)
				// If we need to loop, we reset the data pointer, and tell mad about it
				if (sound->data_size == sound->position && sound->loop_count != 0) {
					sound->position = 0;
					mad_stream_buffer(&sound->stream, sound->data, sound->data_size);
					sound->loop_count--;
				}

				int ret;
				while(true) {
					ret = mad_frame_decode(&sound->frame, &sound->stream);
					
					// There is always some junk in front of the data, 
					// so we continue until we get past it.
					if (ret && sound->stream.error == MAD_ERROR_LOSTSYNC) continue;
					
					break;
				}

				if (ret == -1 && sound->stream.error != MAD_ERROR_BUFLEN) {
					gnash::log_error("Error while decoding MP3-stream, MAD error: %d", sound->stream.error);
					continue;
				} else if (ret == -1 && sound->stream.error == MAD_ERROR_BUFLEN) {
					// the buffer is empty, no more to decode!
					sound->position = sound->data_size;
				} else {
					sound->position += sound->stream.next_frame - sound->stream.buffer - sound->position;
				}

				mad_synth_frame (&sound->synth, &sound->frame);
				
				outsize = sound->synth.pcm.length * ((handler->m_sound_data[i]->stereo == true) ? 4 : 2);

				if (sound->raw_data) delete[] sound->raw_data;
				sound->raw_data = new Uint8[outsize];
				int sample;
				
				int16_t* dst = (int16_t*) sound->raw_data;

				// transfer the decoded samples into the sound-struct, and do some
				// scaling while we're at it.
				for(int f = 0; f < sound->synth.pcm.length; f++)
				{
					for (int e = 0; e < ((handler->m_sound_data[i]->stereo == true) ? 2 : 1); e++){ // channels (stereo/mono)

						mad_fixed_t mad_sample = sound->synth.pcm.samples[e][f];

						// round
						mad_sample += (1L << (MAD_F_FRACBITS - 16));

						// clip
						if (mad_sample >= MAD_F_ONE) mad_sample = MAD_F_ONE - 1;
						else if (mad_sample < -MAD_F_ONE) mad_sample = -MAD_F_ONE;

						// quantize
						sample = mad_sample >> (MAD_F_FRACBITS + 1 - 16);

						if ( sample != (int16_t)sample ) sample = sample < 0 ? -32768 : 32767;

						*dst++ = sample;
					}
				}
				
#endif
				// If we need to convert samplerate...
				if (outsize > 0 && handler->m_sound_data[i]->sample_rate != handler->audioSpec.freq) {
					int16_t* adjusted_data = 0;
					int	adjusted_size = 0;
					int sample_count = outsize / ((handler->m_sound_data[i]->stereo == true) ? 4 : 2);

					// Convert to needed samplerate
					handler->convert_raw_data(&adjusted_data, &adjusted_size, sound->raw_data, sample_count, 0, 
							handler->m_sound_data[i]->sample_rate, handler->m_sound_data[i]->stereo);

					// Hopefully this wont happen
					if (!adjusted_data) { 
						continue;
					}

					// Move the new data to the sound-struct
					if (sound->raw_data) delete[] sound->raw_data;
					sound->raw_data = new Uint8[adjusted_size];
					memcpy(sound->raw_data, adjusted_data, adjusted_size);
					sound->raw_data_size = adjusted_size;
					delete[] adjusted_data;

				} else {
					sound->raw_data_size = outsize;
				}
				
				sound->raw_position = 0;

				// If the volume needs adjustments we call a function to do that
				if (handler->m_sound_data[i]->volume != 100) {
					adjust_volume((int16_t*)(sound->raw_data + sound->raw_position), 
						sound->raw_data_size - sound->raw_position,
						handler->m_sound_data[i]->volume);
				} else if (sound->envelopes != NULL) {
					use_envelopes(sound, buffer_length - index);
				}

				// Then we mix the newly decoded data
				SDL_MixAudio((Uint8*)(stream+index),(const Uint8*) sound->raw_data, 
						buffer_length - index,
						SDL_MIX_MAXVOLUME);

				sound->raw_position = buffer_length - index;

				sound->samples_played += buffer_length - index;
				
			// When the current sound has enough decoded data to fill 
			// the buffer, we do just that.
			} else if (sound->raw_data_size - sound->raw_position >= buffer_length ) {
			
				// If the volume needs adjustments we call a function to do that
				if (handler->m_sound_data[i]->volume != 100) {
					adjust_volume((int16_t*)(sound->raw_data + sound->raw_position), 
						sound->raw_data_size - sound->raw_position,
						handler->m_sound_data[i]->volume);
				} else if (sound->envelopes != NULL) {
					use_envelopes(sound, buffer_length);
				}

				// Mix the raw data
				SDL_MixAudio((Uint8*)(stream),(const Uint8*) (sound->raw_data + sound->raw_position), 
						buffer_length,
						SDL_MIX_MAXVOLUME);

				sound->raw_position += buffer_length;

				sound->samples_played += buffer_length;

			// When the current sound doesn't have anymore data to decode,
			// and doesn't loop (anymore), but still got unplayed data,
			// we put the last data on the stream
			} else if (sound->raw_data_size - sound->raw_position < buffer_length && sound->raw_data_size > sound->raw_position) {
			
				// If the volume needs adjustments we call a function to do that
				if (handler->m_sound_data[i]->volume != 100) {
					adjust_volume((int16_t*)(sound->raw_data + sound->raw_position), 
						sound->raw_data_size - sound->raw_position,
						handler->m_sound_data[i]->volume);
				} else if (sound->envelopes != NULL) {
					use_envelopes(sound, sound->raw_data_size - sound->raw_position);
				}

				// Mix the remaining data
				SDL_MixAudio((Uint8*)(stream),(const Uint8*) (sound->raw_data + sound->raw_position), 
						sound->raw_data_size - sound->raw_position,
						SDL_MIX_MAXVOLUME);
				sound->raw_position = sound->raw_data_size;

			} else {
			}
			
			// Sound is done, remove it from the active list (mp3)
			if (sound->position == sound->data_size && sound->loop_count == 0 && handler->m_sound_data[i]->format == 2) {
#ifdef USE_FFMPEG
				avcodec_close(sound->cc);
				av_parser_close(sound->parser);
#elif defined(USE_MAD)
				mad_synth_finish(&sound->synth);
				mad_frame_finish(&sound->frame);
				mad_stream_finish(&sound->stream);
#endif
				delete[] sound->raw_data;
				handler->m_sound_data[i]->m_active_sounds.erase(handler->m_sound_data[i]->m_active_sounds.begin() + j);
				handler->soundsPlaying--;


			// Sound is done, remove it from the active list (adpcm/native16)
			} else if (sound->loop_count == 0 && handler->m_sound_data[i]->format == 7 && sound->raw_position >= sound->raw_data_size && sound->raw_data_size != 0) {
				handler->m_sound_data[i]->m_active_sounds.erase(handler->m_sound_data[i]->m_active_sounds.begin() + j);
				handler->soundsPlaying--;
			} else if (sound->raw_position == 0 && sound->raw_data_size == 0) {
				handler->m_sound_data[i]->m_active_sounds.erase(handler->m_sound_data[i]->m_active_sounds.begin() + j);
				handler->soundsPlaying--;
			}

		}
	}
	pthread_mutex_unlock(&handler->mutex);

}

// Local Variables:
// mode: C++
// End:

