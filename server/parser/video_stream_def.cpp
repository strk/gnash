// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
// $Id: video_stream_def.cpp,v 1.22 2007/10/19 12:17:28 strk Exp $

#include "video_stream_def.h"
#include "video_stream_instance.h"
#include "render.h"

#ifdef USE_FFMPEG
#include "VideoDecoderFfmpeg.h"
#elif defined(SOUND_GST)
#include "VideoDecoderGst.h"
#endif

namespace gnash {

video_stream_definition::video_stream_definition(uint16_t char_id)
	:
	m_char_id(char_id),
	_width(0),
	_height(0),
	_decoder(NULL)
{
}


video_stream_definition::~video_stream_definition()
{
	for (EmbedFrameMap::iterator i=m_video_frames.begin(), e=m_video_frames.end();
			i!=e; ++i)
	{
		delete i->second;
	}
}


void
video_stream_definition::readDefineVideoStream(stream* in, SWF::tag_type tag, movie_definition* m)
{

	// Character ID has been read already, and was loaded in the constructor

	assert(tag == SWF::DEFINEVIDEOSTREAM);
	assert(!_decoder.get()); // allowed to be called only once

	m_start_frame = m->get_loading_frame();

	m_num_frames = in->read_u16();

	_width = in->read_u16();
	_height = in->read_u16();
	m_bound.enclose_point(0, 0);
	m_bound.expand_to_point(PIXELS_TO_TWIPS(_width), PIXELS_TO_TWIPS(_height));

	m_reserved_flags = in->read_uint(5);
	m_deblocking_flags = in->read_uint(2);
	m_smoothing_flags = in->read_bit(); 

	m_codec_id = static_cast<videoCodecType>(in->read_u8());
#ifdef USE_FFMPEG
	_decoder.reset( new VideoDecoderFfmpeg() );
#elif defined(SOUND_GST)
	_decoder.reset( new VideoDecoderGst() );
#else
	_decoder.reset( new VideoDecoder() );
#endif
	bool ret = _decoder->setup(_width, _height, m_deblocking_flags, m_smoothing_flags, m_codec_id, gnash::render::videoFrameFormat());
	if (!ret) _decoder.reset(new VideoDecoder()); // This is so statically-defined video_stream_def always have a _decoder != NULL

}

void
video_stream_definition::readDefineVideoFrame(stream* in, SWF::tag_type tag, movie_definition* m)
{
	// Character ID has been read already, and was loaded in the constructor

	assert(tag == SWF::VIDEOFRAME);
	assert ( _decoder.get() ); // not allowed to be called for a dynamically-created video_stream_def

	// We don't use the videoframe number, but instead
	// each video frame is tied to the swf-frame where
	// it belongs.
	in->skip_bytes(2); //int frameNum = in->read_u16();

	// We need to make the buffer a FF_INPUT_BUFFER_PADDING_SIZE
	// bigger than the data to avoid libavcodec (ffmpeg) making
	// illegal reads. Also, we must ensure first 23 bits to be 
	// zeroed out. We'll zero out all padding.
	unsigned int padding =  FF_INPUT_BUFFER_PADDING_SIZE;
	unsigned int dataSize = in->get_tag_end_position() - in->get_position();
	unsigned int totSize = dataSize+padding;

	boost::scoped_array<uint8_t> data ( new uint8_t[totSize] );
	for (unsigned int i = 0; i < dataSize; ++i) 
	{
		data[i] = in->read_u8();
	}
	memset(&data[dataSize], 0, padding);  // padd with zeroes

	// TODO: should we pass dataSize instead of totSize to decodeToImage ?
	std::auto_ptr<image::image_base> img ( _decoder->decodeToImage(data.get(), totSize) );

	if ( img.get() )
	{
		// TODO: why don't we use  the frame number specified
		//       in the tag instead of skipping it ?
		size_t frameNum = m->get_loading_frame();

		setFrameData(frameNum, img);
	}
	else
	{
		log_error(_("An error occured while decoding video frame in frame %d"), m->get_loading_frame());
	}
}


character*
video_stream_definition::create_character_instance(character* parent, int id)
{
	character* ch = new video_stream_instance(this, parent, id);
	return ch;
}

image::image_base*
video_stream_definition::get_frame_data(int frameNum)
{
	EmbedFrameMap::iterator it = m_video_frames.find(frameNum);
	if( it != m_video_frames.end() )
	{
		return it->second;
	} else {
		log_debug(_("No video data available for frame %d."), frameNum);
		return NULL;
	}
}

void
video_stream_definition::setFrameData(uint32_t frameNum, std::auto_ptr<image::image_base> image)
{
	image::image_base*& ptr = m_video_frames[frameNum];
	if ( ptr )
	{
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror(_("Mulitple video frames defined for frame %u"), frameNum);
		);
		return;
	}
	ptr = image.release();
}

} // namespace gnash

