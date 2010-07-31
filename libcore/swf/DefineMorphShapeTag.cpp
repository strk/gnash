// DefineMorphShapeTag.cpp:   Load and parse morphing shapes, for Gnash.
//
//   Copyright (C) 2006, 2007, 2008, 2009, 2010 Free Software
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
//


// Based on the public domain morph.cpp of:
// Thatcher Ulrich <tu@tulrich.com>, Mike Shaver <shaver@off.net> 2003,
// Vitalij Alexeev <tishka92@mail.ru> 2004.

#include "DefineMorphShapeTag.h"
#include "MorphShape.h"
#include "SWFStream.h"
#include "movie_definition.h"
#include "MovieClip.h"
#include "GnashNumeric.h"
#include "RunResources.h"
#include "Global_as.h"
#include "Renderer.h"
#include "FillStyle.h"

namespace gnash {
namespace SWF {

void
DefineMorphShapeTag::loader(SWFStream& in, TagType tag, movie_definition& md,
        const RunResources& r)
{
    in.ensureBytes(2);
    const boost::uint16_t id = in.read_u16();

    IF_VERBOSE_PARSE(
            log_parse("DefineMorphShapeTag: id = %d", id);
    );

    DefineMorphShapeTag* morph = new DefineMorphShapeTag(in, tag, md, r, id);
    md.addDisplayObject(id, morph);
}

DefineMorphShapeTag::DefineMorphShapeTag(SWFStream& in, TagType tag,
        movie_definition& md, const RunResources& r, boost::uint16_t id)
    :
    DefinitionTag(id)
{
    read(in, tag, md, r);
}

DisplayObject*
DefineMorphShapeTag::createDisplayObject(Global_as& gl,
        DisplayObject* parent) const
{
    return new MorphShape(getRoot(gl), 0, this, parent);
}

void
DefineMorphShapeTag::display(Renderer& renderer, const MorphShape& inst) const
{
    renderer.drawShape(inst.shape(), inst.get_world_cxform(),
            inst.getWorldMatrix());
}


void
DefineMorphShapeTag::read(SWFStream& in, TagType tag, movie_definition& md,
        const RunResources& r)
{
    assert(tag == DEFINEMORPHSHAPE
        || tag == DEFINEMORPHSHAPE2
        || tag == DEFINEMORPHSHAPE2_);

    SWFRect bounds1, bounds2;
    bounds1.read(in);
    bounds2.read(in);

    if (tag == DEFINEMORPHSHAPE2 || tag == DEFINEMORPHSHAPE2_) {
        // TODO: Use these values.
        SWFRect innerBound1, innerBound2;
        innerBound1.read(in);
        innerBound2.read(in);

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
    
    for (size_t i = 0; i < fillCount; ++i) {
        OptionalFillPair fp = readFills(in, tag, md, true);
        _shape1.addFillStyle(fp.first);
        _shape2.addFillStyle(*fp.second);
    }

    const boost::uint16_t lineCount = in.read_variable_count();
    LineStyle ls1, ls2;
    for (size_t i = 0; i < lineCount; ++i) {
        ls1.read_morph(in, tag, md, r, &ls2);
        _shape1.addLineStyle(ls1);
        _shape2.addLineStyle(ls2);
    }

    _shape1.read(in, tag, md, r);
    in.align();
    _shape2.read(in, tag, md, r);

    // Set bounds as read in *this* tags rather then
    // the one computed from ShapeRecord parser
    // (does it make sense ?)
    _shape1.setBounds(bounds1);
    _shape2.setBounds(bounds2);
    
    // Starting bounds are the same as shape1
    _bounds = bounds1;

    assert(_shape1.fillStyles().size() == _shape2.fillStyles().size());
    assert(_shape1.lineStyles().size() == _shape2.lineStyles().size());

#if 0

    const unsigned edges1 = PathList::computeNumberOfEdges(_shape1.paths());
    const unsigned edges2 = PathList::computeNumberOfEdges(_shape2.paths());

    IF_VERBOSE_PARSE(
      log_parse("morph: "
          "startShape(paths:%d, edges:%u), "
          "endShape(paths:%d, edges:%u)",
          _shape1.paths().size(), edges1,
          _shape2.paths().size(), edges2);
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

} // namespace SWF
} // namespace gnash

// Local Variables:
// mode: C++
// c-basic-offset: 8
// tab-width: 8
// indent-tabs-mode: t
// End:
