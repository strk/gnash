// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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
#include <iostream>

#include "log.h"
#include "GnashNumeric.h"
#include "SWFRect.h"
#include "Point2d.h"

// The Bionic libm doesn't build sincos for ARM, but older versions
// of boost want it. It also turns out AGG using this causes a
// segfault by passing in 0 as an argument for x, so for now we
// ignore that error so debugging can continue.
#ifdef __ANDROID__
void
sincos(double x, double *psin, double *pcos)
{
    if (x > 0) {
        *psin = sin(x);
        *pcos = cos(x);
    } else {
        std::cerr << "FIXME: sincos(0) will segfault!" << std::endl;
        // *psin = 0.244287;
        // *pcos = 0.969703;
    }
}
#endif

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

inline double
rotationX(const SWFMatrix& m)
{
    const double b = m.b();
    const double a = m.a();
    return std::atan2(b, a);
}

inline double
rotationY(const SWFMatrix& m)
{
    const double c = m.c();
    const double d = m.d();
    return std::atan2(-c, d);
}

inline std::int32_t
toFixed16(double a)
{
    return truncateWithFactor<65536>(a);
}

inline std::int32_t
multiplyFixed16(std::int32_t a, std::int32_t b)
{
    return (static_cast<std::int64_t>(a) *
            static_cast<std::int64_t>(b) + 0x8000) >> 16;

}

} // anonymous namepace

void
SWFMatrix::transform(geometry::Point2d& p) const
{
    std::int32_t t0 = multiplyFixed16(_a, p.x) + multiplyFixed16(_c, p.y) + _tx;
    std::int32_t t1 = multiplyFixed16(_b, p.x) + multiplyFixed16(_d, p.y) + _ty;
    p.x = t0;
    p.y = t1;
}

void
SWFMatrix::transform(std::int32_t& x, std::int32_t& y) const
{
    const std::int32_t t0 = multiplyFixed16(_a, x) + multiplyFixed16(_c, y) + _tx;
    const std::int32_t t1 = multiplyFixed16(_b,x) + multiplyFixed16(_d,  y) + _ty;
    x = t0;
    y = t1;
}

void
SWFMatrix::transform(geometry::Range2d<std::int32_t>& r) const
{
    const std::int32_t xmin = r.getMinX();
    const std::int32_t xmax = r.getMaxX();
    const std::int32_t ymin = r.getMinY();
    const std::int32_t ymax = r.getMaxY();

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
    _a = _d = 65536;
    _b = _c = _tx = _ty = 0;
}

void
SWFMatrix::concatenate(const SWFMatrix& m)
{
    SWFMatrix t;
    t._a = multiplyFixed16(_a, m._a) + multiplyFixed16(_c, m._b);
    t._b = multiplyFixed16(_b, m._a) + multiplyFixed16(_d, m._b);
    t._c = multiplyFixed16(_a, m._c) + multiplyFixed16(_c, m._d);
    t._d = multiplyFixed16(_b, m._c) + multiplyFixed16(_d, m._d);
    t._tx = multiplyFixed16(_a, m._tx) + multiplyFixed16(_c, m._ty) + _tx;
    t._ty = multiplyFixed16(_b, m._tx) + multiplyFixed16(_d, m._ty) + _ty;

    *this = t;
}

// Concatenate a translation onto the front of our
// SWFMatrix.  When transforming points, the translation
// happens first, then our original xform.
void
SWFMatrix::concatenate_translation(int xoffset, int yoffset)
{
    _tx += multiplyFixed16(_a, xoffset) + multiplyFixed16(_c, yoffset);
    _ty += multiplyFixed16(_b, xoffset) + multiplyFixed16(_d, yoffset);
}

// Concatenate scales to our SWFMatrix. When transforming points, these 
// scales happen first, then our matrix.
void
SWFMatrix::concatenate_scale(double xscale, double yscale)
{
    _a = multiplyFixed16(_a, toFixed16(xscale));
    _c = multiplyFixed16(_c, toFixed16(yscale));
    _b = multiplyFixed16(_b, toFixed16(xscale));
    _d = multiplyFixed16(_d, toFixed16(yscale)); 
}

// Set this SWFMatrix to a blend of m1 and m2, parameterized by t.
void
SWFMatrix::set_lerp(const SWFMatrix& m1, const SWFMatrix& m2, float t)
{
    _a = lerp<float>(m1._a, m2._a, t);
    _b = lerp<float>(m1._b, m2._b, t);
    _c = lerp<float>(m1._c, m2._c, t);
    _d = lerp<float>(m1._d, m2._d, t);
    _tx = lerp<float>(m1._tx, m2._tx, t);
    _ty = lerp<float>(m1._ty, m2._ty, t);
}

// Set the scale & rotation part of the SWFMatrix.
// angle in radians.
void
SWFMatrix::set_scale_rotation(double x_scale, double y_scale, double angle)
{
    const double cos_angle = std::cos(angle);
    const double sin_angle = std::sin(angle);
    _a = toFixed16(x_scale * cos_angle);
    _c = toFixed16(y_scale * -sin_angle);
    _b = toFixed16(x_scale * sin_angle);
    _d = toFixed16(y_scale * cos_angle); 
}

void
SWFMatrix::set_x_scale(double xscale)
{
    const double rot_x = rotationX(*this);
    _a = toFixed16(xscale * std::cos(rot_x));
    _b = toFixed16(xscale * std::sin(rot_x)); 
}

void
SWFMatrix::set_y_scale(double yscale)
{
    const double rot_y = rotationY(*this);

    _c = -toFixed16(yscale * std::sin(rot_y));
    _d = toFixed16(yscale * std::cos(rot_y));
}

void
SWFMatrix::set_scale(double xscale, double yscale)
{
    const double rotation = get_rotation();
    if ((xscale == 0.0) || (yscale == 0.0)) {
        std::cerr << "FIXME: sincos(0) will segfault!" << std::endl;
    } else {
        set_scale_rotation(xscale, yscale, rotation);
    }
}

void
SWFMatrix::set_rotation(double rotation)
{   
    const double rot_x = rotationX(*this);
    const double rot_y = rotationY(*this);

    const double scale_x = get_x_scale();
    const double scale_y = get_y_scale();
 
    _a = toFixed16(scale_x * std::cos(rotation));
    _b = toFixed16(scale_x * std::sin(rotation)); 
    _c = -toFixed16(scale_y * std::sin(rot_y - rot_x + rotation));
    _d = toFixed16(scale_y * std::cos(rot_y - rot_x + rotation));
}

// Transform point 'p' by our SWFMatrix.  Put the result in *result.
void
SWFMatrix::transform(point* result, const point& p) const
{
    assert(result);

    result->x = multiplyFixed16(_a,  p.x) + multiplyFixed16(_c, p.y) + _tx;
    result->y = multiplyFixed16(_b, p.x) + multiplyFixed16(_d,  p.y) + _ty;
}

void 
SWFMatrix::transform(SWFRect& r) const
{
    if (r.is_null()) return;

    const std::int32_t x1 = r.get_x_min();
    const std::int32_t y1 = r.get_y_min();
    const std::int32_t x2 = r.get_x_max();
    const std::int32_t y2 = r.get_y_max();

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
    const std::int64_t det = determinant();

    if (det == 0) {
        set_identity();
        return *this;
    }

    const double dn = 65536.0 * 65536.0 / det;
    
    const std::int32_t t0 = (std::int32_t)(d() * dn);
    _d = (std::int32_t)(a() * dn);
    _c = (std::int32_t)(-c() * dn);
    _b = (std::int32_t)(-b() * dn);

    const std::int32_t t4 = -(multiplyFixed16(_tx, t0) + multiplyFixed16(_ty, _c));
    _ty = -(multiplyFixed16(_tx, _b) + multiplyFixed16(_ty, _d));

    _a = t0;
    _tx = t4;

    return *this;
}

double
SWFMatrix::get_x_scale() const
{
    const double a2 = std::pow(static_cast<double>(a()), 2);
    const double b2 = std::pow(static_cast<double>(b()), 2);
    return std::sqrt(a2 + b2) / 65536.0;
}

double
SWFMatrix::get_y_scale() const
{
    const double d2 = std::pow(static_cast<double>(d()), 2);
    const double c2 = std::pow(static_cast<double>(c()), 2);
    return std::sqrt(d2 + c2) / 65536.0;
}

double
SWFMatrix::get_rotation() const
{
    return rotationX(*this);
}

// private
std::int64_t
SWFMatrix::determinant() const
{
    // | _a	_c	_tx |
    // | _b	_d	_ty |   = T. Using the Leibniz formula:
    // | 0	0	1  |
    //
    // Det(T) = ( (_a * _d * 1 ) + (_c * _ty * 0) + (_tx * _b *  0) ) -
    //          ( (0  * _d * _tx) + (0  * _ty * _a) + (1 * _c * _b) )
    //        = _a * _d - _b * _c
    return (std::int64_t)a() * d() - (std::int64_t)b() * c();
}

std::ostream&
operator<<(std::ostream& o, const SWFMatrix& m)
{
    // 8 digits and a decimal point.
    const short fieldWidth = 9;

    o << std::endl << "|"
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) 
      << m.a() / 65536.0 << " "
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) 
      << m.c()/ 65536.0 << " "
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) 
      << twipsToPixels(m.tx()) << " |" 
      << std::endl << "|"
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) 
      << m.b()/ 65536.0 << " "
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) 
      << m.d() / 65536.0 << " "
      << std::setw(fieldWidth) << std::fixed << std::setprecision(4) 
      << twipsToPixels(m.ty()) << " |";
      
      return o;
}

}   // end namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
