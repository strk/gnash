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

#ifndef GNASH_SWF_STARTSOUND_TAG_H
#define GNASH_SWF_STARTSOUND_TAG_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "ControlTag.h" // for inheritance
#include "sound_handler.h" // for sound_envelope (composition)
#include "VM.h" // We only need this to get movie_root
#include "swf.h" // for tag_type definition

#include <vector> // for composition
#include <boost/cstdint.hpp> // for boost::uint16_t and friends
 

// Forward declarations
namespace gnash {
	class SWFStream;
	class movie_definition;
	class sound_sample;
    class RunInfo;
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

	/// Number of loops started by an execution of this tag 
	// 
	/// This number is 0 if the sound must be played only once,
	/// 1 to play twice and so on...
	///
	/// It is not known whether a value exists to specify "loop forever"
	///
	int	m_loop_count;

	/// If true this tag actually *stops* the sound rather then playing it.
	//
	/// In a well-formed SWF when this flag is on all others should be off
	/// (no loops, no envelopes, no in/out points).
	///
	bool	m_stop_playback;

	/// In/Out points, currently unsupported
	//
	/// See http://sswf.sourceforge.net/SWFalexref.html#swf_soundinfo
	// unsigned long m_in_point;
	// unsigned long m_out_point;

	/// Sound effects (envelopes) for this start of the sound
	//
	/// See http://sswf.sourceforge.net/SWFalexref.html#swf_envelope
	///
	std::vector<sound::sound_handler::sound_envelope> m_envelopes;

	/// Initialize this StartSoundTag from the SWFStream  
	//
	/// The stream is assumed to be positioned right after the
	/// sound_id field of the tag structure.
	///
	void read(SWFStream& in);


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

	void	execute(MovieClip* /* m */, DisplayList& /* dlist */) const;

	/// Load a SWF::STARTSOUND tag.
	static void loader(SWFStream& in, tag_type tag, movie_definition& m, const RunInfo& r);

};


} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_STARTSOUND_TAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
