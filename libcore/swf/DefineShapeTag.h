// shape.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Quadratic bezier outline shapes, the basis for most SWF rendering.


#ifndef GNASH_SHAPE_CHARACTER_DEF_H
#define GNASH_SHAPE_CHARACTER_DEF_H

#include "DefinitionTag.h" // for inheritance of DefineShapeTag
#include "SWF.h"
#include "ShapeRecord.h"

namespace gnash {
	class SWFStream;
	class cxform;
    class Shape;
	class SWFMatrix;
	class RunResources;
	class Renderer;
}

namespace gnash {
namespace SWF {

/// \brief
/// Represents the outline of one or more shapes, along with
/// information on fill and line styles.
class DefineShapeTag : public DefinitionTag
{
public:

    static void loader(SWFStream& in, TagType tag, movie_definition& m,
            const RunResources& r);

    virtual ~DefineShapeTag() {};

    // Display a Shape character.
    virtual void display(Renderer& renderer, const DisplayObject& inst) const;

    // Create a Shape DisplayObject.
	virtual DisplayObject* createDisplayObject(Global_as& gl,
            DisplayObject* parent) const;
	
    /// Get cached bounds of this shape.
    const SWFRect& bounds() const { return _shape.getBounds(); }

    virtual bool pointTestLocal(boost::int32_t x, boost::int32_t y, 
            const SWFMatrix& wm) const;

protected:

private:

    DefineShapeTag(SWFStream& in, TagType tag, movie_definition& m,
            const RunResources& r, boost::uint16_t id);

    /// The actual shape data is stored in this record.
    const ShapeRecord _shape;

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
