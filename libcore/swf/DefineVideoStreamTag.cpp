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

#include <boost/shared_array.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <memory> 
#include <vector> 

#include "Video_as.h"
#include "RunResources.h"
#include "Video.h"
#include "MediaParser.h" // for VideoInfo
#include "VideoDecoder.h"
#include "SWFStream.h" // for read()
#include "movie_definition.h"
#include "GnashNumeric.h"
#include "Global_as.h"

namespace gnash {
namespace SWF {

DefineVideoStreamTag::DefineVideoStreamTag(SWFStream& in, boost::uint16_t id)
	:
    DefinitionTag(id),
	_width(0),
	_height(0)
{
    read(in);
}

DefineVideoStreamTag::~DefineVideoStreamTag()
{
}


void
DefineVideoStreamTag::loader(SWFStream& in, SWF::TagType tag,
        movie_definition& m, const RunResources& /*r*/)
{
    assert(tag == SWF::DEFINEVIDEOSTREAM); // 60
    
    in.ensureBytes(2);
    boost::uint16_t id = in.read_u16();

    std::auto_ptr<DefineVideoStreamTag> vs(new DefineVideoStreamTag(in, id));

    m.addDisplayObject(id, vs.release());

}

void
DefineVideoStreamTag::read(SWFStream& in)
{

	assert(!_videoInfo.get()); // allowed to be called only once

	// numFrames:2 width:2 height:2 flags:1
	in.ensureBytes(8);

	m_num_frames = in.read_u16();

	_width = in.read_u16();
	_height = in.read_u16();

	m_bound.set_to_point(0, 0);
	m_bound.expand_to_point(pixelsToTwips(_width), pixelsToTwips(_height));

	m_reserved_flags = in.read_uint(5);
	m_deblocking_flags = in.read_uint(2);
	m_smoothing_flags = in.read_bit(); 

	m_codec_id = static_cast<media::videoCodecType>(in.read_u8());

	if (!m_codec_id) {
		IF_VERBOSE_PARSE(
		log_debug("An embedded video stream was created with a 0 Codec "
			  "ID. This probably means the embedded video serves "
			  "to place a NetStream video on the stage. Embedded "
			  "video decoding will thus not take place.");
		);
		return;
	}

	_videoInfo.reset(new media::VideoInfo(m_codec_id, _width, _height,
                0 /*framerate*/, 0 /*duration*/,
                media::CODEC_TYPE_FLASH));
}

void
DefineVideoStreamTag::addVideoFrameTag(
        std::auto_ptr<media::EncodedVideoFrame> frame)
{
	boost::mutex::scoped_lock lock(_video_mutex);
    _video_frames.push_back(frame);
}

DisplayObject*
DefineVideoStreamTag::createDisplayObject(Global_as& gl,
        DisplayObject* parent) const
{
    as_object* obj = createVideoObject(gl);
	DisplayObject* ch = new Video(obj, this, parent);
	return ch;
}

} // namespace SWF
} // namespace gnash

