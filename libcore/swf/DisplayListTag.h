// DisplayListTag.h: DisplayList tag, for Gnash.
// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

#ifndef GNASH_SWF_DISPLAYLISTTAG_H
#define GNASH_SWF_DISPLAYLISTTAG_H

#include "ControlTag.h" // for inheritance


// Forward declarations
namespace gnash {
	class MovieClip;
}

namespace gnash {
namespace SWF {

/// DisplayList tag
//
/// DisplayList tags are all versions of PlaceObject and RemoveObject.
/// They modify the DisplayList of a movie or sprite by
/// placing, moving, replacing or removing DisplayObjects at depths.
///
///
class DisplayListTag : public ControlTag
{
public:

	DisplayListTag(int depth)
		:
		_depth(depth)
	{}

	virtual ~DisplayListTag() {}

    /// All DisplayList tags are state tags.
	virtual void executeState(MovieClip* m, DisplayList& dlist) const = 0;

	/// Return the depth affected by this DisplayList tag
	//
	/// NOTE: the returned depth is always in the
	///       static depth zone (DisplayObject::staticDepthOffset .. -1)
	int getDepth() const { return _depth; }

protected:

	int _depth;

};

} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_DISPLAYLISTTAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
