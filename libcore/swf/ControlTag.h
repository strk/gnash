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

#ifndef GNASH_CONTROL_TAG_H
#define GNASH_CONTROL_TAG_H

#include <boost/noncopyable.hpp>
#include "ref_counted.h"

// Forward declarations
namespace gnash {
    class MovieClip;
    class DisplayList;
}

namespace gnash {
namespace SWF {

/// Control tags are swf tags that control the operation of the movie. 
//
/// These are the events associated with a frame.
///
/// TODO: check whether this needs to be ref_counted. They are generally owned
/// by a sprite_definition or a SWFMovieDefinition.
//
/// TODO: rename this class so it's not the same as the SWF spec. It doesn't
/// exactly correspond to the ControlTag defined there.
class ControlTag : public ref_counted, boost::noncopyable
{
public:
    
    /// Type of ControlTag
    enum Type
    {
        TAG_ACTION = 1 << 0,
        TAG_DLIST  = 1 << 1
    };

	virtual ~ControlTag()
	{
	}

    /// Execute Action tags
    //
    /// Not all tags that have ActionScript code are considered Action tags.
    virtual void executeActions(MovieClip* /*m*/, DisplayList& /*dlist*/) const
	{
	}

	/// Execute "state" or "DisplayList" tags
	//
	/// State tags exist to control the state of MovieClips along the timeline.
    /// They are executed even for skipped frames so that the state is
    /// consistent at each frame. Some tags are considered state tags even
    /// though they only contain ActionScript, e.g. the DoInitAction tag.
	virtual void executeState(MovieClip* /*m*/,  DisplayList& /*dlist*/) const
	{
	}

};

} // namespace SWF
} // namespace gnash

#endif 


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
