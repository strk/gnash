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

// Debug create_sound/delete_sound/play_sound/stop_sound, loops
//#define GNASH_DEBUG_SOUNDS_MANAGEMENT

// Debug sound decoding
//#define GNASH_DEBUG_SOUNDS_DECODING

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
        long start_position, const SoundEnvelopes* envelopes)
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
    //
    // @todo: plug the returned EmbedSoundInst to the set
    //        of InputStream channels !
    //
    /*InputStream* sound =*/ sounddata.createInstance(

            // MediaHandler to use for decoding
            *_mediaHandler,

            // Byte offset position to start decoding from
            // (would be offset to streaming sound block)
            start_position, // or blockOffset

            // Seconds offset
            // WARNING: this is wrong, offset is passed as seconds !!
            // (currently unused anyway)
	        (sounddata.soundinfo->isStereo() ? offSecs : offSecs*2),

            // Volume envelopes to use for this instance
            envelopes,

            // Loop count
            loopCount

    );

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

unsigned int
SDL_sound_handler::tell(int sound_handle)
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
	//EmbedSoundInst* asound = sounddata->_soundInstances.front();
	InputStream* asound = sounddata->_soundInstances.front();

	// Return the playhead position in milliseconds
    unsigned int samplesPlayed = asound->samplesFetched();
	unsigned int ret = samplesPlayed / audioSpec.freq * 1000;
	ret += ((samplesPlayed % audioSpec.freq) * 1000) / audioSpec.freq;
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

unsigned int
EmbedSoundInst::samplesFetched() const
{
    return _samplesFetched;
}

EmbedSoundInst::EmbedSoundInst(const EmbedSound& soundData,
            media::MediaHandler& mediaHandler,
            unsigned long blockOffset, unsigned int secsOffset,
            const SoundEnvelopes* env,
            unsigned int loopCount)
		:

        // should store blockOffset somewhere else too, for resetting
		decodingPosition(blockOffset),

        // should change based on offSecs I guess ?
		playbackPosition(0),

		loopCount(loopCount),
		offSecs(secsOffset),
        envelopes(env),
		current_env(0),
		_samplesFetched(0),

		_decoder(0),
		_soundDef(soundData),
        _decodedData(0)
{
    createDecoder(mediaHandler);
}

/*private*/
void
EmbedSoundInst::createDecoder(media::MediaHandler& mediaHandler)
{
	media::SoundInfo& si = *(_soundDef.soundinfo);

	media::AudioInfo info(
		(int)si.getFormat(), // codeci
		si.getSampleRate(), // sampleRatei
		si.is16bit() ? 2 : 1, // sampleSizei
		si.isStereo(), // stereoi
		0, // duration unknown, does it matter ?
		media::FLASH);

    try
    {
        _decoder = mediaHandler.createAudioDecoder(info);
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
	// If there exist no decoder, then we can't decode!
    // TODO: isn't it documented that an EmbedSoundInst w/out a decoder
    //       means that the EmbedSound data is already decoded ?
	if (!_decoder.get())
    {
        return 0;
    }

    unsigned int fetchedSamples=0;

    while ( nSamples )
    {
        unsigned int availableSamples = decodedSamplesAhead();
        if ( availableSamples )
        {
            boost::int16_t* data = getDecodedData(playbackPosition);
            if ( availableSamples >= nSamples )
            {
                std::copy(data, data+nSamples, to);
                fetchedSamples += nSamples;
                break; // fetched all
            }
            else
            {
                // not enough decoded samples available:
                // copy what we have and go on
                std::copy(data, data+availableSamples, to);
                fetchedSamples += availableSamples;
                to+=availableSamples;
                nSamples-=availableSamples;
                assert ( nSamples );
            }
        }

        // We haven't finished fetching yet, so see if we
        // have more to decode or not

        if ( decodingCompleted() )
        {
            // No more to decode, see if we want to loop...

            if ( loopCount )
            {
#ifdef GNASH_DEBUG_SOUNDS_MANAGEMENT
                log_debug("Loops left: %d", loopCount);
#endif

                // loops ahead, reset playbackPosition to the starting 
                // position and keep looping
                --loopCount;

                // @todo playbackPosition is not necessarely 0
                //       if we were asked to start somewhere after that!!
                playbackPosition=0;

                continue;
            }

            log_debug("Decoding completed and no looping, sound is over");
            break; // fetched what possible, filled with silence the rest
        }

        // More to decode, then decode it
        decodeNextBlock();
    }

	// Update playback position (samples are 16bit)
	playbackPosition += fetchedSamples*2;

	// update samples played
	_samplesFetched += fetchedSamples;

    return fetchedSamples;
}

/*private*/
void
EmbedSoundInst::decodeNextBlock()
{
    assert(!decodingCompleted());

    // Should only be called when no more decoded data
    // are available for fetching.
    // Doing so we know what's the sample number
    // of the first sample in the newly decoded block.
    //
    assert(playbackPosition >= decodedDataSize() );

    // TODO: IMPLEMENT THIS !!
    // SHOUD:
    //   - decode
    //   - increment decodingPosition
    //   - appendDecodedData 

    boost::uint32_t inputSize = 0; // or blockSize
    bool parse = true; // need to parse ?

    // this block figures inputSize (blockSize) and need to parse (parse)
    // @todo: turn into a private function
    {
        const EmbedSound& sndData = _soundDef;
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
                        m.find(decodingPosition);
                if ( it == m.end() )
                {
#ifdef GNASH_DEBUG_MIXING
                    log_debug("Unknown size of ADPCM frame starting at offset %d", decodingPosition);
#endif
                    inputSize = encodedDataSize() - decodingPosition;
                }
                else
                {
                    inputSize = it->second; 
#ifdef GNASH_DEBUG_MIXING
                    log_debug(" frame size for frame starting at offset %d is %d",
                        decodingPosition, inputSize);
#endif
                }
            }
            else
            {
                inputSize = encodedDataSize() - decodingPosition;
#ifdef GNASH_DEBUG_MIXING
                log_debug(" frame size for frame starting at offset %d is unknown, "
                    "using the whole still encoded data (%d bytes)",
                    decodingPosition, inputSize);
#endif
            }
        }
        else
        {
            inputSize = encodedDataSize() - decodingPosition;
#ifdef GNASH_DEBUG_MIXING
            log_debug(" frame size of non-ADPCM frame starting at offset %d is unknown, "
                    "using the whole still encoded data (%d bytes)",
                    decodingPosition, inputSize);
#endif
        }
    }

#ifdef GNASH_DEBUG_SOUNDS_DECODING
    log_debug("  decoding %d bytes", inputSize);
#endif

    boost::uint32_t consumed = 0;
    boost::uint32_t decodedDataSize = 0;
    boost::uint8_t* decodedData = _decoder->decode(
                                      getEncodedData(decodingPosition), 
                                      inputSize,
                                      decodedDataSize,
                                      consumed,
                                      parse);

    decodingPosition += consumed;

    assert(!(decodedDataSize%2));

    // @todo I hope there are no alignment issues in this cast from int8_t* to int16_t* !
    boost::int16_t* samples = reinterpret_cast<boost::int16_t*>(decodedData);
    unsigned int nSamples = decodedDataSize/2;

#ifdef GNASH_DEBUG_MIXING
    log_debug("  applying volume/envelope to %d bytes (%d samples)"
            "of decoded data", decodedDataSize, nSamples);
#endif

	// If the volume needs adjustments we call a function to do that (why are we doing this manually ?)
	if (_soundDef.volume != 100) // volume is a private member
    {
        // TODO: have adjust_volume take samples, not bytes
		adjustVolume(samples, nSamples, _soundDef.volume/100.0);
	}

    /// @todo is use of envelopes really mutually exclusive with
    ///       setting the volume ??
    else if (envelopes) // envelopes are a private member
    {
        unsigned int firstSample = playbackPosition/2;

        // TODO: have applyEnvelopes take samples, not bytes
		applyEnvelopes(samples, nSamples, firstSample, *envelopes);
	}

#ifdef GNASH_DEBUG_MIXING
    log_debug("  appending %d bytes to decoded buffer", decodedDataSize);
#endif


    // decodedData ownership transferred here
    appendDecodedData(decodedData, decodedDataSize);
}


const boost::uint8_t*
EmbedSoundInst::getEncodedData(unsigned long int pos)
{
	return _soundDef.data(pos);
}

/* static private */
void
EmbedSoundInst::adjustVolume(boost::int16_t* data, unsigned int nSamples, float volume)
{
    //log_error("skipping volume adjustment (intentionally)"); return;

	for (unsigned int i=0; i<nSamples; ++i)
    {
		data[i] = data[i] * volume;
	}
}

/* private */
void
EmbedSoundInst::applyEnvelopes(boost::int16_t* samples, unsigned int nSamples,
        unsigned int firstSampleOffset, const SoundEnvelopes& env)
{
    //log_error("skipping envelopes (intentionally)"); return;

    // Number of envelopes defined
	size_t numEnvs = env.size();

    // Nothing to do if we applied all envelopes already
	if ( numEnvs <= current_env)
    {
        return;
    }

	// Not yet time to use the current envelope
	if (env[current_env].m_mark44 >= firstSampleOffset+nSamples)
	{
		return;
	}

    assert(env[current_env].m_mark44 < firstSampleOffset+nSamples);

	// Get next envelope position (absolute samples offset)
	boost::uint32_t next_env_pos = 0;
	if (current_env == (env.size()-1)) {
		// If there is no "next envelope" then set the next envelope start point to be unreachable
		next_env_pos = env[current_env].m_mark44 + nSamples + 1;
	} else {
		next_env_pos = env[current_env+1].m_mark44;
	}

    // Scan all samples in the block, applying the envelope
    // which is in effect in each subportion
	for (unsigned int i=0; i<nSamples/2; i+=2)
    {
        // @todo cache these left/right floats (in the SoundEnvelope class?)
		float left = env[current_env].m_level0 / 32768.0;
		float right = env[current_env].m_level1 / 32768.0;

		samples[i] = samples[i] * left; // Left
		samples[i+1] = samples[i+1] * right; // Right

        // TODO: continue from here (what is the code below doing ??

        // if we encounter the offset of next envelope,
        // switch to it !
		if ( (firstSampleOffset+nSamples-i) >= next_env_pos )
        {
            if ( numEnvs <= ++current_env )
            {
                // no more envelopes to apply
                return;
            }

	        // Get next envelope position (absolute samples offset)
            if (current_env == (env.size()-1)) {
                // If there is no "next envelope" then set the next envelope start point to be unreachable
		        next_env_pos = env[current_env].m_mark44 + nSamples + 1;
            } else {
                next_env_pos = env[current_env+1].m_mark44;
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
/// @return the number of samples actually wrote out.
///         If < nSamples we reached end of the EmbedSoundInst
///
static unsigned int
do_mixing(Uint8* mixTo, EmbedSoundInst& sound, unsigned int nSamples, unsigned int volume)
{
    if ( ! nSamples )
    {
        log_debug("do_mixing: %d bytes are 0 samples, nothing to do");
        return 0;
    }

    boost::scoped_array<boost::int16_t> data(new boost::int16_t[nSamples]);

    unsigned int wroteSamples = sound.fetchSamples(data.get(), nSamples);
    if ( wroteSamples < nSamples )
    {
        // @todo would it be fine to skip the filling if
        //       we pass just the actual wrote samples to
        //       SDL_MixAudio below ?

        log_debug("do_mixing: less samples fetched (%d) then I requested (%d)."
            " Filling the rest with silence",
            wroteSamples, nSamples);

        std::fill(data.get()+wroteSamples, data.get()+nSamples, 0);
    }

	// Mix the raw data
#ifdef GNASH_DEBUG_MIXING
    log_debug("do_mixing: calling SDL_MixAudio with %d bytes of samples", mixLen);
#endif

	int finalVolume = int( SDL_MIX_MAXVOLUME * (volume/100.0) );

    // @todo would it be fine to mix only wroteSamples*2 here ?
    //       there would be no need to fill with zeros in that case ?
	SDL_MixAudio(mixTo, reinterpret_cast<const Uint8*>(data.get()), nSamples*2, finalVolume);

    return wroteSamples;
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
EmbedSound::createInstance(media::MediaHandler& mh,
            unsigned long blockOffset, unsigned int secsOffset,
            const SoundEnvelopes* envelopes,
            unsigned int loopCount)
{
    EmbedSoundInst* ret = new EmbedSoundInst(*this,
                                mh, blockOffset,
                                secsOffset, envelopes,
                                loopCount);

    // Push the sound onto the playing sounds container.
	_soundInstances.push_back(ret);

    return ret;
}

/*private*/
unsigned int
SDL_sound_handler::mixActiveSound(EmbedSoundInst& sound, Uint8* buffer,
        unsigned int mixSamples)
{

#ifdef GNASH_DEBUG_MIXING
    log_debug("Asked to mix-in %d samples of this sound", mixSamples);
#endif

    // Use global volume (sound-specific volume will be used before)
    unsigned int wroteSamples = do_mixing(buffer, sound, mixSamples, getFinalVolume());

    return wroteSamples;
}

/*private*/
void
SDL_sound_handler::mixSoundData(EmbedSound& sounddata, Uint8* buffer, unsigned int buffer_length)
{
    assert(!(buffer_length%2));
    unsigned int nSamples = buffer_length/2;

	for (EmbedSound::Instances::iterator
		 i=sounddata._soundInstances.begin();
		 i != sounddata._soundInstances.end(); // don't cache .end(), isn't necessarely safe on erase
	    )
	{

		// Temp variables to make the code simpler and easier to read
		EmbedSoundInst& sound = *(*i); 

		unsigned int mixed = mixActiveSound(sound, buffer, nSamples);
		if ( mixed < nSamples )
		{
		    // Sound is done, remove it from the active list

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

