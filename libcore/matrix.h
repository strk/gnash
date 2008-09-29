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
#include "rect.h"    // for rect 
#include "Point2d.h" // for Point2d
#include "utility.h" // for TRUST_FLOAT_TO_UINT32_CONVERSION

#include <iostream> 
#include <boost/cstdint.hpp>

// Forward declarations
namespace gnash {
    class SWFStream;
}


namespace gnash {

/// The SWF matrix record.
/// 
/// Conceptually, it represents a 3*3 linear transformation matrix like this:
/// 
///   | scale_x       rotateSkew_y  translate_x |
///   | rotateSkey_x  scale_y       traslate_y  |
///   | 0             0             1           |
/// 
class DSOEXPORT matrix
{
public:

    /// Xscale, 16.16 fixed point. xx in swfdec. 'a' in AS Matrix.
    int sx; 

    /// Xshear, 16.16 fixed point. yx in swfdec. 'b' in AS Matrix.
    int shx;

    /// Xtranslation, TWIPS. x0 in swfdec. 'tx' in AS Matrix.
    int tx; 

    /// Yscale, 16.16 fixed point. yy in swfdec. 'd' in AS Matrix.
    int sy; 

    /// Yshear, 16.16 fixed point. xy in swfdec. 'c' in AS Matrix.
    int shy;

    /// Ytranslation, TWIPS. y0 in swfdec. 'ty' in AS Matrix.
    int ty; 
             
    friend bool operator== (const matrix&, const matrix&);
    friend std::ostream& operator<< (std::ostream&, const matrix&);
    
    /// Defaults to identity
    matrix();

    /// Check validity of the matrix values
    bool    is_valid() const;

    /// Set the matrix to identity.
    void    set_identity();

    /// Concatenate m's transform onto ours. 
    //
    /// When transforming points, m happens first,
    /// then our original xform.
    ///
    void    concatenate(const matrix& m);

    /// Concatenate a translation onto the front of our matrix.
    //
    /// When transforming points, the translation
    /// happens first, then our original xform.
    ///
    void    concatenate_translation(int tx, int ty);

    /// Concatenate scale x and y to the front of our matrix 
    //
    /// When transforming points, these scales happen first, then
    /// our original matrix. 
    /// 
    void    concatenate_scale(double x, double y);

    /// Set this matrix to a blend of m1 and m2, parameterized by t.
    void    set_lerp(const matrix& m1, const matrix& m2, float t);

    /// Set the scale & rotation part of the matrix. angle in radians.
    void    set_scale_rotation(double x_scale, double y_scale, double rotation);

    /// Set x and y scales, rotation is unchanged.
    void    set_scale(double x_scale, double y_scale);

    /// Set x scale, rotation any y scale are unchanged.
    void    set_x_scale(double scale);

    /// Set y scale, rotation and x scale are unchanged.
    void    set_y_scale(double scale);

    /// Set rotation in radians, scales component are unchanged.
    void    set_rotation(double rotation);

    /// Set x translation in TWIPS
    void set_x_translation(int x)
    {
        tx = x;
    }

    /// Set y translation in TWIPS.
    void set_y_translation(int y)
    {
        ty = y;
    }

    /// Set x and y translation in TWIPS.
    void set_translation(int x, int y)
    {
        tx = x;
        ty = y;
    }

    /// Initialize from the SWF input stream.
    void    read(SWFStream& in);

    /// Transform a given point by our matrix
    void    transform(geometry::Point2d& p) const
    {
        boost::int32_t t0 = Fixed16Mul(sx, p.x) + Fixed16Mul(shy, p.y) + tx;
        boost::int32_t t1 = Fixed16Mul(shx,p.x) + Fixed16Mul(sy,  p.y) + ty;
        p.x = t0;
        p.y = t1;
    }

    /// Transform the given point by our matrix.
    void    transform(boost::int32_t& x, boost::int32_t& y) const
    {
        boost::int32_t  t0 = Fixed16Mul(sx, x) + Fixed16Mul(shy, y) + tx;
        boost::int32_t  t1 = Fixed16Mul(shx,x) + Fixed16Mul(sy,  y) + ty;
        x = t0;
        y = t1;
    }
    
    /// Transform point 'p' by our matrix. 
    //
    /// Put the result in *result.
    ///
    void    transform(point* result, const point& p) const;

    /// Transform Range2d<float> 'r' by our matrix. 
    //
    /// NULL and WORLD ranges are untouched.
    ///
    void    transform(geometry::Range2d<float>& r) const;

    void    transform(rect& r) const;
    
    /// Invert this matrix and return the result.
    const matrix& invert();
    
    /// return the magnitude scale of our x coord output
    double   get_x_scale() const;

    /// return the magnitude scale of our y coord output
    double   get_y_scale() const;

    /// return rotation component in radians.
    double   get_rotation() const;

    /// return x translation n TWIPS unit.
    int   get_x_translation() const
    {
        return tx;
    }

    /// return y translation in TWIPS unit.
    int   get_y_translation() const
    {
        return ty;
    }

private: 
    /// Return the determinant of this matrix in 32.32 fixed point format.
    boost::int64_t  determinant() const;

    inline boost::int32_t FloatToFixed16(float a)
    {
#ifdef TRUST_FLOAT_TO_UINT32_CONVERSION
        // truncate when overflow occurs.
        return static_cast<boost::int32_t>(static_cast<boost::uint32_t>(a * 65536.0f));
#else
        boost::int32_t  b;
        if(a >= 0)
        {
            b = static_cast<boost::uint32_t>(std::fmod(a * 65536.0, 4294967296.0));
        }
        else
        {
            b = -static_cast<boost::uint32_t>(std::fmod(-a * 65536.0, 4294967296.0));
        }
        return b;
#endif
    }

    inline boost::int32_t DoubleToFixed16(double a)
    {
#ifdef TRUST_FLOAT_TO_UINT32_CONVERSION
        // truncate when overflow occurs.
        return static_cast<boost::int32_t>(static_cast<boost::uint32_t>(a * 65536.0));
#else
        boost::int32_t  b;
        if(a >= 0)
        {
            b = static_cast<boost::uint32_t>(std::fmod(a * 65536.0, 4294967296.0));
        }
        else
        {
            b = -static_cast<boost::uint32_t>(std::fmod(-a * 65536.0, 4294967296.0));
        }
        return b;
#endif
    }

}; //end of matrix

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

}   // namespace gnash

#endif // GNASH_MATRIX_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
// 
