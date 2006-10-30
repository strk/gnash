// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// 
//

/* $Id: rect.h,v 1.9 2006/10/30 18:16:37 nihilus Exp $ */

#ifndef GNASH_RECT_H
#define GNASH_RECT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"

#include <cassert> // for inlines

// Forward decl
namespace gnash {
	class matrix;
	class stream;
	class point; // is a forward declaration enough for a return type ?
}

namespace gnash {

/// Rectangle class
//
/// used by render handler (?)
///
class DSOLOCAL rect
{
private:

	// TODO: make private 
	float	m_x_min, m_x_max, m_y_min, m_y_max;

public:

	/// Construct a NULL rectangle
	DSOEXPORT  rect();

	/// Construct a rectangle with given coordinates
	rect(float xmin, float ymin, float xmax, float ymax)
		:
		m_x_min(xmin),
		m_x_max(xmax),
		m_y_min(ymin),
		m_y_max(ymax)
	{
		// use the default ctor to make a NULL rect
		assert(m_x_min <= m_x_max);
		assert(m_y_min <= m_y_max);
		// .. or should we raise an exception .. ?
	}

	/// returns true if this is the NULL rectangle
	DSOEXPORT bool is_null() const;

	/// set the rectangle to the NULL value
	void set_null();

	void	read(stream* in);
	void	print() const;
	bool	point_test(float x, float y) const;

	/// Expand this rectangle to enclose the given point.
	void	expand_to_point(float x, float y);

	/// Set ourself to bound the given point
	void	enclose_point(float x, float y);

	/// Return width this rectangle
	float	width() const
	{
		if ( is_null() ) return 0;
		return m_x_max-m_x_min;
	}

	/// Return height this rectangle
	float	height() const
	{
		if ( is_null() ) return 0;
		return m_y_max-m_y_min;
	}

	/// Shift this rectangle horizontally
	//
	/// A positive offset will shift to the right,
	/// A negative offset will shift to the left.
	///
	void shift_x(float offset)
	{
		if ( is_null() ) return;
		m_x_min += offset;
		m_x_max += offset;
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
		if ( is_null() ) return;
		m_y_min += offset;
		m_y_max += offset;
	}

	/// Scale this rectangle horizontally
	//
	/// A positive factor will make the rectangle bigger.
	/// A negative factor will make the rectangle smaller.
	/// A factor of 1 will leave it unchanged.
	///
	void scale_x(float factor)
	{
		if ( is_null() ) return;
		m_x_min *= factor;
		m_x_max *= factor;
	}

	/// Scale this rectangle vertically
	//
	/// A positive factor will make the rectangle bigger.
	/// A negative factor will make the rectangle smaller.
	/// A factor of 1 will leave it unchanged.
	///
	void scale_y(float factor)
	{
		if ( is_null() ) return;
		m_y_min *= factor;
		m_y_max *= factor;
	}

	/// Get min X ordinate.
	//
	/// Don't call this against a null rectangle
	///
	float	get_x_min() const
	{
		assert( ! is_null() );
		return m_x_min;
	}

	/// Get max X ordinate.
	//
	/// Don't call this against a null rectangle
	///
	float	get_x_max() const
	{
		assert( ! is_null() );
		return m_x_max;
	}

	/// Get min Y ordinate.
	//
	/// Don't call this against a null rectangle
	///
	float	get_y_min() const
	{
		assert( ! is_null() );
		return m_y_min;
	}

	/// Get max Y ordinate.
	//
	/// Don't call this against a null rectangle
	///
	float	get_y_max() const
	{
		assert( ! is_null() );
		return m_y_max;
	}

	/// TODO: deprecate this ?
	point	get_corner(int i) const;

	/// Set ourself to bound a rectangle that has been transformed
	/// by m.  This is an axial bound of an oriented (and/or
	/// sheared, scaled, etc) box.
	void	enclose_transformed_rect(const matrix& m, const rect& r);
	
	/// Same as enclose_transformed_rect but expanding the current rect instead
	/// of replacing it.
	void	expand_to_transformed_rect(const matrix& m, const rect& r);
	
	/// Makes union of the given and the current rect
	DSOEXPORT void  expand_to_rect(const rect& r);

	void	set_lerp(const rect& a, const rect& b, float t);
};


}	// namespace gnash

#endif // GNASH_RECT_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
