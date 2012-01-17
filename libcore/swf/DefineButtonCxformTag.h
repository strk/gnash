// DefineButtonCxformTag.h: parse SWF2 button SWFCxForm tag.
// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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

#ifndef GNASH_SWF_DEFINEBUTTONCXFORMTAG_H
#define GNASH_SWF_DEFINEBUTTONCXFORMTAG_H

#include "SWF.h" // for TagType definition

namespace gnash {
    class SWFStream;
    class movie_definition;
}

namespace gnash {
namespace SWF {
    
/// A simple rgb SWFCxForm for SWF2 buttons, superseded by DefineButton2.
//
/// The loader directly modifies a previous DefineButton tag.
/// TODO: should it also modify a DefineButton2 tag? Probably, but not
/// tested.
class DefineButtonCxformTag
{
public:

    static void loader(SWFStream& in, TagType tag, movie_definition& m,
            const RunResources& /*r*/);

};

} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_DEFINEBUTTONCXFORMTAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
