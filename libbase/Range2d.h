// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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
// Original author: Sandro Santilli <strk@keybit.net>
//

#ifndef GNASH_RANGE2D_H
#define GNASH_RANGE2D_H

#include <ostream>
#include <limits>
#include <algorithm>
#include <cassert> // for inlines
#include <cmath> // for floor / ceil
#include <cstdint>

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

namespace detail {
    template <typename U> struct Promote { typedef U type; };
    template <> struct Promote<float> { typedef double type; };
    template <> struct Promote<int> { typedef std::int64_t type; };
    template <> struct Promote<unsigned int> { typedef std::uint64_t type; };
}

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

	/// Templated copy constructor, for casting between range types
	template <typename U>
	Range2d(const Range2d<U>& from)
	{
		if ( from.isWorld() ) {
			setWorld();
		} else if ( from.isNull() ) {
			setNull();
		} else {
			_xmin = roundMin(from.getMinX());
			_ymin = roundMin(from.getMinY());
			_xmax = roundMax(from.getMaxX());
			_ymax = roundMax(from.getMaxY());
		}
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
		_ymin = 0;
		_ymax = 0;
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
	/// This is implemented using the minimum and maximum
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
		_ymin = 0;
		_ymax = 0;
		return *this;
	}

	/// \brief
	/// Return true if this rectangle contains the point with
	/// given coordinates (boundaries are inclusive).
	//
	/// Note that WORLD rectangles contain every point
	/// and NULL rectangles contain no point.
	///
	template <typename U>
	bool contains(U x, U y) const
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
	/// Return true if this rectangle contains the given rectangle.
	//
	/// Note that:
	///
	///	- WORLD ranges contain every range except NULL ones
	///	  and are only contained in WORLD ranges
	///
	///	- NULL ranges contain no ranges and are contained in no ranges.
	///
	bool contains(const Range2d<T>& other) const
	{
		if ( isNull() || other.isNull() ) return false;
		if ( isWorld() ) return true;
		if ( other.isWorld() ) return false;

		return _xmin <= other._xmin &&
			_xmax >= other._xmax &&
			_ymin <= other._ymin &&
			_ymax >= other._ymax;
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
	Range2d<T>& expandTo(T x, T y)
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

	/// Expand this Range2d to enclose the given circle.
	//
	/// @return a reference to this instance
	///
	Range2d<T>& expandToCircle(T x, T y, T radius)
	{
		// A WORLD range already enclose every point
		if ( isWorld() ) return *this;

        expandTo(x-radius, y);
        expandTo(x+radius, y);

        expandTo(x, y-radius);
        expandTo(x, y+radius);

		return *this;
	}

	/// Set ourself to bound the given point
	//
	/// @return a reference to this instance
	///
	Range2d<T>& setTo(T x, T y)
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
	Range2d<T>& setTo(T xmin, T ymin, T xmax, T ymax)
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
	Range2d<T>& shiftX(T offset)
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
	Range2d<T>& shiftY(T offset)
	{
		if ( isNull() || isWorld() ) return *this;
		_ymin += offset;
		_ymax += offset;
		return *this;
	}

	/// Scale this Range2d horizontally
	Range2d<T>& scaleX(float factor)
	{
		return scale(factor, 1);
	}

	/// Scale this Range2d vertically
	Range2d<T>& scaleY(float factor)
	{
		return scale(1, factor);
	}

	/// Scale this Range2d 
	//
	/// WORLD or NULL ranges will be unchanged
	///
	/// For finite ranges:
	///  Any factor of 0 will make the range NULL.
	///  A factor of 1 will leave the corresponding size unchanged.
	///  A factor > 1 will make the corresponding size bigger.
	///  A factor < 1 factor will make the corresponding size smaller.
	///
	/// Computation is done in single floating point precision.
	/// Specializations for integer types ensure that when rounding
	/// back the resulting range is not smaller then the floating
	/// range computed during scaling (in all directions).
	///
	/// Control point is the origin (0,0).
	///
	/// If the range so scaled will hit the numerical limit
	/// of the range an assertion will fail
	/// (TODO: throw an exception instead!).
	///
	/// @param xfactor
	///	The horizontal scale factor. It's a float
	///	to allow for fractional scale even for integer
	///	ranges. 
	///
	/// @param yfactor
	///	The vertical scale factor. It's a float
	///	to allow for fractional scale even for integer
	///	ranges. 
	///
	/// @return a reference to this instance
	///
	Range2d<T>& scale(float xfactor, float yfactor)
	{
		assert(xfactor >= 0 && yfactor >= 0);

		if ( ! isFinite() ) return *this;

		if ( xfactor == 0 || yfactor == 0 )
		{
			return setNull();
		}

		if ( xfactor != 1 )
		{
			_xmin = scaleMin(_xmin, xfactor);
			_xmax = scaleMax(_xmax, xfactor);
			assert(_xmin <= _xmax); // in case of overflow...
		}

		if ( yfactor != 1 )
		{
			_ymin = scaleMin(_ymin, yfactor);
			_ymax = scaleMax(_ymax, yfactor);
			assert(_ymin <= _ymax); // in case of overflow...
		}

		return *this;
	}

	/// Scale this Range2d in both directions with the same factor
	Range2d<T>& scale(float factor)
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
	///	If negative the range will shrink.
	///	See shrinkBy().
	///
	/// @return a reference to this instance
	///
	Range2d<T>& growBy(T amount)
	{
		if ( isNull() || isWorld() || amount==0 ) return *this;

		// NOTE: this trigger a compiler warning when T is an
		//       unsigned type (Coverity CID 1154656 -
		//       logically dead code)
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
	///	If negative the range will grow.
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

		// NOTE: this trigger a compiler warning when T is an
		//       unsigned type (Coverity CID 1154655 -
		//       logically dead code)
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


        /// Get area (width*height)
        //
        typename detail::Promote<T>::type
        getArea() const {
            assert ( !isWorld() );
            if ( isNull() ) return 0;
            return static_cast<typename detail::Promote<T>::type>(_xmax - _xmin)
                   * (_ymax - _ymin);
            // this implementation is for float types, see specialization below
            // for ints...
        }

	/// Expand this range to include the given Range2d
	//
	/// WORLD ranges force result to be the WORLD range.
	/// A NULL range will have no effect on the result.
	///
	void expandTo(const Range2d<T>& r)
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

private:

	T _xmin, _xmax, _ymin, _ymax;

	T scaleMin(T min, float scale) const {
		return roundMin(static_cast<float>(min) * scale);
	}

	T scaleMax(T max, float scale) const {
		return roundMax(static_cast<float>(max) * scale);
	}

	T roundMin(float v) const {
		return static_cast<T>(v);
	}

	T roundMax(float v) const {
		return static_cast<T>(v);
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

/// Specialization of minimum value rounding for int type.
//
/// Use floor.
///
template<> inline int
Range2d<int>::roundMin(float min) const
{
	return static_cast<int>(std::floor(min));
}

/// Specialization of minimum value rounding for unsigned int type.
//
/// Use floor. 
///
template<> inline unsigned int
Range2d<unsigned int>::roundMin(float min) const
{
	return static_cast<unsigned int>(std::floor(min));
}

/// Specialization of maximum value rounding for int type.
//
/// Use ceil. 
///
template<> inline int
Range2d<int>::roundMax(float max) const
{
	return static_cast<int>(std::ceil(max));
}

/// Specialization of maximum value rounding for unsigned int type.
//
/// Use ceil.
///
template<> inline unsigned int
Range2d<unsigned int>::roundMax(float max) const
{
	return static_cast<unsigned int>(std::ceil(max));
}

/// Specialization of area value for int type.
//
/// Add one.
///
template<> inline
detail::Promote<int>::type
Range2d<int>::getArea() const
{
    assert ( !isWorld() );
    if ( isNull() ) return 0;
    return static_cast<detail::Promote<int>::type>(_xmax - _xmin + 1) *
               (_ymax - _ymin + 1);
}

/// Specialization of area value for unsigned int type.
//
/// Add one.
///
template<> inline
detail::Promote<unsigned int>::type
Range2d<unsigned int>::getArea() const
{
    assert ( isFinite() );
    return static_cast<detail::Promote<unsigned int>::type>(_xmax - _xmin + 1) *
              (_ymax - _ymin + 1);
}


} // namespace gnash::geometry
} // namespace gnash

#endif // GNASH_RANGE2D_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
