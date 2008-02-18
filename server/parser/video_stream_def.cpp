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
// $Id: video_stream_def.cpp,v 1.38 2008/02/18 22:25:19 strk Exp $

#include "video_stream_def.h"
#include "video_stream_instance.h"
#include "render.h"
#include "BitsReader.h"

#include "VideoDecoderGst.h"
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

void myunref(GstBuffer* buf)
{
	gst_buffer_unref(buf);
}


video_stream_definition::~video_stream_definition()
{
	std::for_each(_video_frames.begin(), _video_frames.end(), myunref);
}


void
video_stream_definition::readDefineVideoStream(stream* in, SWF::tag_type tag, movie_definition* m)
{

	// Character ID has been read already, and was loaded in the constructor

	assert(tag == SWF::DEFINEVIDEOSTREAM);
	assert(!_decoder.get()); // allowed to be called only once

	m_start_frame = m->get_loading_frame();

	// numFrames:2 width:2 height:2 flags:1 codec:1
	in->ensureBytes(8);

	m_num_frames = in->read_u16();

	_width = in->read_u16();
	_height = in->read_u16();

	m_bound.enclose_point(0, 0);
	m_bound.expand_to_point(PIXELS_TO_TWIPS(_width), PIXELS_TO_TWIPS(_height));

	m_reserved_flags = in->read_uint(5);
	m_deblocking_flags = in->read_uint(2);
	m_smoothing_flags = in->read_bit(); 

	m_codec_id = static_cast<media::videoCodecType>(in->read_u8());

	_decoder.reset( new media::VideoDecoderGst(m_codec_id, _width, _height) );

}

void
video_stream_definition::readDefineVideoFrame(stream* in, SWF::tag_type tag, movie_definition* m)
{
	// Character ID has been read already, and was loaded in the constructor

	assert(tag == SWF::VIDEOFRAME);
	assert ( _decoder.get() ); // not allowed to be called for a dynamically-created video_stream_def

	in->ensureBytes(2);
	unsigned int frameNum = in->read_u16(); // in->skip_bytes(2); 
	if ( m->get_loading_frame() != frameNum )
	{
		log_debug("frameNum field in tag is %d, currently loading frame is "SIZET_FMT", we'll use the latter.",
			frameNum, m->get_loading_frame());
		frameNum = m->get_loading_frame();
	}

	unsigned int dataSize = in->get_tag_end_position() - in->get_position();

	GstBuffer* buffer = gst_buffer_new_and_alloc(dataSize+8);
	memset(GST_BUFFER_DATA(buffer)+dataSize, 0, 8);
	GST_BUFFER_SIZE (buffer) = dataSize;

	if (!buffer) {
		log_error(_("Failed to allocate a buffer of size %d advertised by SWF."),
		dataSize);
		return;
	}
	
	GST_BUFFER_OFFSET(buffer) = frameNum;
	GST_BUFFER_TIMESTAMP(buffer) = GST_CLOCK_TIME_NONE;
	GST_BUFFER_DURATION(buffer) = GST_CLOCK_TIME_NONE;

	in->read((char*)GST_BUFFER_DATA(buffer), dataSize);

	_video_frames.push_back(buffer);
}


character*
video_stream_definition::create_character_instance(character* parent, int id)
{
	character* ch = new video_stream_instance(this, parent, id);
	return ch;
}

bool
has_frame_number(GstBuffer* buf, boost::uint32_t frameNumber)
{
	return GST_BUFFER_OFFSET(buf) == frameNumber;
}

std::auto_ptr<image::image_base>
video_stream_definition::get_frame_data(boost::uint32_t frameNum)
{
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

		gst_buffer_ref(*it); // make sure gstreamer doesn't delete the buffer.
		_last_decoded_frame = GST_BUFFER_OFFSET(*it);
		_decoder->pushRawFrame(*it);	  
	}

	std::auto_ptr<media::gnashGstBuffer> buffer = _decoder->popDecodedFrame();

	// If more data has arrived, replace the buffer with the next frame.
	while (_decoder->peek()) {
		buffer = _decoder->popDecodedFrame();
	}

	return std::auto_ptr<image::image_base>(buffer.release());
}




} // namespace gnash

