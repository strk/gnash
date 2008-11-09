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
#include "SWFMatrix.h"
#include "cxform.h"
#include "movie_definition.h"
#include "character.h"
#include "swf.h"
#include "log.h"
#include "render.h"
#include "Font.h"

#include <vector>

namespace gnash {
namespace SWF {    

bool
TextRecord::read(SWFStream& in, movie_definition& m, int glyphBits,
        int advanceBits, tag_type tag)
{
    _glyphs.clear();

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
        boost::uint16_t fontID = in.read_u16();

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
            log_parse(_("  yOffset = %g"), _yOffset);
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
        log_parse(_("  GlyphEntries: count = %d"), glyphCount);
    );

    in.ensureBits(glyphCount * (glyphBits + advanceBits));
    for (unsigned int i = 0; i < glyphCount; ++i)
    {
        GlyphEntry ge;
        ge.index = in.read_uint(glyphBits);
        ge.advance = static_cast<float>(in.read_sint(advanceBits));
        _glyphs.push_back(ge);
        IF_VERBOSE_PARSE(
            log_parse(_("   glyph%d: index=%d, advance=%g"), i,
                ge.index, ge.advance);
        );
    }

    // Continue parsing more records.
    return true;
}


// Render the given glyph records.
void
TextRecord::displayRecords(const SWFMatrix& this_mat, character* inst,
    const std::vector<SWF::TextRecord>& records, bool embedded)
{

    SWFMatrix mat = inst->getWorldMatrix();
    mat.concatenate(this_mat);

    cxform cx = inst->get_world_cxform();
    SWFMatrix base_matrix = mat;

    // Starting positions.
    float x = 0.0f;
    float y = 0.0f;

    for (std::vector<TextRecord>::const_iterator i = records.begin(),
            e = records.end(); i !=e; ++i)
    {

        // Draw the characters within the current record; i.e. consecutive
        // chars that share a particular style.
        const TextRecord& rec = *i;

        // Used to pass a color on to shape_character::display()
        // FIXME: this isn't very good, especially the line style.
        static std::vector<fill_style> s_dummy_style(1, fill_style());    
        static std::vector<line_style> s_dummy_line_style;

        const Font* fnt = rec.getFont();
        if (!fnt) {
            IF_VERBOSE_MALFORMED_SWF(
                log_swferror("No font in style of TextRecord");
            );
            return;
        }

        // unitsPerEM returns an int, we cast to float to get
        // a float division
        const float unitsPerEM = fnt->unitsPerEM(embedded);
        const float scale = rec.textHeight() / unitsPerEM;

#ifdef GNASH_DEBUG_TEXT_RENDERING
        log_debug("font for TextRecord == %p" static_cast<void*>(fnt));
#endif

        if (rec.hasXOffset()) x = rec.xOffset();
        if (rec.hasYOffset()) y = rec.yOffset();

        boost::int16_t startX = x; // for the underline, if any

        s_dummy_style[0].set_color(rec.color());

        rgba textColor = cx.transform(rec.color());

        for (Glyphs::const_iterator j = rec.glyphs().begin(),
                je = rec.glyphs().end(); j != je; ++j)
        {
            // the glyph entry
            const SWF::TextRecord::GlyphEntry& ge = *j;

            const int index = ge.index;
                
            mat = base_matrix;
            mat.concatenate_translation(x, y);
            mat.concatenate_scale(scale, scale);

            if (index == -1)
            {
#ifdef GNASH_DEBUG_TEXT_RENDERING
    log_error(_("invalid glyph (-1)"));
#endif

#ifdef DRAW_INVALID_GLYPHS_AS_EMPTY_BOXES
                // The EM square is 1024x1024, but usually isn't filled up.
                // We'll use about half the width, and around 3/4 the height.
                // Values adjusted by eye.
                // The Y baseline is at 0; negative Y is up.
                //
                // TODO: FIXME (if we'll ever enable it back): the EM
                //       square is not hard-coded anymore but can be
                //       queried from the font class
                //
                static const boost::int16_t s_empty_char_box[5 * 2] =
                {
                     32,   32,
                    480,   32,
                    480, -656,
                     32, -656,
                     32,   32
                };
                render::draw_line_strip(s_empty_char_box, 5,
                        textColor, mat);  
#endif

            }
            else
            {
                shape_character_def* glyph = fnt->get_glyph(index, embedded);

                // Draw the character using the filled outline.
                if (glyph)
                {
#ifdef GNASH_DEBUG_TEXT_RENDERING
    log_debug(_("render shape glyph using filled outline (render::draw_glyph)"));
#endif

                    gnash::render::draw_glyph(glyph, mat,
                            textColor);
                }
            }
            x += ge.advance;
        }

        if (rec.underline())
        {
            // Underline should end where last displayed glyphs
            // does. 'x' here is where next glyph would be displayed
            // which is normally after some space.
            // For more precise metrics we should substract the advance
            // of last glyph and add the actual size of it.
            // This will only be known if a glyph was actually found,
            // or would be the size of the empty box (arbitrary size)
            //
            boost::int16_t endX = (int)x; // - rec.m_glyphs.back().m_glyph_advance + (480.0*scale);

            // The underline is made to be some pixels below the baseline (0)
            // and scaled so it's further as font size increases.
            //
            // 1/4 the EM square offset far from baseline 
            boost::int16_t posY = int(y+int((unitsPerEM/4)*scale));

            boost::int16_t underline[2 * 2] =
            {
                startX,   posY,
                  endX,   posY,
            };
            render::draw_line_strip(underline, 2, textColor,
                    base_matrix);
        }
    }
}


} // namespace SWF
} // namespace gnash
