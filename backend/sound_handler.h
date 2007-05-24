// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// 
//

/* $Id: sound_handler.h,v 1.9 2007/05/24 08:48:02 strk Exp $ */

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

//
// Sound callback handler.
//
	
// You may define a subclass of this, and pass an instance to
// set_sound_handler().
class DSOEXPORT sound_handler
{
public:

	typedef bool (*aux_streamer_ptr)(void *udata, uint8_t *stream, int len);

	struct sound_envelope
	{
		uint32_t m_mark44;
		uint16_t m_level0;
		uint16_t m_level1;
	};

	enum format_type
	{
		FORMAT_RAW = 0,		// Host-endian 8- or 16-bit
		FORMAT_ADPCM = 1,	// decoded in the tag loader and passed through as NATIVE16
		FORMAT_MP3 = 2,
		FORMAT_UNCOMPRESSED = 3,// Little-endian 8- or 16-bit, should be passed through as FORMAT_NATIVE16
		FORMAT_NELLYMOSER = 6,	// Mystery proprietary format; see nellymoser.com
				
		// gnash tries to convert data to this format when possible:
		FORMAT_NATIVE16 = 7	// gnash extension: 16 bits/sample, native-endian
	};
	// If stereo is true, samples are interleaved w/ left sample first.
	
	// gnash calls at load-time with sound data, to be
	// played later.  You should create a sample with the
	// data, and return a handle that can be used to play
	// it later.  If the data is in a format you can't
	// deal with, then you can return 0 (for example), and
	// then ignore 0's in play_sound() and delete_sound().
	//
	// Assign handles however you like.
	virtual int	create_sound(
		void*		data,
		int		data_bytes,
		int		sample_count,
		format_type	format,
		int		sample_rate,	/* one of 5512, 11025, 22050, 44100 */
		bool		stereo
		) = 0;

	// gnash calls this to fill up soundstreams data
	virtual long	fill_stream_data(void* data, int data_bytes, int sample_count, int handle_id) = 0;

	//	Gives info about the format, samplerate and stereo of the sound in question;
	virtual void get_info(int sound_handle, int* format, bool* stereo) = 0;

	// gnash calls this when it wants you to play the defined sound.
	// loop_count == 0 means play the sound once (1 means play it twice, etc)
	virtual void	play_sound(int sound_handle, int loop_count, int secondOffset, long start, const std::vector<sound_envelope>* envelopes) = 0;

	//	stops all sounds currently playing in a SWF file without stopping the playhead.
	//	Sounds set to stream will resume playing as the playhead moves over the frames they are in.
	virtual void	stop_all_sounds() = 0;

	//	returns the sound volume level as an integer from 0 to 100,
	//	where 0 is off and 100 is full volume. The default setting is 100.
	virtual int	get_volume(int sound_handle) = 0;
	
	//	A number from 0 to 100 representing a volume level. 
	//	100 is full volume and 0 is no volume. The default setting is 100.
	virtual void	set_volume(int sound_handle, int volume) = 0;
		
	// Stop the specified sound if it's playing.
	// (Normally a full-featured sound API would take a
	// handle specifying the *instance* of a playing
	// sample, but SWF is not expressive that way.)
	virtual void	stop_sound(int sound_handle) = 0;
		
	// gnash calls this when it's done with a particular sound.
	virtual void	delete_sound(int sound_handle) = 0;
		
	// gnash calls this to mute audio
	virtual void	mute() = 0;

	// gnash calls this to unmute audio
	virtual void	unmute() = 0;

	//// @return Whether or not sound is muted.
	virtual bool	is_muted() = 0;

	virtual void	attach_aux_streamer(aux_streamer_ptr ptr, void* owner) = 0;
	virtual void	detach_aux_streamer(void* owner) = 0;

	// Converts audio input data to required sample rate/stereo.
	static void	convert_raw_data(int16_t** adjusted_data,
			  int* adjusted_size, void* data, int sample_count,
			  int sample_size, int sample_rate, bool stereo,
			  int m_sample_rate, bool m_stereo);

	virtual ~sound_handler() {};
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
