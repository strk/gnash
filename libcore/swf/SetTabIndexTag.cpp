// SetTabIndexTag.cpp: This tag defines the tab index of any text object (static and dynamic text objects.)
//
//   Copyright (C) 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "SetTabIndexTag.h"
#include "SWFStream.h"
#include "movie_definition.h"
#include "MovieClip.h"
#include "GnashNumeric.h"
#include "RunResources.h"

namespace gnash {
namespace SWF {

void
SetTabIndexTag::loader(SWFStream& in, TagType /*tag*/, movie_definition& /*md*/,
        const RunResources& /*r*/)
{
    in.ensureBytes(2);
    boost::uint16_t depth = in.read_u16();

    IF_VERBOSE_PARSE(
            log_parse("SetTabIndexTag: depth = %d", depth);
    );

    in.ensureBytes(2);
    boost::uint16_t tab_index = in.read_u16();
    UNUSED(tab_index);

    LOG_ONCE( log_unimpl("SetTabIndexTag") );

}

} // namespace SWF
} // namespace gnash

// Local Variables:
// mode: C++
// c-basic-offset: 8
// tab-width: 8
// indent-tabs-mode: t
// End:
