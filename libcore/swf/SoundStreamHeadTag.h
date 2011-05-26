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

#ifndef GNASH_SWF_SOUNDSTREAMHEAD_TAG_H
#define GNASH_SWF_SOUNDSTREAMHEAD_TAG_H


#include <boost/cstdint.hpp> 
 
#include "SWF.h"

// Forward declarations
namespace gnash {
    class SWFStream;
    class movie_definition;
    class RunResources;
}

namespace gnash {
namespace SWF {

class SoundStreamHeadTag
{
public:
    static void loader(SWFStream& in, TagType tag, movie_definition& m,
            const RunResources& r);
};

} // namespace SWF
} // namespace gnash


#endif 


// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
