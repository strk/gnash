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
#include "execute_tag.h" // for sound tags inheritance

// Forward declarations
namespace gnash {
	class movie_definition;
	class stream;
}


namespace gnash {

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

/// SWF Tag StartSound (15) 
//
/// TODO: move in it's own StartSoundTag files...
///
class start_sound_tag : public execute_tag
{
public:
	uint16_t	m_handler_id;
	int	m_loop_count;
	bool	m_stop_playback;
	std::vector<sound_handler::sound_envelope> m_envelopes;

	// envelopes for the current sound instance
	uint32_t* envelopes;

	start_sound_tag()
		:
		m_handler_id(0),
		m_loop_count(0),
		m_stop_playback(false)
	{
	}


	/// \brief
	/// Initialize this StartSound tag from
	/// the stream  & given sample.
	//
	/// Insert ourself into the movie.
	void read(stream* in, int tag_type,
		movie_definition* m, const sound_sample* sam);

	/// StartSound is a "state" tag.
	void	execute_state(sprite_instance* m) const;

	/// This implementation of 'execute' should likely
	/// be the default one.
	void execute(sprite_instance* m) const
	{
		execute_state(m);
	}

	/// TODO: provide execute_reverse ?
	/// (for StartSound would StopSound and vice-versa)
};

/// SWF Tag SoundStreamBlock (19) 
//
/// TODO: move in it's own SoundStreamBlockTag files...
///
class start_stream_sound_tag : public execute_tag
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

	/// StartStreamSound is a "state" tag.
	void	execute_state(sprite_instance* m) const;

	void execute(sprite_instance* m) const
	{
		execute_state(m);
	}
};

} // namespace gnash


#endif // GNASH_SOUND_H
