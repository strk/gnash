// DefineButtonCxformTag.cpp: parse SWF2 DefineButtonCxform tag.
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

#include "RunResources.h"
#include "SWF.h" // for TagType definition
#include "SWFStream.h" // for inlines
#include "movie_definition.h"
#include "DefineButtonTag.h"
#include "DefineButtonCxformTag.h"
#include "utility.h"

namespace gnash {
namespace SWF {

void
DefineButtonCxformTag::loader(SWFStream& in, TagType tag, movie_definition& m,
        const RunResources& /*r*/)
{

    assert(tag == SWF::DEFINEBUTTONCXFORM);

    in.ensureBytes(2);
    const std::uint16_t buttonID = in.read_u16();

    IF_VERBOSE_PARSE (
        log_parse("DefineButtonCxformTag: ButtonId=%d", buttonID);
    );
    
    DefinitionTag* chdef = m.getDefinitionTag(buttonID);
    if (!chdef)
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("DefineButtonCxform refers to an unknown "
                "DisplayObject %d"), buttonID);
        );
        return;
    }

    DefineButtonTag* ch = 
        dynamic_cast<DefineButtonTag*> (chdef);
    if (!ch)
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("DefineButtonCxform refers to DisplayObject ID %d (%s)."
                  " Expected a button definition"),
                  buttonID, typeName(*chdef));
        );
        return;
    }
    
    DefineButtonTag::ButtonRecords& br = ch->buttonRecords();
    for (ButtonRecord& record : br)
    {
        // This will throw a parser exception if not enough bytes are
        // left.
        record.readRGBTransform(in);
    }
}


} // namespace gnash::SWF
} // namespace gnash


