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
#include "SoundInfo.h"

#include "MediaHandler.h"

#include "log.h" // will import boost::format too
#include "GnashException.h" // for SoundException

#include <cmath>
#include <vector>
#include <boost/scoped_array.hpp>
#include <SDL.h>

// Define this to get debugging call about pausing/unpausing audio
//#define GNASH_DEBUG_SDL_AUDIO_PAUSING

// Mixing and decoding debugging
//#define GNASH_DEBUG_MIXING

// Debug create_sound/delete_sound/play_sound/stop_sound
//#define GNASH_DEBUG_SOUNDS_MANAGEMENT

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
namespace sound {


void
SDL_sound_handler::initAudioSpec()
{
	// This is our sound settings
	audioSpec.freq = 44100;

    // Each sample is a signed 16-bit audio in system-endian format
	audioSpec.format = AUDIO_S16SYS; 

    // We want to be pulling samples for 2 channels:
    // {left,right},{left,right},...
	audioSpec.channels = 2;

	audioSpec.callback = SDL_sound_handler::sdl_audio_callback;

	audioSpec.userdata = this;

    //512 - not enough for  videostream
	audioSpec.samples = 2048;	
}


SDL_sound_handler::SDL_sound_handler(const std::string& wavefile)
	:
    soundOpened(false),
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

	for (Sounds::iterator i = _sounds.begin(),
	                      e = _sounds.end(); i != e; ++i)
	{
		EmbedSound* sounddata = *i;
        // The sound may have been deleted already.
        if (!sounddata) continue;

		size_t nInstances = sounddata->_soundInstances.size();

		// Decrement callback clients count 
		soundsPlaying -= nInstances;

        // Increment number of sound stop request for the testing framework
		_soundsStopped += nInstances;

		delete sounddata;
	}
	_sounds.clear();
}

SDL_sound_handler::~SDL_sound_handler()
{
	delete_all_sounds();
	if (soundOpened) SDL_CloseAudio();
	if (file_stream) file_stream.close();

}


int	SDL_sound_handler::create_sound(
	std::auto_ptr<SimpleBuffer> data,
	std::auto_ptr<media::SoundInfo> sinfo)
{

	assert(sinfo.get());

	std::auto_ptr<EmbedSound> sounddata ( new EmbedSound(data, sinfo) );

	// Make sure we're the only thread accessing _sounds here
	boost::mutex::scoped_lock lock(_mutex);

	// the vector takes ownership
	_sounds.push_back(sounddata.release());

	int sound_id = _sounds.size()-1;

#ifdef GNASH_DEBUG_SOUNDS_MANAGEMENT
	log_debug("create_sound: sound %d, format %d", sound_id, _sounds.back()->soundinfo->getFormat());
#endif

	return sound_id;

}

// This gets called when an SWF embedded sound stream gets more data
long
SDL_sound_handler::fill_stream_data(unsigned char* data,
        unsigned int data_bytes, unsigned int /*sample_count*/,
        int handle_id)
{

	boost::mutex::scoped_lock lock(_mutex);

	// @@ does a negative handle_id have any meaning ?
	//    should we change it to unsigned instead ?
	if (handle_id < 0 || (unsigned int) handle_id+1 > _sounds.size())
	{
		delete [] data;
		return -1;
	}

	EmbedSound* sounddata = _sounds[handle_id];

	// If doing ADPCM, knowing the framesize is needed to decode!
	if (sounddata->soundinfo->getFormat() == media::AUDIO_CODEC_ADPCM)
    {
		sounddata->m_frames_size[sounddata->size()] = data_bytes;
	}

	// Handling of the sound data
	size_t start_size = sounddata->size();
	sounddata->append(reinterpret_cast<boost::uint8_t*>(data), data_bytes);

	return start_size;
}


// Play the index'd sample.
void
SDL_sound_handler::play_sound(int sound_handle, int loopCount, int offSecs,
        long start_position, const std::vector<sound_envelope>* envelopes)
{
	boost::mutex::scoped_lock lock(_mutex);

	// Increment number of sound start request for the testing framework
	++_soundsStarted;

    // Check if audio is muted
    if (muted)
	{
		return;
	}

	// Check if the sound exists
	if (sound_handle < 0 || static_cast<unsigned int>(sound_handle) >= _sounds.size())
	{
        log_error("Invalid (%d) sound_handle passed to play_sound, "
                  "doing nothing", sound_handle);
		return;
	}

    // parameter checking
	if (start_position < 0)
    {
        log_error("Negative (%d) start_position passed to play_sound, "
                  "taking as zero ", start_position);
        start_position=0;
    }

    // parameter checking
	if (offSecs < 0)
    {
        log_error("Negative (%d) seconds offset passed to play_sound, "
                  "taking as zero ", offSecs);
        offSecs = 0;
    }

	EmbedSound& sounddata = *(_sounds[sound_handle]);

	// When this is called from a StreamSoundBlockTag,
    // we only start if this sound isn't already playing.
	if ( start_position && sounddata.isPlaying() )
    {
        // log_debug("Stream sound block play request, "
        //           "but an instance of the stream is "
        //           "already playing, so we do nothing");
		return;
	}

	// Make sure sound actually got some data
	if ( sounddata.empty() )
    {
        // @@ should this be a log_error ? or even an assert ?
		IF_VERBOSE_MALFORMED_SWF(
			log_swferror(_("Trying to play sound with size 0"));
		);
		return;
	}

#ifdef GNASH_DEBUG_SOUNDS_MANAGEMENT
	log_debug("play_sound %d called, SoundInfo format is %s", sound_handle, sounddata.soundinfo->getFormat());
#endif


	// Make a "EmbedSoundInst" for this sound which is later placed
    // on the vector of instances of this sound being played
	EmbedSoundInst* sound = sounddata.createInstance(*_mediaHandler);

	// Set the given options of the sound
	sound->decodingPosition = start_position;

    // Offset is stored as stereo
    // WARNING: this is wrong, offset is passed as seconds !! (currently unused anyway)
	sound->offSecs = (sounddata.soundinfo->isStereo() ? offSecs : offSecs*2);

	sound->envelopes = envelopes;

	// Set number of loop we should do. -1 is infinte loop, 0 plays it once, 1 twice etc.
	sound->loopCount = loopCount;

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
	if (sound_handle < 0 || (unsigned int) sound_handle >= _sounds.size())
	{
		// Invalid handle.
		return;
	}

	
	EmbedSound* sounddata = _sounds[sound_handle];

	size_t nInstances = sounddata->_soundInstances.size();

	// Decrement callback clients count 
	soundsPlaying -= nInstances;

    // Increment number of sound stop request for the testing framework
	_soundsStopped += nInstances;

	sounddata->clearInstances();

}


// this gets called when it's done with a sample.
void	SDL_sound_handler::delete_sound(int sound_handle)
{
	boost::mutex::scoped_lock lock(_mutex);

#ifdef GNASH_DEBUG_SOUNDS_MANAGEMENT
    log_debug ("deleting sound :%d", sound_handle);
#endif

	if (sound_handle >= 0 && static_cast<unsigned int>(sound_handle) < _sounds.size())
	{
		delete _sounds[sound_handle];
		_sounds[sound_handle] = NULL;
	}

}

// This will stop all sounds playing. Will cause problems if the soundhandler is made static
// and supplys sound_handling for many SWF's, since it will stop all sounds with no regard
// for what sounds is associated with what SWF.
void	SDL_sound_handler::stop_all_sounds()
{
	boost::mutex::scoped_lock lock(_mutex);

	for (Sounds::iterator i = _sounds.begin(),
	                      e = _sounds.end(); i != e; ++i)
	{
		EmbedSound* sounddata = *i;

        // The sound may have been deleted already.		
		if (!sounddata) continue;
		size_t nInstances = sounddata->_soundInstances.size();

		// Decrement callback clients count 
		soundsPlaying -= nInstances;

        // Increment number of sound stop request for the testing framework
		_soundsStopped += nInstances;

		sounddata->clearInstances();
	}
}


//	returns the sound volume level as an integer from 0 to 100,
//	where 0 is off and 100 is full volume. The default setting is 100.
int	SDL_sound_handler::get_volume(int sound_handle) {

	boost::mutex::scoped_lock lock(_mutex);

	int ret;
	// Check if the sound exists.
	if (sound_handle >= 0 && static_cast<unsigned int>(sound_handle) < _sounds.size())
	{
		ret = _sounds[sound_handle]->volume;
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
	if (sound_handle < 0 || static_cast<unsigned int>(sound_handle) >= _sounds.size())
	{
		// Invalid handle.
	} else {

		// Set volume for this sound. Should this only apply to the active sounds?
		_sounds[sound_handle]->volume = volume;
	}


}
	
media::SoundInfo*
SDL_sound_handler::get_sound_info(int sound_handle)
{

	boost::mutex::scoped_lock lock(_mutex);

	// Check if the sound exists.
	if (sound_handle >= 0 && static_cast<unsigned int>(sound_handle) < _sounds.size())
	{
		return _sounds[sound_handle]->soundinfo.get();
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
	if (sound_handle < 0 || (unsigned int) sound_handle >= _sounds.size())
	{
		// Invalid handle.
		return 0;
	}

	EmbedSound* sounddata = _sounds[sound_handle];

	boost::uint32_t sampleCount = sounddata->soundinfo->getSampleCount();
	boost::uint32_t sampleRate = sounddata->soundinfo->getSampleRate();

	// Return the sound duration in milliseconds
	if (sampleCount > 0 && sampleRate > 0) {
        // TODO: should we cache this in the EmbedSound object ?
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
	if (sound_handle < 0 || (unsigned int) sound_handle >= _sounds.size())
	{
		// Invalid handle.
		return 0;
	}

	EmbedSound* sounddata = _sounds[sound_handle];

	// If there is no active sounds, return 0
	if (sounddata->_soundInstances.empty()) return 0;

	// We use the first active sound of this.
	EmbedSoundInst* asound = sounddata->_soundInstances.front();

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

EmbedSoundInst::EmbedSoundInst(const EmbedSound& soundData,
            media::MediaHandler& mediaHandler)
		:
		decoder(0),
		decodingPosition(0),
		playbackPosition(0),
		loopCount(0),
		offSecs(0),
		current_env(0),
		samples_played(0),
		_encodedData(soundData)
{
	media::SoundInfo& si = *(soundData.soundinfo);

	media::AudioInfo info(
		(int)si.getFormat(), // codeci
		si.getSampleRate(), // sampleRatei
		si.is16bit() ? 2 : 1, // sampleSizei
		si.isStereo(), // stereoi
		0, // duration unknown, does it matter ?
		media::FLASH);

    try
    {
        decoder = mediaHandler.createAudioDecoder(info);
	}
	catch (MediaException& e)
	{
	    log_error("AudioDecoder initialization failed: %s", e.what());
	}
}

// Pointer handling and checking functions
boost::int16_t*
EmbedSoundInst::getDecodedData(unsigned long int pos)
{
	if ( _decodedData.get() )
	{
		assert(pos < _decodedData->size());
		return reinterpret_cast<boost::int16_t*>(_decodedData->data()+pos);
	}
	else return 0;
}

unsigned int 
EmbedSoundInst::fetchSamples(boost::int16_t* to, unsigned int nSamples)
{
    boost::int16_t* data = getDecodedData(playbackPosition);
    unsigned int availableSamples = decodedSamplesAhead();

    if ( availableSamples < nSamples )
    {
        log_error("EmbedSoundInst::fetchSamples: "
            "requested to fetch %s samples, "
            "but only %d decoded samples available "
            "(decoding completed: %d)",
            nSamples, availableSamples, decodingCompleted());

        // pad the leftover part with zeroes
        std::fill(to+availableSamples, to+nSamples, 0);
        //memset(to+availableSamples, 0, (nSamples-availableSamples)*2);

        nSamples = availableSamples;
    }

	// Update playback position (samples are 16bit)
	playbackPosition += nSamples*2;

	// update samples played
	samples_played += nSamples;

    // copy the actual data to output buffer
    std::copy(data, data+nSamples, to);

    return nSamples;
}

const boost::uint8_t*
EmbedSoundInst::getEncodedData(unsigned long int pos)
{
	return _encodedData.data(pos);
}

void EmbedSoundInst::deleteDecodedData()
{
	_decodedData.reset();
}

// AS-volume adjustment
void adjust_volume(boost::int16_t* data, int size, int volume)
{
    //log_error("skipping volume adjustment (intentionally)"); return;

    if ( size%2 != 0 )
    {
        log_error("adjust_volume called for a buffer of an odd number of bytes!"
            " This shouldn't happen as each sample is 16bit.");
    }

	for (int i=0; i < size*0.5; i++) {
		data[i] = data[i] * volume/100;
	}
}

void
EmbedSoundInst::useEnvelopes(unsigned int length)
{
    //log_error("skipping envelopes (intentionally)"); return;

	// Check if this is the time to use envelopes yet
	if (current_env == 0 && (*envelopes)[0].m_mark44 > samples_played+length/2)
	{
		return;
	}

	// switch to the next envelope if needed and possible
	else if (current_env < envelopes->size()-1 && (*envelopes)[current_env+1].m_mark44 >= samples_played)
	{
		++current_env;
	}

	// Current envelope position
	boost::int32_t cur_env_pos = envelopes->operator[](current_env).m_mark44;

	// Next envelope position
	boost::uint32_t next_env_pos = 0;
	if (current_env == (envelopes->size()-1)) {
		// If there is no "next envelope" then set the next envelope start point to be unreachable
		next_env_pos = cur_env_pos + length;
	} else {
		next_env_pos = (*envelopes)[current_env+1].m_mark44;
	}

	unsigned int startpos = 0;
	// Make sure we start adjusting at the right sample
	if (current_env == 0 && (*envelopes)[current_env].m_mark44 > samples_played)
    {
		startpos = playbackPosition + ((*envelopes)[current_env].m_mark44 - samples_played)*2;
	} else {
		startpos = playbackPosition;
	}

	boost::int16_t* data = getDecodedData(startpos);

	for (unsigned int i=0; i < length/2; i+=2) {
		float left = static_cast<float>((*envelopes)[current_env].m_level0 / 32768.0);
		float right = static_cast<float>((*envelopes)[current_env].m_level1 / 32768.0);

		data[i] = static_cast<boost::int16_t>(data[i] * left); // Left
		data[i+1] = static_cast<boost::int16_t>(data[i+1] * right); // Right

		if ((samples_played+(length/2-i)) >= next_env_pos && current_env != (envelopes->size()-1))
        {
			++current_env;
			// Next envelope position
			if (current_env == (envelopes->size()-1))
            {
				// If there is no "next envelope" then set the next envelope start point to be unreachable
				next_env_pos = cur_env_pos + length;
			}
            else
            {
				next_env_pos = (*envelopes)[current_env+1].m_mark44;
			}
		}
	}
}


/// Prepare for mixing/adding (volume adjustments) and mix/add.
//
/// @param mixTo
///     The buffer to mix to
///
/// @param sound
///     The EmbedSoundInst to mix in.
///     Note that decoded data in the EmbedSoundInst will
///     be modified (volumes and envelope adjustments).
///
/// @param mixLen
///     The amount of bytes to mix in. Must be a multiple of 2 !!
///
/// @param volume
///     The volume to use for this mix-in.
///     100 is full volume. 
///
///
static void
do_mixing(Uint8* mixTo, EmbedSoundInst& sound, unsigned int mixLen, unsigned int volume)
{
    assert ( !(mixLen%2) );

    unsigned int nSamples=mixLen/2;
    if ( ! nSamples )
    {
        log_debug("do_mixing: %d bytes are 0 samples, nothing to do");
        return;
    }

    boost::scoped_array<boost::int16_t> data(new boost::int16_t[nSamples]);

    unsigned int wroteSamples = sound.fetchSamples(data.get(), nSamples);
    if ( wroteSamples < nSamples )
    {
        log_debug("do_mixing: less samples fetched (%d) then I requested (%d)", wroteSamples, nSamples);
    }

	// If the volume needs adjustments we call a function to do that (why are we doing this manually ?)
	if (volume != 100)
    {
		adjust_volume(data.get(), wroteSamples*2, volume);
	}

    /// @todo is use of envelopes really mutually exclusive with
    ///       setting the volume ??
    else if (sound.envelopes)
    {
		sound.useEnvelopes(wroteSamples*2);
	}

	// Mix the raw data
#ifdef GNASH_DEBUG_MIXING
    log_debug("do_mixing: calling SDL_MixAudio with %d bytes of samples", mixLen);
#endif
	SDL_MixAudio(mixTo, reinterpret_cast<const Uint8*>(data.get()), mixLen, SDL_MIX_MAXVOLUME);
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

void
SDL_sound_handler::fetchSamples (boost::uint8_t* stream, unsigned int buffer_length)
{
	boost::mutex::scoped_lock lock(_mutex);

	if ( isPaused() ) return;

	int finalVolume = int( SDL_MIX_MAXVOLUME * (getFinalVolume()/100.0) );

	// If nothing to play there is no reason to play
	// Is this a potential deadlock problem?
	if (soundsPlaying == 0 && m_aux_streamer.empty()) {
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
	if ( !m_aux_streamer.empty() )
	{
		boost::scoped_array<boost::uint8_t> buf ( new boost::uint8_t[buffer_length] );

		// Loop through the attached sounds
		CallbacksMap::iterator it = m_aux_streamer.begin();
		CallbacksMap::iterator end = m_aux_streamer.end();
		while (it != end) {
			memset(buf.get(), 0, buffer_length);

			SDL_sound_handler::aux_streamer_ptr aux_streamer = it->second; 
			void* owner = it->first;

			// If false is returned the sound doesn't want to be attached anymore
			bool ret = (aux_streamer)(owner, buf.get(), buffer_length);
			if (!ret) {
				CallbacksMap::iterator it2=it;
				++it2; // before we erase it
				m_aux_streamer.erase(it); // FIXME: isn't this terribly wrong ?
				it = it2;
				// Decrement callback clients count 
				soundsPlaying--;
			} else {
				++it;
			}
			SDL_MixAudio(stream, buf.get(), buffer_length, finalVolume);

		}
	}

    const SDL_sound_handler::Sounds& soundData = _sounds;

	// Run through all the sounds. TODO: don't call .size() at every iteration !
	for (SDL_sound_handler::Sounds::const_iterator i = soundData.begin(),
	            e = soundData.end(); i != e; ++i)
	{
	    // Check whether sound has been deleted first.
        if (!*i) continue;
		mixSoundData(**i, buffer, buffer_length);
	}

	// 
	// WRITE CONTENTS OF stream TO FILE
	//
	if (file_stream) {
            file_stream.write((char*) stream, buffer_length);
            // now, mute all audio
            memset ((void*) stream, 0, buffer_length);
	}

	// Now, after having "consumed" all sounds, blank out
	// the buffer if muted..
	if ( muted )
	{
		memset ((void*) stream, 0, buffer_length);
	}
}

// Callback invoked by the SDL audio thread.
void
SDL_sound_handler::sdl_audio_callback (void *udata, Uint8 *buf, int bufLenIn)
{
	if ( bufLenIn < 0 )
	{
		log_error(_("Negative buffer length in sdl_audio_callback (%d)"), bufLenIn);
		return;
	}

	if ( bufLenIn == 0 )
	{
		log_error(_("Zero buffer length in sdl_audio_callback"));
		return;
	}

	unsigned int bufLen = static_cast<unsigned int>(bufLenIn);

	// Get the soundhandler
	SDL_sound_handler* handler = static_cast<SDL_sound_handler*>(udata);

    handler->fetchSamples(buf, bufLen);
}

void
EmbedSound::append(boost::uint8_t* data, unsigned int size)
{
    // Make sure we're always appropriately padded...
    media::MediaHandler* mh = media::MediaHandler::get(); // TODO: don't use this static !
    const size_t paddingBytes = mh ? mh->getInputPaddingSize() : 0;
    _buf->reserve(_buf->size()+size+paddingBytes);
	_buf->append(data, size);

    // since ownership was transferred...
	delete [] data;
}

EmbedSound::EmbedSound(std::auto_ptr<SimpleBuffer> data,
        std::auto_ptr<media::SoundInfo> info, int nVolume)
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
            log_error("EmbedSound creator didn't appropriately pad sound data. "
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
EmbedSound::clearInstances()
{
	for (Instances::iterator i=_soundInstances.begin(), e=_soundInstances.end(); i!=e; ++i)
	{
		delete *i;
	}
	_soundInstances.clear();
}

EmbedSound::Instances::iterator
EmbedSound::eraseActiveSound(Instances::iterator i)
{
	delete *i;
	return _soundInstances.erase(i);
}

EmbedSoundInst*
EmbedSound::createInstance(media::MediaHandler& mh)
{
    EmbedSoundInst* ret = new EmbedSoundInst(*this, mh);

    // Push the sound onto the playing sounds container.
	_soundInstances.push_back(ret);

    return ret;
}

/*private*/
void
SDL_sound_handler::mixActiveSound(EmbedSoundInst& sound, Uint8* buffer,
        unsigned int mixLen)
{
	// If there exist no decoder, then we can't decode!
    // TODO: isn't it documented that an EmbedSoundInst w/out a decoder
    //       means that the EmbedSound data is already decoded ?
	if (!sound.decoder.get())
    {
        return;
    }

    assert(!(mixLen%2));

    unsigned int mixSamples = mixLen/2;

#ifdef GNASH_DEBUG_MIXING
    log_debug("Asked to mix-in %d samples (%d bytes) of this sound",
        mixSamples, mixLen);
#endif

    const EmbedSound& sndData = sound.getSoundData();

    // concatenate global volume
	int volume = int(sndData.volume*getFinalVolume()/100.0);

	// When the current sound don't have enough decoded data to fill the buffer, 
	// we first mix what is already decoded, then decode some more data, and
	// mix some more until the buffer is full. If a sound loops the magic
	// happens here ;)
	//
	if (sound.decodedSamplesAhead() < mixSamples
		&& ( !sound.decodingCompleted() || sound.loopCount != 0) )
	{
#ifdef GNASH_DEBUG_MIXING
        log_debug("There are %d decoded samples available "
            "(less then the %d requested); "
            "decoding completed:%d; "
            "loopCount:%d",
            sound.decodedSamplesAhead(), mixSamples,
            sound.decodingCompleted(), sound.loopCount);
#endif

		// First we mix what is decoded
		unsigned int index = 0;
		if (sound.decodedSamplesAhead())
		{
			index = sound.decodedSamplesAhead()*2; // each sample is 2 bytes
#ifdef GNASH_DEBUG_MIXING
            log_debug(" mixing %d bytes of available data in...", index);
#endif
			do_mixing(buffer, sound, index, volume);
		}

		// Then we decode some data
		// We loop until the size of the decoded sound is greater than the buffer size,
		// or there is no more to decode.
		unsigned int decoded_size = 0;

		// Delete any previous raw_data
        // @@ Why is this !?!?
		sound.deleteDecodedData();

        /// @todo REWRITE THIS CRAP !!
		while(decoded_size < mixLen)
		{

#ifdef GNASH_DEBUG_MIXING
            log_debug(" decoded_size:%d, mixLen:%d ... we'll want more ...", decoded_size, mixLen);
#endif

			// If we need to loop, we reset the data pointer
			if (sound.decodingCompleted() && sound.loopCount != 0)
            {
#ifdef GNASH_DEBUG_MIXING
                log_debug(" decoding was completed, so we decrement loop count and reset decoding position to 0");
#endif
				sound.loopCount--;
				sound.decodingPosition = 0;
				sound.samples_played = 0;
			}

			// Test if we will get problems... Should not happen...
			assert(!sound.decodingCompleted());
			
			// temp raw buffer
			Uint8* tmp_raw_buffer=0;
			boost::uint32_t tmp_raw_buffer_size = 0;
			boost::uint32_t decodedBytes = 0;

			boost::uint32_t inputSize = 0;
			bool parse = true;
			if (sndData.soundinfo->getFormat() == media::AUDIO_CODEC_ADPCM)
            {
#ifdef GNASH_DEBUG_MIXING
                log_debug(" sound format is ADPCM");
#endif

				parse = false;
				if (!sndData.m_frames_size.empty())
                {
                    const EmbedSound::FrameSizeMap& m = sndData.m_frames_size;
                    EmbedSound::FrameSizeMap::const_iterator it =
                            m.find(sound.decodingPosition);
                    if ( it == m.end() )
                    {
#ifdef GNASH_DEBUG_MIXING
                        log_debug("Unknown size of ADPCM frame starting at offset %d", sound.decodingPosition);
#endif
					    inputSize = sound.encodedDataSize() - sound.decodingPosition;
                    }
                    else
                    {
					    inputSize = it->second; 
#ifdef GNASH_DEBUG_MIXING
                        log_debug(" frame size for frame starting at offset %d is %d",
                            sound.decodingPosition, inputSize);
#endif
                    }
				}
				else
                {
					inputSize = sound.encodedDataSize() - sound.decodingPosition;
#ifdef GNASH_DEBUG_MIXING
                    log_debug(" frame size for frame starting at offset %d is unknown, "
                        "using the whole still encoded data (%d bytes)",
                        sound.decodingPosition, inputSize);
#endif
				}
			}
            else
            {
				inputSize = sound.encodedDataSize() - sound.decodingPosition;
#ifdef GNASH_DEBUG_MIXING
                log_debug(" frame size of non-ADPCM frame starting at offset %d is unknown, "
                        "using the whole still encoded data (%d bytes)",
                        sound.decodingPosition, inputSize);
#endif
			}

#ifdef GNASH_DEBUG_MIXING
			log_debug("  decoding %d bytes", inputSize);
#endif
			tmp_raw_buffer = sound.decoder->decode(sound.getEncodedData(sound.decodingPosition), 
					inputSize, tmp_raw_buffer_size, decodedBytes, parse);

			sound.decodingPosition += decodedBytes;

#ifdef GNASH_DEBUG_MIXING
			log_debug("  appending %d bytes to decoded buffer", tmp_raw_buffer_size);
#endif

			// tmp_raw_buffer ownership transferred here
			sound.appendDecodedData(tmp_raw_buffer, tmp_raw_buffer_size);

			decoded_size += tmp_raw_buffer_size;

			// no more to decode from this sound, so we break the loop
			if ((sound.encodedDataSize() <= sound.decodingPosition && !sound.loopCount)
                    || (!tmp_raw_buffer_size && decodedBytes == 0))
            {
#ifdef GNASH_DEBUG_MIXING
			    log_debug("  no more to decode from this sound"
                          "(encodedDataSize:%d, decodingPosition:%d, loopCount:%d) "
                          "(tmp_raw_buffer_size:%d, decodedBytes:%d)"
                          ", breaking the decoding loop",
                          sound.encodedDataSize(), sound.decodingPosition,
                          sound.loopCount, tmp_raw_buffer_size, decodedBytes);
#endif

				sound.decodingPosition = sound.encodedDataSize();
				break;
			}

		} // end of "decode min. bufferlength data" while loop

		sound.playbackPosition = 0;

		// Determine how much should be mixed
		unsigned int mix_length = 0;
		if (decoded_size >= mixLen - index) {
			mix_length = mixLen - index;
		} else { 
			mix_length = decoded_size;
		}

		if (!sound.decodedSamplesAhead())
		{
			log_error("Something went terribly wrong during mixing of an active sound");
			return; // something went terrible wrong
		}

		do_mixing(buffer+index, sound, mix_length, volume);

	}

	// When the current sound has enough decoded data to fill 
	// the buffer, we do just that.
	else if ( sound.decodedSamplesAhead() && sound.decodedSamplesAhead() >= mixSamples )
	{
#ifdef GNASH_DEBUG_MIXING
        log_debug("There are %d decoded samples available "
            "(enough for the %d requested); "
            "decoding completed:%d; Mixing in %d bytes",
            sound.decodedSamplesAhead(), mixSamples,
            sound.decodingCompleted(), mixLen);
#endif

		do_mixing(buffer, sound, mixLen, volume);
	}

	// When the current sound doesn't have anymore data to decode,
	// and doesn't loop (anymore), but still got unplayed data,
	// we put the last data on the stream
	else if (sound.decodedSamplesAhead() <= mixSamples && sound.decodedSamplesAhead())
	{
#ifdef GNASH_DEBUG_MIXING
        log_debug("There are %d decoded samples available "
            "(better then nothing for the %d requested); "
            "decoding completed:%d; Mixing in %d bytes",
            sound.decodedSamplesAhead(), mixSamples,
            sound.decodingCompleted(), sound.decodedSamplesAhead()*2);
#endif

		do_mixing(buffer, sound, sound.decodedSamplesAhead()*2, volume);
	} 

    else
    {
#ifdef GNASH_DEBUG_MIXING
        log_debug(" None of the conditions verified where met !!!!!!-----------");
        log_debug(
            "decodedSamplesAhead:%d, decodingCompleted:%d, loopCount:%d",
            sound.decodedSamplesAhead(), sound.decodingCompleted(),
            sound.loopCount);
#endif
    }
}

/*private*/
void
SDL_sound_handler::mixSoundData(EmbedSound& sounddata, Uint8* buffer, unsigned int buffer_length)
{
	for (EmbedSound::Instances::iterator
		 i=sounddata._soundInstances.begin();
		 i != sounddata._soundInstances.end(); // don't cache .end(), isn't necessarely safe on erase
	    )
	{

		// Temp variables to make the code simpler and easier to read
		EmbedSoundInst& sound = *(*i); 

		mixActiveSound(sound, buffer, buffer_length);

		// Sound is done, remove it from the active list

		if ( sound.decodingCompleted() && !sound.decodedSamplesAhead() && !sound.loopCount)
		{
            // WARNING: can't use 'sound' anymore from now on!
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

} // gnash.sound namespace 
} // namespace gnash

// Local Variables:
// mode: C++
// End:

