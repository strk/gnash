// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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


/* $Id: DynamicShape.h,v 1.5 2007/07/01 10:54:18 bjacques Exp $ */

#ifndef GNASH_DYNAMIC_SHAPE_H
#define GNASH_DYNAMIC_SHAPE_H


#include "shape_character_def.h"  // for inheritance


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
	void moveTo(float x, float y);

	/// Draw a straight line from current position to given one
	void lineTo(float x, float y);

	/// \brief
	/// Draw a curve from current position to given one
	/// using given control points.
	void curveTo(float cx, float cy, float ax, float ay);

	/// Start drawing with a solid fill
	void beginFill(const rgba& color);

	/// Close an existing filled path, if any.
	void endFill();

	/// Set current line style and start a new path.
	void lineStyle(uint16_t thickness, const rgba& color);

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

	/// Add a path, updating _currpath and recomputing bounds
	//
	/// TODO: make private? Only current user is BitmapMovieDefinition.
	///       It needs this function unless we provide a mean to add a
	///	  Bitmap-Filled path	
	///
	void add_path(const path& pth);

	/// Always call this function before displaying !
	//
	/// It will take care of cleaning up the drawing
	/// and setting up correct fill styles
	void finalize();

private:

	// Initialize a new path, used when changing
	// style or moving the pen
	//
	// The newly added path will use current values
	// for origin, fill and line styles.
	//
	void startNewPath();

	path* _currpath;

	size_t _currfill;

	size_t _currline;

	// Current pen X position
	float _x;

	// Current pen Y position
	float _y;

	// Call this function when the drawing changes !
	void changed() {
		_changed = true;
	}

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
