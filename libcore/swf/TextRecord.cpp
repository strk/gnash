// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#include "TextRecord.h"
#include "SWFStream.h"
#include "movie_definition.h"
#include "swf.h"
#include "log.h"
#include <vector>

namespace gnash {
namespace SWF {    
    
bool
TextRecord::read(SWFStream& in, movie_definition& m, int glyphBits,
        int advanceBits, tag_type tag)
{
    in.ensureBytes(1);
    boost::uint8_t flags = in.read_u8();
		
    if (!flags)
    {
        // This is the end of the text records.
        IF_VERBOSE_PARSE(
            log_parse(_("end text records"));
        );
        return false;
    }

    bool hasFont = (flags >> 3) & 1;
    bool hasColor = (flags >> 2) & 1;
    _hasYOffset = (flags >> 1) & 1;
    _hasXOffset = (flags >> 0) & 1;

    if (hasFont)
    {
        in.ensureBytes(2);
        boost::uint16_t	fontID = in.read_u16();

        _font = m.get_font(fontID);
        if (!_font)
        {
            // What do we do now?
            IF_VERBOSE_PARSE(
                log_parse("Font not found.");
            );
        }
        else
        {
            IF_VERBOSE_PARSE(
                log_parse(_("  has_font: font id = %d (%p)"), fontID,
                    (void*)_font);
            );
        } 
    }

    if (hasColor)
    {
        if (tag == DEFINETEXT) _color.read_rgb(in);
        else _color.read_rgba(in);

        IF_VERBOSE_PARSE(
            log_parse(_("  hasColor"));
        );
    } 

    if (_hasXOffset)
    {
        in.ensureBytes(2);
        _xOffset = in.read_s16();
        IF_VERBOSE_PARSE(
            log_parse(_("  xOffset = %g"), _xOffset);
        );
    }

    if (_hasYOffset)
    {
        in.ensureBytes(2);
        _yOffset = in.read_s16();
        IF_VERBOSE_PARSE(
            log_parse(_("  _yOffset = %g"), _yOffset);
        );
    }

    if (hasFont)
    {
        in.ensureBytes(2);
        _textHeight = in.read_u16();
        IF_VERBOSE_PARSE(
            log_parse(_("  textHeight = %g"), _textHeight);
        );
    }

    in.ensureBytes(1);
    boost::uint8_t glyphCount = in.read_u8();
    if (!glyphCount) return false;

    IF_VERBOSE_PARSE(
        log_parse(_("  glyph_records: count = %d"), glyphCount);
    );

    in.ensureBits(glyphCount * (glyphBits + advanceBits));
    for (unsigned int i = 0; i < glyphCount; ++i)
    {
        GlyphEntry ge;
        ge.index = in.read_uint(glyphBits);
        ge.advance = static_cast<float>(in.read_sint(advanceBits));

        log_parse(_("   glyph%d: index=%d, advance=%g"), i,
            ge.index, ge.advance);

        _glyphs.push_back(ge);
    }

    // Continue parsing more records.
    return true;
}


} // namespace SWF
} // namespace gnash
