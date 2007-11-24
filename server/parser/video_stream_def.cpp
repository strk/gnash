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
// $Id: video_stream_def.cpp,v 1.26 2007/11/24 17:21:45 strk Exp $

#include "video_stream_def.h"
#include "video_stream_instance.h"
#include "render.h"
#include "BitsReader.h"

#ifdef USE_FFMPEG
#include "VideoDecoderFfmpeg.h"
#elif defined(SOUND_GST)
#include "VideoDecoderGst.h"
#endif

namespace gnash {

video_stream_definition::video_stream_definition(uint16_t char_id)
	:
	m_char_id(char_id),
	m_last_decoded_frame(-1),
	_width(0),
	_height(0),
	_decoder(NULL)
{
}


video_stream_definition::~video_stream_definition()
{
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

	m_codec_id = static_cast<media::videoCodecType>(in->read_u8());
#ifdef USE_FFMPEG
	_decoder.reset( new media::VideoDecoderFfmpeg() );
#elif defined(SOUND_GST)
	_decoder.reset( new media::VideoDecoderGst() );
#else
	_decoder.reset( new media::VideoDecoder() );
#endif
	bool ret = _decoder->setup(_width, _height, m_deblocking_flags, m_smoothing_flags, m_codec_id, gnash::render::videoFrameFormat());
	if (!ret) _decoder.reset(new media::VideoDecoder()); // This is so statically-defined video_stream_def always have a _decoder != NULL

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
	unsigned int padding =  _decoder->getPaddingBytes();
	unsigned int dataSize = in->get_tag_end_position() - in->get_position();
	unsigned int totSize = dataSize+padding;

	boost::shared_array<uint8_t> data ( new uint8_t[totSize] );
	for (unsigned int i = 0; i < dataSize; ++i) 
	{
		data[i] = in->read_u8();
	}
	if ( padding ) memset(&data[dataSize], 0, padding);  // pad with zeroes if needed


	// Check what kind of frame this is
	media::videoFrameType ft;
	if (m_codec_id == media::VIDEO_CODEC_H263) {
		// Parse the h263 header to determine the frame type. The position of the
		// info varies if the frame size is custom.
		std::auto_ptr<BitsReader> br (new BitsReader(data.get(), totSize));
		uint32_t tmp = br->read_uint(30);
		tmp = br->read_uint(3);
		if (tmp == 0) tmp = br->read_uint(32);
		else if (tmp == 1) tmp = br->read_uint(16);
		
		// Finally we're at the info, read and use
		tmp = br->read_uint(3);
		if (tmp == 0) ft = media::KEY_FRAME;
		else if (tmp == 1) ft = media::INTER_FRAME;
		else ft = media::DIS_INTER_FRAME;

	} else if (m_codec_id == media::VIDEO_CODEC_VP6 || m_codec_id == media::VIDEO_CODEC_VP6A) {
		// Get the info from the VP6 header
		if (!(data.get()[0] & 0x80)) ft = media::KEY_FRAME;
		else ft = media::INTER_FRAME;

	} else {
		ft = media::KEY_FRAME;
	}

	setFrameData(m->get_loading_frame(), data, totSize, ft);

}


character*
video_stream_definition::create_character_instance(character* parent, int id)
{
	character* ch = new video_stream_instance(this, parent, id);
	return ch;
}

std::auto_ptr<image::image_base>
video_stream_definition::get_frame_data(uint32_t frameNum)
{

	// Check if the requested frame hold any video data.
	EmbedFrameMap::iterator it = m_video_frames.find(frameNum);
	if( it == m_video_frames.end() )
	{
		log_debug(_("No video data available for frame %d."), frameNum);
		return std::auto_ptr<image::image_base>(NULL);
	}

	// rewind to the nearest keyframe, or the last frame we decoded
	while (static_cast<uint32_t>(m_last_decoded_frame+1) != it->first && it->second->frameType != media::KEY_FRAME && it != m_video_frames.begin()) it--;

	std::auto_ptr<image::image_base> ret(NULL);

	// Decode all the frames needed to produce the requested one
	while (it != m_video_frames.end() && it->first <= frameNum) {
		// If this is a disposable interlaced frame, and it is not the
		// last one to be decoded, we skip the decoding.
		if (!(it->second->frameType == media::DIS_INTER_FRAME && it->first != frameNum)) {
			ret.reset(NULL);
			ret = _decoder->decodeToImage(it->second->videoData.get(), it->second->dataSize);
		}
		it++;
	}

	m_last_decoded_frame = frameNum;

	return ret;

}

void
video_stream_definition::setFrameData(uint32_t frameNum, boost::shared_array<uint8_t> data, uint32_t size, media::videoFrameType ft)
{
	EmbedFrameMap::iterator it = m_video_frames.find(frameNum);
	if( it != m_video_frames.end() )
	{
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror(_("Mulitple video frames defined for frame %u"), frameNum);
		);
		return;
	}
	
	boost::shared_ptr<VideoData> vd (new VideoData(data, size,ft));
	m_video_frames[frameNum] = vd;

}

} // namespace gnash

