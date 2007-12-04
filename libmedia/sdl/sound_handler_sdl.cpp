// sound_handler_sdl.cpp: Sound handling using standard SDL
//
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

// $Id: sound_handler_sdl.cpp,v 1.8 2007/12/04 11:45:27 strk Exp $

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sound_handler_sdl.h"
#include "utility.h" // for convert_raw_data
#include "AudioDecoderSimple.h"
#include "AudioDecoderNellymoser.h"

#ifdef USE_FFMPEG
#include "AudioDecoderFfmpeg.h"
#endif

#ifdef USE_MAD
#include "AudioDecoderMad.h"
#endif

#include "log.h"
#include <cmath>
#include <vector>
#include <boost/scoped_array.hpp>
#include <SDL.h>

namespace gnash {
namespace media {

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
	unsigned int data_bytes,
	std::auto_ptr<SoundInfo> sinfo)
// Called to create a sample.  We'll return a sample ID that
// can be use for playing it.
{

	assert(sinfo.get());
	std::auto_ptr<sound_data> sounddata ( new sound_data );

	//sounddata->data_size = data_bytes;
	sounddata->volume = 100;
	sounddata->soundinfo = sinfo;

	boost::mutex::scoped_lock lock(_mutex);

	switch (sounddata->soundinfo->getFormat())
	{
	case AUDIO_CODEC_MP3:
#ifndef USE_FFMPEG
#ifndef USE_MAD
		log_error(_("gnash has not been compiled to handle mp3 audio"));
		return -1;
#endif
#endif
		sounddata->append(reinterpret_cast<uint8_t*>(data), data_bytes);
		break;

	case AUDIO_CODEC_RAW:
	case AUDIO_CODEC_ADPCM:
	case AUDIO_CODEC_UNCOMPRESSED:
	case AUDIO_CODEC_NELLYMOSER:
		sounddata->append(reinterpret_cast<uint8_t*>(data), data_bytes);
		break;

	default:
		// Unhandled format.
		log_error(_("unknown sound format %d requested; gnash does not handle it"), (int)sounddata->soundinfo->getFormat());
		return -1; // Unhandled format, set to NULL.
	}

	m_sound_data.push_back(sounddata.release()); // the vector takes ownership
	int sound_id = m_sound_data.size()-1;

	return sound_id;

}

// this gets called when a stream gets more data
long	SDL_sound_handler::fill_stream_data(unsigned char* data, unsigned int data_bytes, unsigned int /*sample_count*/, int handle_id)

{

	boost::mutex::scoped_lock lock(_mutex);
	// @@ does a negative handle_id have any meaning ?
	//    should we change it to unsigned instead ?
	if (handle_id < 0 || (unsigned int) handle_id+1 > m_sound_data.size())
	{
		delete [] data;
		return -1;
	}
	sound_data* sounddata = m_sound_data[handle_id];

	// If doing ADPCM, knowing the framesize is needed to decode!
	if (sounddata->soundinfo->getFormat() == AUDIO_CODEC_ADPCM) {
		sounddata->m_frames_size[sounddata->size()] = data_bytes;
	}

	// Handling of the sound data
	size_t start_size = sounddata->size();
	sounddata->append(reinterpret_cast<uint8_t*>(data), data_bytes);

	return start_size;
}


void	SDL_sound_handler::play_sound(int sound_handle, int loop_count, int offset, long start_position, const std::vector<sound_envelope>* envelopes)
// Play the index'd sample.
{
	boost::mutex::scoped_lock lock(_mutex);

	// Check if the sound exists, or if audio is muted
	if (sound_handle < 0 || static_cast<unsigned int>(sound_handle) >= m_sound_data.size() || muted)
	{
		// Invalid handle or muted
		return;
	}

	sound_data* sounddata = m_sound_data[sound_handle];

	// If this is called from a streamsoundblocktag, we only start if this
	// sound isn't already playing.
	if (start_position > 0 && sounddata->m_active_sounds.size() > 0) {
		return;
	}

	// Make sure sound actually got some data
	if (sounddata->size() < 1) {
		IF_VERBOSE_MALFORMED_SWF(
			log_swferror(_("Trying to play sound with size 0"));
		);
		return;
	}

	// Make a "active_sound" for this sound which is later placed on the vector of instances of this sound being played
	std::auto_ptr<active_sound> sound ( new active_sound() );

	// Set source data to the active_sound
	sound->set_data(sounddata);

	// Set the given options of the sound
	if (start_position < 0) sound->position = 0;
	else sound->position = start_position;

	if (offset < 0) sound->offset = 0;
	else sound->offset = (sounddata->soundinfo->isStereo() ? offset : offset*2); // offset is stored as stereo

	sound->envelopes = envelopes;

	// Set number of loop we should do. -1 is infinte loop, 0 plays it once, 1 twice etc.
	sound->loop_count = loop_count;

	sound->decoder = NULL;

	switch (sounddata->soundinfo->getFormat()) {
	case AUDIO_CODEC_NELLYMOSER:
	case AUDIO_CODEC_NELLYMOSER_8HZ_MONO:
		sound->decoder = new AudioDecoderNellymoser();

		if (!sound->decoder->setup(sounddata->soundinfo.get())) {
			log_error("The audio decoder can't decode the audio");
			delete sound->decoder;
			sound->decoder = NULL;
		}

		break;
	case AUDIO_CODEC_MP3:
#ifdef USE_MAD
		sound->decoder = new AudioDecoderMad();

		if (!sound->decoder->setup(sounddata->soundinfo.get())) {
			log_error("The audio decoder can't decode the audio");
			delete sound->decoder;
			sound->decoder = NULL;
		}

		break;
#endif
#ifdef USE_FFMPEG
		sound->decoder = new AudioDecoderFfmpeg();

		if (!sound->decoder->setup(sounddata->soundinfo.get())) {
			log_error("The audio decoder can't decode the audio");
			delete sound->decoder;
			sound->decoder = NULL;
		}

		break;
#endif
	case AUDIO_CODEC_ADPCM:
	default:

		sound->decoder = new AudioDecoderSimple();

		if (!sound->decoder->setup(sounddata->soundinfo.get())) {
			log_error("The audio decoder can't decode the audio");
			delete sound->decoder;
			sound->decoder = NULL;
		}

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
	sounddata->m_active_sounds.push_back(sound.release());

	if (soundsPlaying == 1) {
		SDL_PauseAudio(0);
	}

}


void	SDL_sound_handler::stop_sound(int sound_handle)
{
	boost::mutex::scoped_lock lock(_mutex);

	// Check if the sound exists.
	if (sound_handle < 0 || (unsigned int) sound_handle >= m_sound_data.size())
	{
		// Invalid handle.
	} else {
	
		sound_data* sounddata = m_sound_data[sound_handle];
	
		for (boost::int32_t i = (boost::int32_t) sounddata->m_active_sounds.size()-1; i >-1; i--) {

			//active_sound* sound = sounddata->m_active_sounds[i];

			// Stop sound, remove it from the active list
			//sound->delete_raw_data();
			//delete sound->decoder;
			sounddata->m_active_sounds.erase(sounddata->m_active_sounds.begin() + i);
			
			--soundsPlaying;
			++_soundsStopped;
		}
	}

}


// this gets called when it's done with a sample.
void	SDL_sound_handler::delete_sound(int sound_handle)
{
	boost::mutex::scoped_lock lock(_mutex);

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
	boost::mutex::scoped_lock lock(_mutex);

	boost::int32_t num_sounds = (boost::int32_t) m_sound_data.size()-1;
	for (boost::int32_t j = num_sounds; j > -1; j--) {//Optimized
		sound_data* sounddata = m_sound_data[j];
		boost::int32_t num_active_sounds = (boost::int32_t) sounddata->m_active_sounds.size()-1;
		for (boost::int32_t i = num_active_sounds; i > -1; i--) {

			//active_sound* sound = sounddata->m_active_sounds[i];

			// Stop sound, remove it from the active list
			//delete sound->decoder;
			sounddata->m_active_sounds.erase(sounddata->m_active_sounds.begin() + i);
			
			--soundsPlaying;
			++_soundsStopped;
		}
	}
}


//	returns the sound volume level as an integer from 0 to 100,
//	where 0 is off and 100 is full volume. The default setting is 100.
int	SDL_sound_handler::get_volume(int sound_handle) {

	boost::mutex::scoped_lock lock(_mutex);

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

	boost::mutex::scoped_lock lock(_mutex);

	// Check if the sound exists.
	if (sound_handle < 0 || static_cast<unsigned int>(sound_handle) >= m_sound_data.size())
	{
		// Invalid handle.
	} else {

		// Set volume for this sound. Should this only apply to the active sounds?
		m_sound_data[sound_handle]->volume = volume;
	}


}
	
SoundInfo* SDL_sound_handler::get_sound_info(int sound_handle) {

	boost::mutex::scoped_lock lock(_mutex);

	// Check if the sound exists.
	if (sound_handle >= 0 && static_cast<unsigned int>(sound_handle) < m_sound_data.size())
	{
		return m_sound_data[sound_handle]->soundinfo.get();
	} else {
		return NULL;
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
	boost::mutex::scoped_lock lock(_mutex);
	assert(owner);
	assert(ptr);

	if ( ! m_aux_streamer.insert(std::make_pair(owner, ptr)).second )
	{
		// Already in the hash.
		return;
	}

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
	boost::mutex::scoped_lock lock(_mutex);

	CallbacksMap::iterator it2=m_aux_streamer.find(owner);
	if ( it2 != m_aux_streamer.end() )
	{
		// WARNING: erasing would break any iteration in the map
		--soundsPlaying;
		m_aux_streamer.erase(it2);
	}
}

unsigned int SDL_sound_handler::get_duration(int sound_handle)
{
	boost::mutex::scoped_lock lock(_mutex);

	// Check if the sound exists.
	if (sound_handle < 0 || (unsigned int) sound_handle >= m_sound_data.size())
	{
		// Invalid handle.
		return 0;
	}

	sound_data* sounddata = m_sound_data[sound_handle];

	boost::uint32_t sampleCount = sounddata->soundinfo->getSampleCount();
	boost::uint32_t sampleRate = sounddata->soundinfo->getSampleRate();

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

unsigned int SDL_sound_handler::get_position(int sound_handle)
{
	boost::mutex::scoped_lock lock(_mutex);

	// Check if the sound exists.
	if (sound_handle < 0 || (unsigned int) sound_handle >= m_sound_data.size())
	{
		// Invalid handle.
		return 0;
	}

	sound_data* sounddata = m_sound_data[sound_handle];

	// If there is no active sounds, return 0
	if (sounddata->m_active_sounds.size() == 0) return 0;

	// We use the first active sound of this.
	active_sound* asound = sounddata->m_active_sounds[0];

	// Return the playhead position in milliseconds
	unsigned int ret = asound->samples_played / audioSpec.freq * 1000;
	ret += ((asound->samples_played % audioSpec.freq) * 1000) / audioSpec.freq;
	if (audioSpec.channels > 1) ret = ret / audioSpec.channels;
	return ret;
}

sound_handler*
create_sound_handler_sdl()
// Factory.
{
	return new SDL_sound_handler;
}

// Pointer handling and checking functions
uint8_t*
active_sound::get_raw_data_ptr(unsigned long int pos)
{
	if ( _decodedData.get() )
	{
		return _decodedData->data(pos);
	}
	else return 0;
}

uint8_t*
active_sound::get_data_ptr(unsigned long int pos)
{
	assert(_undecodedData);
	return _undecodedData->data(pos);
}

void active_sound::set_data(sound_data* idata)
{
	_undecodedData = idata;
}

void active_sound::deleteDecodedData()
{
	_decodedData.reset();
}

// AS-volume adjustment
void adjust_volume(boost::int16_t* data, int size, int volume)
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
	boost::int32_t cur_env_pos = sound->envelopes->operator[](sound->current_env).m_mark44;

	// Next envelope position
	boost::uint32_t next_env_pos = 0;
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

	boost::int16_t* data = reinterpret_cast<boost::int16_t*>(sound->get_raw_data_ptr(startpos));

	for (unsigned int i=0; i < length/2; i+=2) {
		float left = static_cast<float>((*sound->envelopes)[sound->current_env].m_level0 / 32768.0);
		float right = static_cast<float>((*sound->envelopes)[sound->current_env].m_level1 / 32768.0);

		data[i] = static_cast<boost::int16_t>(data[i] * left); // Left
		data[i+1] = static_cast<boost::int16_t>(data[i+1] * right); // Right

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
		adjust_volume(reinterpret_cast<boost::int16_t*>(data), mix_length, volume);
	} else if (sound->envelopes != NULL) {
		use_envelopes(sound, mix_length);
	}

	// Mix the raw data
	SDL_MixAudio(static_cast<Uint8*>(stream),static_cast<const Uint8*>(data), mix_length, SDL_MIX_MAXVOLUME);

	// Update sound info
	sound->raw_position += mix_length;

	// Sample size is always 2
	sound->samples_played += mix_length / 2;
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

	boost::mutex::scoped_lock lock(handler->_mutex);

	// Mixed sounddata buffer
	Uint8* buffer = stream;
	memset(buffer, 0, buffer_length);

	// call NetStream or Sound audio callbacks
	if ( !handler->m_aux_streamer.empty() )
	{
		boost::scoped_array<uint8_t> buf ( new uint8_t[buffer_length] );

		// Loop through the attached sounds
		CallbacksMap::iterator it = handler->m_aux_streamer.begin();
		CallbacksMap::iterator end = handler->m_aux_streamer.end();
		while (it != end) {
			memset(buf.get(), 0, buffer_length);

			SDL_sound_handler::aux_streamer_ptr aux_streamer = it->second; 
			void* owner = it->first;

			// If false is returned the sound doesn't want to be attached anymore
			bool ret = (aux_streamer)(owner, buf.get(), buffer_length);
			if (!ret) {
				CallbacksMap::iterator it2=it;
				++it2; // before we erase it
				handler->m_aux_streamer.erase(it); // FIXME: isn't this terribly wrong ?
				it = it2;
				handler->soundsPlaying--;
			} else {
				++it;
			}
			SDL_MixAudio(stream, buf.get(), buffer_length, SDL_MIX_MAXVOLUME);

		}
	}

	// Run through all the sounds. TODO: don't call .size() at every iteration !
	for(boost::uint32_t i=0; i < handler->m_sound_data.size(); i++) {
		sound_data* sounddata = handler->m_sound_data[i];
		for(boost::uint32_t j = 0; j < sounddata->m_active_sounds.size(); j++) {

			// Temp variables to make the code simpler and easier to read
			active_sound* sound = sounddata->m_active_sounds[j];

			// If there exist no decoder, then we can't decode!
			if (sound->decoder == NULL) continue;

			// When the current sound dont have enough decoded data to fill the buffer, 
			// we first mix what is already decoded, then decode some more data, and
			// mix some more until the buffer is full. If a sound loops the magic
			// happens here ;)
			if (sound->rawDataSize() - sound->raw_position < buffer_length 
				&& (sound->position < sound->dataSize() || sound->loop_count != 0)) {
				// First we mix what is decoded
				unsigned int index = 0;
				if (sound->rawDataSize() - sound->raw_position > 0)
				{
					index = sound->rawDataSize() - sound->raw_position;

					do_mixing(stream, sound, sound->get_raw_data_ptr(sound->raw_position),
						index, sounddata->volume);

				}

				// Then we decode some data
				// We loop until the size of the decoded sound is greater than the buffer size,
				// or there is no more to decode.
				unsigned int decoded_size = 0;

				// Delete any previous raw_data
				sound->deleteDecodedData();

				while(decoded_size < buffer_length)
				{

					// If we need to loop, we reset the data pointer
					if (sound->dataSize() == sound->position && sound->loop_count != 0) {
						sound->loop_count--;
						sound->position = 0;
						sound->samples_played = 0;
					}

					// Test if we will get problems... Should not happen...
					assert(sound->dataSize() > sound->position);
					
					// temp raw buffer
					Uint8* tmp_raw_buffer;
					boost::uint32_t tmp_raw_buffer_size = 0;
					boost::uint32_t decodedBytes = 0;

					boost::uint32_t inputSize = 0;
					bool parse = true;
					if (sounddata->soundinfo->getFormat() == AUDIO_CODEC_ADPCM) {
						parse = false;
						if (sounddata->m_frames_size.size() > 0) inputSize = sounddata->m_frames_size[sound->position];
						else inputSize = sound->dataSize() - sound->position;
					} else {
						inputSize = sound->dataSize() - sound->position;
					}

					tmp_raw_buffer = sound->decoder->decode(sound->get_data_ptr(sound->position), 
													inputSize, tmp_raw_buffer_size, decodedBytes, parse);

					sound->position += decodedBytes;

					sound->appendDecodedData(tmp_raw_buffer, tmp_raw_buffer_size);

					decoded_size += tmp_raw_buffer_size;

					// no more to decode from this sound, so we break the loop
					if (sound->dataSize() <= sound->position && sound->loop_count == 0 || tmp_raw_buffer_size == 0 && decodedBytes == 0) {
						sound->position = sound->dataSize();
						break;
					}

				} // end of "decode min. bufferlength data" while loop

				sound->raw_position = 0;

				// Determine how much should be mixed
				unsigned int mix_length = 0;
				if (decoded_size >= buffer_length - index) {
					mix_length = buffer_length - index;
				} else { 
					mix_length = decoded_size;
				}
				if (sound->rawDataSize() < 2) continue; // something went terrible wrong
				do_mixing(stream+index, sound, sound->get_raw_data_ptr(0), mix_length, sounddata->volume);

			// When the current sound has enough decoded data to fill 
			// the buffer, we do just that.
			} else if (sound->rawDataSize() > sound->raw_position && sound->rawDataSize() - sound->raw_position > buffer_length ) {

				do_mixing(stream, sound, sound->get_raw_data_ptr(sound->raw_position), 
					buffer_length, sounddata->volume);

			// When the current sound doesn't have anymore data to decode,
			// and doesn't loop (anymore), but still got unplayed data,
			// we put the last data on the stream
			} else if (sound->rawDataSize() - sound->raw_position <= buffer_length && sound->rawDataSize() > sound->raw_position+1) {
			

				do_mixing(stream, sound, sound->get_raw_data_ptr(sound->raw_position), 
					sound->rawDataSize() - sound->raw_position, sounddata->volume);

				sound->raw_position = sound->rawDataSize();
			} 

			// Sound is done, remove it from the active list
			if (sound->position == sound->dataSize() && sound->raw_position == sound->rawDataSize() && sound->loop_count == 0) {
				sounddata->m_active_sounds.erase(sounddata->m_active_sounds.begin() + j);
				handler->soundsPlaying--;
				handler->_soundsStopped++;

			} 
		} // active sounds loop
	} // existing sounds loop

}

} // gnash.media namespace 
} // namespace gnash

// Local Variables:
// mode: C++
// End:

