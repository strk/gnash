// 
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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

#ifndef GNASH_SWF_SCRIPTLIMITSTAG_H
#define GNASH_SWF_SCRIPTLIMITSTAG_H

#include "swf.h" // for TagType definition
#include "SWFStream.h" // for inlines
#include "movie_definition.h"

namespace gnash {
namespace SWF {

/// \brief Sets the desired limits for recursion and timeout for AS scripts
//
/// A loaded movie containing a ScriptLimits tag should change the *global*
/// scriptlimits setting, so this is kept in movie_root rather than the
/// immutable movie_definition. Whenever this tag is parsed, the value in
/// movie_root is overridden.
class ScriptLimitsTag
{
public:

    static void loader(SWFStream& in, TagType tag, movie_definition& /*m*/,
            const RunInfo& /*r*/)
    {

        assert(VM::isInitialized());

        in.ensureBytes(4); // recursion and timeout.

        // We need to get the root movie or the VM from somewhere
        // in order to make the VM not a singleton.
        movie_root& r = VM::get().getRoot();

        const boost::uint16_t recursionLimit = in.read_u16();
        const boost::uint16_t timeoutLimit = in.read_u16();      

        IF_VERBOSE_PARSE (
            log_parse(_("  ScriptLimits tag(%d): recursion: %d, timeout: %d"),
                    tag, recursionLimit, timeoutLimit);
	    );

        r.setScriptLimits(recursionLimit, timeoutLimit);
    }
};

} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_SCRIPTLIMITSTAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
