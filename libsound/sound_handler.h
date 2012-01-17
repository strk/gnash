// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef SOUND_HANDLER_H
#define SOUND_HANDLER_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <string>
#include <vector>
#include <memory>
#include <limits>
#include <set>
#include <boost/scoped_ptr.hpp>

#include "dsodefs.h" // for DSOEXPORT
#include "MediaHandler.h" // for inlined ctor
#include "SoundEnvelope.h" // for SoundEnvelopes typedef
#include "AuxStream.h" // for aux_streamer_ptr typedef
#include "WAVWriter.h" // for dtor visibility 

namespace gnash {
    namespace media {
        class SoundInfo;
    }
    namespace sound {
        class EmbedSound;
        class StreamingSound;
        class StreamingSoundData;
        class InputStream;
    }
    class SimpleBuffer;
}

namespace gnash {

/// Gnash %sound handling subsystem (libsound)
//
/// This subsystem takes care of mixing audio
/// and communicating to the system mixer.
///
namespace sound {

/// %Sound mixer.
//
/// Stores the audio found by the parser and plays on demand.
/// Also allows plugging auxiliary streamers for ActionScript
/// driven %sound (Sound, NetStream).
///
/// The main interface this class exposes to hosting application
/// is fetching of sound samples, in couples of signed 16bit integers
/// (left channel, right channel) in PCM.
/// See fetchSamples.
///       
/// Implementations of this class willing to allow
/// hosting application to use a thread for fetching
/// audio should make sure to take care about mutex
/// protecting the relevant datastores.
/// The default implementation uses NO locking.
///
/// @todo rename to gnash::sound::Mixer ?
///
/// The sound_handler class stores embedded sounds. Embedded sounds can be
/// either streaming sounds (embedded in many StreamSoundBlock tags through
/// the SWF) or event sounds (defined in a single DefineSoundTag).
//
/// The interface is partly divided into separate functions for these types
/// of sound.
/// TODO: separate the functions fully.
class DSOEXPORT sound_handler
{
public:

    virtual ~sound_handler();

    /// Identifier of a streaming sound block
    //
    /// Use coupled with a soundId for fully qualified identifier
    ///
    typedef unsigned long StreamBlockId;

    ////////////////////////////////////////////////
    /// Mixed functions:
    ////////////////////////////////////////////////

    /// Remove all scheduled request for playback of sound buffer slots
    //
    /// This applies both to streaming and event sounds.
    virtual void stop_all_sounds();

    ////////////////////////////////////////////////
    /// Event sound functions:
    ////////////////////////////////////////////////
    
    /// Create a sound buffer slot, for on-demand playback.
    //
    /// @param data         The sound data to be stored. May not be null.
    ///                     This should be appropriately padded (@see
    ///                     MediaHandler::getInputPaddingBytes()), or a
    ///                     reallocation will take place here.
    /// @param sinfo        A SoundInfo object containing info about
    ///                     samplerate, samplecount, stereo etc.
    /// @return             handle for later identification.
    virtual int create_sound(std::auto_ptr<SimpleBuffer> data,
            const media::SoundInfo& sinfo);
        
    /// Remove scheduled requests to play the specified sound buffer slot
    //
    /// Stop all instances of the specified event sound if it's playing.
    /// (Normally a full-featured sound API would take a
    /// handle specifying the *instance* of a playing
    /// sample, but SWF is not expressive that way.)
    //
    /// @param sound_handle     id for the sound to be stopped
    virtual void stopEventSound(int sound_handle);

    /// Stop all instances of all playing event sounds. 
    //
    virtual void stopAllEventSounds();

    /// Discard the sound data for an embedded event sound
    //
    /// Only embedded event sounds are deleted; this happens when the
    /// associated sound_sample is deleted.
    //
    /// @param sound_handle     The id for the event sound to be deleted
    virtual void delete_sound(int sound_handle);

    /// Start playback of an event sound
    //
    /// All scheduled sounds will be played on next output flush.
    ///
    /// @param id
    ///     Id of the sound buffer slot to schedule playback of.
    ///
    /// @param loops
    ///     loops == 0 means play the sound once (1 means play it twice, etc)
    ///
    /// @param inPoint
    ///     Offset in output samples this instance should start
    ///     playing from. These are post-resampling samples (44100 
    ///     for one second of samples).
    ///
    /// @param outPoint
    ///     Offset in output samples this instance should stop
    ///     playing at. These are post-resampling samples (44100 
    ///     for one second of samples).
    ///     Use std::numeric_limits<unsigned int>::max() for no limit
    ///     (default if missing)
    ///
    /// @param env
    ///     Some eventsounds have some volume control mechanism called
    ///     envelopes.
    ///     They basically tells that from sample X the volume should be Y.
    ///
    /// @param allowMultiple
    ///     If false, the sound will not be scheduled if there's another
    ///     instance of it already playing.
    void startSound(int id, int loops, const SoundEnvelopes* env,
                   bool allowMultiple, unsigned int inPoint = 0,
                   unsigned int outPoint = 
                   std::numeric_limits<unsigned int>::max());

    /// Check if an event sound is playing
    //
    /// Note: this should not be used for streaming sounds.
    //
    /// @param id   Id of the sound buffer slot check for being alive
    bool isSoundPlaying(int id) const;
    
    /// Sets the volume for a given event sound.
    //
    /// Only used by the AS Sound class
    ///
    /// @param sound_handle  The handle of the event sound to set volume for.
    /// @param volume        A number from 0 to 100 representing a volume
    ///                      level. 100 is full volume and 0 is no volume.
    ///                      The default setting is 100.
    virtual void set_volume(int sound_handle, int volume);
    
    /// Gets the duration in milliseconds of an event sound.
    //
    /// @param sound_handle     The id of the event sound
    /// @return                 the duration of the sound in milliseconds
    virtual unsigned int get_duration(int sound_handle) const;

    /// Gets the playhead position in milliseconds of an event sound.
    //
    /// @param sound_handle     The id of the event sound
    /// @return                 The playhead position of the sound in
    ///                         milliseconds
    virtual unsigned int tell(int sound_handle) const;

    /// Gets the volume for a given sound buffer slot.
    //
    /// Only use for event sounds!
    ///
    /// @param sound_handle     The sound to get the volume for.
    /// @return                 the sound volume level as an integer from
    ///                         0 to 100, where 0 is off and 100 is full
    ///                         volume. The default setting is 100.
    virtual int get_volume(int sound_handle) const;

    ////////////////////////////////////////////////
    /// Streaming sound functions:
    ////////////////////////////////////////////////

    virtual int createStreamingSound(const media::SoundInfo& sinfo);

    /// Remove scheduled requests to play the specified sound buffer slot
    //
    /// @param sound_handle     The sound_handlers id for the sound to be
    ///                         stopped.
    virtual void stopStreamingSound(int handle);

    /// Append data for a streaming sound.
    ///
    /// Gnash's parser calls this to fill up soundstreams data.
    ///
    /// @param data         The sound data to be stored. May not be null.
    ///                     This should be appropriately padded (@see
    ///                     MediaHandler::getInputPaddingBytes()), or a
    ///                     reallocation will take place here.
    /// @param sampleCount  Number of samples in the data
    /// @param seekSamples  Offset of sound to frame data
    /// @param streamId     The soundhandlers id of the sound we want
    ///                     to add data to
    /// @return             a handler for the new block for use in playStream()
    /// @throw              SoundException on error
    virtual StreamBlockId addSoundBlock(std::auto_ptr<SimpleBuffer> data,
               size_t sampleCount, int seekSamples, int streamId);

    /// Returns a SoundInfo object for the sound with the given id.
    //
    /// Note: This should only be used for streaming sounds.
    //
    /// The SoundInfo object is still owned by the soundhandler.
    ///
    /// @param soundHandle  The soundhandlers id of the sound we want some
    ///                     info about.
    /// @return             a pointer to the SoundInfo object for the sound
    ///                     with the given id or 0 if no such sound exists.
    virtual media::SoundInfo* get_sound_info(int handle) const;

    /// Start playback of a streaming sound, if not playing already
    //
    /// TODO: the samples count in the stream sound head (stored in a SoundInfo
    /// object) should drive the timeline while a stream is playing.
    //
    /// TODO: only the number of samples advertised in each stream block should
    /// be played for mp3 sounds, not the complete data! Currently we don't
    /// store this value.
    //
    /// @param handle   Id of the sound buffer slot schedule playback of.
    /// @param blockId    Identifier of the block to start decoding from.
    void playStream(int handle, StreamBlockId blockId);

    /// Get the identifier for the block playing in a specific stream.
    //
    /// This is used by movie_root to check which part of the stream is
    /// actually playing. Streaming sound is used to synchronize the frame
    /// rate. The returned block id can be matched with a specific frame.
    //
    /// @param handle   The handle of the stream sound to query.
    /// @return         The identifier of the stream block that the
    ///                 sound_handler is currently playing (it may be only
    ///                 approximate, as we can't really know what's coming out
    ///                 of the speakers). Returns -1 if the specified stream
    ///                 sound is not playing or doesn't exist.
    int getStreamBlock(int handle) const;

    ////////////////////////////////////////////////
    /// Sound output functions.
    ////////////////////////////////////////////////

    /// Get the volume to apply to mixed output
    //
    /// @return percent value. 100 is full, 0 is none.
    ///         Can be negative or over 100 too.
    int getFinalVolume() const { return _volume; }

    /// Set the volume to apply to mixed output
    //
    /// @param v percent value. 100 is full, 0 is none.
    ///       Can be negative or over 100 too.
    void setFinalVolume(int v) { _volume = v; }

    /// \brief
    /// Discard all sound inputs (slots and aux streamers)
    /// and clear scheduling
    //
    /// Gnash calls this on movie restart.
    ///
    /// The function should stop all sounds and get ready
    /// for a "parse from scratch" operation.
    virtual void reset();
        
    /// Call this to mute audio
    virtual void mute();

    /// Call this to unmute audio
    virtual void unmute();

    /// Returns whether or not sound is muted.
    //
    /// @return true if muted, false if not
    virtual bool is_muted() const;

    /// gnash calls this to pause audio
    virtual void pause() { _paused=true; }

    /// gnash calls this to unpause audio
    virtual void unpause() { _paused=false; }

    /// return true if audio is paused
    bool isPaused() const { return _paused; }

    /// Plug an external InputStream into the mixer
    //
    /// This is called by AS classes NetStream or Sound to attach callback, so
    /// that audio from the classes will be played through the soundhandler.
    ///
    /// This is actually only used by the SDL sound_handler.
    /// It uses these "auxiliary" streamers to fetch decoded audio data
    /// to mix and send to the output channel.
    ///
    /// The "aux streamer" will be called with the 'udata' pointer as
    /// first argument, then will be passed a pointer to buffer of samples
    /// as second argument and the number of samples to fetch as third.
    ///
    /// The callbacks should fill the given buffer if possible, and return
    /// the number of samples actually fetched and an eof flag (last argument).
    /// The callback will be detached if it sets the 'eof' output parameter
    /// to true.
    ///
    /// @param ptr
    ///     The pointer to the callback function
    ///
    /// @param udata
    ///     User data pointer, passed as first argument to the registered
    ///     callback.
    ///
    /// @return an InputStream pointer, for passing to unplugInputStream.
    ///         Callers do not own the InputStream, and should NOT realy
    ///         on it to be point to allocated memory! It is meant to be used
    ///         as an identifier for the newly created mixer channel.
    ///
    /// @todo change to plugInputStream(InputStream* str),
    ///       implement in base class
    ///
    /// @see unplugInputStream
    ///
    virtual InputStream* attach_aux_streamer(aux_streamer_ptr ptr, void* udata);

    /// Unplug an external InputStream from the mixer
    //
    /// This is called by AS classes NetStream or Sound to dettach callback,
    /// so that audio from the classes no longer will be played through the 
    /// soundhandler.
    //
    /// @param id
    ///     The key identifying the auxiliary streamer, as returned
    ///     by attach_aux_streamer.
    ///
    virtual void unplugInputStream(InputStream* id);

    /// Special test-fuction. Reports how many times a sound has been started
    //
    /// @deprecated Use a TestingSoundHanlder !
    ///
    size_t numSoundsStarted() const { return _soundsStarted; }

    /// Special test-fuction. Reports how many times a sound has been stopped
    //
    /// @deprecated Use a TestingSoundHanlder !
    ///
    size_t numSoundsStopped() const { return _soundsStopped; }

    /// Fetch mixed samples
    //
    /// We run through all the plugged InputStreams fetching decoded
    /// audio blocks and mixing them into the given output stream.
    ///
    /// @param to
    ///     The buffer to write mixed samples to.
    ///     Buffer must be big enough to hold nSamples samples.
    ///
    /// @param nSamples
    ///     The amount of samples to fetch.
    ///     NOTE: this number currently refers to "mono" samples
    ///     due to some bad design decision. It is so expected
    ///     that the user fetches 44100 * 2 samples which has to
    ///     be interpreted as series of left,right channel couples.
    ///     TODO: use actual number of samples so that it's expected
    ///           to fetch 44100 per second and let expose a function
    ///           to give interpretation of what comes back (how many
    ///           bytes per channel, which format).
    ///
    virtual void fetchSamples(boost::int16_t* to, unsigned int nSamples);

    /// Mix nSamples from inSamples to outSamples, with given volume
    //
    /// @param outSamples
    ///     The output samples buffer, to mix to.
    ///     Must be big enough to contain nSamples.
    ///     Can be larger.
    ///
    /// @param inSamples
    ///     The input samples buffer, to mix in.
    ///     It is non-const as this method *is* allowed
    ///     to mess with content, for example to apply
    ///     volume.
    ///     TODO: why not applying volume upstream ?!
    ///
    /// @param nSamples
    ///     The amount of samples to mix in.
    ///
    /// @param volume
    ///     The volume to apply to input samples, as a fraction (0..1)
    ///
    /// TODO: this interface says nothing about how many channels we're
    ///       going to mix. It should, to be a sane interface.
    ///       Maybe, a Mixer instance should be created, initialized
    ///       with number of channels, at each fetching.
    ///
    virtual void mix(boost::int16_t* outSamples, boost::int16_t* inSamples,
                unsigned int nSamples, float volume) = 0;

    /// Request to dump audio to the given filename
    //
    /// Every call to this function starts recording
    /// to a new file, closing any existing other dump.
    ///
    void setAudioDump(const std::string& wavefile);

    /// Check if a streaming sound is playing.
    //
    /// @return true if any streaming sound is playing, false if not.
    bool streamingSound() const;

protected:

    sound_handler(media::MediaHandler* m)
        :
        _soundsStarted(0),
        _soundsStopped(0),
        _paused(false),
        _muted(false),
        _volume(100),
        _mediaHandler(m)
    {
    }

    /// Plug an InputStream to the mixer
    //
    /// @param in
    ///     The InputStream to plug, ownership transferred
    ///
    virtual void plugInputStream(std::auto_ptr<InputStream> in);

    /// Unplug all input streams
    virtual void unplugAllInputStreams();

    /// Does the mixer have input streams ?
    bool hasInputStreams() const;

    /// Stop and delete all sounds
    //
    /// This is used only on reset.
    virtual void delete_all_sounds();

private:

    /// Special test-member. Stores count of started sounds.
    size_t _soundsStarted;

    /// Special test-member. Stores count of stopped sounds.
    size_t _soundsStopped;

    /// True if sound is paused
    bool _paused;

    /// True if sound is muted
    bool _muted;

    /// Final output volume
    int _volume;

    typedef std::vector<EmbedSound*> Sounds;

    /// Vector containing event sounds.
    //
    /// Elements of the vector are owned by this class
    Sounds  _sounds;

    typedef std::vector<StreamingSoundData*> StreamingSounds;

    /// Vector containing streaming sounds.
    //
    /// Elements of the vector are owned by this class
    StreamingSounds _streamingSounds;

    /// Stop all instances of an embedded sound
    void stopEmbedSoundInstances(EmbedSound& def);

    /// Stop all instances of an embedded sound
    void stopEmbedSoundInstances(StreamingSoundData& def);

    typedef std::set<InputStream*> InputStreams;

    /// Sound input streams.
    //
    /// Elements owned by this class.
    InputStreams _inputStreams;

    media::MediaHandler* _mediaHandler;

    /// Unplug any completed input stream
    void unplugCompletedInputStreams();

    boost::scoped_ptr<WAVWriter> _wavWriter;

};

// TODO: move to appropriate specific sound handlers

#ifdef SOUND_SDL
/// @throw a SoundException if fails to initialize audio card.
DSOEXPORT sound_handler* create_sound_handler_sdl(media::MediaHandler* m);
#elif defined(SOUND_AHI)
/// @throw a SoundException if fails to initialize audio card.
DSOEXPORT sound_handler* create_sound_handler_aos4(media::MediaHandler* m);
#elif defined(SOUND_MKIT)
/// @throw a SoundException if fails to create node.
DSOEXPORT sound_handler* create_sound_handler_mkit(media::MediaHandler* m);
#endif

} // gnash.sound namespace 
} // namespace gnash

#endif // SOUND_HANDLER_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
