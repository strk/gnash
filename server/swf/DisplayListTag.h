// DisplayListTag.h: DisplayList tag, for Gnash.
// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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
//
//

/* $Id: DisplayListTag.h,v 1.7 2007/12/04 11:45:33 strk Exp $ */

#ifndef GNASH_SWF_DISPLAYLISTTAG_H
#define GNASH_SWF_DISPLAYLISTTAG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ControlTag.h" // for inheritance
#include "swf.h" // for tag_type definition


// Forward declarations
namespace gnash {
	class stream;
	class sprite_instance;
	class swf_event;
	class movie_definition;
}

namespace gnash {
namespace SWF {

/// DisplayList tag
//
/// DisplayList tags are all versions of PlaceObject and RemoveObject.
/// They modify the DisplayList of a movie or sprite by
/// placing, moving, replacing or removing characters at depths.
///
///
class DisplayListTag : public ControlTag
{
public:

	DisplayListTag(int depth)
		:
		m_depth(depth)
	{}

	virtual ~DisplayListTag() {}

	virtual void execute(sprite_instance* m) const=0;

	void execute_state(sprite_instance* m) const
	{
		execute(m);
	}

	/// Return true if this tag removes a character
	virtual bool isRemove() const { return false; }

	/// Return true if this tag places a character
	virtual bool isPlace() const { return false; }

	/// Return true if this tag replaces a character
	virtual bool isReplace() const { return false; }

	/// Return true if this tag transforms a character
	virtual bool isMove() const { return false; }

	/// Return the depth affected by this DisplayList tag
	//
	/// NOTE: the returned depth is always in the
	///       static depth zone (character::staticDepthOffset .. -1)
	///
	int getDepth() const { return m_depth; }

protected:

	int m_depth;

};

} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_DISPLAYLISTTAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
