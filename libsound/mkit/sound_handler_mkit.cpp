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

/* The volume ranges from 0 - 128 */
#define MIX_MAXVOLUME 128
#define ADJUST_VOLUME(s, v)    (s = (s*v)/MIX_MAXVOLUME)
#define ADJUST_VOLUME_U8(s, v)    (s = (((s-128)*v)/MIX_MAXVOLUME)+128)


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

//    boost::mutex::scoped_lock lock(_mutex);
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
    boost::int16_t *data = (boost::int16_t*) buffer;

    Mkit_sound_handler *that =
        reinterpret_cast<Mkit_sound_handler*>(cookie);

    that->fetchSamples(data, numSamples);
}

void
Mkit_sound_handler::fetchSamples(boost::int16_t* to, unsigned int nSamples)
{
    boost::mutex::scoped_lock lock(_mutex);
    sound_handler::fetchSamples(to, nSamples);

    // TODO: move this to base class !
    if (file_stream)
    {
        // NOTE: if muted, the samples will be silent already
        boost::uint8_t* stream = reinterpret_cast<boost::uint8_t*>(to);
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
    boost::mutex::scoped_lock lock(_mutex);
    sound_handler::delete_all_sounds();
    sound_handler::stop_all_sounds();
}

int
Mkit_sound_handler::create_sound(std::unique_ptr<SimpleBuffer> data,
                                std::unique_ptr<media::SoundInfo> sinfo)
{
    boost::mutex::scoped_lock lock(_mutex);
    return sound_handler::create_sound(data, sinfo);
}

sound_handler::StreamBlockId
Mkit_sound_handler::addSoundBlock(unsigned char* data,
        unsigned int dataBytes, unsigned int nSamples,
        int streamId)
{

    boost::mutex::scoped_lock lock(_mutex);
    return sound_handler::addSoundBlock(data, dataBytes, nSamples, streamId);
}

void
Mkit_sound_handler::stop_sound(int soundHandle)
{
    boost::mutex::scoped_lock lock(_mutex);
    sound_handler::stop_sound(soundHandle);
}


void
Mkit_sound_handler::delete_sound(int soundHandle)
{
    boost::mutex::scoped_lock lock(_mutex);
    sound_handler::delete_sound(soundHandle);
}

void
Mkit_sound_handler::stop_all_sounds()
{
    boost::mutex::scoped_lock lock(_mutex);
    sound_handler::stop_all_sounds();
}


int
Mkit_sound_handler::get_volume(int soundHandle)
{
    boost::mutex::scoped_lock lock(_mutex);
    return sound_handler::get_volume(soundHandle);
}


void
Mkit_sound_handler::set_volume(int soundHandle, int volume)
{
    boost::mutex::scoped_lock lock(_mutex);
    sound_handler::set_volume(soundHandle, volume);
}

media::SoundInfo*
Mkit_sound_handler::get_sound_info(int soundHandle)
{
    boost::mutex::scoped_lock lock(_mutex);
    return sound_handler::get_sound_info(soundHandle);
}

unsigned int
Mkit_sound_handler::get_duration(int soundHandle)
{
    boost::mutex::scoped_lock lock(_mutex);
    return sound_handler::get_duration(soundHandle);
}

unsigned int
Mkit_sound_handler::tell(int soundHandle)
{
    boost::mutex::scoped_lock lock(_mutex);
    return sound_handler::tell(soundHandle);
}

void 
Mkit_sound_handler::MixAudio (boost::uint8_t *dst, const boost::uint8_t *src, boost::uint32_t len, int volume)
{
    //boost::uint16_t format;

    if ( volume == 0 ) 
    {
        return;
    }

    //format = AHIST_S16S;

    /* Actually we have a fixed audio format */
    //switch (format) 
    {
        //case AHIST_S16S:
        {
            boost::int16_t src1, src2;
            int dst_sample;
            const int max_audioval = ((1<<(16-1))-1);
            const int min_audioval = -(1<<(16-1));

            len /= 2;
            while ( len-- ) 
            {
                src1 = ((src[0])<<8|src[1]);
                ADJUST_VOLUME(src1, volume);
                src2 = ((dst[0])<<8|dst[1]);
                src += 2;
                dst_sample = src1+src2;
                if ( dst_sample > max_audioval ) 
                {
                    dst_sample = max_audioval;
                } 
                else
                if ( dst_sample < min_audioval ) 
                {
                    dst_sample = min_audioval;
                }
                dst[1] = dst_sample & 0xFF;
                dst_sample >>= 8;
                dst[0] = dst_sample & 0xFF;
                dst += 2;
            }
        }
        //break;
    }
    
}

void
Mkit_sound_handler::mix(boost::int16_t* outSamples, boost::int16_t* inSamples, unsigned int nSamples, float volume)
{
    //if (!_closing)
    {
        unsigned int nBytes = nSamples*2;

        boost::uint8_t *out = reinterpret_cast<boost::uint8_t*>(outSamples);
        boost::uint8_t* in = reinterpret_cast<boost::uint8_t*>(inSamples);

        MixAudio(out, in, nBytes, MIX_MAXVOLUME*volume);
    }
}

void
Mkit_sound_handler::plugInputStream(std::unique_ptr<InputStream> newStreamer)
{
    boost::mutex::scoped_lock lock(_mutex);

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
Mkit_sound_handler::mute()
{
    boost::mutex::scoped_lock lock(_mutedMutex);
    sound_handler::mute();
}

void
Mkit_sound_handler::unmute()
{
    boost::mutex::scoped_lock lock(_mutedMutex);
    sound_handler::unmute();
}

bool
Mkit_sound_handler::is_muted() const
{
    boost::mutex::scoped_lock lock(_mutedMutex);
    return sound_handler::is_muted();
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
