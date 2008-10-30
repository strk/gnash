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

#include <vector>
#include <memory>
#include <cassert>
#include <cstring>

namespace gnash {
    namespace media {
	    class SoundInfo;
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

/// Sound mixer.
//
/// Stores the audio found by the parser and plays on demand.
/// Can also play sound from AS classes NetStream and Sound using callbacks
/// (see attach_aux_streamer and dettach_aux_streamer).
///
/// You may define a subclass of this, and pass an instance to
/// set_sound_handler().
///
/// NOTE
/// The role of this class is currently somewhat confusing.
/// The main service it should offer is mixing of gnash core sounds.
/// Additionally it currently does on-demand decoding too for
/// some kind of sounds (but not all).
/// Also, it is unspecified what drives consumption of sound.
/// For SDL handler the clock is driven by SDL lib itself.
/// For GST, I have no idea..
/// TODO: clean this up.
///
/// @todo rename to gnash::sound::Mixer
///
class DSOEXPORT sound_handler
{
public:

    /// A %sound input stream
    //
    /// Instance of this class are sounds you can
    /// pull samples from.
    ///
    /// The format of the samples you pull is expected to
    /// be PCM samples as signed 16bit values
    /// in system-endian format.
    ///
    /// It is expected that consecutive samples fetched 
    /// are one for left and one for right stereo channel,
    /// in that order (even indexes for left channel, odd
    /// indexes for right channel).
    ///
    /// Instances of this class would be the input 
    /// for the gnash Mixer (currently sound_handler)
    /// instance.
    ///
    class InputStream {

    public:

        /// Fetch the given amount of samples
        //
        /// @param to
        ///     Output buffer, must be at least nSamples*bytes.
        ///     (or nSamples items sized, being a container of 16bit values).
        ///
        /// @param nSamples
        ///     Number of samples to fetch.
        ///     It is expected that the fetcher fetches samples in multiples
        ///     of 2, being each couple composed by a sample for the left
        ///     channel and a sample for the right channel, in that order.
        ///
        virtual void fetchSamples(boost::int16_t* to, unsigned int nSamples)=0;

        virtual ~InputStream() {}
    };

	/// @see attach_aux_streamer
	/// @todo change third parameter type to unsigned
	/// @todo change second parameter type to boost::uint16_t*
	/// @todo actually, drop this in favor of a SoundInputChannel !
	typedef bool (*aux_streamer_ptr)(void *udata, boost::uint8_t* stream, int len);

	/// Used to control volume for event sounds. It basically tells that from
	/// sample X the volume for left out is Y and for right out is Z. Several
	/// envelopes can be assigned to a sound to make a fade out or similar stuff.
	struct sound_envelope
	{
		boost::uint32_t m_mark44;
		boost::uint16_t m_level0;
		boost::uint16_t m_level1;
	};

	// If stereo is true, samples are interleaved w/ left sample first.
	
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
		) = 0;

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
	/// @param data_bytes
	///     Size of the data in bytes
	///
	/// @param sample_count
	/// 	Number of samples in the data
	///
	/// @param handle_id
	/// 	The soundhandlers id of the sound we want some info about.
	///
	/// @return size of the data buffer before the new data is appended
	///
	virtual long	fill_stream_data(unsigned char* data, unsigned int data_bytes, unsigned int sample_count, int handle_id) = 0;

	/// Returns a pointer to the SoundInfo object for the sound with the given id.
	/// The SoundInfo object is still owned by the soundhandler.
	//
	/// @param soundhandle
	/// The soundhandlers id of the sound we want some info about.
	///
	/// @return a pointer to the SoundInfo object for the sound with the given id.
	///
	virtual media::SoundInfo* get_sound_info(int sound_handle) = 0;

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
	///	class only, not from the actual control tag (StartSound).
	///
	/// @param start
	/// 	When starting a soundstream from a random frame, this tells where in the
	/// 	data the decoding should start, in samples.
	///
	/// @param envelopes
	/// 	Some eventsounds have some volume control mechanism called envelopes.
	/// 	They basically tells that from sample X the volume should be Y.
	///
	/// TODO: add out_point parameter (when to stop playing the sound)
	///
	virtual void	play_sound(int sound_handle, int loop_count, int secondOffset,
					long start, const std::vector<sound_envelope>* envelopes) = 0;

	/// Remove all scheduled request for playback of sound buffer slots
	virtual void	stop_all_sounds() = 0;

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
	virtual int	get_volume(int sound_handle) = 0;

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
	virtual void	set_volume(int sound_handle, int volume) = 0;

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
	virtual void	stop_sound(int sound_handle) = 0;
		
	/// Discard a sound buffer slot
	//
	/// @param sound_handle
	///	The sound_handlers id for the sound to be deleted
	///
	virtual void	delete_sound(int sound_handle) = 0;

	/// \brief
	/// Discard all sound inputs (slots and aux streamers)
	/// and clear scheduling
	//
	/// Gnash calls this on movie restart.
	///
	/// The function should stop all sounds and get ready
	/// for a "parse from scratch" operation.
	///
	virtual void reset() = 0;
		
	/// gnash calls this to mute audio
	virtual void	mute() = 0;

	/// gnash calls this to unmute audio
	virtual void	unmute() = 0;

	/// Returns whether or not sound is muted.
	//
	/// @return true if muted, false if not
	///
	virtual bool	is_muted() = 0;

	/// gnash calls this to pause audio
	virtual void pause() { _paused=true; }

	/// gnash calls this to unpause audio
	virtual void unpause() { _paused=false; }

	/// gnash calls this to unpause audio
	bool isPaused() const { return _paused; }

	/// This is called by AS classes NetStream or Sound to attach callback, so
	/// that audio from the classes will be played through the soundhandler.
	//
	/// This is actually only used by the SDL sound_handler. It uses these "auxiliary"
	/// streamers to fetch decoded audio data to mix and send to the output channel.
	///
	/// The "aux streamer" will be called with the 'udata' pointer as first argument,
	/// then will be passed a buffer pointer as second argument and it's length
	/// as third. The callbacks should fill the given buffer if possible.
	/// The callback should return true if wants to remain attached, false if wants
	/// to be detached. 
	///
	/// @param ptr
	///	The pointer to the callback function
	///
	/// @param udata
	///	User data pointer, passed as first argument to the registered callback.
	/// 	WARNING: this is currently also used to *identify* the callback for later
	///	removal, see detach_aux_streamer. TODO: stop using the data pointer for 
	///	identification purposes and use the callback pointer directly instead.
	///
	virtual void	attach_aux_streamer(aux_streamer_ptr /*ptr*/,
            void* /*owner*/){} // FIXME

	/// This is called by AS classes NetStream or Sound to dettach callback, so
	/// that audio from the classes no longer will be played through the 
	/// soundhandler.
	//
	/// @param udata
	/// 	The key identifying the auxiliary streamer.
	/// 	WARNING: this need currently be the 'udata' pointer passed to attach_aux_streamer.
	///	TODO: get the aux_streamer_ptr as key !!
	///
	virtual void	detach_aux_streamer(void* /*udata*/) {} // FIXME

	virtual ~sound_handler() {};
	
	/// \brief
	/// Gets the duration in milliseconds of an event sound connected
	/// to an AS Sound obejct.
	//
	/// @param sound_handle
	/// The id of the event sound
	///
	/// @return the duration of the sound in milliseconds
	virtual unsigned int get_duration(int sound_handle) = 0;

	/// \brief
	/// Gets the playhead position in milliseconds of an event sound connected
	/// to an AS Sound obejct.
	//
	/// @param sound_handle
	/// The id of the event sound
	///
	/// @return the duration of the sound in milliseconds
	virtual unsigned int tell(int sound_handle) = 0;

	/// Special test-fuction. Reports how many times a sound has been started
	size_t numSoundsStarted() const { return _soundsStarted; }

	/// Special test-fuction. Reports how many times a sound has been stopped
	size_t numSoundsStopped() const { return _soundsStopped; }

protected:


	sound_handler()
		:
		_soundsStarted(0),
		_soundsStopped(0),
		_paused(false),
		_mediaHandler(media::MediaHandler::get()),
		_volume(100)
	{
		assert(_mediaHandler); // for now, we rely on this being always available
	}

	/// Special test-member. Stores count of started sounds.
	size_t _soundsStarted;

	/// Special test-member. Stores count of stopped sounds.
	size_t _soundsStopped;

	/// True if sound is paused
	bool _paused;

	/// The registered MediaHandler at construction time
	media::MediaHandler* _mediaHandler;

private:

	/// Final output volume
	int _volume;

};

// TODO: move to appropriate specific sound handlers
DSOEXPORT sound_handler*	create_sound_handler_sdl();
DSOEXPORT sound_handler*	create_sound_handler_sdl(const std::string& wave_file);
DSOEXPORT sound_handler*	create_sound_handler_gst();
	

} // gnash.sound namespace 
} // namespace gnash

#endif // SOUND_HANDLER_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
