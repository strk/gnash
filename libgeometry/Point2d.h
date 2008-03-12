// Point2d template - for gnash
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

//
// Original author: Sandro Santilli <strk@keybit.net>
//



#ifndef GNASH_POINT2DH
#define GNASH_POINT2DH

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <ostream>
#include <limits>
#include <algorithm>
#include <cassert> // for inlines
#include <iostream> // temporary include for debugging
#include <cmath> // for floor / ceil

namespace gnash {

namespace geometry {

/// 2d Point template class
//
/// The class stores 2 values of the type specified
/// as template argument, representing the the X and Y oordinates.
///
template <typename T>
class Point2d
{
private:

public:

	/// The x ordinate
	T x;

	/// The y ordinate
	T y;

	/// Construct a Point2d with default X and Y ordinates
	//
	Point2d()
		:
		x(T(0.0)),
		y(T(0.0))
	{

	}
	/// Construct a Point2d with given X and Y ordinates
	//
	Point2d(T nx, T ny)
		:
		x(nx),
		y(ny)
	{
	}

	/// Construct a Point2d as an interpolation of the given input points
	//
	/// @param p0 first point
	/// @param p1 second point
	/// @param t interpolation factor, between 0 and 1
	///
	template <typename U>
	Point2d(const Point2d<U>& p0, const Point2d<U>& p1, float t)
		:
		x( p0.x + (p1.x - p0.x) * t ),
		y( p0.y + (p1.y - p0.y) * t )
	{
	}

	/// Set coordinates to given values
	//
	/// @return a reference to this instance
	///
	Point2d<T>& setTo(const T& nx, const T& ny)
	{
		x = nx;
		y = ny;

		return *this;
	}

	/// Set coordinates to the ones of the interpolation between the given input points
	//
	/// @param p0 first point
	/// @param p1 second point
	/// @param t interpolation factor, between 0 and 1
	///
	/// @return a reference to this instance
	///
	Point2d<T>& setTo(const Point2d<T>& p0, const Point2d<T>& p1, float t)
	{
		x = p0.x + (p1.x - p0.x) * t;
		y = p0.y + (p1.y - p0.y) * t;
		return *this;
	}

	/// Return square distance between two points
	template <typename U>
	static
	float squareDistance(const Point2d<T>& p0, const Point2d<U>& p1)
	{
		float hside = p1.x - p0.x;
		float vside = p1.y - p0.y;

		return hside*hside + vside*vside;
	}

	/// Return square distance between this and the given point
	template <typename U>
	float squareDistance(const Point2d<U>& p) const
	{
		return squareDistance(*this, p);
	}

	/// Return distance between this and the given point
	float distance(const Point2d<T>& p) const
	{
		return sqrtf(squareDistance(p));
	}

	template <typename U>
	bool operator== (const Point2d<U>& p) const
	{
		return x == (T)p.x && y == (T)p.y;
	}

	bool operator!=(const Point2d<T>& p) const
	{
		return ! (*this == p);
	}
};

/// Output operator
template <typename T> inline std::ostream&
operator<< (std::ostream& os, const Point2d<T>& p)
{
	return os << "Point2d(" << p.x << "," << p.y << ")";
}



} // namespace gnash::geometry

// for backward compatibility
typedef geometry::Point2d<float> point;
typedef geometry::Point2d<float> float_point;
typedef geometry::Point2d<int> int_point;

} // namespace gnash

#endif // GNASH_POINT2DH


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
