// sound_handler_sdl.cpp: Sound handling using standard SDL
//
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
//

// Based on sound_handler_sdl.cpp by Thatcher Ulrich http://tulrich.com 2003
// which has been donated to the Public Domain.


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "sound_handler_sdl.h"
#include "utility.h" // for convert_raw_data
#include "AudioDecoderSimple.h"
#include "AudioDecoderNellymoser.h"

#include "MediaHandler.h"

// TODO: drop FFMPEG/GST specific stuff, use MediaHandler instead !
#ifdef USE_FFMPEG
#include "AudioDecoderFfmpeg.h"
#elif defined(USE_GST)
#include "AudioDecoderGst.h"
#endif

#include "log.h" // will import boost::format too
#include "GnashException.h" // for SoundException

#include <cmath>
#include <vector>
#include <boost/scoped_array.hpp>
#include <SDL.h>

// Define this to get debugging call about pausing/unpausing audio
//#define GNASH_DEBUG_SDL_AUDIO_PAUSING

namespace { // anonymous

// Header of a wave file
// http://ftp.iptel.org/pub/sems/doc/full/current/wav__hdr_8c-source.html
typedef struct{
     char rID[4];            // 'RIFF'
     long int rLen;        
     char wID[4];            // 'WAVE'
     char fId[4];            // 'fmt '
     long int pcm_header_len;   // varies...
     short int wFormatTag;
     short int nChannels;      // 1,2 for stereo data is (l,r) pairs
     long int nSamplesPerSec;
     long int nAvgBytesPerSec;
     short int nBlockAlign;      
     short int nBitsPerSample;
} WAV_HDR;

// Chunk of wave file
// http://ftp.iptel.org/pub/sems/doc/full/current/wav__hdr_8c-source.html
typedef struct{
    char dId[4];            // 'data' or 'fact'
    long int dLen;
} CHUNK_HDR;

} // end of anonymous namespace

namespace gnash {
namespace media {


void
SDL_sound_handler::initAudioSpec()
{
	// This is our sound settings
	audioSpec.freq = 44100;
	audioSpec.format = AUDIO_S16SYS; // AUDIO_S8 AUDIO_U8;
	audioSpec.channels = 2;
	audioSpec.callback = SDL_sound_handler::sdl_audio_callback;
	audioSpec.userdata = this;
	audioSpec.samples = 2048;		//512 - not enough for  videostream
}


SDL_sound_handler::SDL_sound_handler(const std::string& wavefile)
	: soundOpened(false),
	  soundsPlaying(0),
	  muted(false)
{
	initAudioSpec();

	if (! wavefile.empty() ) {
        file_stream.open(wavefile.c_str());
        if (file_stream.fail()) {
            std::cerr << "Unable to write file '" << wavefile << std::endl;
            exit(1);
	    } else {
                write_wave_header(file_stream);
                std::cout << "# Created 44100 16Mhz stereo wave file:" << std::endl <<
                    "AUDIOFILE=" << wavefile << std::endl;
        }
    }

}

SDL_sound_handler::SDL_sound_handler()
	:
	soundOpened(false),
	soundsPlaying(0),
	muted(false)
{
	initAudioSpec();
}

void
SDL_sound_handler::reset()
{
	//delete_all_sounds();
	stop_all_sounds();
}

void
SDL_sound_handler::delete_all_sounds()
{
	stop_all_sounds();

	boost::mutex::scoped_lock lock(_mutex);

	for (Sounds::iterator i = m_sound_data.begin(),
	                      e = m_sound_data.end(); i != e; ++i)
	{
		sound_data* sounddata = *i;
        // The sound may have been deleted already.
        if (!sounddata) continue;

		size_t nActiveSounds = sounddata->m_active_sounds.size();

		// Decrement callback clients count 
		soundsPlaying -= nActiveSounds;

        // Increment number of sound stop request for the testing framework
		_soundsStopped += nActiveSounds;

		delete sounddata;
	}
	m_sound_data.clear();
}

SDL_sound_handler::~SDL_sound_handler()
{
	delete_all_sounds();
	if (soundOpened) SDL_CloseAudio();
	if (file_stream) file_stream.close();

}


int	SDL_sound_handler::create_sound(
	std::auto_ptr<SimpleBuffer> data,
	std::auto_ptr<SoundInfo> sinfo)
{

    log_debug("create_sound: sound format %d", sinfo->getFormat());

	assert(sinfo.get());

	std::auto_ptr<sound_data> sounddata ( new sound_data(data, sinfo) );

    // Make sure we're the only thread accessing m_sound_data here
	boost::mutex::scoped_lock lock(_mutex);

    // the vector takes ownership
	m_sound_data.push_back(sounddata.release());

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
	sounddata->append(reinterpret_cast<boost::uint8_t*>(data), data_bytes);

	return start_size;
}


// Play the index'd sample.
void
SDL_sound_handler::play_sound(int sound_handle, int loopCount, int offset, long start_position, const std::vector<sound_envelope>* envelopes)
{
	boost::mutex::scoped_lock lock(_mutex);

    // Increment number of sound start request for the testing framework
	++_soundsStarted;

	// Check if the sound exists, or if audio is muted
	if (sound_handle < 0 || static_cast<unsigned int>(sound_handle) >= m_sound_data.size() || muted)
	{
		// Invalid handle or muted
		return;
	}

	sound_data* sounddata = m_sound_data[sound_handle];

	// If this is called from a streamsoundblocktag, we only start if this
	// sound isn't already playing.
	if (start_position > 0 && ! sounddata->m_active_sounds.empty()) {
		return;
	}

	// Make sure sound actually got some data
	if (sounddata->size() < 1) {
		IF_VERBOSE_MALFORMED_SWF(
			log_swferror(_("Trying to play sound with size 0"));
		);
		return;
	}

	// Make a "active_sound" for this sound which is later placed
    // on the vector of instances of this sound being played
	std::auto_ptr<active_sound> sound ( new active_sound() );

	// Set source data to the active_sound
	sound->set_data(sounddata);

	// Set the given options of the sound
	if (start_position < 0) sound->decodingPosition = 0;
	else sound->decodingPosition = start_position;

	if (offset < 0) sound->offset = 0;
	else sound->offset = (sounddata->soundinfo->isStereo() ? offset : offset*2); // offset is stored as stereo

	sound->envelopes = envelopes;

	// Set number of loop we should do. -1 is infinte loop, 0 plays it once, 1 twice etc.
	sound->loopCount = loopCount;

	sound->decoder = NULL;

    try {

	    switch (sounddata->soundinfo->getFormat()) {
	        case AUDIO_CODEC_NELLYMOSER:
	        case AUDIO_CODEC_NELLYMOSER_8HZ_MONO:
		        sound->decoder = new AudioDecoderNellymoser(*(sounddata->soundinfo));
		        break;
	        case AUDIO_CODEC_MP3:
#ifdef USE_FFMPEG
		        sound->decoder = new AudioDecoderFfmpeg(*(sounddata->soundinfo));
		        break;
#elif defined(USE_GST)
		        sound->decoder = new AudioDecoderGst(*(sounddata->soundinfo));
		        break;
#endif
	        case AUDIO_CODEC_ADPCM:
	        default:
                sound->decoder = new AudioDecoderSimple(*(sounddata->soundinfo));
                break;
	    }
	}
	catch (MediaException& e)
	{
	    log_error("AudioDecoder initialization failed: %s", e.what());
	}

    // Push the sound onto the playing sounds container.
    // We want to do this even on audio failures so count
    // of started and stopped sound reflects expectances
    // for testing framework.
	sounddata->m_active_sounds.push_back(sound.release());

	if (!soundOpened) {
		if (SDL_OpenAudio(&audioSpec, NULL) < 0 ) {
			log_error(_("Unable to start SDL sound: %s"), SDL_GetError());
			return;
		}
		soundOpened = true;

	}

	// Increment callback clients count 
	++soundsPlaying;

	if (soundsPlaying == 1) {
#ifdef GNASH_DEBUG_SDL_AUDIO_PAUSING
		log_debug("Unpausing SDL Audio...");
#endif
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
		return;
	}

	
	sound_data* sounddata = m_sound_data[sound_handle];

	size_t nActiveSounds = sounddata->m_active_sounds.size();

	// Decrement callback clients count 
	soundsPlaying -= nActiveSounds;

    // Increment number of sound stop request for the testing framework
	_soundsStopped += nActiveSounds;

	sounddata->clearActiveSounds();

}


// this gets called when it's done with a sample.
void	SDL_sound_handler::delete_sound(int sound_handle)
{
	boost::mutex::scoped_lock lock(_mutex);

    log_debug ("deleting sound :%d", sound_handle);

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

	for (Sounds::iterator i = m_sound_data.begin(),
	                      e = m_sound_data.end(); i != e; ++i)
	{
		sound_data* sounddata = *i;

        // The sound may have been deleted already.		
		if (!sounddata) continue;
		size_t nActiveSounds = sounddata->m_active_sounds.size();

		// Decrement callback clients count 
		soundsPlaying -= nActiveSounds;

        // Increment number of sound stop request for the testing framework
		_soundsStopped += nActiveSounds;

		sounddata->clearActiveSounds();
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
void
SDL_sound_handler::mute()
{
	//stop_all_sounds();
	boost::mutex::scoped_lock lock(_mutex);
	muted = true;
}

// gnash calls this to unmute audio
void
SDL_sound_handler::unmute()
{
	boost::mutex::scoped_lock lock(_mutex);
	muted = false;
}

bool SDL_sound_handler::is_muted()
{
	boost::mutex::scoped_lock lock(_mutex);
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
		// TODO: throw SoundException ?
		return;
	}

	if (!soundOpened) {
		if (SDL_OpenAudio(&audioSpec, NULL) < 0 ) {
        		boost::format fmt = boost::format(
				_("Unable to start aux SDL sound: %s"))
				% SDL_GetError();
			throw SoundException(fmt.str());
		}
		soundOpened = true;
	}

	// Increment callback clients count 
	++soundsPlaying;

#ifdef GNASH_DEBUG_SDL_AUDIO_PAUSING
	log_debug("Unpausing SDL Audio...");
#endif
	SDL_PauseAudio(0);

}

void	SDL_sound_handler::detach_aux_streamer(void* owner)
{
	boost::mutex::scoped_lock lock(_mutex);

	// WARNING: erasing would break any iteration in the map
	CallbacksMap::iterator it2=m_aux_streamer.find(owner);
	if ( it2 != m_aux_streamer.end() )
	{
		// Decrement callback clients count 
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
        // TODO: should we cache this in the sound_data object ?
		unsigned int ret = sampleCount / sampleRate * 1000;
		ret += ((sampleCount % sampleRate) * 1000) / sampleRate;
		//if (sounddata->soundinfo->isStereo()) ret = ret / 2;
		return ret;
	} else {
		return 0;
	}
}

unsigned int SDL_sound_handler::tell(int sound_handle)
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
	if (sounddata->m_active_sounds.empty()) return 0;

	// We use the first active sound of this.
	active_sound* asound = sounddata->m_active_sounds.front();

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

sound_handler*
create_sound_handler_sdl(const std::string& wave_file)
// Factory.
{
	return new SDL_sound_handler(wave_file);
}

// Pointer handling and checking functions
boost::uint8_t*
active_sound::getDecodedData(unsigned long int pos)
{
	if ( _decodedData.get() )
	{
		assert(pos < _decodedData->size());
		return _decodedData->data()+pos;
	}
	else return 0;
}

boost::uint8_t*
active_sound::getEncodedData(unsigned long int pos)
{
	assert(_encodedData);
	return _encodedData->data(pos);
}

void active_sound::set_data(sound_data* idata)
{
	_encodedData = idata;
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
use_envelopes(active_sound& sound, unsigned int length)
{
	// Check if this is the time to use envelopes yet
	if (sound.current_env == 0 && (*sound.envelopes)[0].m_mark44 > sound.samples_played+length/2)
	{
		return;

	}
	// switch to the next envelope if needed and possible
	else if (sound.current_env < sound.envelopes->size()-1 && (*sound.envelopes)[sound.current_env+1].m_mark44 >= sound.samples_played)
	{
		sound.current_env++;
	}

	// Current envelope position
	boost::int32_t cur_env_pos = sound.envelopes->operator[](sound.current_env).m_mark44;

	// Next envelope position
	boost::uint32_t next_env_pos = 0;
	if (sound.current_env == (sound.envelopes->size()-1)) {
		// If there is no "next envelope" then set the next envelope start point to be unreachable
		next_env_pos = cur_env_pos + length;
	} else {
		next_env_pos = (*sound.envelopes)[sound.current_env+1].m_mark44;
	}

	unsigned int startpos = 0;
	// Make sure we start adjusting at the right sample
	if (sound.current_env == 0 && (*sound.envelopes)[sound.current_env].m_mark44 > sound.samples_played) {
		startpos = sound.playbackPosition + ((*sound.envelopes)[sound.current_env].m_mark44 - sound.samples_played)*2;
	} else {
		startpos = sound.playbackPosition;
	}

	boost::int16_t* data = reinterpret_cast<boost::int16_t*>(sound.getDecodedData(startpos));

	for (unsigned int i=0; i < length/2; i+=2) {
		float left = static_cast<float>((*sound.envelopes)[sound.current_env].m_level0 / 32768.0);
		float right = static_cast<float>((*sound.envelopes)[sound.current_env].m_level1 / 32768.0);

		data[i] = static_cast<boost::int16_t>(data[i] * left); // Left
		data[i+1] = static_cast<boost::int16_t>(data[i+1] * right); // Right

		if ((sound.samples_played+(length/2-i)) >= next_env_pos && sound.current_env != (sound.envelopes->size()-1)) {
			sound.current_env++;
			// Next envelope position
			if (sound.current_env == (sound.envelopes->size()-1)) {
				// If there is no "next envelope" then set the next envelope start point to be unreachable
				next_env_pos = cur_env_pos + length;
			} else {
				next_env_pos = (*sound.envelopes)[sound.current_env+1].m_mark44;
			}
		}
	}
}


// Prepare for mixing/adding (volume adjustments) and mix/add.
static void
do_mixing(Uint8* stream, active_sound& sound, Uint8* data, unsigned int mix_length, unsigned int volume)
{
	// If the volume needs adjustments we call a function to do that (why are we doing this manually ?)
	if (volume != 100) {
		adjust_volume(reinterpret_cast<boost::int16_t*>(data), mix_length, volume);
	} else if (sound.envelopes != NULL) {
		use_envelopes(sound, mix_length);
	}

	// Mix the raw data
	SDL_MixAudio(static_cast<Uint8*>(stream),static_cast<const Uint8*>(data), mix_length, SDL_MIX_MAXVOLUME);

	// Update sound info
	sound.playbackPosition += mix_length;

	// Sample size is always 2
	sound.samples_played += mix_length / 2;
}


// write a wave header, using the current audioSpec settings
void SDL_sound_handler::write_wave_header(std::ofstream& outfile)
{
 
  // allocate wav header
  WAV_HDR wav;
  CHUNK_HDR chk;
 
  // setup wav header
  std::strncpy(wav.rID, "RIFF", 4);
  std::strncpy(wav.wID, "WAVE", 4);
  std::strncpy(wav.fId, "fmt ", 4);
 
  wav.nBitsPerSample = ((audioSpec.format == AUDIO_S16SYS) ? 16 : 0);
  wav.nSamplesPerSec = audioSpec.freq;
  wav.nAvgBytesPerSec = audioSpec.freq;
  wav.nAvgBytesPerSec *= wav.nBitsPerSample / 8;
  wav.nAvgBytesPerSec *= audioSpec.channels;
  wav.nChannels = audioSpec.channels;
    
  wav.pcm_header_len = 16;
  wav.wFormatTag = 1;
  wav.rLen = sizeof(WAV_HDR) + sizeof(CHUNK_HDR);
  wav.nBlockAlign = audioSpec.channels * wav.nBitsPerSample / 8;

  // setup chunk header
  std::strncpy(chk.dId, "data", 4);
  chk.dLen = 0;
 
  /* write riff/wav header */
  outfile.write((char *)&wav, sizeof(WAV_HDR));
 
  /* write chunk header */
  outfile.write((char *)&chk, sizeof(CHUNK_HDR));
 
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

	boost::mutex::scoped_lock lock(handler->_mutex);

	if ( handler->isPaused() ) return;

	int finalVolume = int(128*handler->getFinalVolume()/100.0);

	// If nothing to play there is no reason to play
	// Is this a potential deadlock problem?
	if (handler->soundsPlaying == 0 && handler->m_aux_streamer.empty()) {
#ifdef GNASH_DEBUG_SDL_AUDIO_PAUSING
		log_debug("Pausing SDL Audio...");
#endif
		SDL_PauseAudio(1);
		return;
	}

	// Mixed sounddata buffer
	Uint8* buffer = stream;
	memset(buffer, 0, buffer_length);

	// call NetStream or Sound audio callbacks
	if ( !handler->m_aux_streamer.empty() )
	{
		boost::scoped_array<boost::uint8_t> buf ( new boost::uint8_t[buffer_length] );

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
				// Decrement callback clients count 
				handler->soundsPlaying--;
			} else {
				++it;
			}
			SDL_MixAudio(stream, buf.get(), buffer_length, finalVolume);

		}
	}

    const SDL_sound_handler::Sounds& soundData = handler->m_sound_data;

	// Run through all the sounds. TODO: don't call .size() at every iteration !
	for (SDL_sound_handler::Sounds::const_iterator i = soundData.begin(),
	            e = soundData.end(); i != e; ++i)
	{
	    // Check whether sound has been deleted first.
        if (!*i) continue;
		handler->mixSoundData(**i, buffer, buffer_length);
	}

	// 
	// WRITE CONTENTS OF stream TO FILE
	//
	if (handler->file_stream) {
            handler->file_stream.write((char*) stream, buffer_length_in);
            // now, mute all audio
            memset ((void*) stream, 0, buffer_length_in);
	}

	// Now, after having "consumed" all sounds, blank out
	// the buffer if muted..
	if ( handler->muted )
	{
		memset ((void*) stream, 0, buffer_length_in);
	}



}

void
sound_data::append(boost::uint8_t* data, unsigned int size)
{
    // Make sure we're always appropriately padded...
    media::MediaHandler* mh = media::MediaHandler::get(); // TODO: don't use this static !
    const size_t paddingBytes = mh ? mh->getInputPaddingSize() : 0;
    _buf->reserve(_buf->size()+size+paddingBytes);
	_buf->append(data, size);

    // since ownership was transferred...
	delete [] data;
}

sound_data::sound_data(std::auto_ptr<SimpleBuffer> data, std::auto_ptr<SoundInfo> info, int nVolume)
    :
    _buf(data),
    soundinfo(info),
    volume(nVolume)
{
    if ( _buf.get() )
    {
        // Make sure we're appropriately padded (this is an event sound)
        media::MediaHandler* mh = media::MediaHandler::get(); // TODO: don't use this static !
        const size_t paddingBytes = mh ? mh->getInputPaddingSize() : 0;
        if ( _buf->capacity() - _buf->size() < paddingBytes ) {
            log_error("sound_data creator didn't appropriately pad sound data. "
                "We'll do now, but will cost memory copies.");
            _buf->reserve(_buf->size()+paddingBytes);
        }
    }
    else
    {
        _buf.reset(new SimpleBuffer());
    }
}

void
sound_data::clearActiveSounds()
{
	for (ActiveSounds::iterator i=m_active_sounds.begin(), e=m_active_sounds.end(); i!=e; ++i)
	{
		delete *i;
	}
	m_active_sounds.clear();
}

sound_data::ActiveSounds::iterator
sound_data::eraseActiveSound(ActiveSounds::iterator i)
{
	delete *i;
	return m_active_sounds.erase(i);
}

/*private*/
void
SDL_sound_handler::mixActiveSound(active_sound& sound, sound_data& sounddata,
        Uint8* buffer, unsigned int buffer_length)
{
	// If there exist no decoder, then we can't decode!
	if (sound.decoder == NULL) return;

    // concatenate global volume
	int volume = int(sounddata.volume*getFinalVolume()/100.0);

	// When the current sound don't have enough decoded data to fill the buffer, 
	// we first mix what is already decoded, then decode some more data, and
	// mix some more until the buffer is full. If a sound loops the magic
	// happens here ;)
	//
	if (sound.decodedDataSize() - sound.playbackPosition < buffer_length 
		&& (sound.decodingPosition < sound.encodedDataSize() || sound.loopCount != 0))
	{
		// First we mix what is decoded
		unsigned int index = 0;
		if (sound.decodedDataSize() - sound.playbackPosition > 0)
		{
			index = sound.decodedDataSize() - sound.playbackPosition;

			do_mixing(buffer, sound, sound.getDecodedData(sound.playbackPosition),
				index, volume);

		}

		// Then we decode some data
		// We loop until the size of the decoded sound is greater than the buffer size,
		// or there is no more to decode.
		unsigned int decoded_size = 0;

		// Delete any previous raw_data
		sound.deleteDecodedData();

		while(decoded_size < buffer_length)
		{

			// If we need to loop, we reset the data pointer
			if (sound.encodedDataSize() == sound.decodingPosition && sound.loopCount != 0) {
				sound.loopCount--;
				sound.decodingPosition = 0;
				sound.samples_played = 0;
			}

			// Test if we will get problems... Should not happen...
			assert(sound.encodedDataSize() > sound.decodingPosition);
			
			// temp raw buffer
			Uint8* tmp_raw_buffer=0;
			boost::uint32_t tmp_raw_buffer_size = 0;
			boost::uint32_t decodedBytes = 0;

			boost::uint32_t inputSize = 0;
			bool parse = true;
			if (sounddata.soundinfo->getFormat() == AUDIO_CODEC_ADPCM) {
				parse = false;
				if (sounddata.m_frames_size.size() > 0) {
                    inputSize = sounddata.m_frames_size[sound.decodingPosition];
                }
				else {
                    inputSize = sound.encodedDataSize() - sound.decodingPosition;
                }
			} else {
				inputSize = sound.encodedDataSize() - sound.decodingPosition;
			}

			tmp_raw_buffer = sound.decoder->decode(sound.getEncodedData(sound.decodingPosition), 
					inputSize, tmp_raw_buffer_size, decodedBytes, parse);

			sound.decodingPosition += decodedBytes;

			// tmp_raw_buffer ownership transferred here
			sound.appendDecodedData(tmp_raw_buffer, tmp_raw_buffer_size);

			decoded_size += tmp_raw_buffer_size;

			// no more to decode from this sound, so we break the loop
			if ((sound.encodedDataSize() <= sound.decodingPosition && sound.loopCount == 0)
                    || (tmp_raw_buffer_size == 0 && decodedBytes == 0)) {
				sound.decodingPosition = sound.encodedDataSize();
				break;
			}

		} // end of "decode min. bufferlength data" while loop

		sound.playbackPosition = 0;

		// Determine how much should be mixed
		unsigned int mix_length = 0;
		if (decoded_size >= buffer_length - index) {
			mix_length = buffer_length - index;
		} else { 
			mix_length = decoded_size;
		}
		if (sound.decodedDataSize() < 2)
		{
			log_error("Something went terribly wrong during mixing of an active sound");
			return; // something went terrible wrong
		}
		do_mixing(buffer+index, sound, sound.getDecodedData(0), mix_length, volume);

	}

	// When the current sound has enough decoded data to fill 
	// the buffer, we do just that.
	else if (sound.decodedDataSize() > sound.playbackPosition && sound.decodedDataSize() - sound.playbackPosition > buffer_length )
	{

		do_mixing(buffer, sound, sound.getDecodedData(sound.playbackPosition), 
			buffer_length, volume);

	}

	// When the current sound doesn't have anymore data to decode,
	// and doesn't loop (anymore), but still got unplayed data,
	// we put the last data on the stream
	else if (sound.decodedDataSize() - sound.playbackPosition <= buffer_length && sound.decodedDataSize() > sound.playbackPosition+1)
	{
	

		do_mixing(buffer, sound, sound.getDecodedData(sound.playbackPosition), 
			sound.decodedDataSize() - sound.playbackPosition, volume);

		sound.playbackPosition = sound.decodedDataSize();
	} 
}

/*private*/
void
SDL_sound_handler::mixSoundData(sound_data& sounddata, Uint8* buffer, unsigned int buffer_length)
{
	for (sound_data::ActiveSounds::iterator
		 i=sounddata.m_active_sounds.begin();
		 i != sounddata.m_active_sounds.end(); // don't cache .end(), isn't necessarely safe on erase
	    )
	{

		// Temp variables to make the code simpler and easier to read
		active_sound* sound = *i; 

		mixActiveSound(*sound, sounddata, buffer, buffer_length);

		// Sound is done, remove it from the active list
		if (sound->decodingPosition == sound->encodedDataSize() && sound->playbackPosition == sound->decodedDataSize() && sound->loopCount == 0)
		{
			i = sounddata.eraseActiveSound(i);

			// Decrement callback clients count 
			soundsPlaying--;

            // Increment number of sound stop request for the testing framework
			_soundsStopped++;
		} 
		else
		{
			++i;
		}
	}
}

} // gnash.media namespace 
} // namespace gnash

// Local Variables:
// mode: C++
// End:

