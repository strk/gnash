// sound_handler_sdl.h: Sound handling using standard SDL
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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


#ifndef SOUND_HANDLER_SDL_H
#define SOUND_HANDLER_SDL_H


#include "sound_handler.h" // for inheritance

#include <set> // for composition (InputStreams)
#include <SDL_audio.h>
#include <boost/thread/mutex.hpp>

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

/// SDL-based sound_handler
class SDL_sound_handler : public sound_handler
{
private:

    /// The SDL_audio specs
    SDL_AudioSpec audioSpec;

    /// Initialize audio card
    void initAudio();

    void openAudio();

    void closeAudio();

    bool _audioOpened;
    
    /// Mutex for making sure threads doesn't mess things up
    mutable boost::mutex _mutex;

    /// Mutex protecting _muted (defined in base class)
    mutable boost::mutex _mutedMutex;

    // See dox in sound_handler.h
    void mix(boost::int16_t* outSamples, boost::int16_t* inSamples,
                unsigned int nSamples, float volume);


    /// Callback invoked by the SDL audio thread.
    //
    /// This is basically a wrapper around fetchSamples
    ///
    /// @param udata
    ///     User data pointer (SDL_sound_handler instance in our case).
    ///     We'll lock the SDL_sound_handler::_mutex during operations.
    ///
    /// @param stream
    ///     The output stream/buffer to fill
    ///
    /// @param buffer_length_in
    ///     Length of the buffer.
    ///     If zero or negative we log an error and return
    ///     (negative is probably an SDL bug, zero dunno yet).
    ///
    static void sdl_audio_callback (void *udata, Uint8 *stream, int buffer_length_in);

public:

    SDL_sound_handler(media::MediaHandler* m);

    ~SDL_sound_handler();

    virtual int createStreamingSound(const media::SoundInfo& sinfo);

    // See dox in sound_handler.h
    virtual int create_sound(std::auto_ptr<SimpleBuffer> data,
            const media::SoundInfo& sinfo);

    // See dox in sound_handler.h
    // overridden to serialize access to the data buffer slot
    virtual StreamBlockId addSoundBlock(unsigned char* data,
                                       unsigned int data_bytes,
                                       unsigned int sample_count,
                                       int streamId);

    // See dox in sound_handler.h
    virtual void stopEventSound(int sound_handle);

    virtual void stopStreamingSound(int sound_handle);

    // See dox in sound_handler.h
    virtual void    delete_sound(int sound_handle);

    // See dox in sound_handler.h
    virtual void reset();

    // See dox in sound_handler.h
    virtual void    stop_all_sounds();

    // See dox in sound_handler.h
    virtual int get_volume(int sound_handle) const;

    // See dox in sound_handler.h
    virtual void set_volume(int sound_handle, int volume);
        
    // See dox in sound_handler.h
    virtual media::SoundInfo* get_sound_info(int soundHandle) const;

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
    virtual unsigned int get_duration(int sound_handle) const;

    // See dox in sound_handler.h
    virtual unsigned int tell(int sound_handle) const;
    
    // See dox in sound_handler.h
    // Overridden to unpause SDL audio
    void plugInputStream(std::auto_ptr<InputStream> in);

    // Overidden to provide thread safety.
    void unplugInputStream(InputStream* id);

    // See dox in sound_handler.h
    void fetchSamples(boost::int16_t* to, unsigned int nSamples);
};

} // gnash.sound namespace 
} // namespace gnash

#endif // SOUND_HANDLER_SDL_H
