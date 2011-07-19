/* 
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */ 

#ifndef GNASH_FUZZYPIXEL_H
#define GNASH_FUZZYPIXEL_H

#include "Range2d.h"
#include "GnashKey.h" // for namespace key
#include "RGBA.h" // for rgba class (composition)

#include <cmath>
#include <ostream> 

namespace gnash {

/// An utility class used to compare rgba values with a given tolerance
//
/// This is simply a wrapper around an rgba value, with an associated
/// tolerance used when comparing with another FuzzyPixel.
///
/// Currently (but we might change this in the future), the tolerance is
/// used to compare red, green, blue and alpha values.
///
/// A negative tolerance may be used to mark the FuzzyPixel as
/// an "invalid" one, which is a FuzzyPixel that will never be equal
/// to any other. Intollerant FuzzyPixel.
///
/// See operator== for more info.
///
class FuzzyPixel
{

public:

	friend std::ostream& operator<< (std::ostream& o, const FuzzyPixel& p);

	/// Construct an Intollerant FuzzyPixel.
	//
	/// Intollerant means that any comparison will fail.
	/// Actual value of the pixel doesn't make much sense.
	///
	FuzzyPixel()
		:
		_col(0,0,0,0),
		_tol(-1)
	{
	}

	/// Construct a FuzzyPixel with given color and tolerance
	//
	/// @param color
	///	The color value
	///
	/// @param tolerance
	///	The tolerance to use in comparisons.
	///
	FuzzyPixel(const rgba& color, short unsigned tolerance=0)
		:
		_col(color),
		// From short unsigned to signed we hopefully never swap sign.
		// Use the default constructor to build intolerant fuzzy pixels.
		_tol(int(tolerance))
	{
	}

	/// Construct a FuzzyPixel with given values and 0 tolerance.
	//
	/// Use setTolerance to modify the tolerance value.
	///
	/// @param r
	///	The red value.
	///
	/// @param g
	///	The green value.
	///
	/// @param b
	///	The blue value.
	///
	/// @param a
	///	The alpha value.
	///
	FuzzyPixel(boost::uint8_t r, boost::uint8_t g, boost::uint8_t b, boost::uint8_t a)
		:
		_col(r, g, b, a),
		_tol(0)
	{
	}

	/// Set the tolerance to use in comparisons
	//
	/// @param tol
	///	The tolerance to use in comparisons.
	///
	void setTolerance(unsigned short tol)
	{
		_tol = int(tol);
	}

	/// Make this fuzzy pixel intolerant
	void setIntolerant()
	{
		_tol = -1;
	}

	/// Compare two FuzzyPixel using the tolerance of the most tolerant of the two
	//
	/// Note that if any of the two operands is intolerant, any equality
	/// comparison will fail.
	///
	/// See default constructor and setIntolerant for more info.
	///
	bool operator==(const FuzzyPixel& other) const;

	/// Return true if a and b are below a given tolerance
	static bool fuzzyEqual(int a, int b, int tol)
	{
		return std::abs(a-b) <= tol;
	}

private:

	// actual color 
	rgba _col;

	// tolerance value
	int _tol;

};

} // namespace gnash

#endif // _GNASH_FUZZYPIXEL_H
