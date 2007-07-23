//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
//

// Based on sound_handler_sdl.cpp by Thatcher Ulrich http://tulrich.com 2003
// which has been donated to the Public Domain.

// $Id: sound_handler_sdl.cpp,v 1.75 2007/07/23 22:22:25 strk Exp $

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sound_handler_sdl.h"
#include "utility.h" // for convert_raw_data

#include "log.h"
#include <cmath>
#include <vector>
#include <SDL.h>

using namespace boost;

namespace gnash {

SDL_sound_handler::SDL_sound_handler()
	: soundOpened(false),
		soundsPlaying(0),
		muted(false)
{
	// This is our sound settings
	audioSpec.freq = 44100;
	audioSpec.format = AUDIO_S16SYS; // AUDIO_S8 AUDIO_U8;
	audioSpec.channels = 2;
	audioSpec.callback = SDL_sound_handler::sdl_audio_callback;
	audioSpec.userdata = this;
	audioSpec.samples = 2048;		//512 - not enough for  videostream
}

SDL_sound_handler::~SDL_sound_handler()
{
	for (size_t i=0, e=m_sound_data.size(); i < e; ++i)
	{
		stop_sound(i);
		delete_sound(i);
	}
	if (soundOpened) SDL_CloseAudio();
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
		log_error(_("could not allocate memory for sound data"));
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

        mutex::scoped_lock lock(_mutex);

	switch (format)
	{
	case FORMAT_NATIVE16:

		if (data_bytes > 0) {
			convert_raw_data(&adjusted_data, &adjusted_size, data, sample_count, 2, sample_rate, stereo,
					 audioSpec.freq, (audioSpec.channels == 2 ? true : false));
			if (!adjusted_data) {
				log_error(_("Some kind of error occurred with sound data"));
				return -1;
			}
			sounddata->data_size = adjusted_size;
			sounddata->data = (Uint8*) adjusted_data;
		}
		break;

	case FORMAT_MP3:
#ifndef USE_FFMPEG
#ifndef USE_MAD
		log_error(_("gnash has not been compiled to handle mp3 audio"));
		return -1;
#endif
#endif
		sounddata->data = new Uint8[data_bytes];
		if (!sounddata->data) {
			log_error(_("could not allocate space for data in sound handler"));
			return -1;
		}
		memcpy(sounddata->data, data, data_bytes);
		break;

	//case FORMAT_VORBIS:

	case FORMAT_RAW:
	case FORMAT_ADPCM:
	case FORMAT_UNCOMPRESSED:
		// These should have been converted to FORMAT_NATIVE16
		log_error(_("Sound data format not properly converted"));
		return -1;
		break;

	case FORMAT_NELLYMOSER:
		log_unimpl("Nellymoser sound format requested, gnash does not handle it.");
		return -1;

	default:
		// Unhandled format.
		log_error(_("unknown sound format %d requested; gnash does not handle it"), (int)format);
		return -1; // Unhandled format, set to NULL.
	}

	m_sound_data.push_back(sounddata);
	int sound_id = m_sound_data.size()-1;

	return sound_id;

}

// this gets called when a stream gets more data
long	SDL_sound_handler::fill_stream_data(void* data, int data_bytes, int sample_count, int handle_id)
{

	mutex::scoped_lock lock(_mutex);
	// @@ does a negative handle_id have any meaning ?
	//    should we change it to unsigned instead ?
	if (handle_id < 0 || (unsigned int) handle_id+1 > m_sound_data.size()) {
		return -1;
	}
	int start_size = 0;
	sound_data* sounddata = m_sound_data[handle_id];

	// Handling of the sound data
	switch (sounddata->format) {
	case FORMAT_NATIVE16:
	    {
		int16_t*	adjusted_data = 0;
		int	adjusted_size = 0;

		convert_raw_data(&adjusted_data, &adjusted_size,
				 data, sample_count, 2 /*sample size*/,
				 sounddata->sample_rate, sounddata->stereo,
				 audioSpec.freq, (audioSpec.channels == 2));
		if (!adjusted_data || adjusted_size < 1) {
			log_error(_("Some kind of error with resampling sound data"));
			return -1;
		}

		// Reallocate the required memory.
		Uint8* tmp_data = new Uint8[adjusted_size + sounddata->data_size];
		memcpy(tmp_data, sounddata->data, sounddata->data_size);
		memcpy(tmp_data + sounddata->data_size, adjusted_data, adjusted_size);
		if (sounddata->data_size > 0) delete [] sounddata->data;
		sounddata->data = tmp_data;

		start_size = sounddata->data_size;
		sounddata->data_size += adjusted_size;
		std::vector<active_sound*> asounds = sounddata->m_active_sounds;

		// If playback has already started, we also update the active sounds
		for(uint32_t i=0; i < asounds.size(); i++) {
			active_sound* sound = asounds[i];
			sound->raw_data_size = sounddata->data_size;
			sound->set_raw_data(sounddata->data);
		}
	    }
	    break;

	case FORMAT_MP3:
	    {
		// Reallocate the required memory.
		Uint8* tmp_data = new Uint8[data_bytes + sounddata->data_size];
		memcpy(tmp_data, sounddata->data, sounddata->data_size);
		memcpy(tmp_data + sounddata->data_size, data, data_bytes);
		if (sounddata->data_size > 0) delete [] sounddata->data;
		sounddata->data = tmp_data;

		start_size = sounddata->data_size;
		sounddata->data_size += data_bytes;
		std::vector<active_sound*> asounds = sounddata->m_active_sounds;
		
		// If playback has already started, we also update the active sounds
		for(uint32_t i=0; i < asounds.size(); i++) {
			active_sound* sound = asounds[i];
			sound->set_data(sounddata->data);
			sound->data_size = sounddata->data_size;
		}

	    }
	    break;

	default:
		log_error(_("Behavior for this audio codec %d is unknown.  Please send this SWF to the developers"), (int)(sounddata->format));
	}

	return start_size;
}


void	SDL_sound_handler::play_sound(int sound_handle, int loop_count, int offset, long start_position, const std::vector<sound_envelope>* envelopes)
// Play the index'd sample.
{
	mutex::scoped_lock lock(_mutex);

	// Check if the sound exists, or if audio is muted
	if (sound_handle < 0 || static_cast<unsigned int>(sound_handle) >= m_sound_data.size() || muted)
	{
		// Invalid handle or muted
		return;
	}

	sound_data* sounddata = m_sound_data[sound_handle];

	// If this is called from a streamsoundblocktag, we only start if this
	// sound isn't already playing. If a active_sound-struct is existing we
	// assume it is also playing.
	if (start_position > 0 && sounddata->m_active_sounds.size() > 0) {
		return;
	}

	// Make sure sound actually got some data
	if (sounddata->data_size < 1) {
		IF_VERBOSE_MALFORMED_SWF(
			log_swferror(_("Trying to play sound with size 0"));
		);
		return;
	}

	// Make a "active_sound" for this sound which is later placed on the vector of instances of this sound being played
	active_sound* sound = new active_sound;

	// Copy data-info to the active_sound
	sound->data_size = sounddata->data_size;
	sound->set_data(sounddata->data);

	// Set the given options of the sound
	if (start_position < 0) sound->position = 0;
	else sound->position = start_position;

	if (offset < 0) sound->offset = 0;
	else sound->offset = (sounddata->stereo ? offset : offset*2); // offset is stored as stereo

	sound->envelopes = envelopes;
	sound->current_env = 0;
	sound->samples_played = 0;

	// Set number of loop we should do. -1 is infinte loop, 0 plays it once, 1 twice etc.
	sound->loop_count = loop_count;

	if (sounddata->format == FORMAT_MP3) {

#ifdef USE_FFMPEG
		// Init the avdecoder-decoder
		avcodec_init();
		avcodec_register_all();// change this to only register mp3?
		sound->codec = avcodec_find_decoder(CODEC_ID_MP3);

		// Init the parser
		sound->parser = av_parser_init(CODEC_ID_MP3);

		if (!sound->codec) {
			log_error(_("Your FFMPEG can't decode MP3?!"));
			return;
		}

		sound->cc = avcodec_alloc_context();
		avcodec_open(sound->cc, sound->codec);

#elif defined(USE_MAD)
		// Init the mad decoder
		mad_stream_init(&sound->stream);
		mad_frame_init(&sound->frame);
		mad_synth_init(&sound->synth);
#endif

		sound->set_raw_data(NULL);
		sound->raw_position = 0;
		sound->raw_data_size = 0;

	} else {
		sound->raw_data_size = sounddata->data_size;
		sound->set_raw_data(sounddata->data);
		sound->raw_position = 0;
		sound->position = 0;
		sound->data_size = 0;

	}

	if (!soundOpened) {
		if (SDL_OpenAudio(&audioSpec, NULL) < 0 ) {
			log_error(_("Unable to start SDL sound: %s"), SDL_GetError());
			return;
		}
		soundOpened = true;

	}

	++soundsPlaying;
	++_soundsStarted;
	sounddata->m_active_sounds.push_back(sound);

	if (soundsPlaying == 1) {
		SDL_PauseAudio(0);
	}

}


void	SDL_sound_handler::stop_sound(int sound_handle)
{
	mutex::scoped_lock lock(_mutex);

	// Check if the sound exists.
	if (sound_handle < 0 || (unsigned int) sound_handle >= m_sound_data.size())
	{
		// Invalid handle.
	} else {
	
		sound_data* sounddata = m_sound_data[sound_handle];
	
		for (int32_t i = (int32_t) sounddata->m_active_sounds.size()-1; i >-1; i--) {

			active_sound* sound = sounddata->m_active_sounds[i];

			// Stop sound, remove it from the active list (mp3)
			if (sounddata->format == 2) {
#ifdef USE_FFMPEG
				avcodec_close(sound->cc);
				av_parser_close(sound->parser);
#elif defined(USE_MAD)
				mad_synth_finish(&sound->synth);
				mad_frame_finish(&sound->frame);
				mad_stream_finish(&sound->stream);
#endif
				sound->delete_raw_data();
				sounddata->m_active_sounds.erase(sounddata->m_active_sounds.begin() + i);

			// Stop sound, remove it from the active list (adpcm/native16)
			} else {
				sounddata->m_active_sounds.erase(sounddata->m_active_sounds.begin() + i);
			}
			--soundsPlaying;
			++_soundsStopped;
		}
	}

}


// this gets called when it's done with a sample.
void	SDL_sound_handler::delete_sound(int sound_handle)
{
	mutex::scoped_lock lock(_mutex);

	if (sound_handle >= 0 && static_cast<unsigned int>(sound_handle) < m_sound_data.size())
	{
		delete m_sound_data[sound_handle];
		m_sound_data[sound_handle] = NULL;
	}

}

// This will stop all sounds playing. Will cause problems if the soundhandler is made static
// and supplys sound_handling for many SWF's, since it will stop all sounds with no regard
// for what sounds is associated with what SWF.
void	SDL_sound_handler::stop_all_sounds()
{
	mutex::scoped_lock lock(_mutex);

	int32_t num_sounds = (int32_t) m_sound_data.size()-1;
	for (int32_t j = num_sounds; j > -1; j--) {//Optimized
		sound_data* sounddata = m_sound_data[j];
		int32_t num_active_sounds = (int32_t) sounddata->m_active_sounds.size()-1;
		for (int32_t i = num_active_sounds; i > -1; i--) {

			active_sound* sound = sounddata->m_active_sounds[i];

			// Stop sound, remove it from the active list (mp3)
			if (sounddata->format == 2) {
#ifdef USE_FFMPEG
				avcodec_close(sound->cc);
				av_parser_close(sound->parser);
#elif defined(USE_MAD)
				mad_synth_finish(&sound->synth);
				mad_frame_finish(&sound->frame);
				mad_stream_finish(&sound->stream);
#endif
				sound->delete_raw_data();
				sounddata->m_active_sounds.erase(sounddata->m_active_sounds.begin() + i);
				soundsPlaying--;

			// Stop sound, remove it from the active list (adpcm/native16)
			} else {
				sounddata->m_active_sounds.erase(sounddata->m_active_sounds.begin() + i);
				soundsPlaying--;
			}
		}
	}
}


//	returns the sound volume level as an integer from 0 to 100,
//	where 0 is off and 100 is full volume. The default setting is 100.
int	SDL_sound_handler::get_volume(int sound_handle) {

	mutex::scoped_lock lock(_mutex);

	int ret;
	// Check if the sound exists.
	if (sound_handle >= 0 && static_cast<unsigned int>(sound_handle) < m_sound_data.size())
	{
		ret = m_sound_data[sound_handle]->volume;
	} else {
		ret = 0; // Invalid handle
	}
	return ret;
}


//	A number from 0 to 100 representing a volume level.
//	100 is full volume and 0 is no volume. The default setting is 100.
void	SDL_sound_handler::set_volume(int sound_handle, int volume) {

	mutex::scoped_lock lock(_mutex);

	// Check if the sound exists.
	if (sound_handle < 0 || static_cast<unsigned int>(sound_handle) >= m_sound_data.size())
	{
		// Invalid handle.
	} else {

		// Set volume for this sound. Should this only apply to the active sounds?
		m_sound_data[sound_handle]->volume = volume;
	}


}
	
void SDL_sound_handler::get_info(int sound_handle, int* format, bool* stereo) {

	mutex::scoped_lock lock(_mutex);

	// Check if the sound exists.
	if (sound_handle >= 0 && static_cast<unsigned int>(sound_handle) < m_sound_data.size())
	{
		*format = m_sound_data[sound_handle]->format;
		*stereo = m_sound_data[sound_handle]->stereo;
	}

}

// gnash calls this to mute audio
void SDL_sound_handler::mute() {
	stop_all_sounds();
	muted = true;
}

// gnash calls this to unmute audio
void SDL_sound_handler::unmute() {
	muted = false;
}

bool SDL_sound_handler::is_muted()
{
	return muted;
}

void	SDL_sound_handler::attach_aux_streamer(aux_streamer_ptr ptr, void* owner)
{
	mutex::scoped_lock lock(_mutex);
	assert(owner);
	assert(ptr);

	aux_streamer_ptr p;
	if (m_aux_streamer.get(owner, &p))
	{
		// Already in the hash.
		return;
	}
	m_aux_streamer[owner] = ptr;

	++soundsPlaying;

	if (!soundOpened) {
		if (SDL_OpenAudio(&audioSpec, NULL) < 0 ) {
			log_error(_("Unable to start aux SDL sound: %s"), SDL_GetError());
			return;
		}
		soundOpened = true;
	}
	SDL_PauseAudio(0);

}

void	SDL_sound_handler::detach_aux_streamer(void* owner)
{
	mutex::scoped_lock lock(_mutex);
	aux_streamer_ptr p;	
	if (m_aux_streamer.get(owner, &p))
	{
		--soundsPlaying;
		m_aux_streamer.erase(owner);
	}

}


sound_handler*
create_sound_handler_sdl()
// Factory.
{
	return new SDL_sound_handler;
}


// Pointer handling and checking functions
uint8_t* active_sound::get_raw_data_ptr(unsigned long int pos) {
	assert(raw_data_size > pos);
	return raw_data + pos;
}

uint8_t* active_sound::get_data_ptr(unsigned long int pos) {
	assert(data_size > pos);
	return data + pos;
}

void active_sound::set_raw_data(uint8_t* iraw_data) {
	raw_data = iraw_data;
}

void active_sound::set_data(uint8_t* idata) {
	data = idata;
}

void active_sound::delete_raw_data() {
	delete [] raw_data;
}

// AS-volume adjustment
void adjust_volume(int16_t* data, int size, int volume)
{
	for (int i=0; i < size*0.5; i++) {
		data[i] = data[i] * volume/100;
	}
}

// envelope-volume adjustment
static void
use_envelopes(active_sound* sound, unsigned int length)
{
	// Check if this is the time to use envelopes yet
	if (sound->current_env == 0 && (*sound->envelopes)[0].m_mark44 > sound->samples_played+length/2)
	{
		return;

	}
	// switch to the next envelope if needed and possible
	else if (sound->current_env < sound->envelopes->size()-1 && (*sound->envelopes)[sound->current_env+1].m_mark44 >= sound->samples_played)
	{
		sound->current_env++;
	}

	// Current envelope position
	int32_t cur_env_pos = sound->envelopes->operator[](sound->current_env).m_mark44;

	// Next envelope position
	uint32_t next_env_pos = 0;
	if (sound->current_env == (sound->envelopes->size()-1)) {
		// If there is no "next envelope" then set the next envelope start point to be unreachable
		next_env_pos = cur_env_pos + length;
	} else {
		next_env_pos = (*sound->envelopes)[sound->current_env+1].m_mark44;
	}

	unsigned int startpos = 0;
	// Make sure we start adjusting at the right sample
	if (sound->current_env == 0 && (*sound->envelopes)[sound->current_env].m_mark44 > sound->samples_played) {
		startpos = sound->raw_position + ((*sound->envelopes)[sound->current_env].m_mark44 - sound->samples_played)*2;
	} else {
		startpos = sound->raw_position;
	}

	int16_t* data = reinterpret_cast<int16_t*>(sound->get_raw_data_ptr(startpos));

	for (unsigned int i=0; i < length/2; i+=2) {
		float left = static_cast<float>((*sound->envelopes)[sound->current_env].m_level0 / 32768.0);
		float right = static_cast<float>((*sound->envelopes)[sound->current_env].m_level1 / 32768.0);

		data[i] = static_cast<int16_t>(data[i] * left); // Left
		data[i+1] = static_cast<int16_t>(data[i+1] * right); // Right

		if ((sound->samples_played+(length/2-i)) >= next_env_pos && sound->current_env != (sound->envelopes->size()-1)) {
			sound->current_env++;
			// Next envelope position
			if (sound->current_env == (sound->envelopes->size()-1)) {
				// If there is no "next envelope" then set the next envelope start point to be unreachable
				next_env_pos = cur_env_pos + length;
			} else {
				next_env_pos = (*sound->envelopes)[sound->current_env+1].m_mark44;
			}
		}
	}
}


// Prepare for mixing/adding (volume adjustments) and mix/add.
static void
do_mixing(Uint8* stream, active_sound* sound, Uint8* data, unsigned int mix_length, unsigned int volume) {
	// If the volume needs adjustments we call a function to do that
	if (volume != 100) {
		adjust_volume(reinterpret_cast<int16_t*>(data), mix_length, volume);
	} else if (sound->envelopes != NULL) {
		use_envelopes(sound, mix_length);
	}

	// Mix the raw data
	SDL_MixAudio(static_cast<Uint8*>(stream),static_cast<const Uint8*>(data), mix_length, SDL_MIX_MAXVOLUME);

	// Update sound info
	sound->raw_position += mix_length;
	sound->samples_played += mix_length;
}


// Callback invoked by the SDL audio thread.
void SDL_sound_handler::sdl_audio_callback (void *udata, Uint8 *stream, int buffer_length_in)
{
	if ( buffer_length_in < 0 )
	{
		log_error(_("Negative buffer length in sdl_audio_callback (%d)"), buffer_length_in);
		return;
	}

	if ( buffer_length_in == 0 )
	{
		log_error(_("Zero buffer length in sdl_audio_callback"));
		return;
	}

	unsigned int buffer_length = static_cast<unsigned int>(buffer_length_in);

	// Get the soundhandler
	SDL_sound_handler* handler = static_cast<SDL_sound_handler*>(udata);

	// If nothing to play there is no reason to play
	// Is this a potential deadlock problem?
	if (handler->soundsPlaying == 0 && handler->m_aux_streamer.size() == 0) {
		SDL_PauseAudio(1);
		return;
	}

	mutex::scoped_lock lock(handler->_mutex);

	// Mixed sounddata buffer
	Uint8* buffer = stream;
	memset(buffer, 0, buffer_length);

	// call NetStream or Sound audio callbacks
	if (handler->m_aux_streamer.size() > 0)
	{
		uint8_t* buf = new uint8_t[buffer_length];

		// Loop through the attached sounds
		hash_wrapper< void*, sound_handler::aux_streamer_ptr >::iterator it = handler->m_aux_streamer.begin();
		hash_wrapper< void*, sound_handler::aux_streamer_ptr >::iterator end = handler->m_aux_streamer.end();
		while (it != end) {
			memset(buf, 0, buffer_length);

			SDL_sound_handler::aux_streamer_ptr aux_streamer = it->second; //handler->m_aux_streamer[i]->ptr;
			void* owner = it->first;

			// If false is returned the sound doesn't want to be attached anymore
			bool ret = (aux_streamer)(owner, buf, buffer_length);
			if (!ret) {
				handler->m_aux_streamer.erase(it++);
				handler->soundsPlaying--;
			} else {
				++it;
			}
			SDL_MixAudio(stream, buf, buffer_length, SDL_MIX_MAXVOLUME);

		}
		delete [] buf;
	}

#ifdef WIN32	// hack
	return;
#endif

	for(uint32_t i=0; i < handler->m_sound_data.size(); i++) {
		sound_data* sounddata = handler->m_sound_data[i];
		for(uint32_t j = 0; j < sounddata->m_active_sounds.size(); j++) {

			active_sound* sound = sounddata->m_active_sounds[j];

			// When the current sound dont have enough decoded data to fill the buffer, 
			// we first mix what is already decoded, then decode some more data, and
			// mix some more until the buffer is full. If a sound loops the magic
			// happens here ;)
			if (sound->raw_data_size - sound->raw_position < buffer_length 
				&& (sound->position < sound->data_size || sound->loop_count != 0)) {

				// First we mix what is decoded
				unsigned int index = 0;
				if (sound->raw_data_size - sound->raw_position > 0) {
					index = sound->raw_data_size - sound->raw_position;

					do_mixing(stream, sound, sound->get_raw_data_ptr(sound->raw_position),
						index, sounddata->volume);

				}

				// If this isn't MP3 (which means its NATIVE16) there is nothing to decode,
				// reusing the available data is the only option.
				if (sounddata->format != 2) {
					if (index < buffer_length) {
						sound->loop_count--;
						sound->raw_position = 0;
						unsigned int mix_length = ((buffer_length - index) > sound->raw_data_size ? sound->raw_data_size : (buffer_length - index));
						do_mixing(stream+index, sound, sound->get_raw_data_ptr(sound->raw_position), 
							mix_length, sounddata->volume);
					}
					continue;
				}
				
				// Then we decode some data
				// We loop until the size of the decoded sound is greater than the buffer size,
				// or there is no more to decode.
				unsigned int decoded_size = 0;
				sound->raw_data_size = 0;
				while(decoded_size < buffer_length) {

					// If we need to loop, we reset the data pointer
					if (sound->data_size == sound->position && sound->loop_count != 0) {
						sound->loop_count--;
						sound->position = 0;
					}

					// Test if we will get problems... Should not happen...
					assert(sound->data_size > sound->position);
					
					// temp raw buffer
					Uint8* tmp_raw_buffer;
					unsigned int tmp_raw_buffer_size;
					int outsize = 0;

#ifdef USE_FFMPEG
					tmp_raw_buffer = new Uint8[AVCODEC_MAX_AUDIO_FRAME_SIZE];
					tmp_raw_buffer_size = AVCODEC_MAX_AUDIO_FRAME_SIZE;

					long bytes_decoded = 0;

					while (outsize == 0 && sound->position < sound->data_size) {
						uint8_t* frame;
						int framesize;

						bytes_decoded = av_parser_parse(sound->parser, sound->cc, &frame, &framesize,
									static_cast<uint8_t *>(sound->get_data_ptr(sound->position)), sound->data_size - sound->position,
									0 ,0);	//pts, dts

						int tmp = 0;
#ifdef FFMPEG_AUDIO2
						outsize = AVCODEC_MAX_AUDIO_FRAME_SIZE;
						tmp = avcodec_decode_audio2(sound->cc, (int16_t*)(tmp_raw_buffer), &outsize, frame, framesize);
#else
						tmp = avcodec_decode_audio(sound->cc, (int16_t*)(tmp_raw_buffer), &outsize, frame, framesize);
#endif

						if (bytes_decoded < 0 || tmp < 0 || outsize < 0) {
							log_error(_("Error while decoding MP3-stream.  Upgrading ffmpeg/libavcodec might fix this issue."));
							// Setting data position to data size will get the sound removed
							// from the active sound list later on.
							sound->position = sound->data_size;
							break;
						}

						sound->position += bytes_decoded;
					}

#elif defined(USE_MAD)

					// Setup the mad decoder
					mad_stream_buffer(&sound->stream, sound->get_data_ptr(sound->position), sound->data_size-sound->position);

					int ret;
					const unsigned char* old_next_frame = sound->stream.next_frame;
					int loops = 0;
					while(true) {

						ret = mad_frame_decode(&sound->frame, &sound->stream);
						loops++;
						
						// There is always some junk in front of the data, 
						// so we continue until we get past it.
						if (ret && sound->stream.error == MAD_ERROR_LOSTSYNC) continue;
						
						// Error handling is done by relooping (max. 8 times) and just hooping that it will work...
						if (loops > 8) break;
						if (ret == -1 && sound->stream.error != MAD_ERROR_BUFLEN && MAD_RECOVERABLE(sound->stream.error)) {
							log_error(_("Recoverable error while decoding MP3-stream, MAD error: %s"), mad_stream_errorstr (&sound->stream));
							continue;
						}
						
						break;
					}

					if (ret == -1 && sound->stream.error != MAD_ERROR_BUFLEN) {
						log_error(_("Unrecoverable error while decoding MP3-stream, MAD error: %s"), mad_stream_errorstr (&sound->stream));
						sound->position = sound->data_size;
						continue;
					} else if (ret == -1 && sound->stream.error == MAD_ERROR_BUFLEN) {
						// the buffer is empty, no more to decode!
						sound->position = sound->data_size;
					} else {
						sound->position += sound->stream.next_frame - old_next_frame;
					}

					mad_synth_frame (&sound->synth, &sound->frame);
					
					outsize = sound->synth.pcm.length * ((sounddata->stereo == true) ? 4 : 2);

					tmp_raw_buffer = new Uint8[outsize];
					int sample;
					
					int16_t* dst = reinterpret_cast<int16_t*>(tmp_raw_buffer);

					// transfer the decoded samples into the sound-struct, and do some
					// scaling while we're at it.
					for(int f = 0; f < sound->synth.pcm.length; f++)
					{
						for (int e = 0; e < ((sounddata->stereo == true) ? 2 : 1); e++){ // channels (stereo/mono)

							mad_fixed_t mad_sample = sound->synth.pcm.samples[e][f];

							// round
							mad_sample += (1L << (MAD_F_FRACBITS - 16));

							// clip
							if (mad_sample >= MAD_F_ONE) mad_sample = MAD_F_ONE - 1;
							else if (mad_sample < -MAD_F_ONE) mad_sample = -MAD_F_ONE;

							// quantize
							sample = mad_sample >> (MAD_F_FRACBITS + 1 - 16);

							if ( sample != static_cast<int16_t>(sample) ) sample = sample < 0 ? -32768 : 32767;

							*dst++ = sample;
						}
					}
#endif

					// If we need to convert samplerate or/and from mono to stereo...
					if (outsize > 0 && (sounddata->sample_rate != handler->audioSpec.freq || !sounddata->stereo)) {

						int16_t* adjusted_data = 0;
						int	adjusted_size = 0;
						int sample_count = outsize / ((sounddata->stereo == true) ? 4 : 2);

						// Convert to needed samplerate
						convert_raw_data(&adjusted_data, &adjusted_size, tmp_raw_buffer, sample_count, 0, 
								sounddata->sample_rate, sounddata->stereo,
								handler->audioSpec.freq, (handler->audioSpec.channels == 2 ? true : false));

						// Hopefully this wont happen
						if (!adjusted_data) {
							log_error(_("Error in sound sample conversion"));
							continue;
						}

						// Move the new data to the sound-struct
						delete[] tmp_raw_buffer;
						tmp_raw_buffer = reinterpret_cast<Uint8*>(adjusted_data);
						tmp_raw_buffer_size = adjusted_size;

					} else {
						tmp_raw_buffer_size = outsize;
					}

					Uint8* tmp_buf = new Uint8[decoded_size + tmp_raw_buffer_size];
					sound->raw_data_size = 1;
					memcpy(tmp_buf, sound->get_raw_data_ptr(0), decoded_size);
					memcpy(tmp_buf+decoded_size, tmp_raw_buffer, tmp_raw_buffer_size);
					decoded_size += tmp_raw_buffer_size;
					sound->delete_raw_data();
					sound->set_raw_data(tmp_buf);
					delete[] tmp_raw_buffer;

					// no more to decode from this sound, so we break the loop
					if (sound->data_size <= sound->position && sound->loop_count == 0) {
						break;
					}

				} // end of "decode min. bufferlength data" while loop

				sound->raw_data_size = decoded_size;
								
				sound->raw_position = 0;

				// Determine how much should be mixed
				unsigned int mix_length = 0;
				if (decoded_size >= buffer_length - index) {
					mix_length = buffer_length - index;
				} else { 
					mix_length = decoded_size;
				}
				if (sound->raw_data_size < 2) continue; // something went terrible wrong
				do_mixing(stream+index, sound, sound->get_raw_data_ptr(0), mix_length, sounddata->volume);

			// When the current sound has enough decoded data to fill 
			// the buffer, we do just that.
			} else if (sound->raw_data_size - sound->raw_position > buffer_length ) {
			
				do_mixing(stream, sound, sound->get_raw_data_ptr(sound->raw_position), 
					buffer_length, sounddata->volume);

			// When the current sound doesn't have anymore data to decode,
			// and doesn't loop (anymore), but still got unplayed data,
			// we put the last data on the stream
			} else if (sound->raw_data_size - sound->raw_position <= buffer_length && sound->raw_data_size > sound->raw_position+1) {
			

				do_mixing(stream, sound, sound->get_raw_data_ptr(sound->raw_position), 
					sound->raw_data_size - sound->raw_position, sounddata->volume);

				sound->raw_position = sound->raw_data_size;
			} 

			// Sound is done, remove it from the active list (mp3)
			if (sound->position == sound->data_size && sound->loop_count == 0 && sounddata->format == 2) {
#ifdef USE_FFMPEG
				avcodec_close(sound->cc);
				av_parser_close(sound->parser);
#elif defined(USE_MAD)
				mad_synth_finish(&sound->synth);
				mad_frame_finish(&sound->frame);
				mad_stream_finish(&sound->stream);
#endif
				sound->delete_raw_data();
				sounddata->m_active_sounds.erase(sounddata->m_active_sounds.begin() + j);
				handler->soundsPlaying--;


			// Sound is done, remove it from the active list (adpcm/native16)
			} else if (sound->loop_count == 0 && sounddata->format == 7 && sound->raw_position >= sound->raw_data_size && sound->raw_data_size != 0) {
				sounddata->m_active_sounds.erase(sounddata->m_active_sounds.begin() + j);
				handler->soundsPlaying--;
			} else if (sound->raw_position == 0 && sound->raw_data_size == 0) {
				sounddata->m_active_sounds.erase(sounddata->m_active_sounds.begin() + j);
				handler->soundsPlaying--;
			}

		} // active sounds loop
	} // existing sounds loop

}

} // namespace gnash

// Local Variables:
// mode: C++
// End:

