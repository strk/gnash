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

#ifndef GNASH_SOUND_H
#define GNASH_SOUND_H


#include "resource.h" // for sound_sample inheritance
#include "sound_handler.h"
#include "ControlTag.h" // for sound tags inheritance

// Forward declarations
namespace gnash {
	class movie_definition;
	class stream;
}


namespace gnash {

/// TODO: document this
//
/// QUESTION: why is this a resource ?
///           does it really need to be ref-counted ?
///
class sound_sample: public resource
{
public:
	int	m_sound_handler_id;

	sound_sample(int id)
		:
		m_sound_handler_id(id)
	{
	}

	~sound_sample();

	sound_sample* cast_to_sound_sample() { return this; }
};

/// SWF Tag SoundStreamBlock (19) 
//
/// TODO: move in it's own SoundStreamBlockTag files...
///
class start_stream_sound_tag : public ControlTag
{
public:
	uint16_t	m_handler_id;
	long		m_start;
	int		latency;

	start_stream_sound_tag()
		:
		m_handler_id(0),
		m_start(0),
		latency(0)
	{
	}


	/// \brief
	/// Initialize this StartSound tag
	/// from the stream & given sample.
	//
	/// Insert ourself into the movie.
	void	read(movie_definition* m, int handler_id, long start);

	void	execute(sprite_instance* m) const;

	/// StartStreamSound is not a "state" tag?
	void execute_state(sprite_instance* m) const;
};

} // namespace gnash


#endif // GNASH_SOUND_H
