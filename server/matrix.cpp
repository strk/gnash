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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "matrix.h"
#include "stream.h" // for reading from SWF
#include "types.h" // for TWIPS_TO_PIXEL define
                   // (should probably not use it though)
#include "log.h"
#include "utility.h" // for utility::infinite_to_fzero, utility::isFinite

namespace gnash {

matrix	matrix::identity;

matrix::matrix()
{
	// Default to identity.
	set_identity();
}


bool
matrix::is_valid() const
{
	return utility::isFinite(m_[0][0])
		&& utility::isFinite(m_[0][1])
		&& utility::isFinite(m_[0][2])
		&& utility::isFinite(m_[1][0])
		&& utility::isFinite(m_[1][1])
		&& utility::isFinite(m_[1][2]);
}


void
matrix::set_identity()
// Set the matrix to identity.
{
	memset(&m_[0], 0, sizeof(m_));
	m_[0][0] = 1;
	m_[1][1] = 1;
}

void
matrix::concatenate(const matrix& m)
// Concatenate m's transform onto ours.  When
// transforming points, m happens first, then our
// original xform.
{
	matrix	t;
	t.m_[0][0] = m_[0][0] * m.m_[0][0] + m_[0][1] * m.m_[1][0];
	t.m_[1][0] = m_[1][0] * m.m_[0][0] + m_[1][1] * m.m_[1][0];
	t.m_[0][1] = m_[0][0] * m.m_[0][1] + m_[0][1] * m.m_[1][1];
	t.m_[1][1] = m_[1][0] * m.m_[0][1] + m_[1][1] * m.m_[1][1];
	t.m_[0][2] = m_[0][0] * m.m_[0][2] + m_[0][1] * m.m_[1][2] + m_[0][2];
	t.m_[1][2] = m_[1][0] * m.m_[0][2] + m_[1][1] * m.m_[1][2] + m_[1][2];

	*this = t;
}


void
matrix::concatenate_translation(float tx, float ty)
// Concatenate a translation onto the front of our
// matrix.  When transforming points, the translation
// happens first, then our original xform.
{
	m_[0][2] += utility::infinite_to_fzero(m_[0][0] * tx + m_[0][1] * ty);
	m_[1][2] += utility::infinite_to_fzero(m_[1][0] * tx + m_[1][1] * ty);
}


void
matrix::concatenate_scale(float scale)
// Concatenate a uniform scale onto the front of our
// matrix.  When transforming points, the scale
// happens first, then our original xform.
{
	m_[0][0] *= utility::infinite_to_fzero(scale);
	m_[0][1] *= utility::infinite_to_fzero(scale);
	m_[1][0] *= utility::infinite_to_fzero(scale);
	m_[1][1] *= utility::infinite_to_fzero(scale);
}

void	
matrix::concatenate_scales(float x, float y)
// Just like concatenate_scale() but with different scales for x/y
{
	matrix m2; m2.set_scale_rotation(x, y, 0);
	concatenate(m2);

#if 0 // the code below only works when x and y scales are equal,
      // see testsuite/server/MatrixTest.cpp
	m_[0][0] *= utility::infinite_to_fzero(x);
	m_[0][1] *= utility::infinite_to_fzero(x);
	m_[1][0] *= utility::infinite_to_fzero(y);
	m_[1][1] *= utility::infinite_to_fzero(y);
#endif
}

void
matrix::set_lerp(const matrix& m1, const matrix& m2, float t)
// Set this matrix to a blend of m1 and m2, parameterized by t.
{
    using utility::flerp;
	m_[0][0] = flerp(m1.m_[0][0], m2.m_[0][0], t);
	m_[1][0] = flerp(m1.m_[1][0], m2.m_[1][0], t);
	m_[0][1] = flerp(m1.m_[0][1], m2.m_[0][1], t);
	m_[1][1] = flerp(m1.m_[1][1], m2.m_[1][1], t);
	m_[0][2] = flerp(m1.m_[0][2], m2.m_[0][2], t);
	m_[1][2] = flerp(m1.m_[1][2], m2.m_[1][2], t);
}


void
matrix::set_scale_rotation(float x_scale, float y_scale, float angle)
// Set the scale & rotation part of the matrix.
// angle in radians.
{
	float	cos_angle = cosf(angle);
	float	sin_angle = sinf(angle);
	m_[0][0] = utility::infinite_to_fzero(x_scale * cos_angle);
	m_[0][1] = utility::infinite_to_fzero(y_scale * -sin_angle);
	m_[1][0] = utility::infinite_to_fzero(x_scale * sin_angle);
	m_[1][1] = utility::infinite_to_fzero(y_scale * cos_angle);
}

void
matrix::set_x_scale(float xscale)
{
	float rotation = get_rotation();
	float yscale = get_y_scale();
	set_scale_rotation(xscale, yscale, rotation);
}

void
matrix::set_y_scale(float yscale)
{
	float rotation = get_rotation();
	float xscale = get_x_scale();
	set_scale_rotation(xscale, yscale, rotation);
}

void
matrix::set_scale(float xscale, float yscale)
{
	float rotation = get_rotation();
	set_scale_rotation(xscale, yscale, rotation);
}

void
matrix::set_rotation(float rotation)
{
	float xscale = get_x_scale();
	float yscale = get_y_scale();
	set_scale_rotation(xscale, yscale, rotation);
}


void
matrix::read(stream& in)
// Initialize from the stream.
{
	in.align();

	set_identity();

	in.ensureBits(1);
	bool	has_scale = in.read_bit(); 
	if (has_scale)
	{
		in.ensureBits(5);
		int	scale_nbits = in.read_uint(5);

		in.ensureBits(scale_nbits*2);
		m_[0][0] = in.read_sint(scale_nbits) / 65536.0f;
		m_[1][1] = in.read_sint(scale_nbits) / 65536.0f;
	}

	in.ensureBits(1);
	bool	has_rotate = in.read_bit();
	if (has_rotate)
	{
		in.ensureBits(5);
		int	rotate_nbits = in.read_uint(5);

		in.ensureBits(rotate_nbits*2);
		m_[1][0] = in.read_sint(rotate_nbits) / 65536.0f;
		m_[0][1] = in.read_sint(rotate_nbits) / 65536.0f;
	}

	in.ensureBits(5);
	int	translate_nbits = in.read_uint(5);
	if (translate_nbits > 0)
	{
		in.ensureBits(translate_nbits*2);
		m_[0][2] = (float) in.read_sint(translate_nbits);
		m_[1][2] = (float) in.read_sint(translate_nbits);
	}

	//IF_VERBOSE_PARSE(log_parse("  mat: has_scale = %d, has_rotate = %d\n", has_scale, has_rotate));
}


void
matrix::transform(point* result, const point& p) const
// Transform point 'p' by our matrix.  Put the result in
// *result.
{
	assert(result);

	result->x = m_[0][0] * p.x + m_[0][1] * p.y + m_[0][2];
	result->y = m_[1][0] * p.x + m_[1][1] * p.y + m_[1][2];
}

void
matrix::transform(geometry::Range2d<float>& r) const
{
	if ( ! r.isFinite() ) return;

	float xmin = r.getMinX();
	float xmax = r.getMaxX();
	float ymin = r.getMinY();
	float ymax = r.getMaxY();

        point p0(xmin, ymin);
        point p1(xmin, ymax);
        point p2(xmax, ymax);
        point p3(xmax, ymin);

        transform(p0);
        transform(p1);
        transform(p2);
        transform(p3);

        r.setTo(p0.x, p0.y);
        r.expandTo(p1.x, p1.y);
        r.expandTo(p2.x, p2.y);
        r.expandTo(p3.x, p3.y);
}

void
matrix::transform_vector(point* result, const point& v) const
// Transform vector 'v' by our matrix. Doesn't apply translation.
// Put the result in *result.
{
	assert(result);

	result->x = m_[0][0] * v.x + m_[0][1] * v.y;
	result->y = m_[1][0] * v.x + m_[1][1] * v.y;
}

void
matrix::transform_by_inverse(point* result, const point& p) const
// Transform point 'p' by the inverse of our matrix.  Put result in *result.
{
	// @@ TODO optimize this!
	matrix	m;
	m.set_inverse(*this);
	m.transform(result, p);
}

void
matrix::transform_by_inverse(point& p) const
// Transform point 'p' by the inverse of our matrix.  
{
	// @@ TODO optimize this!
	matrix	m;
	m.set_inverse(*this);
	m.transform(p);
}

void
matrix::transform_by_inverse(geometry::Range2d<float>& r) const
{
	// @@ TODO optimize this!
	matrix	m;
	m.set_inverse(*this);
	m.transform(r);
}


void
matrix::set_inverse(const matrix& m)
// Set this matrix to the inverse of the given matrix.
{
	assert(this != &m);

	// Invert the rotation part.
	float	det = m.m_[0][0] * m.m_[1][1] - m.m_[0][1] * m.m_[1][0];
	if (det == 0.0f)
	{
		// Not invertible.
		//abort();	// castano: this happens sometimes! (ie. sample6.swf)

		// Arbitrary fallback.
		set_identity();
		m_[0][2] = -m.m_[0][2];
		m_[1][2] = -m.m_[1][2];
	}
	else
	{
		float	inv_det = 1.0f / det;
		m_[0][0] = m.m_[1][1] * inv_det;
		m_[1][1] = m.m_[0][0] * inv_det;
		m_[0][1] = -m.m_[0][1] * inv_det;
		m_[1][0] = -m.m_[1][0] * inv_det;

		m_[0][2] = -(m_[0][0] * m.m_[0][2] + m_[0][1] * m.m_[1][2]);
		m_[1][2] = -(m_[1][0] * m.m_[0][2] + m_[1][1] * m.m_[1][2]);
	}
}


bool
matrix::does_flip() const
// Return true if this matrix reverses handedness.
{
	float	det = m_[0][0] * m_[1][1] - m_[0][1] * m_[1][0];

	return det < 0.f;
}


float
matrix::get_determinant() const
// Return the determinant of the 2x2 rotation/scale part only.
{
	return m_[0][0] * m_[1][1] - m_[1][0] * m_[0][1];
}


float
matrix::get_max_scale() const
// Return the maximum scale factor that this transform
// applies.  For assessing scale, when determining acceptable
// errors in tesselation.
{
	// @@ not 100% sure what the heck I'm doing here.  I
	// think this is roughly what I want; take the max
	// length of the two basis vectors.

	//float	basis0_length2 = m_[0][0] * m_[0][0] + m_[0][1] * m_[0][1];
	float	basis0_length2 = m_[0][0] * m_[0][0] + m_[1][0] * m_[1][0];

	//float	basis1_length2 = m_[1][0] * m_[1][0] + m_[1][1] * m_[1][1];
	float	basis1_length2 = m_[0][1] * m_[0][1] + m_[1][1] * m_[1][1];

	float	max_length2 = std::max(basis0_length2, basis1_length2);
	return sqrtf(max_length2);
}

float
matrix::get_x_scale() const
{
	// Scale is applied before rotation, must match implementation
	// in set_scale_rotation
	float	scale = sqrtf(m_[0][0] * m_[0][0] + m_[1][0] * m_[1][0]);

	// Are we turned inside out?
	if (get_determinant() < 0.f)
	{
		scale = -scale;
	}

	return scale;
}

float
matrix::get_y_scale() const
{
	// Scale is applied before rotation, must match implementation
	// in set_scale_rotation
	return sqrtf(m_[1][1] * m_[1][1] + m_[0][1] * m_[0][1]);
}

float
matrix::get_rotation() const
{
	if (get_determinant() < 0.f)
	{
		// We're turned inside out; negate the
		// x-component used to compute rotation.
		//
		// Matches get_x_scale().
		//
		// @@ this may not be how Macromedia does it!  Test this!
		return atan2f(m_[1][0], -m_[0][0]);
	}
	else
	{
		return atan2f(m_[1][0], m_[0][0]);
	}
}

std::ostream& operator<< (std::ostream& o, const matrix& m)
{
    // 8 digits and a decimal point.
    const short fieldWidth = 9;

    o << std::endl << "| "
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) << m.m_[0][0] << " "
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) << m.m_[0][1] << " "
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) << TWIPS_TO_PIXELS(m.m_[0][2])
      << " |" 
      << std::endl << "| "
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) << m.m_[1][0] << " "
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) << m.m_[1][1] << " "
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) << TWIPS_TO_PIXELS(m.m_[1][2])
      << " |";
      
      return o;
}

}	// end namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
