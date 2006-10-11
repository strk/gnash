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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
// Original author: Thatcher Ulrich <tu@tulrich.com> 2003
//
// $Id: matrix.h,v 1.1 2006/10/11 08:08:36 strk Exp $ 
//

#ifndef GNASH_MATRIX_H
#define GNASH_MATRIX_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"

namespace gnash {

class point;
class stream;

/// Matrix type, used by render handler.
//
/// This type stores the top two rows of a 3x3 matrix whose
/// bottom row is (0 0 1). This matrix lets you define any combination of
/// scaling, motion and rotation (including flipping) in just 6 numbers.
/// Better yet, the same distortion is always represented by the same set
/// of 6 numbers.
///
/// The matrix looks like this (last line implicit):
///
///   | scale_x  x_dep_y  translate_x |
///   | y_dep_x  scale_y  translate_y |
///   |   0         0          1      |
///
class DSOEXPORT matrix
{
public:
	/// \brief
	/// Top two rows of a 3x3 matrix whose bottom row is 
	/// assumed to be | 0 0 1 |
	///
	///	| scale_x  x_dep_y  translate_x |
	///	| y_dep_x  scale_y  translate_y |
	///
	float	m_[2][3];
	
	/// The identity matrix (no transforms)
	//
	/// Identity matrix is:
	///
	///	| 1 0 0 |
	///	| 0 1 0 |
	///
	static matrix	identity;
	
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

	/// Set this matrix to a blend of m1 and m2, parameterized by t.
	void	set_lerp(const matrix& m1, const matrix& m2, float t);

	/// Set the scale & rotation part of the matrix. angle in radians.
	void	set_scale_rotation(float x_scale, float y_scale, float rotation);

	/// Initialize from the SWF input stream.
	void	read(stream* in);

	/// Debug log.
	void	print() const;

	/// Transform point 'p' by our matrix. 
	//
	/// Put the result in *result.
	///
	void	transform(point* result, const point& p) const;

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

	/// Set this matrix to the inverse of the given matrix.
	void	set_inverse(const matrix& m);

	/// Return true if this matrix reverses handedness.
	bool	does_flip() const;	

	/// Return the determinant of the 2x2 rotation/scale part only.
	float	get_determinant() const;

	/// Return the maximum scale factor that this transform applies.
	//
	/// For assessing scale, when determining acceptable
	/// errors in tesselation.
	///
	float	get_max_scale() const;	

	/// return the magnitude scale of our x coord output
	float	get_x_scale() const;

	/// return the magnitude scale of our y coord output
	float	get_y_scale() const;

	/// return our rotation component (in radians)
	float	get_rotation() const;
};


}	// namespace gnash

#endif // GNASH_MATRIX_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
