// DefineMorphShapeTag.h:   Load and parse morphing shapes, for Gnash.
//
//   Copyright (C) 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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
// Based on the public domain work of Mike Shaver <shaver@off.net> 2003,
// Vitalij Alexeev <tishka92@mail.ru> 2004.

#ifndef GNASH_SWF_MORPH_SHAPE_H
#define GNASH_SWF_MORPH_SHAPE_H

#include "SWF.h"
#include "ShapeRecord.h"
#include "DefinitionTag.h"

// Forward declarations.
namespace gnash {
    class movie_definition;
    class SWFStream;
	class RunResources;
    class MorphShape;
    class Renderer;
    class Transform;
}

namespace gnash {
namespace SWF {

/// DefineMorphShape tag
//
class DefineMorphShapeTag : public DefinitionTag
{
public:

    static void loader(SWFStream& in, TagType tag, movie_definition& m,
            const RunResources& r);

    virtual ~DefineMorphShapeTag() {}

	virtual DisplayObject* createDisplayObject(Global_as& gl,
            DisplayObject* parent) const;

    void display(Renderer& renderer, const ShapeRecord& shape,
            const Transform& base) const;

    const ShapeRecord& shape1() const { 
        return _shape1;
    }
    
    const ShapeRecord& shape2() const { 
        return _shape2;
    }

private:

    DefineMorphShapeTag(SWFStream& in, SWF::TagType tag, movie_definition& md,
            const RunResources& r, boost::uint16_t id);
    
    /// Read a DefineMorphShape tag from stream
    //
    /// Throw ParserException if the tag is malformed
    ///
    /// @param in
    ///	The stream to read the definition from.
    ///	Tag type is assumed to have been read already
    ///
    /// @param TagType
    ///	Type of the tag.
    ///	Need be SWF::DEFINEMORPHSHAPE or an assertion would fail.
    ///	TODO: drop ?
    ///
    /// @param md
    ///	Movie definition. Used to resolv DisplayObject ids for fill styles.
    ///	Must be not-null or would segfault. 
    ///
    void read(SWFStream& in, SWF::TagType tag, movie_definition& m,
            const RunResources& r);

    ShapeRecord _shape1;
    ShapeRecord _shape2;
    
    SWFRect _bounds;

};

} // namespace SWF
} // namespace gnash


#endif 

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
