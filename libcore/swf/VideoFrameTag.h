// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#ifndef GNASH_SWF_VIDEOFRAMETAG_H
#define GNASH_SWF_VIDEOFRAMETAG_H

#include "DefinitionTag.h"
#include "movie_definition.h"
#include "SWF.h"
#include "SWFRect.h" // for composition
#include "ControlTag.h"
#include "VideoDecoder.h"

#include <memory> // for auto_ptr

namespace gnash {
    // Forward declarations
    class SWFStream;
}

namespace gnash {
namespace SWF {

class VideoFrameTag 
{
public:

	/// Read tag SWF::VIDEOFRAME
	//
	/// The DisplayObject_id (used to find this instance in the DisplayObject's
    /// dictionary) is assumed to have been already read.
	///
	/// This function is allowed to be called zero or more times, as long
	/// as a DefineVideoStreamTag was read before.
	static void loader(SWFStream& in, SWF::TagType tag, movie_definition& m,
            const RunResources& r);

};

} // namespace SWF
} // namespace gnash


#endif // GNASH_VIDEO_STREAM_DEF_H
