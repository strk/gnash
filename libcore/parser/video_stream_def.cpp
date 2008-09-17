// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

// 

#include "video_stream_def.h"
#include "video_stream_instance.h"
#include "render.h"
#include "BitsReader.h"
//#include "MediaHandler.h"
#include "MediaParser.h" // for VideoInfo
#include "VideoDecoder.h"
#include "SWFStream.h" // for read()

#include <boost/bind.hpp>


namespace gnash {

video_stream_definition::video_stream_definition(boost::uint16_t char_id)
	:
	m_char_id(char_id),
	_width(0),
	_height(0)
{
}

video_stream_definition::~video_stream_definition()
{
	std::for_each(_video_frames.begin(), _video_frames.end(),
		      boost::checked_deleter<media::EncodedVideoFrame>());
}


void
video_stream_definition::readDefineVideoStream(SWFStream& in, SWF::tag_type tag, movie_definition& /*m*/)
{
	// Character ID has been read already, and was loaded in the constructor

	assert(tag == SWF::DEFINEVIDEOSTREAM);
	assert(!_videoInfo.get()); // allowed to be called only once

	//m_start_frame = m->get_loading_frame();

	// numFrames:2 width:2 height:2 flags:1
	in.ensureBytes(8);

	m_num_frames = in.read_u16();

	_width = in.read_u16();
	_height = in.read_u16();

	m_bound.set_to_point(0, 0);
	m_bound.expand_to_point(PIXELS_TO_TWIPS(_width), PIXELS_TO_TWIPS(_height));

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

	_videoInfo.reset( new media::VideoInfo(m_codec_id, _width, _height, 0 /*framerate*/, 0 /*duration*/, media::FLASH /*typei*/) );
}

void
video_stream_definition::readDefineVideoFrame(SWFStream& in, SWF::tag_type tag, movie_definition& /*m*/)
{
	// Character ID has been read already, and was loaded in the constructor

	assert(tag == SWF::VIDEOFRAME);

	// TODO: skip if there's no MediaHandler registered ?

	in.ensureBytes(2);
	unsigned int frameNum = in.read_u16(); 

	const unsigned int dataLength = in.get_tag_end_position() - in.tell();
	
	boost::uint8_t* buffer = new uint8_t[dataLength + 8]; // FIXME: catch bad_alloc

	const size_t bytesRead = in.read(reinterpret_cast<char*>(buffer), dataLength);

    if (bytesRead < dataLength)
    {
        throw ParserException(_("Could not read enough bytes when parsing "
                                "VideoFrame tag. Perhaps we reached the "
                                "end of the stream!"));
    }	
	
	memset(buffer + bytesRead, 0, 8);

	using namespace media;

	EncodedVideoFrame* frame = new EncodedVideoFrame(buffer, dataLength, frameNum);

	boost::mutex::scoped_lock lock(_video_mutex);

	_video_frames.push_back(frame);
}


character*
video_stream_definition::create_character_instance(character* parent, int id)
{
	character* ch = new video_stream_instance(this, parent, id);
	return ch;
}

bool
has_frame_number(media::EncodedVideoFrame* frame, boost::uint32_t frameNumber)
{
	return frame->frameNum() == frameNumber;
}

void
video_stream_definition::getEncodedFrameSlice(boost::uint32_t from, boost::uint32_t to, EmbedFrameVec& ret)
{
	assert(from<=to);

	boost::mutex::scoped_lock lock(_video_mutex);

	EmbedFrameVec::iterator it=_video_frames.begin(), itEnd=_video_frames.end();
	for (; it!=itEnd; ++it)
	{
		media::EncodedVideoFrame* frame = *it;
		if ( frame->frameNum() >= from )
		{
			break;
		}
	}

	if (it==itEnd) return; // no element was >= from

	// push remaining frames 
	for (; it!=itEnd; ++it)
	{
		media::EncodedVideoFrame* frame = *it;
		if ( frame->frameNum() > to ) break; // went too far
		ret.push_back(frame);
	}

}


} // namespace gnash

