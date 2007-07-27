// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

// 
//

/* $Id: sound_handler.h,v 1.21 2007/07/27 15:16:43 strk Exp $ */

/// \page sound_handler_intro Sound handler introduction
///
/// This page must be written, volunteers ? :)
///

#ifndef SOUND_HANDLER_H
#define SOUND_HANDLER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h" // for DSOEXPORT
#include "tu_types.h"

#include <vector>

namespace gnash {
	class stream;
}

namespace gnash {

/// Sound handler.
//
/// Stores the audio found by the parser and plays on demand.
/// Can also play sound from AS classes NetStream and Sound using callbacks
/// (see attach_aux_streamer and dettach_aux_streamer).
///
/// You may define a subclass of this, and pass an instance to
/// set_sound_handler().
///
class DSOEXPORT sound_handler
{
public:

	// See attach_aux_streamer
	// TODO: change third parameter type to unsigned
	typedef bool (*aux_streamer_ptr)(void *udata, uint8_t *stream, int len);

	/// Used to control volume for event sounds. It basically tells that from
	/// sample X the volume for left out is Y and for right out is Z. Several
	/// envelopes can be assigned to a sound to make a fade out or similar stuff.
	struct sound_envelope
	{
		uint32_t m_mark44;
		uint16_t m_level0;
		uint16_t m_level1;
	};

	/// Format type that the sound can be.
	enum format_type
	{
		FORMAT_RAW = 0,		// Host-endian 8- or 16-bit
		FORMAT_ADPCM = 1,	// decoded in the tag loader and passed through as NATIVE16
		FORMAT_MP3 = 2,
		FORMAT_UNCOMPRESSED = 3,// Little-endian 8- or 16-bit, should be passed through as FORMAT_NATIVE16
		FORMAT_NELLYMOSER_8HZ_MONO = 5,	// According to ffmpeg
		FORMAT_NELLYMOSER = 6,	// Mystery proprietary format; see nellymoser.com
				
		// gnash tries to convert data to this format when possible:
		FORMAT_NATIVE16 = 7	// gnash extension: 16 bits/sample, native-endian
	};
	// If stereo is true, samples are interleaved w/ left sample first.
	
	/// gnash's parser calls this to create sounds to be played later.
	//
	/// @param data
	/// The data to be stored. For soundstream this is NULL.
	///
	/// @param data_bytes
	/// The size of the data to be stored. For soundstream this is 0.
	///
	/// @param sample_count
	/// Number of samples in the data.
	///
	/// @param format
	/// Defines what sound type the data is.
	///
	/// @param sample_rate
	/// The sample rate of the sound.
	///
	/// @param stereo
	/// Defines whether the sound is in stereo.
	///
	/// @return the id given by the soundhandler for later identification.
	///
	virtual int	create_sound(
		void*		data,
		int		data_bytes,
		int		sample_count,
		format_type	format,
		int		sample_rate,	/* one of 5512, 11025, 22050, 44100 */
		bool		stereo
		) = 0;

	/// gnash's parser calls this to fill up soundstreams data
	//
	/// @param data
	/// The sound data to be saved.
	///
	/// @param data_bytes
	/// Size of the data in bytes
	///
	/// @param sample_count
	/// Number of samples in the data
	///
	/// @param handle_id
	/// The soundhandlers id of the sound we want some info about.
	///
	virtual long	fill_stream_data(void* data, int data_bytes, int sample_count, int handle_id) = 0;

	/// Gives info about the format, samplerate and stereo of the sound in question.
	//
	/// @param soundhandle
	/// The soundhandlers id of the sound we want some info about.
	///
	/// @param format
	/// Here the format id will be placed.
	///
	/// @param stereo
	/// Here will be placed whether or not the sound is stereo.
	///
	virtual void get_info(int sound_handle, int* format, bool* stereo) = 0;

	/// gnash calls this when it wants you to play the defined sound.
	//
	/// @param sound_handle
	///	The sound_handlers id for the sound to start playing
	///
	/// @param loop_count
	/// loop_count == 0 means play the sound once (1 means play it twice, etc)
	///
	/// @param secondOffset
	/// When starting soundstreams there sometimes is a offset to make the sound
	/// start at the exact right moment.
	///
	/// @param start
	/// When starting a soundstream from a random frame, this tells where in the
	/// data the decoding should start.
	///
	/// @param envelopes
	/// Some eventsounds have some volume control mechanism called envelopes.
	/// They basically tells that from sample X the volume should be Y.
	///
	virtual void	play_sound(int sound_handle, int loop_count, int secondOffset, long start, const std::vector<sound_envelope>* envelopes) = 0;

	/// stops all sounds currently playing in a SWF file without stopping the playhead.
	/// Sounds set to stream will resume playing as the playhead moves over the frames they are in.
	virtual void	stop_all_sounds() = 0;

	/// Gets the volume for a given sound. Only used by the AS Sound class
	//
	/// @param sound_handle
	///	The sound_handlers id for the sound to be deleted
	///
	/// @return the sound volume level as an integer from 0 to 100,
	/// where 0 is off and 100 is full volume. The default setting is 100.
	virtual int	get_volume(int sound_handle) = 0;
	
	/// Sets the volume for a given sound. Only used by the AS Sound class
	//
	/// @param sound_handle
	///	The sound_handlers id for the sound to be deleted
	///
	/// @param volume
	/// A number from 0 to 100 representing a volume level. 
	/// 100 is full volume and 0 is no volume. The default setting is 100.
	///
	virtual void	set_volume(int sound_handle, int volume) = 0;
		
	/// Stop the specified sound if it's playing.
	/// (Normally a full-featured sound API would take a
	/// handle specifying the *instance* of a playing
	/// sample, but SWF is not expressive that way.)
	//
	/// @param sound_handle
	///	The sound_handlers id for the sound to be deleted
	///
	virtual void	stop_sound(int sound_handle) = 0;
		
	/// gnash calls this when it's done with a particular sound.
	//
	/// @param sound_handle
	///	The sound_handlers id for the sound to be deleted
	///
	virtual void	delete_sound(int sound_handle) = 0;
		
	/// gnash calls this to mute audio
	virtual void	mute() = 0;

	/// gnash calls this to unmute audio
	virtual void	unmute() = 0;

	/// Returns whether or not sound is muted.
	//
	/// @return true if muted, false if not
	///
	virtual bool	is_muted() = 0;

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
	virtual void	attach_aux_streamer(aux_streamer_ptr ptr, void* owner) = 0;

	/// This is called by AS classes NetStream or Sound to dettach callback, so
	/// that audio from the classes no longer will be played through the 
	/// soundhandler.
	//
	/// @param udata
	/// 	The key identifying the auxiliary streamer.
	/// 	WARNING: this need currently be the 'udata' pointer passed to attach_aux_streamer.
	///	TODO: get the aux_streamer_ptr as key !!
	///
	virtual void	detach_aux_streamer(void* udata) = 0;

	sound_handler()
		:
		_soundsStarted(0),
		_soundsStopped(0)
	{}

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
	virtual unsigned int get_position(int sound_handle) = 0;

	/// Special test-fuction. Reports how many times a sound has been started
	size_t numSoundsStarted() const { return _soundsStarted; }

	/// Special test-fuction. Reports how many times a sound has been stopped
	size_t numSoundsStopped() const { return _soundsStopped; }

protected:

	/// Special test-member. Stores count of started sounds.
	size_t _soundsStarted;

	/// Special test-member. Stores count of stopped sounds.
	size_t _soundsStopped;
};

// TODO: move to appropriate specific sound handlers
DSOEXPORT sound_handler*	create_sound_handler_sdl();
DSOEXPORT sound_handler*	create_sound_handler_gst();
DSOEXPORT sound_handler*	create_sound_handler_test();

	

}	// namespace gnash

#endif // SOUND_HANDLER_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
