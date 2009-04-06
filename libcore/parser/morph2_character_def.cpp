// morph_character_def.cpp:   Load and render morphing shapes, for Gnash.
//
//   Copyright (C) 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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


// Based on the public domain morph.cpp of:
// Thatcher Ulrich <tu@tulrich.com>, Mike Shaver <shaver@off.net> 2003,
// Vitalij Alexeev <tishka92@mail.ru> 2004.

#include "morph2_character_def.h"
#include "MorphShape.h"
#include "SWFStream.h"
#include "render.h"
#include "movie_definition.h"
#include "MovieClip.h"
#include "GnashNumeric.h"

namespace gnash {

/// Functors for path and style manipulation.
morph_character_def::morph_character_def()
    :
    _shape1(new shape_character_def),
    _shape2(new shape_character_def)
{
}

DisplayObject*
morph_character_def::createDisplayObject(DisplayObject* parent, int id)
{
    return new MorphShape(this, parent, id);
}

void
morph_character_def::display(const MorphShape& inst)
{
    // display
    render::drawMorph(*this, inst);
}


void morph_character_def::read(SWFStream& in, SWF::TagType tag,
        movie_definition& md)
{
    assert(tag == SWF::DEFINEMORPHSHAPE
        || tag == SWF::DEFINEMORPHSHAPE2
        || tag == SWF::DEFINEMORPHSHAPE2_);

    rect bound1, bound2;
    bound1.read(in);
    bound2.read(in);

    if (tag == SWF::DEFINEMORPHSHAPE2 || tag == SWF::DEFINEMORPHSHAPE2_) {
        // TODO: Use these values.
        rect inner_bound1, inner_bound2;
        inner_bound1.read(in);
        inner_bound2.read(in);
        // This should be used -- first 6 bits reserved, then
        // 'non-scaling' stroke, then 'scaling' stroke -- these can be
        // used to optimize morphing.
        
        in.ensureBytes(1);
        static_cast<void>(in.read_u8());
    }

    in.ensureBytes(4);
    // Offset. What is this for?
    static_cast<void>(in.read_u32());

    // Next line will throw ParserException on malformed SWF
    const boost::uint16_t fillCount = in.read_variable_count();
    
    fill_style fs1, fs2;
    for (size_t i = 0; i < fillCount; ++i) {
        fs1.read(in, tag, md, &fs2);
        _shape1->addFillStyle(fs1);
        _shape2->addFillStyle(fs2);
    }

    const boost::uint16_t lineCount = in.read_variable_count();
    line_style ls1, ls2;
    for (size_t i = 0; i < lineCount; ++i) {
        ls1.read_morph(in, tag, md, &ls2);
        _shape1->addLineStyle(ls1);
        _shape2->addLineStyle(ls2);
    }

    _shape1->read(in, tag, false, md);
    in.align();
    _shape2->read(in, tag, false, md);

    // Set bounds as read in *this* tags rather then
    // the one computed from shape_character_def parser
    // (does it make sense ?)
    _shape1->set_bound(bound1);
    _shape2->set_bound(bound2);
    
    // Update bounds.
    // TODO: is this necessary?
    _bounds.expand_to_rect(_shape1->get_bound());
    _bounds.expand_to_rect(_shape2->get_bound());

    assert(_shape1->fillStyles().size() == _shape2->fillStyles().size());
    assert(_shape1->lineStyles().size() == _shape2->lineStyles().size());

#if 0
    const unsigned edges1 = PathList::computeNumberOfEdges(_shape1->paths());
    const unsigned edges2 = PathList::computeNumberOfEdges(_shape2->paths());

    IF_VERBOSE_PARSE(
      log_parse("morph: "
          "startShape(paths:%d, edges:%u), "
          "endShape(paths:%d, edges:%u)",
          _shape1->paths().size(), edges1,
          _shape2->paths().size(), edges2);
    );

    IF_VERBOSE_MALFORMED_SWF(
        // It is perfectly legal to have a different number of paths,
        // edges count should be the same instead
        if (edges1 != edges2) {
            log_swferror(_("Different number of edges "
                "in start (%u) and end (%u) shapes "
                "of a morph"), edges1, edges1);
        }

    );
#endif
}

} // namespace gnash

// Local Variables:
// mode: C++
// c-basic-offset: 8
// tab-width: 8
// indent-tabs-mode: t
// End:
