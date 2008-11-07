// DefineButtonCxformTag.cpp: parse SWF2 DefineButtonCxform tag.
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

#include "swf.h" // for tag_type definition
#include "SWFStream.h" // for inlines
#include "movie_definition.h"
#include "button_character_def.h"

namespace gnash {
namespace SWF {

void
DefineButtonCxformTag::loader(SWFStream& in, tag_type tag, movie_definition& m,
        const RunInfo& /*r*/)
{

    assert(tag == SWF::DEFINEBUTTONCXFORM);

    in.ensureBytes(2);
    const boost::uint16_t buttonID = in.read_u16();

    IF_VERBOSE_PARSE (
        log_debug("DefineButtonCxformTag: ButtonId=%d", buttonID);
    );
    
    character_def* chdef = m.get_character_def(buttonID);
    if (!chdef)
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("DefineButtonCxform refers to an unknown "
                "character %d"), buttonID);
        );
        return;
    }

    button_character_definition* ch = 
        dynamic_cast<button_character_definition*> (chdef);
    if (!ch)
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("DefineButtonCxform refers to character ID %d (%s)."
                  " Expected a button definition"),
                  buttonID, typeName(*chdef));
        );
        return;
    }
    
    button_character_definition::ButtonRecords& br = ch->buttonRecords();
    for (button_character_definition::ButtonRecords::iterator i = br.begin(),
            e = br.end(); i != e; ++i)
    {
        // This will throw a parser exception if not enough bytes are
        // left.
        (*i).m_button_cxform.read_rgb(in);
        IF_VERBOSE_PARSE(
            log_parse("Read DefineButtonCxform: %s", (*i).m_button_cxform);
        );
    }
}


} // namespace gnash::SWF
} // namespace gnash


