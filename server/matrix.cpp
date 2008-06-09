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

matrix::matrix()
{
    // Default to identity.
    sx = sy = 65536;
    shx = shy = tx = ty = 0;
}

void
matrix::read(SWFStream& in)
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
		sx = in.read_sint(scale_nbits);
		sy = in.read_sint(scale_nbits);
	}

	in.ensureBits(1);
	bool	has_rotate = in.read_bit();
	if (has_rotate)
	{
		in.ensureBits(5);
		int	rotate_nbits = in.read_uint(5);

		in.ensureBits(rotate_nbits*2);
		shx = in.read_sint(rotate_nbits);
		shy = in.read_sint(rotate_nbits);
	}

	in.ensureBits(5);
	int	translate_nbits = in.read_uint(5);
	if (translate_nbits > 0)
	{
		in.ensureBits(translate_nbits*2);
		tx = (float) in.read_sint(translate_nbits);
		ty = (float) in.read_sint(translate_nbits);
	}
}

bool
matrix::is_valid() const
{
	return true;
}


void
matrix::set_identity()
// Set the matrix to identity.
{
    sx = sy = 65536;
    shx = shy = tx = ty = 0;
}

void
matrix::concatenate(const matrix& m)
// Concatenate m's transform onto ours.  When
// transforming points, m happens first, then our
// original xform.
{
	matrix	t;
	t.sx =  Fixed16Mul(sx, m.sx)  + Fixed16Mul(shy, m.shx);
	t.shx = Fixed16Mul(shx, m.sx) + Fixed16Mul(sy, m.shx);
	t.shy = Fixed16Mul(sx, m.shy) + Fixed16Mul(shy, m.sy);
	t.sy =  Fixed16Mul(shx, m.shy)+ Fixed16Mul(sy, m.sy);
	t.tx =  Fixed16Mul(sx, m.tx)  + Fixed16Mul(shy, m.ty) + tx;
	t.ty =  Fixed16Mul(shx, m.tx) + Fixed16Mul(sy, m.ty)  + ty;

	*this = t;
}


void
matrix::concatenate_translation(float xoffset, float yoffset)
// Concatenate a translation onto the front of our
// matrix.  When transforming points, the translation
// happens first, then our original xform.
{
	tx += Fixed16Mul(sx,  xoffset) + Fixed16Mul(shy, yoffset);
	ty += Fixed16Mul(shx, xoffset) + Fixed16Mul(sy, yoffset);
}


void
matrix::concatenate_scale(float scale)
// Concatenate a uniform scale onto the front of our
// matrix.  When transforming points, the scale
// happens first, then our original xform.
{
	sx *= utility::infinite_to_fzero(scale);
	shy *= utility::infinite_to_fzero(scale);
	shx *= utility::infinite_to_fzero(scale);
	sy *= utility::infinite_to_fzero(scale);
}

void	
matrix::concatenate_scales(float x, float y)
// Just like concatenate_scale() but with different scales for x/y
{
	matrix m2; 
    m2.set_scale_rotation(x, y, 0);
	concatenate(m2);
}

void
matrix::set_lerp(const matrix& m1, const matrix& m2, float t)
// Set this matrix to a blend of m1 and m2, parameterized by t.
{
    using utility::flerp;
	sx = flerp(m1.sx, m2.sx, t);
	shx = flerp(m1.shx, m2.shx, t);
	shy = flerp(m1.shy, m2.shy, t);
	sy = flerp(m1.sy, m2.sy, t);
	tx = flerp(m1.tx, m2.tx, t);
	ty = flerp(m1.ty, m2.ty, t);
}


void
matrix::set_scale_rotation(float x_scale, float y_scale, float angle)
// Set the scale & rotation part of the matrix.
// angle in radians.
{
	float	cos_angle = cosf(angle);
	float	sin_angle = sinf(angle);
	sx  = 65536.0f * x_scale * cos_angle;
	shy = 65536.0f * y_scale * -sin_angle;
	shx = 65536.0f * x_scale * sin_angle;
	sy  = 65536.0f * y_scale * cos_angle;
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
matrix::transform(point* result, const point& p) const
// Transform point 'p' by our matrix.  Put the result in *result.
{
	assert(result);

	result->x = Fixed16Mul(sx,  p.x) + Fixed16Mul(shy, p.y) + tx;
	result->y = Fixed16Mul(shx, p.x) + Fixed16Mul(sy,  p.y) + ty;
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

	result->x = Fixed16Mul(sx, v.x) + Fixed16Mul(shy, v.y);
	result->y = Fixed16Mul(sy, v.x) + Fixed16Mul(shx, v.y);
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

	boost::int64_t  det = m.get_determinant();
	if (det == 0)
	{
        // cann't invert this matrix, set it to identity.
        // this might happen when xscale == yscale == 0
		set_identity();
	}
	else
	{
		double  inv_det = 65536.0 * 65536.0 / det;
		sx  = (boost::int32_t)(m.sy * inv_det);
		sy  = (boost::int32_t)(m.sx * inv_det);
		shy = -(boost::int32_t)(m.shy * inv_det);
		shx = -(boost::int32_t)(m.shx * inv_det);
        
        tx = -( Fixed16Mul(sx,  m.tx) + Fixed16Mul(shy, m.ty) );
        ty = -( Fixed16Mul(shx, m.tx) + Fixed16Mul(sy,  m.ty) );
	}
}


bool
matrix::does_flip() const
// Return true if this matrix reverses handedness.
{
    return ((boost::int64_t)sx * sy) < ((boost::int64_t)shx * shy);
}


boost::int64_t
matrix::get_determinant() const
// Return the 32.32 fixed point determinant of this matrix.
{
	return (boost::int64_t)sx * sy - (boost::int64_t)shx * shy;
}

float
matrix::get_x_scale() const
{
	return sqrtf(((float)sx * sx + (float)shx * shx)) / 65536.0f;
}

float
matrix::get_y_scale() const
{
	return sqrtf(((float)sy * sy + (float)shy * shy)) / 65536.0f;
}

float
matrix::get_rotation() const
{
	if (get_determinant() < 0)
	{
        // TODO: check this.
		return atan2f(shx, -sx);
	}
	else
	{
		return atan2f(shx, sx);
	}
}

std::ostream& operator<< (std::ostream& o, const matrix& m)
{
    // 8 digits and a decimal point.
    const short fieldWidth = 9;

    o << std::endl << "|"
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) 
      << m.sx/65536.0 << " "
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) 
      << m.shy/65536.0 << " "
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) 
      << TWIPS_TO_PIXELS(m.tx) << " |" 
      << std::endl << "|"
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) 
      << m.shx/65536.0 << " "
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) 
      << m.sy/65536.0 << " "
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) 
      << TWIPS_TO_PIXELS(m.ty) << " |";
      
      return o;
}

}	// end namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
