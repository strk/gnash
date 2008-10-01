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

#ifdef HAVE_PTHREADS
#include <pthread.h>
#endif

#include "matrix.h"
#include "SWFStream.h" // for reading from SWF
#include "log.h"

#include <cmath>
#include <iomanip>

// Define this to use new math for matrix operation.
// This is for testing, zou is still working on it
#define NEW_MATRIX_MATH 1

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
    bool    has_scale = in.read_bit(); 
    if (has_scale)
    {
        in.ensureBits(5);
        int scale_nbits = in.read_uint(5);

        in.ensureBits(scale_nbits * 2);
        sx = in.read_sint(scale_nbits);
        sy = in.read_sint(scale_nbits);
    }

    in.ensureBits(1);
    bool  has_rotate = in.read_bit();
    if (has_rotate)
    {
        in.ensureBits(5);
        int rotate_nbits = in.read_uint(5);

        in.ensureBits(rotate_nbits * 2);
        shx = in.read_sint(rotate_nbits);
        shy = in.read_sint(rotate_nbits);
    }

    in.ensureBits(5);
    int translate_nbits = in.read_uint(5);
    if (translate_nbits > 0)
    {
        in.ensureBits(translate_nbits * 2);
        tx = in.read_sint(translate_nbits);
        ty = in.read_sint(translate_nbits);
    }
}

bool
matrix::is_valid() const
{
    // The integer matrix is always valid now from outside.
    // swallow it if anything wrong inside this class.
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
// original matrix.
{
    matrix  t;
    t.sx =  Fixed16Mul(sx, m.sx)  + Fixed16Mul(shy, m.shx);
    t.shx = Fixed16Mul(shx, m.sx) + Fixed16Mul(sy, m.shx);
    t.shy = Fixed16Mul(sx, m.shy) + Fixed16Mul(shy, m.sy);
    t.sy =  Fixed16Mul(shx, m.shy)+ Fixed16Mul(sy, m.sy);
    t.tx =  Fixed16Mul(sx, m.tx)  + Fixed16Mul(shy, m.ty) + tx;
    t.ty =  Fixed16Mul(shx, m.tx) + Fixed16Mul(sy, m.ty)  + ty;

    *this = t;
}

void
matrix::concatenate_translation(int xoffset, int yoffset)
// Concatenate a translation onto the front of our
// matrix.  When transforming points, the translation
// happens first, then our original xform.
{
    tx += Fixed16Mul(sx,  xoffset) + Fixed16Mul(shy, yoffset);
    ty += Fixed16Mul(shx, xoffset) + Fixed16Mul(sy, yoffset);
}

void
matrix::concatenate_scale(double xscale, double yscale)
// Concatenate scales to our matrix. When transforming points, these 
// scales happen first, then our matirx.
{
    sx  = Fixed16Mul(sx, DoubleToFixed16(xscale));
    shy = Fixed16Mul(shy,DoubleToFixed16(yscale));
    shx = Fixed16Mul(shx,DoubleToFixed16(xscale));
    sy  = Fixed16Mul(sy, DoubleToFixed16(yscale)); 
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
matrix::set_scale_rotation(double x_scale, double y_scale, double angle)
// Set the scale & rotation part of the matrix.
// angle in radians.
{
    double   cos_angle = cos(angle);
    double   sin_angle = sin(angle);
    sx  = DoubleToFixed16(x_scale * cos_angle);
    shy = DoubleToFixed16(y_scale * -sin_angle);
    shx = DoubleToFixed16(x_scale * sin_angle);
    sy  = DoubleToFixed16(y_scale * cos_angle); 
}

void
matrix::set_x_scale(double xscale)
{
#ifdef NEW_MATRIX_MATH
    double rot_x = atan2((double)shx, (double)sx);
    sx  =  DoubleToFixed16(xscale * cos(rot_x));
    shx =  DoubleToFixed16(xscale * sin(rot_x)); 
#else
    double angle = get_rotation();
    double cos_v = cos(angle);
    double sin_v = sin(angle);
    sx  =  DoubleToFixed16(xscale * cos_v);
    shx =  DoubleToFixed16(xscale * sin_v);
#endif
}

void
matrix::set_y_scale(double yscale)
{
#ifdef NEW_MATRIX_MATH
    double rot_y = std::atan2((double)(-shy), (double)(sy));

    shy = -DoubleToFixed16(yscale * std::sin(rot_y));
    sy  =  DoubleToFixed16(yscale * std::cos(rot_y));

#else
    double angle = get_rotation();
    double cos_v = cos(angle);
    double sin_v = sin(angle);
    shy =  - DoubleToFixed16(yscale * sin_v);
    sy  =  DoubleToFixed16(yscale * cos_v);
#endif
}

void
matrix::set_scale(double xscale, double yscale)
{
    double rotation = get_rotation();
    set_scale_rotation(xscale, yscale, rotation); 
}

void
matrix::set_rotation(double rotation)
{   
#ifdef NEW_MATRIX_MATH
    double rot_x = atan2((double)shx,    (double)sx);
    double rot_y = atan2((double)(-shy), (double)sy);
    double scale_x = get_x_scale();
    double scale_y = get_y_scale();
 
    sx  = DoubleToFixed16(scale_x * cos(rotation));
    shx = DoubleToFixed16(scale_x * sin(rotation)); 
    shy = -DoubleToFixed16(scale_y * sin(rot_y - rot_x + rotation));
    sy  =  DoubleToFixed16(scale_y * cos(rot_y - rot_x + rotation));
#else
    double xscale = get_x_scale();
    double yscale = get_y_scale();
    set_scale_rotation(xscale, yscale, rotation);
#endif
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
matrix::transform(rect& r) const
{
    if ( r.is_null() ) return;

    boost::int32_t x1 = r.get_x_min();
    boost::int32_t y1 = r.get_y_min();
    boost::int32_t x2 = r.get_x_max();
    boost::int32_t y2 = r.get_y_max();

    point p0(x1, y1);
    point p1(x2, y1);
    point p2(x2, y2);
    point p3(x1, y2);

    transform(p0);
    transform(p1);
    transform(p2);
    transform(p3);

    r.set_to_point(p0.x, p0.y);
    r.expand_to_point(p1.x, p1.y);
    r.expand_to_point(p2.x, p2.y);
    r.expand_to_point(p3.x, p3.y);
}

const matrix&
matrix::invert()
// invert this matrix and return the result.
{
    boost::int64_t det = determinant();
    if(det == 0)
    {
        //log_debug("Matrix not invertible, setting to identity on invert request");
        // tested in misc-ming.all/matrix_test.c (seek "matrix inversion")
        set_identity();
    }
    else
    {
        double  d = 65536.0 * 65536.0 / det;
        boost::int32_t t0, t4;
        
        t0  = (boost::int32_t)(sy * d);
        sy  = (boost::int32_t)(sx * d);
        shy = (boost::int32_t)(-shy * d);
        shx = (boost::int32_t)(-shx * d);

        t4 = - ( Fixed16Mul(tx, t0) + Fixed16Mul(ty, shy) );
        ty = - ( Fixed16Mul(tx, shx)+ Fixed16Mul(ty, sy) );

        sx = t0;
        tx = t4;
    }

    return *this;
}

double
matrix::get_x_scale() const
{
    return sqrt(((double)sx * sx + (double)shx * shx)) / 65536.0;
}

double
matrix::get_y_scale() const
{
    return sqrt(((double)sy * sy + (double)shy * shy)) / 65536.0;
}

double
matrix::get_rotation() const
{
    return atan2(shx, sx); // more successes in misc-ming.all/matrix_test.c
}

// private
boost::int64_t
matrix::determinant() const
// Return the 32.32 fixed point determinant of this matrix.
{
    // | sx	shy	tx |
    // | shx	sy	ty |   = T. Using the Leibniz formula:
    // | 0	0	1  |
    //
    // Det(T) = ( (sx * sy * 1 ) + (shy * ty * 0) + (tx * shx *  0) ) -
    //          ( (0  * sy * tx) + (0  * ty * sx) + (1 * shy * shx) )
    //        = sx * sy - shx * shy

    return (boost::int64_t)sx * sy - (boost::int64_t)shx * shy;
}

std::ostream& operator<< (std::ostream& o, const matrix& m)
{
    // 8 digits and a decimal point.
    const short fieldWidth = 9;

    o << std::endl << "|"
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) 
      << m.sx / 65536.0 << " "
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) 
      << m.shy/ 65536.0 << " "
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) 
      << TWIPS_TO_PIXELS(m.tx) << " |" 
      << std::endl << "|"
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) 
      << m.shx/ 65536.0 << " "
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) 
      << m.sy / 65536.0 << " "
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) 
      << TWIPS_TO_PIXELS(m.ty) << " |";
      
      return o;
}

}   // end namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
