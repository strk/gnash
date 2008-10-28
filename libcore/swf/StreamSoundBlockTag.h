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

#ifndef GNASH_SWF_STREAMSOUNDBLOCK_TAG_H
#define GNASH_SWF_STREAMSOUNDBLOCK_TAG_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "ControlTag.h" // for inheritance
#include "swf.h" // for tag_type definition

#include <boost/cstdint.hpp> // for boost::uint16_t and friends
 

// Forward declarations
namespace gnash {
	class SWFStream;
	class movie_definition;
}

namespace gnash {
namespace SWF {

/// SWF Tag StreamSoundBlock (19).
//
///
/// Virtual control tag for syncing streaming sound to playhead
///
/// Gnash will register instances of this ControlTag in the 
/// frame containing blocks of a streaming sound, which is
/// occurrences of SWF Tag StreamSoundBlock (19).
///
/// The tag will then be used to start playing the specific block
/// in sync with the frame playhead.
///
class StreamSoundBlockTag : public ControlTag
{

	/// Id of the stream this tag should play
	boost::uint16_t	m_handler_id;

	/// Offset in the stream buffer to play
	long		m_start;

	//int		latency;

public:

	/// Create a ControlTag playing the given sample when executed.
	//
	/// @param handlerId
	///	Identifier of the stream to play.
	///
	/// @param start
	///	Offset to start playback from.
	///	(Should be offset of the associated sound block).
	///
	StreamSoundBlockTag(int handlerId, long start)
		:
		m_handler_id(handlerId),
		m_start(start)
	{}

	/// Start the associated block of sound
	void execute(MovieClip* m, DisplayList& /*dlist*/) const;

	/// Load an SWF::SOUNDSTREAMBLOCK (19) tag.
	static void loader(SWFStream& in, tag_type tag, movie_definition& m, const RunInfo& r);

	/// Not a "state" (DisplayList?) tag, do doesn't need to provide execute_state
	//void execute_state(MovieClip* m) const {}
};


} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_STREAMSOUNDBLOCK_TAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
