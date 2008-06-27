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
#include "MediaHandler.h"
#include "MediaParser.h" // for VideoInfo
#include "VideoDecoder.h"
#include "stream.h" // for read()

#include <boost/bind.hpp>


namespace gnash {

video_stream_definition::video_stream_definition(boost::uint16_t char_id)
	:
	m_char_id(char_id),
	_last_decoded_frame(-1),
	_width(0),
	_height(0),
	_decoder(NULL)
{
}

video_stream_definition::~video_stream_definition()
{
	std::for_each(_video_frames.begin(), _video_frames.end(),
		      boost::checked_deleter<media::EncodedVideoFrame>());
}


void
video_stream_definition::readDefineVideoStream(SWFStream* in, SWF::tag_type tag, movie_definition* m)
{
	// Character ID has been read already, and was loaded in the constructor

	assert(tag == SWF::DEFINEVIDEOSTREAM);
	assert(!_decoder.get()); // allowed to be called only once

	m_start_frame = m->get_loading_frame();

	// numFrames:2 width:2 height:2 flags:1
	in->ensureBytes(8);

	m_num_frames = in->read_u16();

	_width = in->read_u16();
	_height = in->read_u16();

	m_bound.set_to_point(0, 0);
	m_bound.expand_to_point(PIXELS_TO_TWIPS(_width), PIXELS_TO_TWIPS(_height));

	m_reserved_flags = in->read_uint(5);
	m_deblocking_flags = in->read_uint(2);
	m_smoothing_flags = in->read_bit(); 

	m_codec_id = static_cast<media::videoCodecType>(in->read_u8());

	if (!m_codec_id) {
		IF_VERBOSE_PARSE(
		log_debug("An embedded video stream was created with a 0 Codec "
			  "ID. This probably means the embedded video serves "
			  "to place a NetStream video on the stage. Embedded "
			  "video decoding will thus not take place.");
		);
		return;
	}

	media::MediaHandler* mh = media::MediaHandler::get();
	if ( ! mh )
	{
		LOG_ONCE( log_error(_("No Media handler registered, "
			"won't be able to decode embedded video")) );
		return;
	}

	media::VideoInfo info(m_codec_id, _width, _height, 0 /*framerate*/, 0 /*duration*/, media::FLASH /*typei*/);
	_decoder = mh->createVideoDecoder(info); 
	if ( ! _decoder.get() )
	{
		log_error(_("Could not create video decoder for codec id %d"),
			m_codec_id);
	}
}

void
video_stream_definition::readDefineVideoFrame(SWFStream* in, SWF::tag_type tag, movie_definition* m)
{
	// Character ID has been read already, and was loaded in the constructor

	assert(tag == SWF::VIDEOFRAME);
	if ( ! _decoder.get() ) return; // --enable-media=none - TODO: create a NullVideoDecoder ?
	assert ( _decoder.get() ); // not allowed to be called for a dynamically-created video_stream_def

	in->ensureBytes(2);
	unsigned int frameNum = in->read_u16(); // in->skip_bytes(2); 
	if ( m->get_loading_frame() != frameNum )
	{
		log_debug("frameNum field in tag is %d, currently loading frame is %d, we'll use the latter.",
			frameNum, m->get_loading_frame());
		frameNum = m->get_loading_frame();
	}

	const unsigned int dataLength = in->get_tag_end_position() - in->tell();
	
	boost::uint8_t* buffer = new uint8_t[dataLength + 8]; // FIXME: catch bad_alloc

	const size_t bytesRead = in->read(reinterpret_cast<char*>(buffer), dataLength);

    if (bytesRead < dataLength)
    {
        throw ParserException(_("Tag boundary reported past end of stream!"));
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

std::auto_ptr<image::image_base>
video_stream_definition::get_frame_data(boost::uint32_t frameNum)
{
	boost::mutex::scoped_lock lock(_video_mutex);

	if (_video_frames.empty()) {
		return std::auto_ptr<image::image_base>();
	}
	// Check if the requested frame holds any video data.
	EmbedFrameVec::iterator it = std::find_if(_video_frames.begin(),
	  _video_frames.end(), boost::bind(has_frame_number, _1, frameNum));
	
	// FIXME: although we return nothing here, we should return the
	// previously decoded frame.
	if( it == _video_frames.end() )	{
		return std::auto_ptr<image::image_base>();
	}

	// We are going backwards, so start from the beginning.	
	if (_last_decoded_frame > boost::int32_t(frameNum)) {
		_last_decoded_frame = -1;
	}
	
	// Push all the frames after the previously decoded frame, until the
	// target frame has been reached.
	while (_last_decoded_frame != boost::int32_t(frameNum)) {
		it = std::find_if(_video_frames.begin(),
			_video_frames.end(), boost::bind(has_frame_number, _1,
							 _last_decoded_frame));

		if (it == _video_frames.end()) {
			it = _video_frames.begin();
		} else {
			++it;
		}
		
		if (it == _video_frames.end()) {
			return std::auto_ptr<image::image_base>();
		}

		_last_decoded_frame = (*it)->frameNum();
		_decoder->push(*(*it));
	}

	std::auto_ptr<image::rgb> buffer = _decoder->pop();

	// If more data has arrived, replace the buffer with the next frame.
	while (_decoder->peek()) {
		buffer = _decoder->pop();
	}

	return std::auto_ptr<image::image_base>(buffer.release());
}




} // namespace gnash

