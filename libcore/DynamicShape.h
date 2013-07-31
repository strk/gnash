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



#ifndef GNASH_DYNAMIC_SHAPE_H
#define GNASH_DYNAMIC_SHAPE_H

#include <vector>
#include "LineStyle.h" 
#include "ShapeRecord.h"

namespace gnash {
    class DisplayObject;
    class Renderer;
    class FillStyle;
    class GradientRecord;
    class Transform;
}

namespace gnash {

/// The DynamicShape class represents a mutable shape.
//
/// It is provides mutating functions for the SWF::ShapeRecord class that
/// are used in the Flash drawing API.
//
/// DynamicShape objects are not refcounted, so must be stack-allocated or
/// wrapped in smart pointers.
class DynamicShape
{
public:

	DynamicShape();

	~DynamicShape() {}

	/// Remove all paths and style informations
	void clear();

	/// Move pen to given coordinates
	void moveTo(boost::int32_t x, boost::int32_t y);

	/// Draw a straight line from current position to given one
	void lineTo(boost::int32_t x, boost::int32_t y, int swfVersion);

	/// \brief
	/// Draw a curve from current position to given one
	/// using given control points.
	void curveTo(boost::int32_t cx, boost::int32_t cy, 
                 boost::int32_t ax, boost::int32_t ay, int swfVersion);

	/// Start drawing with a solid fill
	void beginFill(const FillStyle& f);

	/// Close an existing filled path, if any.
	void endFill();

    const SWFRect& getBounds() const {
        return _shape.getBounds();
    }

    void setBounds(const SWFRect& bounds) {
        _shape.setBounds(bounds);
    }

    /// Display a DynamicShape object.
    void display(Renderer& renderer, const Transform& xform) const;

	/// Set current line style and start a new path.
	//
	/// @param thickness
	/// @param color
	/// @param vScale
	/// @param hScale
	/// @param noClose
	/// @param startCapStyle
	/// @param endCapStyle
	/// @param joinStyle
	/// @param miterLimitFactor
	void lineStyle(boost::uint16_t thickness, const rgba& color,
		bool vScale=true, bool hScale=true,
		bool pixelHinting=false,
		bool noClose=false,
		CapStyle startCapStyle=CAP_ROUND,
		CapStyle endCapStyle=CAP_ROUND,
		JoinStyle joinStyle=JOIN_ROUND,
		float miterLimitFactor=1.0f);

	/// Reset line style to no style and start a new path.
	void resetLineStyle();

	/// \brief
	/// Add a fill style, possibly reusing an existing
	/// one if existent.
	//
	/// @return the 1-based offset of the fill style,
	///	either added or found.
	///	This offset is the one required to properly
	///	reference it in gnash::path instances.
	///
	size_t addFillStyle(const FillStyle& stl);

	/// \brief
	/// Add a line style, possibly reusing an existing
	/// one if existent.
	//
	/// @return the 1-based offset of the line style,
	///	either added or found.
	///	This offset is the one required to properly
	///	reference it in gnash::path instances.
	///
	size_t add_line_style(const LineStyle& stl);

	// Override from DefineShapeTag to call ::finalize
	// NOTE: this is not correct in that a call to hitTest should
	//       not force closing the path being drawn.
	//       Instead, the closeup should be "temporary" and in
	//       the pointTestLocal itself (but only for dynamic drawing).
	//       We need to add a testcase for this as we currently have none.
	//       The testcase would look like this:
	//
	//       moveTo(0, 0); lineTo(10, 0); lineTo(10, 10); // an L shape so far
	//       hitTest(8, 2, true); !hitTest(2, 8, true); // imaginarly forming a closed triangle as hitTest is concerned
	//       lineTo(0, 10); lineTo(0, 0); // explicitly closed as a square now
	//       hitTest(8, 2, true); hitTest(2, 8, true); // effectively forming a closed square
	//
	//       In the test above, permanently closing on hit-test (what this implementation does)
	//       would result in a triangle and a stroke, which should fail the last hitTest(2,8).
	//
	//
	bool pointTestLocal(boost::int32_t x, boost::int32_t y,
            const SWFMatrix& wm) const
	{
		finalize();
		return _shape.pointTest(x, y, wm);
	}

    const SWF::ShapeRecord& shapeRecord() const {
        return _shape;
    }

	/// Add a path, updating _currpath and recomputing bounds
	//
	/// TODO: make private? Only current user is BitmapMovieDefinition.
	///       It needs this function unless we provide a mean to add a
	///	  Bitmap-Filled path	
	///
	void add_path(const Path& pth);

	/// Always call this function before displaying !
	//
	/// It will take care of cleaning up the drawing
	/// and setting up correct fill styles
	void finalize() const;

private:

	/// Initialize a new path
	//
	/// Used when changing style or moving the pen
	///
	/// The newly added path will use current values
	/// for origin, fill and line styles.
	///
	/// If newShape is true the new shape will start a new subshape.
	void startNewPath(bool newShape);



	Path* _currpath;

	size_t _currfill;

	size_t _currline;

	// Current pen X position
	boost::int32_t  _x;

	// Current pen Y position
	boost::int32_t  _y;

	mutable bool _changed;

	mutable SWF::Subshape _currsubshape;

    /// The actual SWF::ShapeRecord wrapped by this class.
    //
    /// Mutable for lazy finalization.
    mutable SWF::ShapeRecord _shape;
};

}	// end namespace gnash


#endif // GNASH_DYNAMIC_SHAPE_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
