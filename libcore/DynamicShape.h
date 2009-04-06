// 
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "shape_character_def.h"  // for inheritance
#include "styles.h" // for cap_style_e and join_style_e enums


namespace gnash {
    class fill_style;
}

namespace gnash {

/// \brief
/// Represents the outline of one or more shapes, along with
/// information on fill and line styles.
class DynamicShape : public shape_character_def
{
public:

	DynamicShape();

	virtual ~DynamicShape() {}

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
	void beginFill(const rgba& color);

	/// Start drawing with a linear gradient fill
	void beginLinearGradientFill(const std::vector<gradient_record>& grad, const SWFMatrix& mat);

	/// Start drawing with a radial gradient fill
	void beginRadialGradientFill(const std::vector<gradient_record>& grad, const SWFMatrix& mat);

	/// Close an existing filled path, if any.
	void endFill();

	/// Set current line style and start a new path.
	//
	/// @param thickness
	///
	/// @param color
	///
	/// @param vScale
	///
	/// @param hScale
	///
	/// @param noClose
	///
	/// @param startCapStyle
	///
	/// @param endCapStyle
	///
	/// @param joinStyle
	///
	/// @param miterLimitFactor
	///
	void lineStyle(boost::uint16_t thickness, const rgba& color,
		bool vScale=true, bool hScale=true,
		bool pixelHinting=false,
		bool noClose=false,
		cap_style_e startCapStyle=CAP_ROUND,
		cap_style_e endCapStyle=CAP_ROUND,
		join_style_e joinStyle=JOIN_ROUND,
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
	size_t add_fill_style(const fill_style& stl);

	/// \brief
	/// Add a line style, possibly reusing an existing
	/// one if existent.
	//
	/// @return the 1-based offset of the line style,
	///	either added or found.
	///	This offset is the one required to properly
	///	reference it in gnash::path instances.
	///
	size_t add_line_style(const line_style& stl);

	// Override from shape_character_def to call ::finalize
	// NOTE: this is not correct in that a call to hitTest should
	//       not force closing the path being drawn.
	//       Instead, the closeup should be "temporary" and int
	//       the point_test_local itself (but only for dynamic drawing).
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
	bool point_test_local(boost::int32_t x, boost::int32_t y,
            const SWFMatrix& wm)
	{
		finalize();
		return geometry::pointTestLocal(_paths, _line_styles, x, y, wm);
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
	void finalize();

private:

	/// Initialize a new path
	//
	/// Used when changing style or moving the pen
	///
	/// The newly added path will use current values
	/// for origin, fill and line styles.
	///
	/// If newShape is true the new shape will start a new subshape.
	///
	void startNewPath(bool newShape);

	Path* _currpath;

	size_t _currfill;

	size_t _currline;

	// Current pen X position
	boost::int32_t  _x;

	// Current pen Y position
	boost::int32_t  _y;

	// Call this function when the drawing changes !
	void changed() { _changed = true; }

	bool _changed;
};

}	// end namespace gnash


#endif // GNASH_DYNAMIC_SHAPE_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
