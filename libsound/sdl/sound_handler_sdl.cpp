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
#include "SoundInfo.h"
#include "EmbedSound.h"
#include "EmbedSoundInst.h" // for upcasting to InputStream
#include "AuxStream.h" // for use..

#include "log.h" // will import boost::format too
#include "GnashException.h" // for SoundException

//#include <cmath>
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

// Debug samples fetching
//#define GNASH_DEBUG_SAMPLES_FETCHING

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
        EmbedSound* sdef = *i;

        // The sound may have been deleted already.
        if (!sdef) continue;

        size_t nInstances = sdef->numPlayingInstances();

        log_debug("delete_all_sounds: marking %d instances of embedded sound as stopped",
                    nInstances);

        // Increment number of sound stop request for the testing framework
        _soundsStopped += nInstances;

        delete sdef;
    }
    _sounds.clear();
}

void
SDL_sound_handler::unplugAllInputStreams()
{
    boost::mutex::scoped_lock lock(_mutex);

    for (InputStreams::iterator it=_inputStreams.begin(),
                                itE=_inputStreams.end();
            it != itE; ++it)
    {
        delete *it;
    }
    _inputStreams.clear();
}

SDL_sound_handler::~SDL_sound_handler()
{
    delete_all_sounds();
    unplugAllInputStreams();
    if (soundOpened) SDL_CloseAudio();
    if (file_stream) file_stream.close();

}


int SDL_sound_handler::create_sound(
    std::auto_ptr<SimpleBuffer> data,
    std::auto_ptr<media::SoundInfo> sinfo)
{

    assert(sinfo.get());

    std::auto_ptr<EmbedSound> sounddata ( new EmbedSound(data, sinfo) );

    // Make sure we're the only thread accessing _sounds here
    boost::mutex::scoped_lock lock(_mutex);

    int sound_id = _sounds.size();

#ifdef GNASH_DEBUG_SOUNDS_MANAGEMENT
    log_debug("create_sound: sound %d, format %s %s %dHz, %d samples (%d bytes)",
        sound_id, sounddata->soundinfo->getFormat(),
        sounddata->soundinfo->isStereo() ? "stereo" : "mono",
        sounddata->soundinfo->getSampleRate(),
        sounddata->soundinfo->getSampleCount(),
        sounddata->size());
#endif

    // the vector takes ownership
    _sounds.push_back(sounddata.release());

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
    std::auto_ptr<InputStream> sound ( sounddata.createInstance(

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

    ) );

    plugInputStream(sound);
}


void
SDL_sound_handler::stop_sound(int sound_handle)
{
    boost::mutex::scoped_lock lock(_mutex);

    // Check if the sound exists.
    if (sound_handle < 0 || (unsigned int) sound_handle >= _sounds.size())
    {
        log_debug("stop_sound(%d): invalid sound id", sound_handle);
        // Invalid handle.
        return;
    }

    
    EmbedSound* sounddata = _sounds[sound_handle];
    if ( ! sounddata )
    {
        log_error("stop_sound(%d): sound was deleted", sound_handle);
        return;
    }

#ifdef GNASH_DEBUG_SOUNDS_MANAGEMENT
    log_debug("stop_sound %d called", sound_handle);
#endif

    stopEmbedSoundInstances(*sounddata);

}


// this gets called when it's done with a sample.
void
SDL_sound_handler::delete_sound(int sound_handle)
{
    boost::mutex::scoped_lock lock(_mutex);

    // Check if the sound exists
    if (sound_handle < 0 || static_cast<unsigned int>(sound_handle) >= _sounds.size())
    {
        log_error("Invalid (%d) sound_handle passed to delete_sound, "
                  "doing nothing", sound_handle);
        return;
    }

#ifdef GNASH_DEBUG_SOUNDS_MANAGEMENT
    log_debug ("deleting sound :%d", sound_handle);
#endif

    EmbedSound* def = _sounds[sound_handle];
    if ( ! def )
    {
        log_error("sound_handle passed to delete_sound (%d) "
                  "already deleted", sound_handle);
        return;
    }
    
    stopEmbedSoundInstances(*def);
    delete def;
    _sounds[sound_handle] = NULL;

}

void   
SDL_sound_handler::stopEmbedSoundInstances(EmbedSound& def)
{
    // Assert _mutex is locked ...

    typedef std::vector<InputStream*> InputStreamVect;
    InputStreamVect playing;
    def.getPlayingInstances(playing);

    // Now, for each playing InputStream, unplug it!
    // NOTE: could be optimized...
    for (InputStreamVect::iterator i=playing.begin(), e=playing.end();
            i!=e; ++i)
    {
#ifdef GNASH_DEBUG_SOUNDS_MANAGEMENT
        log_debug(" unplugging input stream %p from stopEmbedSoundInstances", *i);
#endif

        unplugInputStreamNonLocking(*i);
    }

    def.clearInstances();
}

void   
SDL_sound_handler::stop_all_sounds()
{
    boost::mutex::scoped_lock lock(_mutex);

    for (Sounds::iterator i = _sounds.begin(),
                          e = _sounds.end(); i != e; ++i)
    {
        EmbedSound* sounddata = *i;
        stopEmbedSoundInstances(*sounddata);
    }
}


//  returns the sound volume level as an integer from 0 to 100,
//  where 0 is off and 100 is full volume. The default setting is 100.
int SDL_sound_handler::get_volume(int sound_handle) {

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


//  A number from 0 to 100 representing a volume level.
//  100 is full volume and 0 is no volume. The default setting is 100.
void    SDL_sound_handler::set_volume(int sound_handle, int volume) {

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

bool
SDL_sound_handler::is_muted()
{
    boost::mutex::scoped_lock lock(_mutex);
    return muted;
}

InputStream*
SDL_sound_handler::attach_aux_streamer(aux_streamer_ptr ptr, void* owner)
{
    boost::mutex::scoped_lock lock(_mutex);
    assert(owner);
    assert(ptr);

    std::auto_ptr<InputStream> newStreamer ( new AuxStream(ptr, owner) );

    InputStream* ret = newStreamer.get();

    plugInputStream(newStreamer);

    return ret;
}

void
SDL_sound_handler::unplugInputStream(InputStream* id)
{
    boost::mutex::scoped_lock lock(_mutex);

    unplugInputStreamNonLocking(id);
}

void
SDL_sound_handler::unplugInputStreamNonLocking(InputStream* id)
{
    // WARNING: erasing would break any iteration in the set
    InputStreams::iterator it2=_inputStreams.find(id);
    if ( it2 == _inputStreams.end() )
    {
        log_error("SDL_sound_handler::unplugInputStream: "
                "Aux streamer %p not found. ",
                id);
        return; // we won't delete it, as it's likely deleted already
    }

    _inputStreams.erase(it2);

    // Increment number of sound stop request for the testing framework
    _soundsStopped++;

#ifdef GNASH_DEBUG_SOUNDS_MANAGEMENT
    log_debug("Unplugged InputStream %p", id);
#endif

    // Delete the InputStream (we own it..)
    delete id;
}

unsigned int
SDL_sound_handler::get_duration(int sound_handle)
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
    if ( ! sounddata->isPlaying() ) return 0;

    // We use the first active sound of this.
    InputStream* asound = sounddata->firstPlayingInstance();

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


/// Prepare for mixing/adding (volume adjustments) and mix/add.
//
/// @param mixTo
///     The buffer to mix to
///
/// @param sound
///     The InputStream to mix in.
///
/// @param nSamples
///     The amount of samples to mix in. Must be a multiple of 2 !!
///
/// @param volume
///     The volume to use for this mix-in.
///     100 is full volume. 
///
/// @return the number of samples actually wrote out.
///         If < nSamples we reached end of the InputStream
///
static unsigned int
do_mixing(Uint8* mixTo, InputStream& sound, unsigned int nSamples, unsigned int volume)
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
void
SDL_sound_handler::write_wave_header(std::ofstream& outfile)
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
    if (_inputStreams.empty())
    {
#ifdef GNASH_DEBUG_SDL_AUDIO_PAUSING
        log_debug("Pausing SDL Audio...");
#endif
        SDL_PauseAudio(1);
        return;
    }

    // Mixed sounddata buffer
    Uint8* buffer = stream;
    std::fill(buffer, buffer+buffer_length, 0);

    // call NetStream or Sound audio callbacks
    if ( !_inputStreams.empty() )
    {
        assert(!(buffer_length%2));
        unsigned int nSamples = buffer_length/2;
        boost::scoped_array<boost::int16_t> buf ( new boost::int16_t[nSamples] );

#ifdef GNASH_DEBUG_SAMPLES_FETCHING
        log_debug("Fetching %d samples for %d input streams", nSamples, _inputStreams.size());
#endif

        // Loop through the aux streamers sounds
        InputStreams::iterator it = _inputStreams.begin();
        InputStreams::iterator end = _inputStreams.end();
        while (it != end)
        {
            InputStream* is = *it;

            // Should we use mixIn here instead of doing manually ?

            unsigned int wrote = is->fetchSamples(buf.get(), nSamples);
            if ( wrote < nSamples )
            {
                // fill what wasn't written
                std::fill(buf.get()+wrote, buf.get()+nSamples, 0);
            }

#ifdef GNASH_DEBUG_SAMPLES_FETCHING
            log_debug("  fetched %d/%d samples from input stream %p"
                    " (%d samples fetchehd in total)",
                    wrote, nSamples, is, is->samplesFetched());
#endif

            // On EOF, detach
            if (is->eof())
            {
                // InputStream EOF, detach
                InputStreams::iterator it2=it;
                ++it2; // before we erase it
                InputStreams::size_type erased = _inputStreams.erase(is);
                if ( erased != 1 ) {
                    log_error("Expected 1 InputStream element, found %d", erased);
                    abort();
                }
                it = it2;

                log_debug("fetchSamples: marking stopped InputStream %p (on EOF)", is);

                // WARNING! deleting the InputStream here means
                //          a lot of things will happen from a
                //          separate thread. Instead, if we
                //          extend sound_handler interface to
                //          have an unplugCompletedInputStreams
                //          we may call it at heart-beating intervals
                //          and drop any threading paranoia!
                delete is;

                // Increment number of sound stop request for the testing framework
                _soundsStopped++;
            }
            else
            {
                ++it;
            }

            SDL_MixAudio(stream, reinterpret_cast<Uint8*>(buf.get()), buffer_length, finalVolume);

        }
    }

    // 
    // WRITE CONTENTS OF stream TO FILE
    //
    if (file_stream)
    {
            file_stream.write((char*) stream, buffer_length);
            // now, mute all audio
            std::fill(stream, stream+buffer_length, 0);
    }

    // Now, after having "consumed" all sounds, blank out
    // the buffer if muted..
    if ( muted )
    {
        std::fill(stream, stream+buffer_length, 0);
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

/*private*/
unsigned int
SDL_sound_handler::mixIn(InputStream& sound, Uint8* buf, unsigned int nSamples)
{

#ifdef GNASH_DEBUG_MIXING
    log_debug("Asked to mix-in %d samples of this sound", mixSamples);
#endif

    // Use global volume (sound-specific volume will be used before)
    unsigned int wrote = do_mixing(buf, sound, nSamples, getFinalVolume());

    return wrote;
}

void
SDL_sound_handler::plugInputStream(std::auto_ptr<InputStream> newStreamer)
{
    InputStream* newStream = newStreamer.get(); // for debugging

    if ( ! _inputStreams.insert(newStreamer.release()).second )
    {
        // this should never happen !
        log_error("_inputStreams container still has a pointer "
            "to deleted InputStream %p!", newStreamer.get());
        // FIXME: replace the old element with the new one !
        abort();
    }

#ifdef GNASH_DEBUG_SOUNDS_MANAGEMENT
    log_debug("Plugged InputStream %p", newStream);
#endif

    // Increment number of sound start request for the testing framework
    ++_soundsStarted;

    if (!soundOpened) {
        if (SDL_OpenAudio(&audioSpec, NULL) < 0 ) {
                boost::format fmt = boost::format(
                _("Unable to open SDL audio: %s"))
                % SDL_GetError();
            throw SoundException(fmt.str());
        }
        soundOpened = true;
    }

#ifdef GNASH_DEBUG_SDL_AUDIO_PAUSING
    log_debug("Unpausing SDL Audio...");
#endif
    SDL_PauseAudio(0);
}

} // gnash.sound namespace 
} // namespace gnash

// Local Variables:
// mode: C++
// End:

