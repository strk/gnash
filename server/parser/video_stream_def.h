// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// 
// $Id: video_stream_def.h,v 1.4 2007/02/09 16:40:42 tgc Exp $

#ifndef GNASH_VIDEO_STREAM_DEF_H
#define GNASH_VIDEO_STREAM_DEF_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "character_def.h"
#include "stream.h" // for read()
#include "movie_definition.h"
#include "swf.h"
#include "rect.h" // for composition
#include "execute_tag.h"
#include "embedVideoDecoder.h"
#include "image.h"

namespace gnash {

class video_stream_definition : public character_def
{
public:

	video_stream_definition(uint16_t char_id);
	~video_stream_definition();


	character* create_character_instance(character* parent, int id);
	void	read(stream* in, SWF::tag_type tag, movie_definition* m);
	const rect&	get_bound() const	{
		return m_unused_rect;
	}

	embedVideoDecoder* get_decoder();

	void get_frame_data(int frameNum, uint8_t** data, int* size);

	uint16_t m_width;
	uint16_t m_height;

private:

	uint16_t m_char_id;
	uint8_t m_reserved_flags;
	uint8_t m_deblocking_flags;
	bool m_smoothing_flags;

	uint16_t m_start_frame;
	uint16_t m_num_frames;

	// 0: extern file
	// 2: H.263
	// 3: screen video (Flash 7+ only)
	// 4: VP6
	uint8_t m_codec_id;
	rect m_unused_rect;

	/// The undecoded video frames
	std::vector<uint8_t*>	m_video_frames;

	/// Size the undecoded video frames
	std::vector<int>	m_video_frames_size;
};

}	// end namespace gnash


#endif // GNASH_VIDEO_STREAM_DEF_H
