// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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
//

#ifndef GNASH_SWF_STARTSOUND_TAG_H
#define GNASH_SWF_STARTSOUND_TAG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ControlTag.h" // for inheritance
#include "sound_handler.h" // for sound_envelope (composition)
#include "swf.h" // for tag_type definition

#include <vector> // for composition
#include <boost/cstdint.hpp> // for uint16_t and friends
 

// Forward declarations
namespace gnash {
	class stream;
	class movie_definition;
	class sound_sample;
}

namespace gnash {
namespace SWF {

/// SWF Tag StartSound (15) 
class StartSoundTag : public ControlTag
{
	uint16_t	m_handler_id;
	int	m_loop_count;
	bool	m_stop_playback;
	std::vector<sound_handler::sound_envelope> m_envelopes;

	/// Envelopes for the current sound instance
	//
	/// TODO: define ownership
	///
	uint32_t* envelopes;

	/// \brief
	/// Initialize this StartSoundTag from the stream  
	//
	/// The stream is assumed to be positioned right after the
	/// sound_id field of the tag structure.
	///
	void read(stream* in, int tag_type, movie_definition* m);


	/// Create a StartSoundTag for starting the given sound sample
	//
	/// @param sound_handler_id
	/// Sound sample identifier as provided by sound_handler (sic!)
	///
	StartSoundTag(int sound_id)
		:
		m_handler_id(sound_id),
		m_loop_count(0),
		m_stop_playback(false)
	{
	}

public:

	void	execute(sprite_instance* m) const;

	/// Load a SWF::STARTSOUND tag.
	static void loader(stream* in, tag_type tag, movie_definition* m);

};


} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_STARTSOUND_TAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
