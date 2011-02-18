// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#include <boost/assign/list_of.hpp>
#include <vector>

#include "TypesParser.h"
#include "SWFStream.h"
#include "SWFMatrix.h"
#include "SWFCxForm.h"
#include "smart_ptr.h"
#include "movie_definition.h"
#include "DisplayObject.h"
#include "SWF.h"
#include "log.h"
#include "Font.h"
#include "Renderer.h"


namespace gnash {
namespace SWF {    

bool
TextRecord::read(SWFStream& in, movie_definition& m, int glyphBits,
        int advanceBits, TagType tag)
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
                    _font.get());
            );
        } 
    }

    if (hasColor)
    {
        if (tag == DEFINETEXT) _color = readRGB(in);
        else _color = readRGBA(in);

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
        log_parse(_("  GlyphEntries: count = %d"),
            static_cast<int>(glyphCount));
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


///Render the given glyph records.
//
/// Display of device fonts is complicated in Gnash as we use the same
/// rendering process as for embedded fonts.
//
/// The shape and position of a font relies on the concatenated transformation
/// of its containing DisplayObject and all parent DisplayObjects.
//
/// Device fonts have the peculiarity that the glyphs are always scaled
/// equally in both dimensions, using the y scale only. However, indentation
/// and left margin (the starting x position) *are* scaled using the given
/// x scale. The translation is applied as normal.
//
/// The proprietary player does not display rotated or skewed device fonts.
/// Gnash does.
void
TextRecord::displayRecords(Renderer& renderer, const Transform& xform,
        const TextRecords& records, bool embedded)
{

    const SWFCxForm& cx = xform.colorTransform;
    const SWFMatrix& mat = xform.matrix;

    // Starting positions.
    double x = 0.0;
    double y = 0.0;

    for (TextRecords::const_iterator i = records.begin(), e = records.end();
            i !=e; ++i) {

        // Draw the DisplayObjects within the current record; i.e. consecutive
        // chars that share a particular style.
        const TextRecord& rec = *i;

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

        // If we are displaying a device font, we will not be applying the
        // matrix's x scale to each glyph. As the indentation and left
        // margin are affected by the x scale, we must set it manually here.
        // Worse, as we will be applying the y scale, we cancel that out
        // in advance.
        if (rec.hasXOffset()) x =
            embedded ? rec.xOffset() :
                rec.xOffset() * mat.get_x_scale() / mat.get_y_scale();

        if (rec.hasYOffset()) y = rec.yOffset();

        // Save for the underline, if any
        const boost::int16_t startX = x; 

        rgba textColor = cx.transform(rec.color());

        // Device fonts have no transparency.
        if (!embedded) textColor.m_a = 0xff;

        for (Glyphs::const_iterator j = rec.glyphs().begin(),
                je = rec.glyphs().end(); j != je; ++j) {

            const TextRecord::GlyphEntry& ge = *j;

            const int index = ge.index;
                
            SWFMatrix m;
            if (embedded) m = mat;
            else {
                // Device fonts adopt the concatenated translation.
                m.concatenate_translation(mat.tx(), mat.ty());
                // Device fonts have each glyph scaled in both dimensions
                // by the matrix's y scale.
                const double textScale = mat.get_y_scale();
                m.concatenate_scale(textScale, textScale);
            }

            m.concatenate_translation(x, y);
            m.concatenate_scale(scale, scale);

            if (index == -1) {

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
                static const std::vector<point> emptyCharBox =
                    boost::assign::list_of (point(32, 32))
                                           (point(480, 32))
                                           (point(480, -656))
                                           (point(32, -656))
                                           (point(32,32));
                renderer.drawLine(emptyCharBox, textColor, m);
#endif

            }
            else {
                ShapeRecord* glyph = fnt->get_glyph(index, embedded);

                // Draw the DisplayObject using the filled outline.
                if (glyph) renderer.drawGlyph(*glyph, textColor, m);
            }
            x += ge.advance;
        }

        if (rec.underline()) {
            // Underline should end where last displayed glyphs
            // does. 'x' here is where next glyph would be displayed
            // which is normally after some space.
            // For more precise metrics we should substract the advance
            // of last glyph and add the actual size of it.
            // This will only be known if a glyph was actually found,
            // or would be the size of the empty box (arbitrary size)
            //
            boost::int16_t endX = static_cast<boost::int16_t>(x); 

            // The underline is made to be some pixels below the baseline (0)
            // and scaled so it's further as font size increases.
            //
            // 1/4 the EM square offset far from baseline 
            boost::int16_t posY = int(y+int((unitsPerEM/4)*scale));

            const std::vector<point> underline = boost::assign::list_of
                (point(startX, posY))
                (point(endX, posY));

            renderer.drawLine(underline, textColor, mat);
        }
    }
}


} // namespace SWF
} // namespace gnash
