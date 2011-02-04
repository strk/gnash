// RemoveObjectTag.h: RemoveObject* tag for Gnash.
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

#ifndef GNASH_SWF_REMOVEOBJECTTAG_H
#define GNASH_SWF_REMOVEOBJECTTAG_H

#include "DisplayListTag.h" // for inheritance
#include "SWF.h" // for TagType definition

// Forward declarations
namespace gnash {
	class SWFStream;
	class MovieClip;
	class swf_event;
	class movie_definition;
	class DisplayList;
    class RunResources;
}

namespace gnash {
namespace SWF {

/// SWF Tag RemoveObject (5) or RemoveObject2 (28)
//
/// The RemoveObject tag removes the DisplayObject instance at the
/// specified depth.
class RemoveObjectTag : public DisplayListTag
{
public:

	RemoveObjectTag()
		:
		DisplayListTag(-1)
	{}

	/// Read SWF::REMOVEOBJECT or SWF::REMOVEOBJECT2 
	void read(SWFStream& in, TagType tag);

	/// Remove object at specified depth from MovieClip DisplayList.
	void executeState(MovieClip* m, DisplayList& dlist) const;

	static void loader(SWFStream& in, TagType tag, movie_definition& m,
            const RunResources& r);

private:

    int _id;

};

} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_REMOVEOBJECTTAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
