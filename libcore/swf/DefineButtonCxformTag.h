// DefineButtonCxformTag.h: parse SWF2 button cxform tag.
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

#ifndef GNASH_SWF_DEFINEBUTTONCXFORMTAG_H
#define GNASH_SWF_DEFINEBUTTONCXFORMTAG_H

#include "swf.h" // for tag_type definition
#include "SWFStream.h" // for inlines
#include "movie_definition.h"
#include "button_character_def.h"

namespace gnash {
namespace SWF {

/// \brief Sets the desired limits for recursion and timeout for AS scripts
//
/// A loaded movie containing a ScriptLimits tag should change the *global*
/// scriptlimits setting, so this is kept in movie_root rather than the
/// immutable movie_definition. Whenever this tag is parsed, the value in
/// movie_root is overridden.
class DefineButtonCxformTag
{
public:
    static void loader(SWFStream& in, tag_type tag, movie_definition& m, const RunInfo& r)
    {

        assert(tag == SWF::DEFINEBUTTONCXFORM);

        in.ensureBytes(2);
        const boost::uint16_t buttonID = in.read_u16();

        IF_VERBOSE_PARSE (
            log_debug("DefineButtonCxformTag: ButtonId=%d", buttonID);
        );
        
        character_def* chdef = m.get_character_def(buttonID);
        if ( ! chdef )
        {
            IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("DefineButtonCxform refers to an unknown "
                    "character %d"), buttonID);
            );
            return;
        }

        button_character_definition* ch = 
            dynamic_cast<button_character_definition*> (chdef);
        if ( ! ch )
        {
            IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("DefineButtonCxform refers to character ID %d (%s)."
                      " Expected a button definition"),
                      buttonID, typeName(*chdef));
            );
            return;
        }

    ch->readDefineButtonCxform(in, m);
    }
};

} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_DEFINEBUTTONCXFORMTAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
