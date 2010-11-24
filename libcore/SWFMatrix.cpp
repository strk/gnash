// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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
// Original author: Thatcher Ulrich <tu@tulrich.com> 2003
//

#include "SWFMatrix.h"

#include <cmath>
#include <iomanip>

#include "log.h"
#include "GnashNumeric.h"
#include "SWFRect.h"
#include "Point2d.h"


// This class intentionally uses overflows, which are not allowed in
// signed types; apart from being UB always, in practice it produces
// different results on different platforms.
//
// To avoid this, all calculations where an overflow could occur
// should use only unsigned types, but assign to the signed SWFMatrix
// members using only signed types. This would be much easier
// if the matrix values were also unsigned but were converted to
// signed for external users.
namespace gnash {

namespace {

inline boost::int32_t
toFixed16(double a)
{
    return truncateWithFactor<65536>(a);
}

inline boost::int32_t
multiplyFixed16(boost::int32_t a, boost::int32_t b)
{
    // Overflows are permitted only in unsigned types, so
    // convert using two's complement first.
    const boost::uint32_t mult = 
        (to_unsigned<boost::int64_t>(a) * to_unsigned<boost::int64_t>(b) + 0x8000) >> 16;

    // Convert back.
    return to_signed<boost::uint32_t>(mult);
}

} // anonymous namepace

void
SWFMatrix::transform(geometry::Point2d& p) const
{
    boost::int32_t t0 = multiplyFixed16(sx, p.x) + multiplyFixed16(shy, p.y) + tx;
    boost::int32_t t1 = multiplyFixed16(shx,p.x) + multiplyFixed16(sy,  p.y) + ty;
    p.x = t0;
    p.y = t1;
}

void
SWFMatrix::transform(boost::int32_t& x, boost::int32_t& y) const
{
    const boost::int32_t t0 = multiplyFixed16(sx, x) + multiplyFixed16(shy, y) + tx;
    const boost::int32_t t1 = multiplyFixed16(shx,x) + multiplyFixed16(sy,  y) + ty;
    x = t0;
    y = t1;
}

void
SWFMatrix::transform(geometry::Range2d<boost::int32_t>& r) const
{
    const boost::int32_t xmin = r.getMinX();
    const boost::int32_t xmax = r.getMaxX();
    const boost::int32_t ymin = r.getMinY();
    const boost::int32_t ymax = r.getMaxY();

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
SWFMatrix::set_identity()
{
    sx = sy = 65536;
    shx = shy = tx = ty = 0;
}

void
SWFMatrix::concatenate(const SWFMatrix& m)
{
    SWFMatrix t;
    t.sx =  multiplyFixed16(sx, m.sx)  + multiplyFixed16(shy, m.shx);
    t.shx = multiplyFixed16(shx, m.sx) + multiplyFixed16(sy, m.shx);
    t.shy = multiplyFixed16(sx, m.shy) + multiplyFixed16(shy, m.sy);
    t.sy =  multiplyFixed16(shx, m.shy)+ multiplyFixed16(sy, m.sy);
    t.tx =  multiplyFixed16(sx, m.tx)  + multiplyFixed16(shy, m.ty) + tx;
    t.ty =  multiplyFixed16(shx, m.tx) + multiplyFixed16(sy, m.ty)  + ty;

    *this = t;
}

// Concatenate a translation onto the front of our
// SWFMatrix.  When transforming points, the translation
// happens first, then our original xform.
void
SWFMatrix::concatenate_translation(int xoffset, int yoffset)
{
    tx += multiplyFixed16(sx,  xoffset) + multiplyFixed16(shy, yoffset);
    ty += multiplyFixed16(shx, xoffset) + multiplyFixed16(sy, yoffset);
}

// Concatenate scales to our SWFMatrix. When transforming points, these 
// scales happen first, then our matirx.
void
SWFMatrix::concatenate_scale(double xscale, double yscale)
{
    sx  = multiplyFixed16(sx, toFixed16(xscale));
    shy = multiplyFixed16(shy,toFixed16(yscale));
    shx = multiplyFixed16(shx,toFixed16(xscale));
    sy  = multiplyFixed16(sy, toFixed16(yscale)); 
}

// Set this SWFMatrix to a blend of m1 and m2, parameterized by t.
void
SWFMatrix::set_lerp(const SWFMatrix& m1, const SWFMatrix& m2, float t)
{
    sx = lerp<float>(m1.sx, m2.sx, t);
    shx = lerp<float>(m1.shx, m2.shx, t);
    shy = lerp<float>(m1.shy, m2.shy, t);
    sy = lerp<float>(m1.sy, m2.sy, t);
    tx = lerp<float>(m1.tx, m2.tx, t);
    ty = lerp<float>(m1.ty, m2.ty, t);
}

// Set the scale & rotation part of the SWFMatrix.
// angle in radians.
void
SWFMatrix::set_scale_rotation(double x_scale, double y_scale, double angle)
{
    const double cos_angle = std::cos(angle);
    const double sin_angle = std::sin(angle);
    sx  = toFixed16(x_scale * cos_angle);
    shy = toFixed16(y_scale * -sin_angle);
    shx = toFixed16(x_scale * sin_angle);
    sy  = toFixed16(y_scale * cos_angle); 
}

void
SWFMatrix::set_x_scale(double xscale)
{
    const double rot_x =
        std::atan2(static_cast<double>(shx), static_cast<double>(sx));
    sx = toFixed16(xscale * std::cos(rot_x));
    shx = toFixed16(xscale * std::sin(rot_x)); 
}

void
SWFMatrix::set_y_scale(double yscale)
{
    const double rot_y =
        std::atan2(static_cast<double>(-shy), static_cast<double>(sy));

    shy = -toFixed16(yscale * std::sin(rot_y));
    sy = toFixed16(yscale * std::cos(rot_y));
}

void
SWFMatrix::set_scale(double xscale, double yscale)
{
    const double rotation = get_rotation();
    set_scale_rotation(xscale, yscale, rotation); 
}

void
SWFMatrix::set_rotation(double rotation)
{   
    const double rot_x =
        std::atan2(static_cast<double>(shx), static_cast<double>(sx));
    const double rot_y =
        std::atan2(static_cast<double>(-shy), static_cast<double>(sy));
    const double scale_x = get_x_scale();
    const double scale_y = get_y_scale();
 
    sx = toFixed16(scale_x * std::cos(rotation));
    shx = toFixed16(scale_x * std::sin(rotation)); 
    shy = -toFixed16(scale_y * std::sin(rot_y - rot_x + rotation));
    sy = toFixed16(scale_y * std::cos(rot_y - rot_x + rotation));
}

// Transform point 'p' by our SWFMatrix.  Put the result in *result.
void
SWFMatrix::transform(point* result, const point& p) const
{
    assert(result);

    result->x = multiplyFixed16(sx,  p.x) + multiplyFixed16(shy, p.y) + tx;
    result->y = multiplyFixed16(shx, p.x) + multiplyFixed16(sy,  p.y) + ty;
}

void 
SWFMatrix::transform(SWFRect& r) const
{
    if (r.is_null()) return;

    const boost::int32_t x1 = r.get_x_min();
    const boost::int32_t y1 = r.get_y_min();
    const boost::int32_t x2 = r.get_x_max();
    const boost::int32_t y2 = r.get_y_max();

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

// invert this SWFMatrix and return the result.
SWFMatrix&
SWFMatrix::invert()
{
    const boost::int64_t det = determinant();
    if (det == 0) {
        set_identity();
        return *this;
    }

    const double d = 65536.0 * 65536.0 / det;
    
    const boost::int32_t t0 = (boost::int32_t)(sy * d);
    sy  = (boost::int32_t)(sx * d);
    shy = (boost::int32_t)(-shy * d);
    shx = (boost::int32_t)(-shx * d);

    const boost::int32_t t4 = - (multiplyFixed16(tx, t0) + multiplyFixed16(ty, shy));
    ty = - (multiplyFixed16(tx, shx) + multiplyFixed16(ty, sy));

    sx = t0;
    tx = t4;

    return *this;
}

double
SWFMatrix::get_x_scale() const
{
    return std::sqrt((static_cast<double>(sx) * sx +
                static_cast<double>(shx) * shx)) / 65536.0;
}

double
SWFMatrix::get_y_scale() const
{
    return std::sqrt((static_cast<double>(sy) * sy +
                static_cast<double>(shy) * shy)) / 65536.0;
}

double
SWFMatrix::get_rotation() const
{
    // more successes in misc-ming.all/SWFMatrix_test.c
    return std::atan2(static_cast<double>(shx), sx); 
}

// private
boost::int64_t
SWFMatrix::determinant() const
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

std::ostream&
operator<<(std::ostream& o, const SWFMatrix& m)
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
