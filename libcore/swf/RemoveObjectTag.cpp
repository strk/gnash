// RemoveObjectTag.cpp: RemoveObject* tag for Gnash.
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
//


#include "RunResources.h"
#include "RemoveObjectTag.h"
#include "MovieClip.h"
#include "SWF.h" // for TagType definition
#include "log.h"
#include "SWFStream.h"
#include "smart_ptr.h"

namespace gnash {
namespace SWF {

void
RemoveObjectTag::read(SWFStream& in, TagType tag)
{
	assert(tag == SWF::REMOVEOBJECT || tag == SWF::REMOVEOBJECT2);

	if (tag == SWF::REMOVEOBJECT) {
		// Older SWFs allow multiple objects at the same depth;
		// this m_id disambiguates. Later SWFs just use one
		// object per depth.
        //
        // NB Gnash has never used this!
		in.ensureBytes(2);
		_id = in.read_u16();
	}

    in.ensureBytes(2);
	_depth = in.read_u16() + DisplayObject::staticDepthOffset;
}

void
RemoveObjectTag::executeState(MovieClip* m, DisplayList& dlist) const
{
    m->set_invalidated();
    dlist.removeDisplayObject(_depth);
}

/* public static */
void
RemoveObjectTag::loader(SWFStream& in, TagType tag, movie_definition& m,
        const RunResources& /*r*/)
{
    assert(tag == SWF::REMOVEOBJECT || tag == SWF::REMOVEOBJECT2);

    boost::intrusive_ptr<RemoveObjectTag> t(new RemoveObjectTag);
    t->read(in, tag);

    const int depth = t->getDepth();

    IF_VERBOSE_PARSE(
        log_parse(_("  remove_object_2(%d)"), depth);
    );

    // Ownership transferred to movie_definition
    m.addControlTag(t);
}

} // namespace gnash::SWF
} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
