// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

#ifndef GNASH_SWF_STARTSOUND_TAG_H
#define GNASH_SWF_STARTSOUND_TAG_H

#include "ControlTag.h" // for inheritance
#include "sound_handler.h" // for sound_envelope (composition)
#include "SWF.h" // for TagType definition
#include "SoundInfoRecord.h"

#include <boost/cstdint.hpp> // for boost::uint16_t and friends
 

// Forward declarations
namespace gnash {
	class SWFStream;
	class movie_definition;
	class sound_sample;
    class RunResources;
}

namespace gnash {
namespace SWF {

/// SWF Tag StartSound (15) 
//
/// See http://sswf.sourceforge.net/SWFalexref.html#tag_startsound
///
class StartSoundTag : public ControlTag
{
	/// This should be a reference to an earlier DefineSound tag id
	/// but in this implementation is instead the sound_handler specific
	/// identifier corresponding to it.
	/// movie_definition keeps a mapping between SWF-defined DefineSound
	/// identifier and sound_handler-provided identifier.
	/// This one is the latter, probably so with the intention of avoiding
	/// a lookup at every execution...
	///
	boost::uint16_t	m_handler_id;

	/// Create a StartSoundTag for starting the given sound sample
	//
	/// The stream is assumed to be positioned right after the
	/// sound_id field of the tag structure.
	///
    /// @param in   The SWFStream to initialize the tag from.
	/// @param sound_handler_id
	/// Sound sample identifier as provided by sound_handler (sic!)
	///
	StartSoundTag(SWFStream& in, int sound_id)
		:
		m_handler_id(sound_id)
	{
        _soundInfo.read(in);
	}

    SoundInfoRecord _soundInfo;

public:

    // This is not a state tag.
	void executeActions(MovieClip* /* m */, DisplayList& /* dlist */) const;

	/// Load a SWF::STARTSOUND tag.
	static void loader(SWFStream& in, TagType tag, movie_definition& m,
            const RunResources& r);

};

/// StartSound2Tag (SWF9)
//
/// Very similar to StartSoundTag, but uses a SoundClassName instead of
/// DisplayObject ID. This is not implemented.
class StartSound2Tag
{
public:

	/// Load a SWF::STARTSOUND2 tag.
	static void loader(SWFStream& in, TagType tag, movie_definition& m,
            const RunResources& r);
};

} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_STARTSOUND_TAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
