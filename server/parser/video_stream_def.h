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
// $Id: video_stream_def.h,v 1.23 2008/02/22 14:20:49 strk Exp $

#ifndef GNASH_VIDEO_STREAM_DEF_H
#define GNASH_VIDEO_STREAM_DEF_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "character_def.h"
#include "stream.h" // for read()
#include "movie_definition.h"
#include "swf.h"
#include "rect.h" // for composition
#include "ControlTag.h"

#ifdef SOUND_GST
# include "VideoDecoderGst.h"
#elif defined(USE_FFMPEG)
# include "VideoDecoderFfmpeg.h"
#endif

#include "image.h"

#include <map>
#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp> 

namespace gnash {


/// Class used to store data for the undecoded embedded video frames.
/// Contains the data, the data size and the type of the frame
class VideoData {
public:
	VideoData(boost::shared_array<boost::uint8_t> data, boost::uint32_t size, media::videoFrameType ft)
		:
		videoData(data),
		dataSize(size),
		frameType(ft)
	{
	}

	~VideoData()
	{
	}

	boost::shared_array<boost::uint8_t> videoData;
	boost::uint32_t dataSize;
	media::videoFrameType frameType;
};

class video_stream_definition : public character_def
{
public:

	/// Construct a video stream definition with given ID
	//
	/// NOTE: for dynamically created definitions (ActionScript Video class instances)
	///       you can use an id of -1. See character_def constructor, as that's the
	///	  one which will eventually get passed the id.
	///
	video_stream_definition(boost::uint16_t char_id);

	~video_stream_definition();


	character* create_character_instance(character* parent, int id);

	/// Read tag SWF::DEFINEVIDEOSTREAM 
	//
	/// The character_id is assumed to have been already read by caller.
	///
	/// This function is allowed to be called only *once* for each
	/// instance of this class.
	///
	void readDefineVideoStream(stream* in, SWF::tag_type tag, movie_definition* m);

	/// Read tag SWF::VIDEOFRAME
	//
	/// The character_id (used to find this instance in the character's dictionary)
	/// is assumed to have been already read.
	///
	/// This function is allowed to be called zero or more times, as long
	/// as readDefineVideoStream was read before.
	///
	void readDefineVideoFrame(stream* in, SWF::tag_type tag, movie_definition* m);

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
	std::auto_ptr<image::image_base> get_frame_data(boost::uint32_t frameNum);

private:

	/// Id of this character definition, set by constructor.
	///
	/// The id is currently set to -1 when the definition is actually
	/// created dynamically (instantiating the ActionScript Video class)
	///
	boost::uint16_t m_char_id;

	/// Reserved flags read from DEFINEVIDEOSTREAM tag
	boost::uint8_t m_reserved_flags;

	/// Flags read from DEFINEVIDEOSTREAM tag
	boost::uint8_t m_deblocking_flags;

	/// Smoothing flag, as read from DEFINEVIDEOSTREAM tag
	bool m_smoothing_flags;

	/// Frame in which the DEFINEVIDEOSTREAM was found
	boost::uint16_t m_start_frame;

	/// Number of frames in the embedded video, as reported
	/// by the DEFINEVIDEOSTREAM tag
	///
	boost::uint16_t m_num_frames;

	/// Codec ID as read from DEFINEVIDEOSTREAM tag
	//
	/// 0: extern file
	/// 2: H.263
	/// 3: screen video (Flash 7+ only)
	/// 4: VP6
	///
	media::videoCodecType m_codec_id;

	/// Bounds of the video, as read from the DEFINEVIDEOSTREAM tag.
	rect m_bound;

	/// The undecoded video frames and its size, using the swf-frame number as key
	//
	/// Elements of this vector are owned by this instance, and will be deleted 
	/// at instance destruction time.
	///
#ifdef SOUND_GST
	typedef std::vector<GstBuffer*> EmbedFrameVec;
#elif defined(USE_FFMPEG)
	typedef std::vector<uint8_t*> EmbedFrameVec;
#endif

	EmbedFrameVec _video_frames;

	/// Last decoded frame number
	boost::int32_t _last_decoded_frame;

	/// Width of the video
	boost::uint32_t _width;

	/// Height of the video
	boost::uint32_t _height;

	/// The decoder used to decode the video frames
#ifdef SOUND_GST
	boost::scoped_ptr<media::VideoDecoderGst> _decoder;
#elif defined(USE_FFMPEG)
	boost::scoped_ptr<media::VideoDecoderFfmpeg> _decoder;
#endif
};

}	// end namespace gnash


#endif // GNASH_VIDEO_STREAM_DEF_H
