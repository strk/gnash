// sound_handler_sdl.cpp: Sound handling using standard SDL
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#include "sound_handler_sdl.h"
#include "SoundInfo.h"
#include "EmbedSound.h"
#include "AuxStream.h" // for use..
#include "GnashSleep.h"

#include "log.h" // will import boost::format too
#include "GnashException.h" // for SoundException

#include <vector>
#include <SDL.h>

// Define this to get debugging call about pausing/unpausing audio
//#define GNASH_DEBUG_SDL_AUDIO_PAUSING

// Mixing and decoding debugging
//#define GNASH_DEBUG_MIXING


namespace gnash {
namespace sound {


void
SDL_sound_handler::initAudio()
{
    // NOTE: we open and close the audio card for the sole purpose
    //       of throwing an exception on error (unavailable audio
    //       card). Normally we'd want to open the audio card only
    //       when needed (it has a cost in number of wakeups).
    openAudio();

#ifdef WIN32
    // SDL can hang on windows if SDL_CloseAudio() is called immediately
    // after SDL_OpenAudio(). It's evidently to do with threading, but
    // internal to SDL. This is a tacky solution, but it's only windows.
    gnashSleep(1);
#endif

    closeAudio();

}

void
SDL_sound_handler::openAudio()
{
    if (_audioOpened) return; // nothing to do

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
    audioSpec.samples = 1024;   

    if (SDL_OpenAudio(&audioSpec, nullptr) < 0) {
            boost::format fmt = boost::format(_("Couldn't open SDL audio: %s"))
                % SDL_GetError();
        throw SoundException(fmt.str());
    }

    _audioOpened = true;
}

void
SDL_sound_handler::closeAudio()
{
    SDL_CloseAudio();
    _audioOpened = false;
}


SDL_sound_handler::SDL_sound_handler(media::MediaHandler* m)
    :
    sound_handler(m),
    _audioOpened(false)
{
    initAudio();
}

void
SDL_sound_handler::reset()
{
    std::lock_guard<std::mutex> lock(_mutex);
    sound_handler::stop_all_sounds();
}

SDL_sound_handler::~SDL_sound_handler()
{
    std::lock_guard<std::mutex> lock(_mutex);

#ifdef GNASH_DEBUG_SDL_AUDIO_PAUSING
    log_debug("Pausing SDL Audio on destruction");
#endif
    SDL_PauseAudio(1);

    // this one takes 2 seconds on my machine ...
    SDL_CloseAudio();

}

int
SDL_sound_handler::createStreamingSound(const media::SoundInfo& sinfo)
{
    std::lock_guard<std::mutex> lock(_mutex);
    return sound_handler::createStreamingSound(sinfo);
}

int
SDL_sound_handler::create_sound(std::unique_ptr<SimpleBuffer> data,
                                const media::SoundInfo& sinfo)
{
    std::lock_guard<std::mutex> lock(_mutex);
    return sound_handler::create_sound(std::move(data), sinfo);
}

sound_handler::StreamBlockId
SDL_sound_handler::addSoundBlock(std::unique_ptr<SimpleBuffer> buf,
        size_t sampleCount, int seekSamples, int handle)
{

    std::lock_guard<std::mutex> lock(_mutex);
    return sound_handler::addSoundBlock(std::move(buf), sampleCount, seekSamples, handle);
}


void
SDL_sound_handler::stopEventSound(int soundHandle)
{
    std::lock_guard<std::mutex> lock(_mutex);
    sound_handler::stopEventSound(soundHandle);
}

void
SDL_sound_handler::stopAllEventSounds()
{
    std::lock_guard<std::mutex> lock(_mutex);
    sound_handler::stopAllEventSounds();
}

void
SDL_sound_handler::stopStreamingSound(int soundHandle)
{
    std::lock_guard<std::mutex> lock(_mutex);
    sound_handler::stopStreamingSound(soundHandle);
}


void
SDL_sound_handler::delete_sound(int soundHandle)
{
    std::lock_guard<std::mutex> lock(_mutex);
    sound_handler::delete_sound(soundHandle);
}

void   
SDL_sound_handler::stop_all_sounds()
{
    std::lock_guard<std::mutex> lock(_mutex);
    sound_handler::stop_all_sounds();
}


int
SDL_sound_handler::get_volume(int soundHandle) const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return sound_handler::get_volume(soundHandle);
}


void   
SDL_sound_handler::set_volume(int soundHandle, int volume)
{
    std::lock_guard<std::mutex> lock(_mutex);
    sound_handler::set_volume(soundHandle, volume);
}
    
media::SoundInfo*
SDL_sound_handler::get_sound_info(int soundHandle) const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return sound_handler::get_sound_info(soundHandle);
}

unsigned int
SDL_sound_handler::get_duration(int soundHandle) const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return sound_handler::get_duration(soundHandle);
}

unsigned int
SDL_sound_handler::tell(int soundHandle) const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return sound_handler::tell(soundHandle);
}

sound_handler*
create_sound_handler_sdl(media::MediaHandler* m)
{
    return new SDL_sound_handler(m);
}

void
SDL_sound_handler::fetchSamples(std::int16_t* to, unsigned int nSamples)
{
    std::lock_guard<std::mutex> lock(_mutex);
    sound_handler::fetchSamples(to, nSamples);

    // If nothing is left to play there is no reason to keep polling.
    if ( ! hasInputStreams() )
    {
#ifdef GNASH_DEBUG_SDL_AUDIO_PAUSING
        log_debug("Pausing SDL Audio...");
#endif
        SDL_PauseAudio(1);
    }
}

// Callback invoked by the SDL audio thread.
void
SDL_sound_handler::sdl_audio_callback(void *udata, Uint8 *buf, int bufLenIn)
{
    if (bufLenIn < 0) {
        log_error(_("Negative buffer length in sdl_audio_callback (%d)"),
                bufLenIn);
        return;
    }

    if (bufLenIn == 0) {
        log_error(_("Zero buffer length in sdl_audio_callback"));
        return;
    }

    unsigned int bufLen = static_cast<unsigned int>(bufLenIn);
    std::int16_t* samples = reinterpret_cast<std::int16_t*>(buf);

    // 16 bit per sample, 2 channels == 4 bytes per fetch ?
    assert(!(bufLen%4));

    unsigned int nSamples = bufLen/2;

    //log_debug("Fetching %d bytes (%d samples)", bufLen, nSamples);

    // Get the soundhandler
    SDL_sound_handler* handler = static_cast<SDL_sound_handler*>(udata);
    handler->fetchSamples(samples, nSamples);
}

void
SDL_sound_handler::mix(std::int16_t* outSamples, std::int16_t* inSamples,
            unsigned int nSamples, float volume)
{
    Uint8* out = reinterpret_cast<Uint8*>(outSamples);
    Uint8* in = reinterpret_cast<Uint8*>(inSamples);
    unsigned int nBytes = nSamples*2;
    
    SDL_MixAudio(out, in, nBytes, SDL_MIX_MAXVOLUME*volume);
}

void
SDL_sound_handler::plugInputStream(std::unique_ptr<InputStream> newStreamer)
{
    std::lock_guard<std::mutex> lock(_mutex);

    sound_handler::plugInputStream(std::move(newStreamer));

    { // TODO: this whole block should only be executed when adding
      // the first stream. 

#ifdef GNASH_DEBUG_SDL_AUDIO_PAUSING
        log_debug("Unpausing SDL Audio on inpust stream plug...");
#endif
        openAudio(); // lazy sound card initialization
        SDL_PauseAudio(0); // start polling data from us 
    }
}

void
SDL_sound_handler::mute()
{
    std::lock_guard<std::mutex> lock(_mutedMutex);
    sound_handler::mute();
}

void
SDL_sound_handler::unmute()
{
    std::lock_guard<std::mutex> lock(_mutedMutex);
    sound_handler::unmute();
}

bool
SDL_sound_handler::is_muted() const
{
    std::lock_guard<std::mutex> lock(_mutedMutex);
    return sound_handler::is_muted();
}

void
SDL_sound_handler::pause() 
{
    closeAudio();
    sound_handler::pause();
}

void
SDL_sound_handler::unpause() 
{
    if (hasInputStreams()) {
        openAudio();
        SDL_PauseAudio(0);
    }

    sound_handler::unpause();
}

void
SDL_sound_handler::unplugInputStream(InputStream* id)
{
    std::lock_guard<std::mutex> lock(_mutex);

    sound_handler::unplugInputStream(id);
}

} // gnash.sound namespace 
} // namespace gnash

// Local Variables:
// mode: C++
// End:

