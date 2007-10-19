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
// $Id: video_stream_def.h,v 1.14 2007/10/19 09:30:24 strk Exp $

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
#include "VideoDecoder.h"
#include "image.h"

#include <map>
#include <boost/shared_array.hpp>
#include <boost/scoped_ptr.hpp> 

namespace gnash {

class video_stream_definition : public character_def
{
public:

	/// Construct a video stream definition with given ID
	//
	/// NOTE: for dynamically created definitions (ActionScript Video class instances)
	///       you can use an id of -1. See character_def constructor, as that's the
	///	  one which will eventually get passed the id.
	///
	video_stream_definition(uint16_t char_id);

	~video_stream_definition();


	character* create_character_instance(character* parent, int id);

	/// Read tag SWF::DEFINEVIDEOSTREAM or SWF::VIDEOFRAME
	//
	/// For DEFINEVIDEOSTREAM tag, the character_id is assumed to have been
	/// already read by caller.
	///
	/// For VIDEOFRAME, again, the character_id (which contains association
	/// between the VIDEOFRAME tag and the VIDEOSTREAM defined before) is
	/// assumed to have been already read.
	///
	/// For clarity, a *single* instance of this class should theoretically
	/// read a DEFINEVIDEOSTREAM on first call and zero or more VIDEOFRAME
	/// tags.
	///
	/// TODO: separate the two reader functions, provide a constructor
	///       reading the DEFINEVIDEOSTREAM and only expose the parser
	///	      for VIDEOFRAME (to ensure, at C++ level, that we won't
	///       parse DEFINEVIDEOSTREAM twice).
	///
	void	read(stream* in, SWF::tag_type tag, movie_definition* m);

	/// Return local video bounds in twips
	const rect&	get_bound() const
	{
		return m_bound;
	}

	/// Get the Video frame associated with the given SWF frame number
	//
	/// @param frameNum
	///	0-based SWF frame number of which we want to fetch associated Video frame.
	///
	/// @return pointer (possibly NULL) to an image. The ownership is with the callee
	///
	image::image_base* get_frame_data(int frameNum);

private:

	/// Id of this character definition, set by constructor.
	///
	/// The id is currently set to -1 when the definition is actually
	/// created dynamically (instantiating the ActionScript Video class)
	///
	uint16_t m_char_id;

	/// Reserved flags read from DEFINEVIDEOSTREAM tag
	uint8_t m_reserved_flags;

	/// Flags read from DEFINEVIDEOSTREAM tag
	uint8_t m_deblocking_flags;

	/// Smoothing flag, as read from DEFINEVIDEOSTREAM tag
	bool m_smoothing_flags;

	/// Frame in which the DEFINEVIDEOSTREAM was found
	uint16_t m_start_frame;

	/// Number of frames in the embedded video, as reported
	/// by the DEFINEVIDEOSTREAM tag
	///
	uint16_t m_num_frames;

	/// Codec ID as read from DEFINEVIDEOSTREAM tag
	//
	/// 0: extern file
	/// 2: H.263
	/// 3: screen video (Flash 7+ only)
	/// 4: VP6
	///
	videoCodecType m_codec_id;

	/// Bounds of the video, as read from the DEFINEVIDEOSTREAM tag.
	rect m_bound;

	/// The undecoded video frames and its size, using the swf-frame number as key
	//
	/// Elements of this map are owned by this instance, and will be deleted 
	/// at instance destruction time.
	///
	typedef std::map<uint32_t, image::image_base*> EmbedFrameMap;
	EmbedFrameMap m_video_frames;

	/// Width of the video
	uint32_t _width;

	/// Height of the video
	uint32_t _height;

	/// The decoder used to decode the video frames
	boost::scoped_ptr<VideoDecoder> _decoder;
};

}	// end namespace gnash


#endif // GNASH_VIDEO_STREAM_DEF_H
