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
// $Id: matrix.cpp,v 1.2 2006/10/11 09:03:56 strk Exp $ 
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "matrix.h"
#include "stream.h" // for reading from SWF
#include "types.h" // for TWIPS_TO_PIXEL define
                   // (should probably not use it though)
#include "log.h"

#ifndef HAVE_ISFINITE
# ifndef isfinite 
#  define isfinite finite
# endif 
#endif 

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
	return isfinite(m_[0][0])
		&& isfinite(m_[0][1])
		&& isfinite(m_[0][2])
		&& isfinite(m_[1][0])
		&& isfinite(m_[1][1])
		&& isfinite(m_[1][2]);
}


void
matrix::set_identity()
// Set the matrix to identity.
{
	memset(&m_[0], 0, sizeof(m_));
	m_[0][0] = 1;
	m_[1][1] = 1;
}

std::ostream& operator<< (std::ostream& os, const matrix& m)
{
	os << "| " << m.m_[0][0] << " "
		<< m.m_[0][1] << " "
		<< TWIPS_TO_PIXELS(m.m_[0][2]) << " |"
		<< std::endl;

	os << "| " << m.m_[1][0] << " "
		<< m.m_[1][1] << " "
		<< TWIPS_TO_PIXELS(m.m_[1][2])
		<< " |" << std::endl;

	return os;
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
	m_[0][2] += infinite_to_fzero(m_[0][0] * tx + m_[0][1] * ty);
	m_[1][2] += infinite_to_fzero(m_[1][0] * tx + m_[1][1] * ty);
}


void
matrix::concatenate_scale(float scale)
// Concatenate a uniform scale onto the front of our
// matrix.  When transforming points, the scale
// happens first, then our original xform.
{
	m_[0][0] *= infinite_to_fzero(scale);
	m_[0][1] *= infinite_to_fzero(scale);
	m_[1][0] *= infinite_to_fzero(scale);
	m_[1][1] *= infinite_to_fzero(scale);
}


void
matrix::set_lerp(const matrix& m1, const matrix& m2, float t)
// Set this matrix to a blend of m1 and m2, parameterized by t.
{
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
	m_[0][0] = infinite_to_fzero(x_scale * cos_angle);
	m_[0][1] = infinite_to_fzero(y_scale * -sin_angle);
	m_[1][0] = infinite_to_fzero(x_scale * sin_angle);
	m_[1][1] = infinite_to_fzero(y_scale * cos_angle);
}


void
matrix::read(stream* in)
// Initialize from the stream.
{
	in->align();

	set_identity();

	int	has_scale = in->read_uint(1);
	if (has_scale)
	{
		int	scale_nbits = in->read_uint(5);
		m_[0][0] = in->read_sint(scale_nbits) / 65536.0f;
		m_[1][1] = in->read_sint(scale_nbits) / 65536.0f;
	}
	int	has_rotate = in->read_uint(1);
	if (has_rotate)
	{
		int	rotate_nbits = in->read_uint(5);
		m_[1][0] = in->read_sint(rotate_nbits) / 65536.0f;
		m_[0][1] = in->read_sint(rotate_nbits) / 65536.0f;
	}

	int	translate_nbits = in->read_uint(5);
	if (translate_nbits > 0)
	{
		m_[0][2] = (float) in->read_sint(translate_nbits);
		m_[1][2] = (float) in->read_sint(translate_nbits);
	}

	//IF_VERBOSE_PARSE(log_msg("  mat: has_scale = %d, has_rotate = %d\n", has_scale, has_rotate));
}


void
matrix::print() const
// Debug log.
{
	IF_VERBOSE_PARSE( 
	log_parse("| %4.4f %4.4f %4.4f |", m_[0][0], m_[0][1], TWIPS_TO_PIXELS(m_[0][2]));
	log_parse("| %4.4f %4.4f %4.4f |", m_[1][0], m_[1][1], TWIPS_TO_PIXELS(m_[1][2]));
	);
}

void
matrix::transform(point* result, const point& p) const
// Transform point 'p' by our matrix.  Put the result in
// *result.
{
	assert(result);

	result->m_x = m_[0][0] * p.m_x + m_[0][1] * p.m_y + m_[0][2];
	result->m_y = m_[1][0] * p.m_x + m_[1][1] * p.m_y + m_[1][2];
}

void
matrix::transform_vector(point* result, const point& v) const
// Transform vector 'v' by our matrix. Doesn't apply translation.
// Put the result in *result.
{
	assert(result);

	result->m_x = m_[0][0] * v.m_x + m_[0][1] * v.m_y;
	result->m_y = m_[1][0] * v.m_x + m_[1][1] * v.m_y;
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
matrix::set_inverse(const matrix& m)
// Set this matrix to the inverse of the given matrix.
{
	assert(this != &m);

	// Invert the rotation part.
	float	det = m.m_[0][0] * m.m_[1][1] - m.m_[0][1] * m.m_[1][0];
	if (det == 0.0f)
	{
		// Not invertible.
		//assert(0);	// castano: this happens sometimes! (ie. sample6.swf)

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
	float	basis0_length2 = m_[0][0] * m_[0][0] + m_[0][1] * m_[0][1];
	float	basis1_length2 = m_[1][0] * m_[1][0] + m_[1][1] * m_[1][1];
	float	max_length2 = fmax(basis0_length2, basis1_length2);
	return sqrtf(max_length2);
}

float
matrix::get_x_scale() const
{
	float	scale = sqrtf(m_[0][0] * m_[0][0] + m_[0][1] * m_[0][1]);

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
	return sqrtf(m_[1][1] * m_[1][1] + m_[1][0] * m_[1][0]);
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


}	// end namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
