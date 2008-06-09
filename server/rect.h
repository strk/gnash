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


#ifndef GNASH_RECT_H
#define GNASH_RECT_H

#include "dsodefs.h"

#include "Range2d.h"
#include "Point2d.h"

#include <cassert> // for inlines
#include <iostream> // for output operator

// Forward decl
namespace gnash {
	class matrix;
	class SWFStream;
}

namespace gnash {

/// \brief
/// Rectangle class, basically a wrapper around a
/// gnash::geometry::Range2d<float> with a few
/// additional methods for reading it from the
/// SWFStream ..
//
/// used by render handler (?)
///
class DSOLOCAL rect
{
private:

	geometry::Range2d<float> _range;

public:

	/// Ouput operator
	friend std::ostream& operator<< (std::ostream& os, const rect& rect);

	/// Construct a NULL rectangle
	rect()
		:
		_range()
	{}

	/// Construct a rectangle with given coordinates
	rect(float xmin, float ymin, float xmax, float ymax)
		:
		_range(xmin, ymin, xmax, ymax)
	{
	}

	/// returns true if this is the NULL rectangle
	bool is_null() const
	{
		return _range.isNull();
	}

	/// returns true if this is the WORLD rectangle
	bool is_world() const
	{
		return _range.isWorld();
	}

	/// set the rectangle to the NULL value
	void set_null()
	{
		_range.setNull();
	}

	/// set the rectangle to the WORLD value
	void set_world()
	{
		_range.setWorld();
	}

	/// Read a bit-packed rectangle from an SWF stream
	//
	/// Format of the bit-packed rectangle is:
	///
	///     bits  | name  | description
	///     ------+-------+-------------------------
	///	  5   | nbits | number of bits used in subsequent values
	///	nbits | xmin  | minimum X value
	///	nbits | xmax  | maximum X value
	///	nbits | ymin  | minimum Y value
	///	nbits | ymax  | maximum Y value
	///
	/// If max values are less then min values the SWF is malformed;
	/// in this case this method will raise an swf_error and set the
	/// rectangle to the NULL rectangle. See is_null().
	///	
	///
	void	read(SWFStream& in);

	// TODO: drop this, currently here to avoid touching all callers
	void	read(SWFStream* in) { read(*in); }

	void	print() const;

	/// Return true if the specified point is inside this rect.
	bool	point_test(float x, float y) const
	{
		return _range.contains(x, y);
	}

	/// Expand this rectangle to enclose the given point.
	void	expand_to_point(float x, float y)
	{
		_range.expandTo(x, y);
	}
	
	/// Expand this rectangle to enclose the given circle.
	void	expand_to_circle(float x, float y, float radius)
	{
		_range.expandToCircle(x, y, radius);
	}

	/// Set ourself to bound the given point
	void	enclose_point(float x, float y)
	{
		_range.setTo(x, y);
	}

	/// Return width this rectangle
	float	width() const
	{
		return _range.width();
	}

	/// Return height this rectangle
	float	height() const
	{
		return _range.height();
	}

	/// Shift this rectangle horizontally
	//
	/// A positive offset will shift to the right,
	/// A negative offset will shift to the left.
	///
	void shift_x(float offset)
	{
		_range.shiftX(offset);
	}

	/// Shift this rectangle vertically
	//
	/// A positive offset will increment y values.
	/// A negative offset will decrement y values.
	///
	/// TODO: document what the orientation is supposed	
	///	  to be (up/down) ?
	///
	void shift_y(float offset)
	{
		_range.shiftY(offset);
	}

	/// Scale this rectangle horizontally
	//
	/// A positive factor will make the rectangle bigger.
	/// A negative factor will make the rectangle smaller.
	/// A factor of 1 will leave it unchanged.
	///
	void scale_x(float factor)
	{
		_range.scaleX(factor);
	}

	/// Scale this rectangle vertically
	//
	/// A positive factor will make the rectangle bigger.
	/// A negative factor will make the rectangle smaller.
	/// A factor of 1 will leave it unchanged.
	///
	void scale_y(float factor)
	{
		_range.scaleY(factor);
	}

	/// Get min X ordinate.
	//
	/// Don't call this against a null rectangle
	///
	float	get_x_min() const
	{
		return _range.getMinX();
	}

	/// Get max X ordinate.
	//
	/// Don't call this against a null rectangle
	///
	float	get_x_max() const
	{
		return _range.getMaxX();
	}

	/// Get min Y ordinate.
	//
	/// Don't call this against a null rectangle
	///
	float	get_y_min() const
	{
		return _range.getMinY();
	}

	/// Get max Y ordinate.
	//
	/// Don't call this against a null rectangle
	///
	float	get_y_max() const
	{
		return _range.getMaxY();
	}

	/// Get one of the rect verts.
	//
	/// Don't call on a NULL rect !
	///
	/// TODO: deprecate this ?
	point	get_corner(int i) const;

	/// \brief
	/// Make sure that the given point falls
	/// in this rectangle, modifying it's coordinates
	/// if needed.
	///
	/// Don't call against a NULL rectangle !
	///
	void clamp(point& p) const;

	/// Set ourself to bound a rectangle that has been transformed
	/// by m.  This is an axial bound of an oriented (and/or
	/// sheared, scaled, etc) box.
	void	enclose_transformed_rect(const matrix& m, const rect& r);
	
	/// Same as enclose_transformed_rect but expanding the current rect instead
	/// of replacing it.
	DSOEXPORT void	expand_to_transformed_rect(const matrix& m, const rect& r);
	
	/// Makes union of the given and the current rect
	DSOEXPORT void  expand_to_rect(const rect& r);

	void	set_lerp(const rect& a, const rect& b, float t);


	/// \brief
	/// Returns a const reference to the underlying
	/// Range2d object.
	const geometry::Range2d<float> & getRange() const
	{
		return _range;
	}

	/// Return a string representation for this rectangle
	std::string toString() const;
};

inline std::ostream&
operator<< (std::ostream& os, const rect& rect)
{
	return os << rect._range;
}

}	// namespace gnash

#endif // GNASH_RECT_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
