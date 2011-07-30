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

#include <boost/cstdint.hpp> 

#include "ControlTag.h" // for inheritance
#include "SWF.h" // for TagType definition
#include "sound_handler.h" // for StreamBlockId identifier

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
public:

    /// Start the associated block of sound
    void executeActions(MovieClip* m, DisplayList& dlist) const;

    /// Load an SWF::SOUNDSTREAMBLOCK (19) tag.
    static void loader(SWFStream& in, TagType tag, movie_definition& m,
            const RunResources& r);

private:

    /// Create a ControlTag playing the given sample when executed.
    //
    /// @param streamId     identifier of the stream to play.
    /// @param blockId      identifier of the stream block to play.
    ///
    /// This should only be constructed using the loader() function.
    StreamSoundBlockTag(boost::uint16_t streamId,
                        sound::sound_handler::StreamBlockId blockId)
        :
        _handler_id(streamId),
        _blockId(blockId)
    {}

    /// Id of the stream this tag should play
    const boost::uint16_t _handler_id;

    /// Offset in the stream buffer to play
    const sound::sound_handler::StreamBlockId _blockId;

};


} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_STREAMSOUNDBLOCK_TAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
