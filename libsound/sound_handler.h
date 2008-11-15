// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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


/// \page sound_handler_intro Sound handler introduction
///
/// The implementation of this class *must* be thread safe!
///

#ifndef SOUND_HANDLER_H
#define SOUND_HANDLER_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "dsodefs.h" // for DSOEXPORT
#include "MediaHandler.h" // for inlined ctor
#include "SoundEnvelope.h" // for SoundEnvelopes typedef
#include "AuxStream.h" // for aux_stramer_ptr typedef

#include <vector>
#include <memory>
#include <cassert>
#include <cstring>
#include <set> // for composition

namespace gnash {
    namespace media {
	    class SoundInfo;
    }
    namespace sound {
        class EmbedSound;
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
class DSOEXPORT sound_handler
{
public:

	/// Create a sound buffer slot, for on-demand playback.
	//
	/// @param data
	/// 	The data to be stored. For soundstream this is NULL.
	///	    The data is in encoded format, with format specified
	///	    with the sinfo parameter, this is to allow on-demand
	///	    decoding (if the sound is never played, it's never decoded).
	///
	/// @param sinfo
	/// 	A SoundInfo object contained in an auto_ptr, which contains info about samplerate,
	/// 	samplecount, stereo and more. The SoundObject must be not-NULL!
	///
	/// @return the id given by the soundhandler for later identification.
	///
	virtual int	create_sound(
		std::auto_ptr<SimpleBuffer> data,
		std::auto_ptr<media::SoundInfo> sinfo
		);

	/// Append data to an existing sound buffer slot.
	//
	///
	/// Gnash's parser calls this to fill up soundstreams data.
	/// TODO: the current code uses memory reallocation to grow the sound,
	/// which is suboptimal; instead, we should maintain sound sources as a 
	/// list of buffers, to avoid reallocations.
	///
	/// @param data
	/// 	The sound data to be saved, allocated by new[]. Ownership is transferred.
	///	TODO: define a class for containing both data and data_bytes ? or use vector ?
	///
	/// @param dataBytes
	///     Size of the data in bytes
	///
	/// @param sampleCount
	/// 	Number of samples in the data
	///
	/// @param handleId
	/// 	The soundhandlers id of the sound we want some info about.
	///
	/// @return size of the data buffer before the new data is appended
	///
	virtual long fill_stream_data(unsigned char* data, unsigned int dataBytes,
                                  unsigned int sampleCount, int handleId);

	/// Returns a pointer to the SoundInfo object for the sound with the given id.
    //
	/// The SoundInfo object is still owned by the soundhandler.
	///
	/// @param soundHandle
	/// The soundhandlers id of the sound we want some info about.
	///
	/// @return a pointer to the SoundInfo object for the sound with the given id.
	///
	virtual media::SoundInfo* get_sound_info(int soundHandle);

	/// Schedule playing of a sound buffer slot
	//
	/// All scheduled sounds will be played on next output flush.
	///
	/// @param sound_handle
	///	Id of the sound buffer slot schedule playback of.
	///
	/// @param loop_count
	/// 	loop_count == 0 means play the sound once (1 means play it twice, etc)
	///
	/// @param secondOffset
	/// 	When starting event sounds there sometimes is a offset to make the sound
	/// 	start at the exact right moment. Gnash supports this troough 'Sound' AS
	///     class only, not from the actual control tag (StartSound). Units given in 
    ///     seconds.
	///
	/// @param start
	/// 	When starting a soundstream from a random frame, this tells where in the
	/// 	data the decoding should start, in bytes.
	///     If non-zero, the sound will only start when no other instances of it
	///     are already playing.
	///     @todo use unsigned
	///
	/// @param envelopes
	/// 	Some eventsounds have some volume control mechanism called envelopes.
	/// 	They basically tells that from sample X the volume should be Y.
	///
	/// TODO: add out_point parameter (when to stop playing the sound)
	///
	virtual void	play_sound(int sound_handle, int loop_count, int secondOffset,
					long start, const SoundEnvelopes* envelopes);

	/// Remove all scheduled request for playback of sound buffer slots
	virtual void	stop_all_sounds();

	/// Gets the volume for a given sound buffer slot.
	//
	/// Only used by the AS Sound class
	///
	/// @param sound_handle
	///	The sound_handlers id for the sound to be deleted
	///
	/// @return the sound volume level as an integer from 0 to 100,
	/// 	where 0 is off and 100 is full volume. The default setting is 100.
	///
	virtual int	get_volume(int sound_handle);

	/// Get the volume to apply to mixed output
	//
	/// @return percent value. 100 is full, 0 is none.
	///        Can be negative or over 100 too.
	///
	int getFinalVolume() { return _volume; }
	
	/// Sets the volume for a given sound buffer slot.
	//
	/// Only used by the AS Sound class
	///
	/// @param sound_handle
	///	The sound_handlers id for the sound to be deleted
	///
	/// @param volume
	/// 	A number from 0 to 100 representing a volume level. 
	/// 	100 is full volume and 0 is no volume.
	///	The default setting is 100.
	///
	virtual void set_volume(int sound_handle, int volume);

	/// Set the volume to apply to mixed output
	//
	/// @param v percent value. 100 is full, 0 is none.
	///       Can be negative or over 100 too.
	///
	void setFinalVolume(int v) { _volume=v; }
		
	/// Remove scheduled requests to play the specified sound buffer slot
	//
	/// Stop the specified sound if it's playing.
	/// (Normally a full-featured sound API would take a
	/// handle specifying the *instance* of a playing
	/// sample, but SWF is not expressive that way.)
	//
	/// @param sound_handle
	///	The sound_handlers id for the sound to be deleted
	///
	virtual void	stop_sound(int sound_handle);

	/// Discard a sound buffer slot
	//
	/// @param sound_handle
	///	The sound_handlers id for the sound to be deleted
	///
	virtual void delete_sound(int sound_handle);

    // Stop and delete all sounds
    virtual void delete_all_sounds();

	/// \brief
	/// Discard all sound inputs (slots and aux streamers)
	/// and clear scheduling
	//
	/// Gnash calls this on movie restart.
	///
	/// The function should stop all sounds and get ready
	/// for a "parse from scratch" operation.
	///
	virtual void reset();
		
	/// Call this to mute audio
	virtual void mute();

	/// Call this to unmute audio
	virtual void unmute();

	/// Returns whether or not sound is muted.
	//
	/// @return true if muted, false if not
	///
	virtual bool is_muted() const;

	/// gnash calls this to pause audio
	virtual void pause() { _paused=true; }

	/// gnash calls this to unpause audio
	virtual void unpause() { _paused=false; }

	/// gnash calls this to unpause audio
	bool isPaused() const { return _paused; }

	/// Plug an external InputStream into the mixer
	//
	/// This is called by AS classes NetStream or Sound to attach callback, so
	/// that audio from the classes will be played through the soundhandler.
	///
	/// This is actually only used by the SDL sound_handler. It uses these "auxiliary"
	/// streamers to fetch decoded audio data to mix and send to the output channel.
	///
	/// The "aux streamer" will be called with the 'udata' pointer as first argument,
	/// then will be passed a pointer to buffer of samples as second argument and the 
	///  number of samples to fetch as third.
	///
	/// The callbacks should fill the given buffer if possible, and return the number
	/// of samples actually fetched and an eof flag (last argument).
	/// The callback will be detached if it sets the 'eof' output parameter to true.
	///
	/// @param ptr
	///	    The pointer to the callback function
	///
	/// @param udata
	///	    User data pointer, passed as first argument to the registered callback.
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
	virtual InputStream* attach_aux_streamer(aux_streamer_ptr ptr, void* udata)
	{ ptr=ptr; /*unused*/ udata=udata; /*unused*/ return 0; } 

	/// Unplug an external InputStream from the mixer
	//
	/// This is called by AS classes NetStream or Sound to dettach callback, so
	/// that audio from the classes no longer will be played through the 
	/// soundhandler.
	//
	/// @param id
	/// 	The key identifying the auxiliary streamer, as returned
    ///     by attach_aux_streamer.
	///
	virtual void unplugInputStream(InputStream* id);

	virtual ~sound_handler() {};
	
	/// \brief
	/// Gets the duration in milliseconds of an event sound connected
	/// to an AS Sound obejct.
	//
	/// @param sound_handle
	/// The id of the event sound
	///
	/// @return the duration of the sound in milliseconds
    ///
	virtual unsigned int get_duration(int sound_handle);

	/// \brief
	/// Gets the playhead position in milliseconds of an event sound connected
	/// to an AS Sound obejct.
	//
	/// @param sound_handle
	/// The id of the event sound
	///
	/// @return the duration of the sound in milliseconds
    ///
	virtual unsigned int tell(int sound_handle);

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
    /// We run trough all the plugged InputStreams fetching decoded
    /// audio blocks and mixing them into the given output stream.
    ///
    /// @param to
    ///     The buffer to write mixed samples to.
    ///     Buffer must be big enough to hold nSamples samples.
    ///
    /// @param nSamples
    ///     The amount of samples to fetch.
    ///
    virtual void fetchSamples(boost::int16_t* to, unsigned int nSamples);

protected:


	sound_handler()
		:
		_soundsStarted(0),
		_soundsStopped(0),
		_paused(false),
		_muted(false),
		_mediaHandler(media::MediaHandler::get()),
		_volume(100),
        _sounds(),
        _inputStreams()
	{
		assert(_mediaHandler); // for now, we rely on this being always available
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
                unsigned int nSamples, float volume);

private:

	/// Special test-member. Stores count of started sounds.
	size_t _soundsStarted;

	/// Special test-member. Stores count of stopped sounds.
	size_t _soundsStopped;

	/// True if sound is paused
	bool _paused;

    /// True if sound is muted
    bool _muted;

	/// The registered MediaHandler at construction time
	media::MediaHandler* _mediaHandler;

	/// Final output volume
	int _volume;

    typedef std::vector<EmbedSound*> Sounds;

    /// Vector containing all sounds.
    //
    /// Elements of the vector are owned by this class
    ///
    Sounds  _sounds;

    /// Stop all instances of an embedded sound
    void stopEmbedSoundInstances(EmbedSound& def);

    typedef std::set< InputStream* > InputStreams;

    /// Sound input streams.
    //
    /// Elements owned by this class.
    ///
    InputStreams _inputStreams;

    /// Unplug any completed input stream
    void unplugCompletedInputStreams();

};

// TODO: move to appropriate specific sound handlers

/// @throw a SoundException if fails to initialize audio card.
DSOEXPORT sound_handler*	create_sound_handler_sdl();
/// @throw a SoundException if fails to initialize audio card.
DSOEXPORT sound_handler*	create_sound_handler_sdl(const std::string& wave_file);
	

} // gnash.sound namespace 
} // namespace gnash

#endif // SOUND_HANDLER_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
