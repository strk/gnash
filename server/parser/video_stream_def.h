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
// $Id: video_stream_def.h,v 1.12 2007/09/27 23:59:56 tgc Exp $

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
	///	  for VIDEOFRAME (to ensure, at C++ level, that we won't
	///       parse DEFINEVIDEOSTREAM twice).
	///
	void	read(stream* in, SWF::tag_type tag, movie_definition* m);

	/// Return local video bounds in twips
	const rect&	get_bound() const
	{
		return m_bound;
	}

	/// Return a newly created embedded-video decoder
	//
	/// The type of decoder returned currently depends
	/// on compile-time defines (FFMPG/GST/none)
	///
	/// The returned decoder will be initialized with
	/// data kept as member of this class
	/// (width/height/codec_id/videoFrameFormat)
	/// Note that videoFrameFormat is fetched from the
	/// current renderer.
	///
	/// This function *never* returns a NULL pointer.
	///
	std::auto_ptr<VideoDecoder> get_decoder();

	/// Get the Video frame associated with the given SWF frame number
	//
	/// @param frameNum
	///	0-based SWF frame number of which we want to fetch associated Video frame.
	///
	/// @param data
	///	Output parameter. If a video frame is available for the specified SWF frame,
	///	then the given pointer (*data) will be set to point to a memory buffer owned
	///	by this instance; otherwise (no video frame available) the given pointer will
	///	be set to zero.
	///
	/// @param size
	///	Output parameter. If a video frame is available for the specified SWF frame,
	///	then the given integer (*size) will be set to the size of the memory buffer
	///	returned in the data parameter; otherwise (no video frame available) the given
	///	integer will be set to zero.
	///
	/// TODO: return pointer (possibly NULL) to a structure with data&size ? (simpler)
	///
	void get_frame_data(int frameNum, uint8_t** data, int* size);

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
	/// TODO: define an enumeration for the above values
	///
	videoCodecType m_codec_id;

	/// Bounds of the video, as read from the DEFINEVIDEOSTREAM tag.
	rect m_bound;

	/// The undecoded video frames and its size, using the swf-frame number as key
	//
	/// Elements of this map are owned by this instance, and will be deleted 
	/// at instance destruction time.
	///
	typedef std::pair< boost::shared_array<uint8_t>, uint32_t> EmbedFrame;
	typedef std::map<uint32_t, EmbedFrame > EmbedFrameMap;
	EmbedFrameMap m_video_frames;

};

}	// end namespace gnash


#endif // GNASH_VIDEO_STREAM_DEF_H
