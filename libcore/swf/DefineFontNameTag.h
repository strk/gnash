// DefineFontNameTag.cpp: read a DefineFontName tag.
// 
//   Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#include "SWF.h"
#include "SWFStream.h"
#include "Font.h"
#include "RunResources.h"
#include "movie_definition.h"

#ifndef GNASH_SWF_DEFINEFONTNAMETAG_H
#define GNASH_SWF_DEFINEFONTNAMETAG_H

namespace gnash {
namespace SWF {

/// Process a DefineFontName tag.
//
/// This simple reads display name and copyright name, and seems
/// to have little real point.
class DefineFontNameTag
{
public:

    // Set font name for a font.
    static void loader(SWFStream& in, TagType tag, movie_definition& m,
            const RunResources& /*r*/)
    {
        assert(tag == SWF::DEFINEFONTNAME);

        in.ensureBytes(2);
        boost::uint16_t fontID = in.read_u16();

        Font* f = m.get_font(fontID);
        if (!f)
        {
            IF_VERBOSE_MALFORMED_SWF(
                log_swferror(_("define_font_name_loader: can't find "
                        "font with id %d"), fontID);
            );
            return;
        }

        Font::FontNameInfo fontName;
        in.read_string(fontName.displayName);
        in.read_string(fontName.copyrightName);

        f->addFontNameInfo(fontName);
    }

};

}
}

#endif
