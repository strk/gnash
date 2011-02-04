// Point2d template - for gnash
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
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
// Original author: Sandro Santilli <strk@keybit.net>
//

#ifndef GNASH_POINT2DH
#define GNASH_POINT2DH

#include <ostream>
#include <cmath>    // for sqrt()
#include <boost/cstdint.hpp>

namespace gnash {
namespace geometry { 
    
/// 2D Point class
//
/// A point which contains a x and a y coorinate in TWIPS.
/// 
class Point2d
{
public:

	/// The x coordinate
	boost::int32_t  x;  // TWIPS

	/// The y coordinate
	boost::int32_t  y;  // TWIPS

	/// Construct a Point2d with default x and y coordinates
	Point2d()
		:
		x(0), y(0)
	{
	}

	/// Construct a Point2d with given x and y ordinates
	Point2d(boost::int32_t cx, boost::int32_t cy)
		:
		x(cx), y(cy)
	{
	}

	/// Construct a Point2d as an interpolation of the given input points
	//
	/// @param p0 first point
	/// @param p1 second point
	/// @param t interpolation factor, between 0 and 1
	///
	Point2d(const Point2d& p0, const Point2d& p1, float t)
		:
		x( p0.x + (boost::int32_t)((p1.x - p0.x) * t)),
		y( p0.y + (boost::int32_t)((p1.y - p0.y) * t))
	{
	}

	/// Set coordinates to given values
	//
	/// @return a reference to this instance
	///
	Point2d& setTo(const boost::int32_t cx, const boost::int32_t cy)
	{
		x = cx;  
        y = cy;
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
	Point2d& setTo(const Point2d& p0, const Point2d& p1, float t)
	{
		x = p0.x + (boost::int32_t)((p1.x - p0.x) * t);
		y = p0.y + (boost::int32_t)((p1.y - p0.y) * t);
		return *this;
	}

	/// Return square distance between two given points.
	static
	boost::int64_t squareDistance(const Point2d& p0, const Point2d& p1)
	{
		boost::int64_t hside = p1.x - p0.x;
		boost::int64_t vside = p1.y - p0.y;

		return hside*hside + vside*vside;
	}

	/// Return square distance between this and the given point
	boost::int64_t squareDistance(const Point2d& p) const
	{
		return squareDistance(*this, p);
	}

	/// Return distance between this and the given point
	boost::int32_t distance(const Point2d& p) const
	{
	    return (boost::int32_t)( std::sqrt( static_cast<double>(squareDistance(p)) ) );
	}

	bool operator== (const Point2d& p) const
	{
		return (x == p.x) && (y == p.y);
	}

	bool operator!=(const Point2d& p) const
	{
		return ! (*this == p);
	}
};

/// Output operator
inline std::ostream&
operator<< (std::ostream& os, const Point2d& p)
{
	return os << "Point2d(" << p.x << "," << p.y << ")";
}

} // namespace gnash::geometry

typedef geometry::Point2d  point;

} // namespace gnash

#endif // GNASH_POINT2DH

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
