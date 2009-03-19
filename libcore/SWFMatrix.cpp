// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "SWFMatrix.h"
#include "SWFStream.h" // for reading from SWF
#include "log.h"
#include "GnashNumeric.h"

#include <cmath>
#include <iomanip>

namespace gnash {

namespace {

inline boost::int32_t
DoubleToFixed16(double a)
{
    return truncateWithFactor<65536>(a);
}

inline boost::int32_t
Fixed16Mul(boost::int32_t a, boost::int32_t b)
{
    // truncate when overflow occurs.
    return static_cast<boost::int32_t>(
            (static_cast<boost::int64_t>(a) *
             static_cast<boost::int64_t>(b) + 0x8000) >> 16);
}

} // anonymous namepace

SWFMatrix::SWFMatrix()
{
    // Default to identity.
    sx = sy = 65536;
    shx = shy = tx = ty = 0;
}

void
SWFMatrix::read(SWFStream& in)
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
SWFMatrix::is_valid() const
{
    // The integer SWFMatrix is always valid now from outside.
    // swallow it if anything wrong inside this class.
    return true;
}

void
SWFMatrix::transform(geometry::Point2d& p) const
{
    boost::int32_t t0 = Fixed16Mul(sx, p.x) + Fixed16Mul(shy, p.y) + tx;
    boost::int32_t t1 = Fixed16Mul(shx,p.x) + Fixed16Mul(sy,  p.y) + ty;
    p.x = t0;
    p.y = t1;
}

void
SWFMatrix::transform(boost::int32_t& x, boost::int32_t& y) const
{
    boost::int32_t  t0 = Fixed16Mul(sx, x) + Fixed16Mul(shy, y) + tx;
    boost::int32_t  t1 = Fixed16Mul(shx,x) + Fixed16Mul(sy,  y) + ty;
    x = t0;
    y = t1;
}

void
SWFMatrix::set_identity()
// Set the SWFMatrix to identity.
{
    sx = sy = 65536;
    shx = shy = tx = ty = 0;
}

void
SWFMatrix::concatenate(const SWFMatrix& m)
// Concatenate m's transform onto ours.  When
// transforming points, m happens first, then our
// original SWFMatrix.
{
    SWFMatrix  t;
    t.sx =  Fixed16Mul(sx, m.sx)  + Fixed16Mul(shy, m.shx);
    t.shx = Fixed16Mul(shx, m.sx) + Fixed16Mul(sy, m.shx);
    t.shy = Fixed16Mul(sx, m.shy) + Fixed16Mul(shy, m.sy);
    t.sy =  Fixed16Mul(shx, m.shy)+ Fixed16Mul(sy, m.sy);
    t.tx =  Fixed16Mul(sx, m.tx)  + Fixed16Mul(shy, m.ty) + tx;
    t.ty =  Fixed16Mul(shx, m.tx) + Fixed16Mul(sy, m.ty)  + ty;

    *this = t;
}

void
SWFMatrix::concatenate_translation(int xoffset, int yoffset)
// Concatenate a translation onto the front of our
// SWFMatrix.  When transforming points, the translation
// happens first, then our original xform.
{
    tx += Fixed16Mul(sx,  xoffset) + Fixed16Mul(shy, yoffset);
    ty += Fixed16Mul(shx, xoffset) + Fixed16Mul(sy, yoffset);
}

void
SWFMatrix::concatenate_scale(double xscale, double yscale)
// Concatenate scales to our SWFMatrix. When transforming points, these 
// scales happen first, then our matirx.
{
    sx  = Fixed16Mul(sx, DoubleToFixed16(xscale));
    shy = Fixed16Mul(shy,DoubleToFixed16(yscale));
    shx = Fixed16Mul(shx,DoubleToFixed16(xscale));
    sy  = Fixed16Mul(sy, DoubleToFixed16(yscale)); 
}

void
SWFMatrix::set_lerp(const SWFMatrix& m1, const SWFMatrix& m2, float t)
// Set this SWFMatrix to a blend of m1 and m2, parameterized by t.
{
    sx = flerp(m1.sx, m2.sx, t);
    shx = flerp(m1.shx, m2.shx, t);
    shy = flerp(m1.shy, m2.shy, t);
    sy = flerp(m1.sy, m2.sy, t);
    tx = flerp(m1.tx, m2.tx, t);
    ty = flerp(m1.ty, m2.ty, t);
}

void
SWFMatrix::set_scale_rotation(double x_scale, double y_scale, double angle)
// Set the scale & rotation part of the SWFMatrix.
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
SWFMatrix::set_x_scale(double xscale)
{
    double rot_x = atan2((double)shx, (double)sx);
    sx  =  DoubleToFixed16(xscale * cos(rot_x));
    shx =  DoubleToFixed16(xscale * sin(rot_x)); 
}

void
SWFMatrix::set_y_scale(double yscale)
{
    double rot_y = std::atan2((double)(-shy), (double)(sy));

    shy = -DoubleToFixed16(yscale * std::sin(rot_y));
    sy  =  DoubleToFixed16(yscale * std::cos(rot_y));
}

void
SWFMatrix::set_scale(double xscale, double yscale)
{
    double rotation = get_rotation();
    set_scale_rotation(xscale, yscale, rotation); 
}

void
SWFMatrix::set_rotation(double rotation)
{   
    double rot_x = atan2((double)shx,    (double)sx);
    double rot_y = atan2((double)(-shy), (double)sy);
    double scale_x = get_x_scale();
    double scale_y = get_y_scale();
 
    sx  = DoubleToFixed16(scale_x * cos(rotation));
    shx = DoubleToFixed16(scale_x * sin(rotation)); 
    shy = -DoubleToFixed16(scale_y * sin(rot_y - rot_x + rotation));
    sy  =  DoubleToFixed16(scale_y * cos(rot_y - rot_x + rotation));
}

void
SWFMatrix::transform(point* result, const point& p) const
// Transform point 'p' by our SWFMatrix.  Put the result in *result.
{
    assert(result);

    result->x = Fixed16Mul(sx,  p.x) + Fixed16Mul(shy, p.y) + tx;
    result->y = Fixed16Mul(shx, p.x) + Fixed16Mul(sy,  p.y) + ty;
}

void
SWFMatrix::transform(geometry::Range2d<float>& r) const
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
SWFMatrix::transform(rect& r) const
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

const SWFMatrix&
SWFMatrix::invert()
// invert this SWFMatrix and return the result.
{
    boost::int64_t det = determinant();
    if(det == 0)
    {
        //log_debug("Matrix not invertible, setting to identity on invert request");
        // tested in misc-ming.all/SWFMatrix_test.c (seek "SWFMatrix inversion")
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
SWFMatrix::get_x_scale() const
{
    return sqrt(((double)sx * sx + (double)shx * shx)) / 65536.0;
}

double
SWFMatrix::get_y_scale() const
{
    return sqrt(((double)sy * sy + (double)shy * shy)) / 65536.0;
}

double
SWFMatrix::get_rotation() const
{
    // more successes in misc-ming.all/SWFMatrix_test.c
    return atan2(static_cast<double>(shx), sx); 
}

// private
boost::int64_t
SWFMatrix::determinant() const
// Return the 32.32 fixed point determinant of this SWFMatrix.
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

std::ostream& operator<< (std::ostream& o, const SWFMatrix& m)
{
    // 8 digits and a decimal point.
    const short fieldWidth = 9;

    o << std::endl << "|"
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) 
      << m.sx / 65536.0 << " "
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) 
      << m.shy/ 65536.0 << " "
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) 
      << twipsToPixels(m.tx) << " |" 
      << std::endl << "|"
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) 
      << m.shx/ 65536.0 << " "
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) 
      << m.sy / 65536.0 << " "
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) 
      << twipsToPixels(m.ty) << " |";
      
      return o;
}

}   // end namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
