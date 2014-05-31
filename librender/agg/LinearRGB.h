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

#ifndef GNASH_AGG_LINEAR_INTERPOLATOR_H
#define GNASH_AGG_LINEAR_INTERPOLATOR_H

#include <utility>

#include <cmath>

namespace gnash {

/// Convert linear RGB colorspace to sRGB
double
linearToSRGB(double s)
{
    const double a = 0.055;
    if (s <= 0.0031308) return 12.92 * s;
    return (1 + a) * std::pow(s, 1 / 2.4) - a;
}

template<typename T>
T
cdiff(T a, T b, double ratio)
{
    const int diff = b - a;
    const double d = linearToSRGB((diff < 0) ? 1 - ratio : ratio);
    if (diff < 0) {
        return b - d * diff;
    }
    return a + d * diff;
}

/// Interpolate in the linear RGB colorspace
//
/// This is an inefficient interpolation method because the
/// ratio is transformed on the fly to convert between linear RGB
/// and sRGB colorspaces.
template<class ColorT>
struct linear_rgb_interpolator
{
public:
    typedef ColorT color_type;

    linear_rgb_interpolator(color_type c1, color_type c2, 
        size_t len)
        :
        _c1(std::move(c1)),
        _c2(std::move(c2)),
        _len(len),
        _count(0)
    {}

    void operator++() {
        ++_count;
    }

    color_type color() const {
        const double ratio = static_cast<double>(_count) / _len;
        return color_type(
                cdiff(_c1.r, _c2.r, ratio),
                cdiff(_c1.g, _c2.g, ratio),
                cdiff(_c1.b, _c2.b, ratio),
                _c1.a + (_c2.a - _c1.a) * ratio);
    }

private:
    color_type _c1;
    color_type _c2;
    size_t _len;
    size_t _count;
};

}

#endif
