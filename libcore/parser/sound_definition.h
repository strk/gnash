// 
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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


#include "ExportableResource.h"
#include "ControlTag.h" // for sound tags inheritance
#include "RunResources.h" // TODO: drop.

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
/// @todo move definition to sound_handler.h and possibly nest
/// inside sound_handler itself ?
/// @todo   work out why this is a resource, what a resource is useful for,
///         and if it really needs access to a sound_handler rather than
///         belonging to it.
class sound_sample: public ExportableResource
{
public:
	int	m_sound_handler_id;

	sound_sample(int id, const RunResources& r)
		:
		m_sound_handler_id(id),
        _runInfo(r)
	{
	}

	/// delete the actual sound sample from the currently registered
	/// sound handler.
	~sound_sample();

    /// This is necessary because at least some sound_samples are
    /// destroyed after the movie_root has been destroyed, so that
    /// access through the VM (which assumes movie_root exists) causes
    /// a segfault.
    const RunResources& _runInfo;

};

} // namespace gnash


#endif // GNASH_SOUND_H
