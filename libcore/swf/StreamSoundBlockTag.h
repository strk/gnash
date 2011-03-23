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

#ifndef GNASH_SWF_STREAMSOUNDBLOCK_TAG_H
#define GNASH_SWF_STREAMSOUNDBLOCK_TAG_H

#include "ControlTag.h" // for inheritance
#include "SWF.h" // for TagType definition
#include "sound_handler.h" // for StreamBlockId identifier

#include <boost/cstdint.hpp> // for boost::uint16_t and friends
 

// Forward declarations
namespace gnash {
    class SWFStream;
    class movie_definition;
    class RunResources;
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
    boost::uint16_t m_handler_id;

    /// Offset in the stream buffer to play
    sound::sound_handler::StreamBlockId _blockId;

    //int       latency;

public:

    /// Get the identifier of the sound stream this block belongs to
    //
    /// TODO: why is this an uint16_t if sound_handler::create_sound
    ///       returns an int ? I vote for a sound_handler::SoundId typedef
    ///
    boost::uint16_t getStreamId() const { return m_handler_id; }

    /// Start the associated block of sound
    void executeActions(MovieClip* m, DisplayList& dlist) const;

    /// Load an SWF::SOUNDSTREAMBLOCK (19) tag.
    static void loader(SWFStream& in, TagType tag, movie_definition& m,
            const RunResources& r);

private:

    /// Create a ControlTag playing the given sample when executed.
    //
    /// @param streamId
    /// Identifier of the stream to play.
    ///
    /// @param blockId
    /// Identifier of the stream block to play.
    ///
    /// This should only be constructed using the loader() function.
    StreamSoundBlockTag(int streamId,
                        sound::sound_handler::StreamBlockId blockId)
        :
        m_handler_id(streamId),
        _blockId(blockId)
    {}
};


} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_STREAMSOUNDBLOCK_TAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
