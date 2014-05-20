// sound_handler_haiku.h: Sound handling using Mkit media kit
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

#ifndef SOUND_HANDLER_MKIT_H
#define SOUND_HANDLER_MKIT_H

#include "sound_handler.h" // for inheritance

#include <set> // for composition (InputStreams)
#include <boost/thread/mutex.hpp>
#include <memory>

#include <SoundPlayer.h>

// Forward declarations
namespace gnash {
    class SimpleBuffer;
    namespace sound {
        class EmbedSound;
        class InputStream;
    }
}

namespace gnash {
namespace sound {

/// Mkit media kit based sound_handler
class Mkit_sound_handler : public sound_handler
{
    std::unique_ptr<BSoundPlayer> _soundplayer;

    /// play buffer handler function
    static void FillNextBuffer(void *cookie, void *buffer, size_t size,
            const media_raw_audio_format &format);

    /// @throw SoundException on error
    void openAudio();

    bool _audioopen;

    /// Mutex for making sure threads doesn't mess things up
    boost::mutex _mutex;

    /// Mutex protecting _muted (defined in base class)
    mutable boost::mutex _mutedMutex;

    // See dox in sound_handler.h
    void mix(boost::int16_t* outSamples, boost::int16_t* inSamples,
                unsigned int nSamples, float volume);

    void MixAudio (boost::uint8_t *dst, const boost::uint8_t *src, boost::uint32_t len, int volume);

public:
    Mkit_sound_handler(media::MediaHandler* m);

    ~Mkit_sound_handler();

    // See dox in sound_handler.h
    virtual int create_sound(std::unique_ptr<SimpleBuffer> data, std::unique_ptr<media::SoundInfo> sinfo);

    // See dox in sound_handler.h
    // overridden to serialize access to the data buffer slot
    virtual StreamBlockId addSoundBlock(unsigned char* data,
                                       unsigned int data_bytes,
                                       unsigned int sample_count,
                                       int streamId);

    // See dox in sound_handler.h
    virtual void    stop_sound(int sound_handle);

    // See dox in sound_handler.h
    virtual void    delete_sound(int sound_handle);

    // See dox in sound_handler.h
    virtual void reset();

    // See dox in sound_handler.h
    virtual void    stop_all_sounds();

    // See dox in sound_handler.h
    virtual int get_volume(int sound_handle);

    // See dox in sound_handler.h
    virtual void    set_volume(int sound_handle, int volume);

    // See dox in sound_handler.h
    virtual media::SoundInfo* get_sound_info(int soundHandle);

    // See dox in sound_handler.h
    // overridden to serialize access to the _muted member
    virtual void mute();

    // See dox in sound_handler.h
    // overridden to serialize access to the _muted member
    virtual void unmute();

    // See dox in sound_handler.h
    // overridden to serialize access to the _muted member
    virtual bool is_muted() const;

    // See dox in sound_handler.h
    // overridden to close audio card
    virtual void pause();

    // See dox in sound_handler.h
    // overridden to open audio card
    virtual void unpause();

    // See dox in sound_handler.h
    virtual unsigned int get_duration(int sound_handle);

    // See dox in sound_handler.h
    virtual unsigned int tell(int sound_handle);

    // See dox in sound_handler.h
    // Overridden to unpause SDL audio
    void plugInputStream(std::unique_ptr<InputStream> in);

    // See dox in sound_handler.h
    void fetchSamples(boost::int16_t* to, unsigned int nSamples);
};

} // gnash.sound namespace 
} // namespace gnash

#endif // SOUND_HANDLER_MKIT_H
