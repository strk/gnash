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
// Original author: Sandro Santilli <strk@keybit.net>
//


/* $Id: Range2d.h,v 1.5 2006/12/08 08:27:24 strk Exp $ */

#ifndef GNASH_RANGE2D_H
#define GNASH_RANGE2D_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ostream>
#include <limits>
#include <algorithm>
#include <cassert> // for inlines
#include <iostream> // temporary include for debugging

namespace gnash {

namespace geometry {

/// Kinds of a range
enum RangeKind {
	/// Valid range, using finite values
	finiteRange,

	/// A NULL range is a range enclosing NO points.
	nullRange,

	/// \brief
	/// A WORLD range2d is a range including
	/// all points on the plane. 
	// 
	/// Note that scaling, shifting and unioning 
	/// will NOT change a WORLD range.
	///
	worldRange
};

/// 2d Range template class
//
/// The class stores 4 values of the type specified
/// as template argument, representing the set of points
/// enclosed by the given min and max values for the 2 dimensions,
/// and provides methods for manipulating them.
/// The parameter type must be a numeric type.
///
/// The two dimensions are called X and Y.
///
/// Note that the range is "open", which means that the points
/// on its boundary are considered internal to the range.
///
///
template <typename T>
class Range2d
{
private:

	T _xmin, _xmax, _ymin, _ymax;

public:

	/// Ouput operator
	template <typename U>
	friend std::ostream& operator<< (std::ostream& os, const Range2d<U>& rect);

	/// Equality operator
	//
	/// This is needed to take NULL kind into account
	/// since we don't explicitly set all members when constructing
	/// NULL ranges
	///
	template <typename U>
	friend bool operator== (const Range2d<U>& r1, const Range2d<U>& r2);

	/// Inequality operator
	//
	/// This is needed to take NULL kind into account
	/// since we don't explicitly set all members when constructing
	/// NULL ranges
	///
	template <typename U>
	friend bool operator!= (const Range2d<U>& r1, const Range2d<U>& r2);

	/// Return a rectangle being the intersetion of the two rectangles
	//
	/// Any NULL operand will make the result also NULL.
	///
	template <typename U> friend Range2d<U>
	Intersection(const Range2d<U>& r1, const Range2d<U>& r2);

	/// Return a rectangle being the union of the two rectangles
	template <typename U> friend Range2d<U>
	Union(const Range2d<U>& r1, const Range2d<U>& r2);

	/// Construct a Range2d of the given kind.
	//
	/// The default is building a nullRange.
	/// If finiteRange is given the range will be set to
	/// enclose the origin.
	///
	/// See RangeKind
	///
	Range2d(RangeKind kind=nullRange)
		:
		_xmin(T()),
		_xmax(T()),
		_ymin(T()),
		_ymax(T())
	{
		switch ( kind )
		{
			case worldRange:
				setWorld();
				break;
			case nullRange:
				setNull();
				break;
			default:
			case finiteRange:
				break;
		}
	}

	/// Construct a finite Range2d with the given values
	//
	/// Make sure that the min <= max, or an assertion
	/// would fail. We could as well swap the values
	/// in this case, but it is probably better to
	/// force caller to deal with this, as a similar
	/// case might as well expose a bug in the code.
	///
	Range2d(T xmin, T ymin, T xmax, T ymax)
		:
		_xmin(xmin),
		_xmax(xmax),
		_ymin(ymin),
		_ymax(ymax)
	{
		// use the default ctor to make a NULL Range2d
		assert(_xmin <= _xmax);
		assert(_ymin <= _ymax);
		// .. or should we raise an exception .. ?
	}

	/// Returns true if this is the NULL Range2d
	bool isNull() const
	{
		return _xmax < _xmin;
	}

	/// Set the Range2d to the NULL value
	//
	/// @return a reference to this instance
	///
	Range2d<T>& setNull()
	{
		_xmin = std::numeric_limits<T>::max();
		_xmax = std::numeric_limits<T>::min();
		return *this;
	}

	/// Returns true if this is the WORLD Range2d
	bool isWorld() const
	{
		return _xmax == std::numeric_limits<T>::max()
			&& _xmin == std::numeric_limits<T>::min();
	}

	/// Returns true if this is a finite Range2d
	//
	/// See RangeKind::finiteRange
	///
	bool isFinite() const
	{
		return ( ! isNull() && ! isWorld() );
	}

	/// Set the Range2d to the WORLD value
	//
	/// This is implemented using the minimun and maximun
	/// values of the parameter type.
	///
	/// See RangeType::worldRange
	///
	/// @return a reference to this instance
	///
	Range2d<T>& setWorld()
	{
		_xmin = std::numeric_limits<T>::min();
		_xmax = std::numeric_limits<T>::max();
		return *this;
	}

	/// \brief
	/// Return true if this rectangle contains the point with
	/// given coordinates (boundaries are inclusive).
	//
	/// Note that WORLD rectangles contain every point
	/// and NULL rectangles contain no point.
	///
	bool contains(T x, T y) const
	{
		if ( isNull() ) return false;
		if ( isWorld() ) return true;
		if (x < _xmin || x > _xmax || y < _ymin || y > _ymax)
		{
			return false;
		}
		return true;
	}

	/// \brief
	/// Return true if this rectangle intersects the point with
	/// given coordinates (boundaries are inclusive).
	//
	/// Note that NULL rectangles don't intersect anything
	/// and WORLD rectangles intersects everything except a NULL rectangle.
	///
	bool intersects(const Range2d<T>& other) const
	{
		if ( isNull() || other.isNull() ) return false;
		if ( isWorld() || other.isWorld() ) return true;

		if ( _xmin > other._xmax ) return false;
		if ( _xmax < other._xmin ) return false;
		if ( _ymin > other._ymax ) return false;
		if ( _ymax < other._ymin ) return false;
		return true;
	}

	/// Expand this Range2d to enclose the given point.
	//
	/// @return a reference to this instance
	///
	Range2d<T> expandTo(T x, T y)
	{
		// A WORLD range already enclose every point
		if ( isWorld() ) return *this;

		if ( isNull() ) 
		{
			setTo(x,y);
		}
		else
		{
			_xmin = std::min(_xmin, x);
			_ymin = std::min(_ymin, y);
			_xmax = std::max(_xmax, x);
			_ymax = std::max(_ymax, y);
		}

		return *this;
	}

	/// Set ourself to bound the given point
	//
	/// @return a reference to this instance
	///
	Range2d<T> setTo(T x, T y)
	{
		_xmin = _xmax = x;
		_ymin = _ymax = y;
		return *this;
	}

	/// Set coordinates to given values
	//
	/// Make sure that the min <= max, or an assertion
	/// would fail. We could as well swap the values
	/// in this case, but it is probably better to
	/// force caller to deal with this, as a similar
	/// case might as well expose a bug in the code.
	//
	/// @return a reference to this instance
	///
	Range2d<T> setTo(T xmin, T ymin, T xmax, T ymax)
	{
		_xmin = xmin;
		_xmax = xmax;
		_ymin = ymin;
		_ymax = ymax;

		// use the default ctor to make a NULL Range2d
		assert(_xmin <= _xmax);
		assert(_ymin <= _ymax);

		return *this;
	}

	/// Return width this Range2d
	//
	/// Don't call this function on a WORLD rectangle!
	///
	T width() const
	{
		assert ( ! isWorld() );
		if ( isNull() ) return 0;
		return _xmax-_xmin;
	}

	/// Return height this Range2dangle
	//
	/// Don't call this function on a WORLD rectangle!
	///
	T height() const
	{
		assert ( ! isWorld() );
		if ( isNull() ) return 0;
		return _ymax-_ymin;
	}

	/// Shift this Range2dangle horizontally
	//
	/// A positive offset will shift to the right,
	/// A negative offset will shift to the left.
	///
	/// WORLD or NULL ranges will be unchanged
	///
	/// @return a reference to this instance
	///
	Range2d<T> shiftX(T offset)
	{
		if ( isNull() || isWorld() ) return *this;
		_xmin += offset;
		_xmax += offset;
		return *this;
	}

	/// Shift this Range2dangle vertically
	//
	/// A positive offset will increment y values.
	/// A negative offset will decrement y values.
	///
	/// WORLD or NULL ranges will be unchanged
	///
	/// @return a reference to this instance
	///
	Range2d<T> shiftY(T offset)
	{
		if ( isNull() || isWorld() ) return *this;
		_ymin += offset;
		_ymax += offset;
		return *this;
	}

	/// Scale this Range2d horizontally
	//
	/// A positive factor will make the Range2dangle wider.
	/// A negative factor will make the Range2dangle narrower.
	/// A factor of 1 will leave it unchanged.
	/// Control point is the origin (0,0).
	///
	/// WORLD or NULL ranges will be unchanged
	///
	/// If the range so scaled will hit the numerical limit
	/// of the range an assertion will fail
	/// (TODO: throw an exception instead!).
	///
	/// @return a reference to this instance
	///
	Range2d<T> scaleX(T factor)
	{
		if ( ! isFinite() ) return *this;
		_xmin *= factor;
		_xmax *= factor;
		assert(_xmin < _xmax); // in case of overflow...
		return *this;
	}

	/// Scale this Range2dangle vertically
	//
	/// A positive factor will make the Range2dangle taller.
	/// A negative factor will make the Range2dangle shorter.
	/// A factor of 1 will leave it unchanged.
	/// Control point is the origin (0,0).
	///
	/// If the range so scaled will hit the numerical limit
	/// of the range an assertion will fail
	/// (TODO: throw an exception instead!).
	///
	/// @return a reference to this instance
	///
	Range2d<T> scaleY(T factor)
	{
		if ( ! isFinite() ) return *this;
		_ymin *= factor;
		_ymax *= factor;
		assert(_ymin < _ymax); // in case of overflow...

		return *this;
	}

	/// Scale this Range2d in both directions
	//
	/// A positive factor will make the Range2dangle bigger.
	/// A negative factor will make the Range2dangle smaller.
	/// A factor of 1 will leave it unchanged.
	/// Control point is the origin (0,0).
	///
	/// If the range so scaled will hit the numerical limit
	/// of the range an assertion will fail
	/// (TODO: throw an exception instead!).
	///
	/// @param xfactor
	///	The horizontal scale factor 
	///
	/// @param yfactor
	///	The vertical scale factor 
	///
	/// @return a reference to this instance
	///
	Range2d<T> scale(T xfactor, T yfactor)
	{
		if ( ! isFinite() ) return *this;

		_xmin *= xfactor;
		_xmax *= xfactor;
		assert(_xmin < _xmax); // in case of overflow...

		_ymin *= yfactor;
		_ymax *= yfactor;
		assert(_ymin < _ymax); // in case of overflow...

		return *this;
	}

	/// Scale this Range2d in both directions with the same factor
	Range2d<T> scale(T factor)
	{
		return scale(factor, factor);
	}

	/// Grow this range by the given amout in all directions.
	//
	/// WORLD or NULL ranges will be unchanged.
	///
	/// If a growing range hits the numerical limit for T
	/// it will be set to the WORLD range.
	///
	/// @param amount
	/// 	The amount of T to grow this range in all directions.
	///	If negative the range will shrink.
	///	If negative (where T allows that) the range will shrink:
	///	See shrinkBy().
	///
	/// @return a reference to this instance
	///
	Range2d<T>& growBy(T amount)
	{
		if ( isNull() || isWorld() || amount==0 ) return *this;

		// NOTE: whith will likely trigger a compiler
		//       warning when T is an unsigned type
		if ( amount < 0 ) return shrinkBy(-amount);

		T newxmin = _xmin - amount;
		if (newxmin > _xmin ) return setWorld();
		else _xmin = newxmin;

		T newxmax = _xmax + amount;
		if (newxmax < _xmax ) return setWorld();
		else _xmax = newxmax;

		T newymin = _ymin - amount;
		if (newymin > _ymin ) return setWorld();
		else _ymin = newymin;

		T newymax = _ymax + amount;
		if (newymax < _ymax ) return setWorld();
		else _ymax = newymax;

		return *this;

	}

	/// Shirnk this range by the given amout in all directions.
	//
	/// WORLD or NULL ranges will be unchanged.
	///
	/// If a shrinking range will collapse in either the horizontal
	/// or vertical dimension it will be set to the NULL range.
	///
	/// @param amount
	/// 	The amount of T to shink this range in all directions.
	///	If negative (where T allows that) the range will grow:
	///	See growBy().
	///
	/// @return a reference to this instance
	///
	/// NOTE: This method assumes that the numerical type used
	///       as parameter does allow both positive and negative
	///       values. Using this method against an instance of
	///	  an 'unsigned' Range2d will likely raise unexpected
	///	  results.
	/// 
	/// TODO: change the interface to never make the Range null,
	///       as we might always use the Range *center* point
	///       instead of forgetting about it!
	///
	Range2d<T>& shrinkBy(T amount)
	{
		if ( isNull() || isWorld() || amount==0 ) return *this;

		// NOTE: whith will likely trigger a compiler
		//       warning when T is an unsigned type
		if ( amount < 0 ) return growBy(-amount);

		// Turn this range into the NULL range
		// if any dimension collapses.
		// Don't use width() and height() to 
		// avoid superflous checks.

		if ( _xmax - _xmin <= amount ) return setNull();
		if ( _ymax - _ymin <= amount ) return setNull();

		_xmin += amount;
		_ymin += amount;
		_xmax -= amount;
		_ymax -= amount;

		return *this;

	}

	/// Get min X ordinate.
	//
	/// Don't call this against a NULL or WORLD Range2
	///
	T getMinX() const
	{
		assert ( isFinite() );
		return _xmin;
	}

	/// Get max X ordinate.
	//
	/// Don't call this against a NULL or WORLD Range2d
	///
	T getMaxX() const
	{
		assert ( isFinite() );
		return _xmax;
	}

	/// Get min Y ordinate.
	//
	/// Don't call this against a NULL or WORLD Range2d
	///
	T getMinY() const
	{
		assert ( isFinite() );
		return _ymin;
	}

	/// Get max Y ordinate.
	//
	/// Don't call this against a NULL or WORLD Range2d
	///
	T getMaxY() const
	{
		assert ( isFinite() );
		return _ymax;
	}

	/// Expand this range to include the given Range2d
	//
	/// WORLD ranges force result to be the WORLD range.
	/// A NULL range will have no effect on the result.
	///
	void  expandTo(const Range2d<T>& r)
	{
		if ( r.isNull() )
		{
			// the given range will add nothing
			return; 
		}

		if ( isNull() ) 
		{
			// being null ourself, we'll equal the given range
			*this = r;
			return;
		}

		if ( isWorld() || r.isWorld() )
		{
			// union with world is always world...
			setWorld();
			return;
		}

		_xmin = std::min(_xmin, r._xmin);
		_xmax = std::max(_xmax, r._xmax);
		_ymin = std::min(_ymin, r._ymin);
		_ymax = std::max(_ymax, r._ymax);

	}


};

template <typename T> inline std::ostream&
operator<< (std::ostream& os, const Range2d<T>& rect)
{
	if ( rect.isNull() ) return os << "Null range";
	if ( rect.isWorld() ) return os << "World range";

	return os << "Finite range (" << rect._xmin << "," << rect._ymin
		<< " " << rect._xmax << "," << rect._ymax << ")";
}

template <typename T> inline bool
operator== (const Range2d<T>& r1, const Range2d<T>& r2)
{
	// These checks are needed becase
	// we don't initialize *all* memebers
	// when setting to Null or World

	if ( r1.isNull() ) return r2.isNull();
	if ( r2.isNull() ) return r1.isNull();
	if ( r1.isWorld() ) return r2.isWorld();
	if ( r2.isWorld() ) return r1.isWorld();

	return r1._xmin == r2._xmin && r1._ymin == r2._ymin &&
		r1._xmax == r2._xmax && r1._ymax == r2._ymax;
}

template <typename T> inline bool
operator!= (const Range2d<T>& r1, const Range2d<T>& r2)
{
	return ! ( r1 == r2 );
}

/// Return true of the two ranges intersect (boundaries included)
template <typename T> inline bool
Intersect(const Range2d<T>& r1, const Range2d<T>& r2)
{
	return r1.intersects(r2);
}

/// Return a rectangle being the union of the two rectangles
template <typename T> inline Range2d<T>
Union(const Range2d<T>& r1, const Range2d<T>& r2)
{
	Range2d<T> ret = r1;
	ret.expandTo(r2);
	return ret;
}

/// Return a rectangle being the intersetion of the two rectangles
//
/// Any NULL operand will make the result also NULL.
///
template <typename T> inline Range2d<T>
Intersection(const Range2d<T>& r1, const Range2d<T>& r2)
{
	if ( r1.isNull() || r2.isNull() ) {
		// NULL ranges intersect nothing
		return Range2d<T>(nullRange); 
	}

	if ( r1.isWorld() ) {
		// WORLD range intersect everything
		return r2;
	}

	if ( r2.isWorld() ) {
		// WORLD range intersect everything
		return r1;
	}

	if ( ! r1.intersects(r2) ) {
		// No intersection results in a NULL range
		return Range2d<T>(nullRange); 
	}

	return Range2d<T> (
		std::max(r1._xmin, r2._xmin), // xmin
		std::max(r1._ymin, r2._ymin), // ymin
		std::min(r1._xmax, r2._xmax), // xmax
		std::min(r1._ymax, r2._ymax)  // ymax
	);

}



} // namespace gnash::geometry
} // namespace gnash

#endif // GNASH_RANGE2D_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
