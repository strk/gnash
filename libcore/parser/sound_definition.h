// 
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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
#include "ControlTag.h" // for sound tags inheritance

// Forward declarations
namespace gnash {
	class movie_definition;
}


namespace gnash {

/// An identifier for a sound sample managed by a sound_handler
//
/// Definition tags will maintain a mapping between SWF-defined id
/// of the sound and these identifiers.
///
/// Specifying an identifier for use by sound_handler would likely be
/// claner, anyway this is what it is *currently*.
///
/// NOTE that the destructor of this identifier (thus becoming a "smart"
/// identifier) requests the currently registered sound_handler removal
/// of the identified sound_sample. This *might* be the reason why
/// it is a ref-counted thing (hard to belive...).
///
///
/// QUESTION: why is this a resource ?
///           does it really need to be ref-counted ?
///
/// @todo move definition to sound_handler.h and possibly nest inside sound_handler itself ?
///
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

	/// delete the actual sound sample from the currently registered
	/// sound handler.
	~sound_sample();

	sound_sample* cast_to_sound_sample() { return this; }
};

} // namespace gnash


#endif // GNASH_SOUND_H
