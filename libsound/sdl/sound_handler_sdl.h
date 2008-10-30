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
#include "AudioDecoder.h"
#include "SimpleBuffer.h"

#include "log.h"

#include <vector>
#include <map> // for composition

#include <iconv.h>
#include <SDL_audio.h>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>


namespace gnash {
namespace sound {

class EmbeddedSoundInstance;

/// Used to hold the sounddata when doing on-demand-decoding
class EmbeddedSound
{
	/// The undecoded data
	std::auto_ptr<SimpleBuffer> _buf;

    void ensureBufferPadding();

public:

    /// Construct a sound with given data, info and volume.
    //
    /// @param data The encoded sound data. May be the NULL pointer for streaming sounds,
    ///     in which case data will be appended later using ::append()
    ///
    /// @param info encoding info
    ///
    /// @pararm nVolume initial volume (0..100). Optional, defaults to 100.
    ///
	EmbeddedSound(std::auto_ptr<SimpleBuffer> data, std::auto_ptr<media::SoundInfo> info, int nVolume=100);

	~EmbeddedSound();

	/// Object holding information about the sound
	std::auto_ptr<media::SoundInfo> soundinfo;

	typedef std::map<boost::uint32_t,boost::uint32_t> FrameSizeMap;

	/// Maps frame sizes to start-of-frame offsets
	FrameSizeMap m_frames_size;

	/// Append size bytes to this sound
	//
	/// @param data
	///	Data bytes, allocated with new[]. Ownership transferred.
	///
	/// @param size
	///	Size of the 'data' buffer.
	///
	void append(boost::uint8_t* data, unsigned int size);

	/// Return size of the data buffer
	size_t size() const 
	{
		return _buf->size();
	}

	/// Return a pointer to the underlying buffer
	const boost::uint8_t* data() const {
		return _buf->data();
	}

	/// Return a pointer to the underlying buffer
	boost::uint8_t* data() {
		return _buf->data();
	}

	/// Return a pointer to an offset in the underlying buffer
	//
	/// @param pos The offset value.
	/// 	An assertion will fail if pos > size()
	///
	const boost::uint8_t* data(size_t pos) const {
		assert(pos < _buf->size());
		return _buf->data()+pos;
	}

	/// Return a pointer to an offset in the underlying buffer
	//
	/// @param pos The offset value.
	/// 	An assertion will fail if pos > size()
	///
	boost::uint8_t* data(size_t pos) {
		assert(pos < _buf->size());
		return _buf->data()+pos;
	}

	/// Volume for AS-sounds, range: 0-100.
	/// It's the SWF range that is represented here.
	int volume;

	/// Vector containing the active instances of this sounds being played
	//
	/// NOTE: This class *owns* all active sounds
	///
	typedef std::list<EmbeddedSoundInstance*> ActiveSounds;

	ActiveSounds _soundInstances;

	/// Drop all active sounds
	void clearActiveSounds();

	/// Drop an active sound (by iterator)
	//
	/// @return iterator after the one being erased
	///
	ActiveSounds::iterator eraseActiveSound(ActiveSounds::iterator i);
};

/// Playing instance of a defined sound (EmbeddedSound)
//
/// This class contains a pointer to the EmbeddedSound used for playing
/// and an optional SimpleBuffer to use when decoding is needed.
///
/// When the SimpleBuffer is NULL we'll play the EmbeddedSound bytes directly
/// (we assume they are decoded already)
///
class EmbeddedSoundInstance
{
public:
	EmbeddedSoundInstance(const EmbeddedSound& soundData)
		:
		decoder(0),
		decodingPosition(0),
		playbackPosition(0),
		loopCount(0),
		offset(0),
		current_env(0),
		samples_played(0),
		_encodedData(soundData)
	{}

	~EmbeddedSoundInstance()
	{
		deleteDecodedData();
	}

	/// The decoder object used to convert the data into the playable format
	std::auto_ptr<media::AudioDecoder> decoder;

	/// Current decoding position in the encoded stream
	unsigned long decodingPosition;

	/// Current playback position in the decoded stream
	unsigned long playbackPosition;

	/// Numbers of loops: -1 means loop forever, 0 means play once.
	/// For every loop completed, it is decremented.
	long loopCount;

	/// Offset to make playback start in-sync, only used with mp3 streams.
	unsigned int offset;

	/// Sound envelopes for the current sound, which determine the volume level
	/// from a given position. Only used with event sounds.
	const std::vector<sound_handler::sound_envelope>* envelopes;

	/// Index of current envelope.
	boost::uint32_t current_env;

	/// Number of samples played so far.
	unsigned long samples_played;

	/// Get the sound definition this object is an instance of
	const EmbeddedSound& getSoundData() {    
        return _encodedData;
    }

    // See dox in sound_handler.h (InputStream)
    unsigned int fetchSamples(boost::int16_t* to, unsigned int nSamples);

	/// Release resources associated with decoded data, if any.
	//
	/// After this call, the EmbeddedSoundInstance will have no decoded data
	/// buffer, thus any pointer to the decoded data will be fetched
	/// from the undecoded one.
	///
	void deleteDecodedData();

	/// Append size bytes to this raw data 
	//
	/// @param data
	///	Data bytes, allocated with new[]. Ownership transferred.
	///
	/// @param size
	///	Size of the 'data' buffer.
	///
	void appendDecodedData(boost::uint8_t* data, unsigned int size)
	{
		if ( ! _decodedData.get() )
		{
			_decodedData.reset( new SimpleBuffer );
		}
  
		_decodedData->append(data, size);
		delete [] data; // ownership transferred...
	}
  
	/// Set decoded data
	//
	/// @param data
	///	Data bytes, allocated with new[]. Ownership transferred.
	///
	/// @param size
	///	Size of the 'data' buffer.
	///
	void setDecodedData(boost::uint8_t* data, unsigned int size)
	{
		if ( ! _decodedData.get() )
		{
			_decodedData.reset( new SimpleBuffer() );
		}

		_decodedData->resize(0); // shouldn't release memory
		_decodedData->append(data, size);
		delete [] data; // ownership transferred...
	}

	size_t encodedDataSize() const
	{
		return _encodedData.size();
	}

    /// Apply envelope-volume adjustments
    //
    /// @param length Amount of bytes to envelope-adjust
    ///        @todo take samples
    ///
    /// @todo make private, have it called from fetchSamples
    ///
    /// @todo document where does the envelope application 
    ///       start from!
    ///
    void useEnvelopes(unsigned int length);

	/// Returns the data pointer in the encoded datastream
	/// for the given position. Boundaries are checked.
    //
    /// @todo make private
    ///
	const boost::uint8_t* getEncodedData(unsigned long int pos);

    /// Return number of already-decoded samples available
    /// from playback position on
    unsigned int decodedSamplesAhead()
    {
        unsigned int bytesAhead = decodedDataSize() - playbackPosition;
        assert(!(bytesAhead%2));

        unsigned int samplesAhead = bytesAhead/2;
        return samplesAhead;
    }

    /// Return true if there's nothing more to decode
    bool decodingCompleted() const
    {
        // example: 10 bytes of encoded data, decodingPosition 8 : more to decode
        // example: 10 bytes of encoded data, decodingPosition 10 : nothing more to decode

        return ( decodingPosition >= encodedDataSize() );
    }
  
private:

    /// Return full size of the decoded data buffer
	size_t decodedDataSize() const
	{
		if ( _decodedData.get() )
		{
			return _decodedData->size();
		}
		else return 0;
	}

    /// \brief
	/// Returns the data pointer in the decoded datastream
	/// for the given byte-offset.
    //
    /// Boundaries are checked.
    ///
    /// @param pos offsets in bytes. This should usually be
    ///        a multiple of two, since decoded data is
    ///        composed of signed 16bit PCM samples..
    ///
	boost::int16_t* getDecodedData(unsigned long int pos);

	/// The encoded data
	const EmbeddedSound& _encodedData;

	/// The decoded buffer
	//
	/// If NULL, the _encodedData will be considered
	/// decoded instead
	///
	std::auto_ptr<SimpleBuffer> _decodedData;

};

// This is here as it needs definition of EmbeddedSoundInstance
EmbeddedSound::~EmbeddedSound()
{
	clearActiveSounds();
}

// Use SDL and ffmpeg/mad/nothing to handle sounds.
class SDL_sound_handler : public sound_handler
{
public:

	typedef std::vector<EmbeddedSound*> Sounds;

private:
	/// AS classes (NetStream, Sound) audio callbacks
	typedef std::map< void* /* owner */, aux_streamer_ptr /* callback */> CallbacksMap;
	CallbacksMap m_aux_streamer;

	/// Vector containing all sounds.
	//
	/// Elements of the vector are owned by this class
	///
	Sounds	_sounds;

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

	void mixSoundData(EmbeddedSound& sounddata, Uint8* stream, unsigned int buffer_length);

	/// @param sound
	///     The active sound to mix in
	//
	/// @param mixTo
	///     The buffer to mix sound into
	///
	/// @param mixLen
	///     The amount of bytes to mix in
	///
	void mixActiveSound(EmbeddedSoundInstance& sound, Uint8* mixTo, unsigned int mixLen);

	/// Callback invoked by the SDL audio thread.
	//
	/// This is basically a wrapper around fetchSamples
	///
	/// @param udata
	///	    User data pointer (SDL_sound_handler instance in our case).
	///	    We'll lock the SDL_sound_handler::_mutex during operations.
	///
	/// @param stream
	/// 	The output stream/buffer to fill
	///
	/// @param buffer_length_in
	///	    Length of the buffer.
	///	    If zero or negative we log an error and return
	///	    (negative is probably an SDL bug, zero dunno yet).
	///
	static void sdl_audio_callback (void *udata, Uint8 *stream, int buffer_length_in);

public:

	SDL_sound_handler();

	SDL_sound_handler(const std::string& wave_file);

	~SDL_sound_handler();

	// See dox in sound_handler.h
	virtual int	create_sound(std::auto_ptr<SimpleBuffer> data, std::auto_ptr<media::SoundInfo> sinfo);

	// see dox in sound_handler.h
	virtual long	fill_stream_data(unsigned char* data, unsigned int data_bytes,
					 unsigned int sample_count, int handle_id);

	// See dox in sound_handler.h
	virtual void	play_sound(int sound_handle, int loopCount, int offset,
				   long start_position, const std::vector<sound_envelope>* envelopes);

	// See dox in sound_handler.h
	virtual void	stop_sound(int sound_handle);

	// See dox in sound_handler.h
	virtual void	delete_sound(int sound_handle);

	// See dox in sound_handler.h
	virtual void reset();

	// See dox in sound_handler.h
	virtual void	stop_all_sounds();

	// See dox in sound_handler.h
	virtual int	get_volume(int sound_handle);

	// See dox in sound_handler.h
	virtual void	set_volume(int sound_handle, int volume);
		
	// See dox in sound_handler.h
	virtual media::SoundInfo* get_sound_info(int sound_handle);

	// See dox in sound_handler.h
	virtual void	mute();

	// See dox in sound_handler.h
	virtual void	unmute();

	// See dox in sound_handler.h
	virtual bool	is_muted();

	// See dox in sound_handler.h
	virtual unsigned int get_duration(int sound_handle);

	// See dox in sound_handler.h
	virtual unsigned int tell(int sound_handle);
	
	// See dox in sound_handler.h
	virtual void	attach_aux_streamer(aux_streamer_ptr ptr, void* owner);

	// See dox in sound_handler.h
	virtual void	detach_aux_streamer(void* owner);

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
