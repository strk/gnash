// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#include "DefineVideoStreamTag.h"

#include <algorithm>

#include "RunResources.h"
#include "VideoFrameTag.h"
#include "VideoDecoder.h"
#include "SWFStream.h" // for read()
#include "movie_definition.h"
#include "utility.h"

namespace gnash {
namespace SWF {


void
VideoFrameTag::loader(SWFStream& in, SWF::TagType tag, movie_definition& m,
        const RunResources& /*r*/)
{
    assert(tag == SWF::VIDEOFRAME);

    in.ensureBytes(2);
    std::uint16_t id = in.read_u16();
    DefinitionTag* chdef = m.getDefinitionTag(id);

    if (!chdef)
    {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("VideoFrame tag refers to unknown video "
                    "stream id %d"), id);
        );
        return;
    }

    DefineVideoStreamTag* vs = dynamic_cast<DefineVideoStreamTag*> (chdef);
    if (!vs)
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("VideoFrame tag refers to a non-video DisplayObject "
                "%d (%s)"), id, typeName(*chdef));
        );
        return;
    }

    // TODO: skip if there's no MediaHandler registered ?

    const unsigned short padding = 8;

    in.ensureBytes(3);
    unsigned int frameNum = in.read_u16();

    const media::VideoInfo* info = vs->getVideoInfo();

    if (info && info->codec == media::VIDEO_CODEC_SCREENVIDEO) {
        // According to swfdec, every SV frame comes with keyframe
        // and format identifiers (4 bits each), but these are not
        // part of the codec bitstream and break the decoder.
        (void) in.read_u8();
    }

	
    const unsigned int dataLength = in.get_tag_end_position() - in.tell();

    // FIXME: catch bad_alloc
    std::uint8_t* buffer = new std::uint8_t[dataLength + padding];

    const size_t bytesRead = in.read(reinterpret_cast<char*>(buffer),
                                     dataLength);

    if (bytesRead < dataLength)
    {
        throw ParserException(_("Could not read enough bytes when parsing "
                                "VideoFrame tag. Perhaps we reached the "
                                "end of the stream!"));
    }	
	
    std::fill_n(buffer + bytesRead, padding, 0);

    using namespace media;

    std::unique_ptr<EncodedVideoFrame> frame(
            new EncodedVideoFrame(buffer, dataLength, frameNum));

    vs->addVideoFrameTag(std::move(frame));
}

} // namespace SWF
} // namespace gnash

