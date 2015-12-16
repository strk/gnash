// sound_handler_haiku.cpp: Sound handling using Haiku media kit
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

#include "sound_handler_mkit.h"

#include "SoundInfo.h"
#include "EmbedSound.h"
#include "AuxStream.h" // for use..

#include "log.h" // will import boost::format too
#include "GnashException.h" // for SoundException

#include <vector>

#include <SoundPlayer.h>

// Define this to get debugging call about pausing/unpausing audio
//#define GNASH_DEBUG_MKIT_AUDIO_PAUSING

// Mixing and decoding debugging
//#define GNASH_DEBUG_MIXING

namespace gnash {
namespace sound {

Mkit_sound_handler::Mkit_sound_handler(media::MediaHandler* m)
    :
    sound_handler(m),
    _audioopen(false)
{
}

void
Mkit_sound_handler::openAudio()
{
    if (_audioopen == true)
        return;
    _audioopen = true;

    media_raw_audio_format format;
    format.format = media_raw_audio_format::B_AUDIO_SHORT;
    format.channel_count = 2;
    format.frame_rate = 44100;
    format.byte_order = (B_HOST_IS_BENDIAN) ?
        B_MEDIA_BIG_ENDIAN : B_MEDIA_LITTLE_ENDIAN;
    format.buffer_size = media_raw_audio_format::wildcard.buffer_size;

    _soundplayer.reset(new BSoundPlayer(&format, "Gnash",
            Mkit_sound_handler::FillNextBuffer, NULL, this));
    if (B_OK != _soundplayer->InitCheck())
        throw SoundException(_("Unable to open audio"));
    _soundplayer->Start();
}

Mkit_sound_handler::~Mkit_sound_handler()
{
    if (_soundplayer != NULL)
        _soundplayer->Stop(true, true);

//    std::lock_guard<std::mutex> lock(_mutex);
//#ifdef GNASH_DEBUG_HAIKU_AUDIO_PAUSING
//    log_debug("Pausing Mkit Audio on destruction");
//#endif
//    SDL_PauseAudio(1);
//
//    lock.unlock();

    // we already locked, so we call 
    // the base class (non-locking) deleter
    delete_all_sounds();

    unplugAllInputStreams();

//    SDL_CloseAudio();

    if (file_stream) file_stream.close();
}

void
Mkit_sound_handler::FillNextBuffer(void *cookie, void *buffer, size_t size,
        const media_raw_audio_format &format)
{
    (void) format;

    size_t numSamples =
        size / sizeof(uint16);
    std::int16_t *data = (std::int16_t*) buffer;

    Mkit_sound_handler *that =
        reinterpret_cast<Mkit_sound_handler*>(cookie);

    that->fetchSamples(data, numSamples);
}

void
Mkit_sound_handler::fetchSamples(std::int16_t* to, unsigned int nSamples)
{
    std::lock_guard<std::mutex> lock(_mutex);
    sound_handler::fetchSamples(to, nSamples);

    // TODO: move this to base class !
    if (file_stream)
    {
        // NOTE: if muted, the samples will be silent already
        std::uint8_t* stream = reinterpret_cast<std::uint8_t*>(to);
        unsigned int len = nSamples*2;
        file_stream.write((char*) stream, len);

        // now, mute all audio
        std::fill(to, to+nSamples, 0);
    }

    // If nothing is left to play there is no reason to keep polling.
    if ( ! hasInputStreams() )
    {
#ifdef GNASH_DEBUG_HAIKU_AUDIO_PAUSING
        log_debug("Pausing Mkit Audio...");
#endif
        sound_handler::pause();
    }
}

void
Mkit_sound_handler::reset()
{
    std::lock_guard<std::mutex> lock(_mutex);
    sound_handler::delete_all_sounds();
    sound_handler::stop_all_sounds();
}

int
Mkit_sound_handler::create_sound(std::unique_ptr<SimpleBuffer> data,
                                std::unique_ptr<media::SoundInfo> sinfo)
{
    std::lock_guard<std::mutex> lock(_mutex);
    return sound_handler::create_sound(data, sinfo);
}

sound_handler::StreamBlockId
Mkit_sound_handler::addSoundBlock(unsigned char* data,
        unsigned int dataBytes, unsigned int nSamples,
        int streamId)
{

    std::lock_guard<std::mutex> lock(_mutex);
    return sound_handler::addSoundBlock(data, dataBytes, nSamples, streamId);
}

void
Mkit_sound_handler::stop_sound(int soundHandle)
{
    std::lock_guard<std::mutex> lock(_mutex);
    sound_handler::stop_sound(soundHandle);
}


void
Mkit_sound_handler::delete_sound(int soundHandle)
{
    std::lock_guard<std::mutex> lock(_mutex);
    sound_handler::delete_sound(soundHandle);
}

void
Mkit_sound_handler::stop_all_sounds()
{
    std::lock_guard<std::mutex> lock(_mutex);
    sound_handler::stop_all_sounds();
}


int
Mkit_sound_handler::get_volume(int soundHandle)
{
    std::lock_guard<std::mutex> lock(_mutex);
    return sound_handler::get_volume(soundHandle);
}


void
Mkit_sound_handler::set_volume(int soundHandle, int volume)
{
    std::lock_guard<std::mutex> lock(_mutex);
    sound_handler::set_volume(soundHandle, volume);
}

media::SoundInfo*
Mkit_sound_handler::get_sound_info(int soundHandle)
{
    std::lock_guard<std::mutex> lock(_mutex);
    return sound_handler::get_sound_info(soundHandle);
}

unsigned int
Mkit_sound_handler::get_duration(int soundHandle)
{
    std::lock_guard<std::mutex> lock(_mutex);
    return sound_handler::get_duration(soundHandle);
}

unsigned int
Mkit_sound_handler::tell(int soundHandle)
{
    std::lock_guard<std::mutex> lock(_mutex);
    return sound_handler::tell(soundHandle);
}

void
Mkit_sound_handler::plugInputStream(std::unique_ptr<InputStream> newStreamer)
{
    std::lock_guard<std::mutex> lock(_mutex);

    sound_handler::plugInputStream(newStreamer);

    { // TODO: this whole block should only be executed when adding
      // the first stream.

#ifdef GNASH_DEBUG_Mkit_AUDIO_PAUSING
        log_debug("Unpausing Mkit Audio on inpust stream plug...");
#endif
        openAudio();
        sound_handler::unpause();

    }
}

void
Mkit_sound_handler::pause()
{
    log_debug(_("Mkit: Mkit_sound_handler::pause"));
    if (_soundplayer != NULL)
        _soundplayer->SetHasData(false);
    sound_handler::pause();
    log_debug(_("Mkit: paused"));
}

void
Mkit_sound_handler::unpause()
{
    if ( hasInputStreams() )
    {
        log_debug(_("Mkit: Mkit_sound_handler::unpause"));
        if (_soundplayer != NULL)
            _soundplayer->SetHasData(true);
        sound_handler::unpause();
        log_debug(_("Mkit: unpaused"));
    }
}


sound_handler*
create_sound_handler_mkit(media::MediaHandler* m)
// Factory.
{
    return new Mkit_sound_handler(m);
}

} // gnash.sound namespace 
} // namespace gnash

// Local Variables:
// mode: C++
// End:
