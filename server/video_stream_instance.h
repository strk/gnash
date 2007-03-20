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
// $Id: video_stream_instance.h,v 1.9 2007/03/20 15:01:20 strk Exp $

#ifndef GNASH_VIDEO_STREAM_INSTANCE_H
#define GNASH_VIDEO_STREAM_INSTANCE_H

#include "character.h" // for inheritance
#include "video_stream_def.h"
#include "embedVideoDecoder.h"
#include "snappingrange.h"

// Forward declarations
namespace gnash {
	class NetStream;
}

namespace gnash {

class video_stream_instance : public character
{

public:

	video_stream_definition*	m_def;
	
	// m_video_source - A Camera object that is capturing video data or a NetStream object.
	// To drop the connection to the Video object, pass null for source.
	// FIXME: don't use as_object, but a more meaningful type
	as_object* m_video_source;

	video_stream_instance(video_stream_definition* def,
			character* parent, int id);

	~video_stream_instance();

	virtual void	advance(float delta_time);
	void	display();

	void add_invalidated_bounds(InvalidatedRanges& ranges, bool force);

	/// Set the input stream for this video
	void setStream(boost::intrusive_ptr<NetStream> ns);

private:

	// Who owns this ? Should it be an intrusive ptr ?
	boost::intrusive_ptr<NetStream> _ns;

	embedVideoDecoder* m_decoder;
};

}	// end namespace gnash


#endif // GNASH_VIDEO_STREAM_INSTANCE_H
