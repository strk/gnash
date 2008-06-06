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
//
// Original author: Thatcher Ulrich <tu@tulrich.com> 2003
//
//

#ifndef GNASH_MATRIX_H
#define GNASH_MATRIX_H

#include "dsodefs.h" // for DSOEXPORT
#include "Range2d.h" // for transforming Range2d<float>
#include "Point2d.h" // for transforming Point2d<float> (typedefe'd to point)

#include <boost/cstdint.hpp>
#include <iosfwd>
#include <iomanip>

// Forward declarations
namespace gnash {
	class stream;
}


namespace gnash {

/// The SWF matrix record.
/// 
/// Conceptuall, it represents a 3*3 linear transformation matrix like this:
/// 
///   | scale_x       rotateSkew_y  translate_x |
///   | rotateSkey_x  scale_y       traslate_y  |
///   | 0             0             1           |
/// 
class DSOEXPORT matrix
{
public:

	int sx;  // Xscale, 16.16 fixed point.
    int shx; // Xshear, 16.16 fixed point. 
    int tx;  // Xtranslation, TWIPS.
    int sy;  // Yscale, 16.16 fixed point.
    int shy; // Yshear, 16.16 fixed point.
    int ty;  // Ytranslation, TWIPS.
             
	friend bool operator== (const matrix&, const matrix&);
	friend std::ostream& operator<< (std::ostream&, const matrix&);
	
	/// Defaults to identity
	matrix();

	/// Check validity of the matrix values
	bool	is_valid() const;

	/// Set the matrix to identity.
	void	set_identity();

	/// Concatenate m's transform onto ours. 
	//
	/// When transforming points, m happens first,
	/// then our original xform.
	///
	void	concatenate(const matrix& m);

	/// Concatenate a translation onto the front of our matrix.
	//
	/// When transforming points, the translation
	/// happens first, then our original xform.
	///
	void	concatenate_translation(float tx, float ty);

	/// Concatenate a uniform scale onto the front of our matrix.
	//
	/// When transforming points, the scale
	/// happens first, then our original xform.
	///
	void	concatenate_scale(float s);

	/// Just like concatenate_scale() but with different scales for x/y 
	void	concatenate_scales(float x, float y);

	/// Set this matrix to a blend of m1 and m2, parameterized by t.
	void	set_lerp(const matrix& m1, const matrix& m2, float t);

	/// Set the scale & rotation part of the matrix. angle in radians.
	void	set_scale_rotation(float x_scale, float y_scale, float rotation);

	/// Set the scale part of the matrix, will keep current rotation
	void	set_scale(float x_scale, float y_scale);

	/// Set the X scale part of the matrix, will keep current rotation and Y scale
	void	set_x_scale(float scale);

	/// Set the Y scale part of the matrix, will keep current rotation and X scale
	void	set_y_scale(float scale);

	/// Set the rotation part of the matrix, will keep current scale. Angle in radians.
	void	set_rotation(float rotation);

	/// Set x translation
	void set_x_translation(float x)
	{
		tx = x;
	}

	/// Set y translation
	void set_y_translation(float y)
	{
		ty = y;
	}

	void set_translation(float x, float y)
	{
		tx = x;
		ty = y;
	}

	/// Initialize from the SWF input stream.
	void	read(stream& in);

	// temp hack, should drop..
	void	read(stream* in) { read(*in); }

	/// Transform point 'p' by our matrix. 
	//
	/// Put the result in *result.
	///
	void	transform(point* result, const point& p) const;

	// Transform point 'p' by our matrix.
	template <typename T>
	void transform(geometry::Point2d<T>& p) const
	{
		transform(p.x, p.y);
	}

	/// Transform point 'p' by the inverse of our matrix. 
	void	transform_by_inverse(point& p) const;

	/// Transform point 'x,y' by our matrix. 
	template <typename T>
	void
	transform(T& x, T& y) const
	// Transform point 'x,y' by our matrix.
	{
		float nx = (sx / 65536.0f) * x + (shy / 65536.0f) * y + tx;
		float ny = (shx / 65536.0f) * x + (sy / 65536.0 ) * y + ty;
		x = nx;
		y = ny;
	}

	/// Transform vector 'v' by our matrix. Doesn't apply translation.
	//
	/// Put the result in *result.
	///
	void	transform_vector(point* result, const point& p) const;

	/// Transform point 'p' by the inverse of our matrix. 
	//
	/// Put result in *result.
	///
	void	transform_by_inverse(point* result, const point& p) const;

	/// Transform Range2d<float> 'r' by our matrix. 
	//
	/// NULL and WORLD ranges are untouched.
	///
	void	transform(geometry::Range2d<float>& r) const;

	/// Transform Range2d<float> 'r' by the inverse our matrix. 
	//
	/// NULL and WORLD ranges are untouched.
	///
	void	transform_by_inverse(geometry::Range2d<float>& r) const;


	/// Set this matrix to the inverse of the given matrix.
	void	set_inverse(const matrix& m);

	/// Return true if this matrix reverses handedness.
	bool	does_flip() const;	

	/// return the magnitude scale of our x coord output
	float	get_x_scale() const;

	/// return the magnitude scale of our y coord output
	float	get_y_scale() const;

	/// return our rotation component (in radians)
	float	get_rotation() const;

	/// return the canonical x translation
	float	get_x_translation() const
	{
		return tx;
	}

	/// return the canonical y translation
	float	get_y_translation() const
	{
		return ty;
	}

	/// Return the determinant of this matrix in 32.32 fixed point format.
	boost::int64_t	get_determinant() const;
};

inline bool operator== (const matrix& a, const matrix& b)
{
	return	
        a.sx  == b.sx  &&
		a.shx == b.shx &&
		a.tx  == b.tx  &&
		a.sy  == b.sy  &&
		a.shy == b.shy &&
		a.ty  == b.ty;
}

}	// namespace gnash

#endif // GNASH_MATRIX_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
