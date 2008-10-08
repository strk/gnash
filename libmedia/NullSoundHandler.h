// NullSoundHandler - fake sound handler, for testing gnash
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


#ifndef NULL_SOUND_HANDLER_H
#define NULL_SOUND_HANDLER_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "sound_handler.h" // for inheritance
#include "dsodefs.h" // for DSOEXPORT

#include <vector>
#include <memory>
#include <cassert>
#include <cstring>

namespace gnash {

namespace media {

/// Null Sound handler, for testing 
class DSOEXPORT NullSoundHandler : public sound_handler
{
public:

	// See dox in sound_handler.h 
	virtual int	create_sound(
		std::auto_ptr<SimpleBuffer> /*data*/,
		std::auto_ptr<SoundInfo> /*sinfo*/
		)
	{
		return 0;
	}

	// See dox in sound_handler.h 
	// implement?
	virtual long	fill_stream_data(unsigned char* /*data*/, unsigned int /*data_bytes*/, unsigned int /*sample_count*/, int /*handle_id*/)
	{
		return 0;
	}

	// See dox in sound_handler.h 
	virtual SoundInfo* get_sound_info(int /*sound_handle*/) { return 0; }

	// See dox in sound_handler.h 
	virtual void play_sound(int /*sound_handle*/, int /*loop_count*/, int /*secondOffset*/, long /*start*/,
		const std::vector<sound_envelope>* /*envelopes*/)
	{
	}

	// See dox in sound_handler.h 
	virtual void	stop_all_sounds() {}

	// See dox in sound_handler.h 
	// TODO: implement here
	virtual int	get_volume(int /*sound_handle*/) { return 0; }

	// See dox in sound_handler.h 
	// TODO: implement here
	virtual void	set_volume(int /*sound_handle*/, int /*volume*/) {}

	// See dox in sound_handler.h 
	virtual void	stop_sound(int /*sound_handle*/) {}
		
	// See dox in sound_handler.h 
	virtual void	delete_sound(int /*sound_handle*/) {}

	// See dox in sound_handler.h 
	virtual void reset() {}
		
	// See dox in sound_handler.h (why is this virtual anyway ?)
	virtual void	mute() {}

	// See dox in sound_handler.h (why is this virtual anyway ?)
	virtual void	unmute() {}

	// See dox in sound_handler.h (why is this virtual anyway ?)
	virtual bool	is_muted() { return false; }

#ifdef USE_FFMPEG
	// See dox in sound_handler.h
	virtual void	attach_aux_streamer(aux_streamer_ptr /*ptr*/, void* /*owner*/) {}

	// See dox in sound_handler.h
	virtual void	detach_aux_streamer(void* /*udata*/) {}
#endif

	// See dox in sound_handler.h
	virtual unsigned int get_duration(int /*sound_handle*/) { return 0; }

	// See dox in sound_handler.h
	virtual unsigned int tell(int /*sound_handle*/) { return 0; }

};
	
} // gnash.media namespace 
}	// namespace gnash

#endif // NULL_SOUND_HANDLER_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
