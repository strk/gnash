// sound_handler_sdl.h: Sound handling using standard SDL
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


#ifndef SOUND_HANDLER_SDL_H
#define SOUND_HANDLER_SDL_H


#include "sound_handler.h" // for inheritance

#include <vector> // for composition (Sounds)
#include <set> // for composition (InputStreams)
#include <fstream> // for composition (file_stream)
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
public:

    typedef std::vector<EmbedSound*> Sounds;

private:

    typedef std::set< InputStream* > InputStreams;

    /// Sound input streams.
    //
    /// Elements owned by this class.
    ///
    InputStreams _inputStreams;

    /// Unplug all input streams
    void unplugAllInputStreams();

    /// Vector containing all sounds.
    //
    /// Elements of the vector are owned by this class
    ///
    Sounds  _sounds;

    /// Is sound device opened?
    bool soundOpened;

    /// The SDL_audio specs
    SDL_AudioSpec audioSpec;

    void initAudioSpec();
    
    /// Keeps track of numbers of playing sounds
    int soundsPlaying;

    /// Is the audio muted?
    bool muted;
    
    /// Mutex for making sure threads doesn't mess things up
    boost::mutex _mutex;

    // stop and delete all sounds
    void delete_all_sounds();

    /// File stream for dump file
    std::ofstream file_stream;

    // write a .WAV file header
    void write_wave_header(std::ofstream& outfile);

    void mixSoundData(EmbedSound& sounddata, Uint8* stream, unsigned int buffer_length);

    /// Mix an InputStream in
    //
    /// @param sound
    ///     The active sound to mix in
    //
    /// @param mixTo
    ///     The buffer to mix sound into
    ///
    /// @param nSamples
    ///     The number of samples to mix in
    ///
    /// @return the number of samples mixed in. 
    ///         If < nSamples we reached EOF of the EmbedSoundInst
    ///
    unsigned int mixIn(InputStream& sound, Uint8* mixTo, unsigned int nSamples);

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

    SDL_sound_handler();

    SDL_sound_handler(const std::string& wave_file);

    ~SDL_sound_handler();

    // See dox in sound_handler.h
    virtual int create_sound(std::auto_ptr<SimpleBuffer> data, std::auto_ptr<media::SoundInfo> sinfo);

    // see dox in sound_handler.h
    virtual long    fill_stream_data(unsigned char* data, unsigned int data_bytes,
                     unsigned int sample_count, int handle_id);

    // See dox in sound_handler.h
    virtual void    play_sound(int sound_handle, int loopCount, int offset,
                   long start_position, const SoundEnvelopes* envelopes);

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
    virtual void    mute();

    // See dox in sound_handler.h
    virtual void    unmute();

    // See dox in sound_handler.h
    virtual bool    is_muted();

    // See dox in sound_handler.h
    virtual unsigned int get_duration(int sound_handle);

    // See dox in sound_handler.h
    virtual unsigned int tell(int sound_handle);
    
    // See dox in sound_handler.h
    virtual InputStream* attach_aux_streamer(aux_streamer_ptr ptr, void* owner);

    // See dox in sound_handler.h
    virtual void unplugInputStream(InputStream* id);

    /// Refills the output buffer with data.
    //
    /// We run trough all the attached auxiliary streamers fetching decoded
    /// audio blocks and mixing them into the given output stream.
    ///
    /// If sound is compresssed (mp3) a mp3-frame is decoded into a buffer,
    /// and resampled if needed. When the buffer has been sampled, another
    /// frame is decoded until all frames has been decoded.
    /// If a sound is looping it will be decoded from the beginning again.
    ///
    /// @param to
    ///     The buffer to refill
    ///
    /// @param nBytes
    ///     The amount of bytes the output buffer holds
    ///
    /// @todo make this a virtual method in the base class
    ///
    void fetchSamples(boost::uint8_t* to, unsigned int nBytes);
};

} // gnash.sound namespace 
} // namespace gnash

#endif // SOUND_HANDLER_SDL_H
