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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "SWFMatrix.h"
#include "Point2d.h"
#include <ostream>
#include <cmath>

#include "check.h"

using namespace gnash;

// for double comparison
struct D {
    double _d;
    double _t; // tolerance

    D(double d) : _d(d), _t(1e-3) {}

    // Set tolerance
    D(double d, double t) : _d(d), _t(t) {}

    // Return true if the difference between the two
    // doubles is below the minimum tolerance defined for the two
    bool operator==(const D& d)
    {
        double tol = std::min(_t, d._t);
        double delta = std::abs(_d - d._d);
        bool ret = delta < tol;
        //cout << "D " << _d << "operator==(const D " << d._d <<") returning " << ret << " (delta is " << delta << ") " << endl;
        return ret;
    }
};
std::ostream& operator<<(std::ostream& os, const D& d)
{
    return os << d._d << " [tol: " << d._t << "]";
}

int
main(int /*argc*/, char** /*argv*/)
{
    // 
    // Test boost types, SWFMatrix design is rely on this.
    // 
    // Note: If any of the following tests fails, your boost library
    // is bogus or hasn't been installed properly.
    // 
    check_equals(sizeof(boost::int8_t),   1);
    check_equals(sizeof(boost::uint8_t),  1);
    check_equals(sizeof(boost::int16_t),  2);
    check_equals(sizeof(boost::uint16_t), 2);
    check_equals(sizeof(boost::int32_t),  4);
    check_equals(sizeof(boost::uint32_t), 4);
    check_equals(sizeof(boost::int64_t),  8);
    check_equals(sizeof(boost::uint64_t), 8);

    // 
    //  Test identity SWFMatrix.
    // 
    SWFMatrix identity; 
    check_equals(identity.get_x_scale(), 1);
    check_equals(identity.get_y_scale(), 1);
    check_equals(identity.get_rotation(), 0);
    check_equals(identity.get_x_translation(), 0);
    check_equals(identity.get_y_translation(), 0);

    check_equals(identity.invert(), identity);

    //
    // Test parameter setting and getting, interfaces for AS.
    //
    SWFMatrix m1;
    m1.set_scale_rotation(1, 3, 0);
    check_equals(m1.get_x_scale(), 1);
    check_equals(m1.get_y_scale(), 3);
    check_equals(m1.get_rotation(), 0);
    check_equals(m1.get_x_translation(), 0);
    check_equals(m1.get_y_translation(), 0);

    m1.set_scale(1.5, 2.5);
    check_equals(D(m1.get_x_scale()), 1.5);
    check_equals(D(m1.get_y_scale()), 2.5);
    check_equals(D(m1.get_rotation()), 0);
    check_equals(m1.get_x_translation(), 0);
    check_equals(m1.get_y_translation(), 0);

    m1.set_scale(34, 4);
    check_equals(D(m1.get_x_scale()), 34);
    check_equals(D(m1.get_y_scale()), 4);
    check_equals(D(m1.get_rotation()), 0);
    check_equals(m1.get_x_translation(), 0);
    check_equals(m1.get_y_translation(), 0);

    m1.set_scale_rotation(1, 1, 2);
    check_equals(D(m1.get_x_scale()), 1);
    check_equals(D(m1.get_y_scale()), 1);
    check_equals(D(m1.get_rotation()), 2);
    check_equals(m1.get_x_translation(), 0);
    check_equals(m1.get_y_translation(), 0);

    m1.set_x_scale(2);
    check_equals(D(m1.get_x_scale()), 2);
    check_equals(D(m1.get_y_scale()), 1);
    check_equals(D(m1.get_rotation()), 2);
    check_equals(m1.get_x_translation(), 0);
    check_equals(m1.get_y_translation(), 0);

    m1.set_scale(1, 2);
    check_equals(D(m1.get_x_scale()), 1);
    check_equals(D(m1.get_y_scale()), 2);
    check_equals(D(m1.get_rotation()), 2);
    check_equals(m1.get_x_translation(), 0);
    check_equals(m1.get_y_translation(), 0);

    m1.set_rotation(0);
    check_equals(D(m1.get_x_scale()), 1);
    check_equals(D(m1.get_y_scale()), 2);
    check_equals(D(m1.get_rotation()), 0);
    check_equals(m1.get_x_translation(), 0);
    check_equals(m1.get_y_translation(), 0);

    m1.set_translation(5, 6);
    check_equals(D(m1.get_x_scale()), 1);
    check_equals(D(m1.get_y_scale()), 2);
    check_equals(D(m1.get_rotation()), 0);
    check_equals(m1.get_x_translation(), 5);
    check_equals(m1.get_y_translation(), 6);

    m1.set_rotation(2);
    check_equals(D(m1.get_x_scale()), 1);
    check_equals(D(m1.get_y_scale()), 2);
    check_equals(D(m1.get_rotation()), 2);
    check_equals(m1.get_x_translation(), 5);
    check_equals(m1.get_y_translation(), 6);

    SWFMatrix m2;
    check_equals(D(m2.get_rotation()), 0);
    m2.set_x_scale(16);
    check_equals(D(m2.get_x_scale()), 16);
    check_equals(D(m2.get_y_scale()), 1);
    check_equals(D(m2.get_rotation()), 0);
    m2.set_x_scale(-16);
    check_equals(D(m2.get_x_scale()), 16);
    check_equals(D(m2.get_y_scale()), 1);
    check_equals(D(m2.get_rotation()), 3.14159);
    m2.set_x_scale(16);
    check_equals(D(m2.get_x_scale()), 16);
    check_equals(D(m2.get_y_scale()), 1);
    check_equals(D(m2.get_rotation()), 3.14159);
    m2.set_x_scale(16);
    m2.set_y_scale(-64);
    check_equals(D(m2.get_x_scale()), 16);
    check_equals(D(m2.get_y_scale()), 64);
    check_equals(D(m2.get_rotation()), 3.14159);
    m2.set_x_scale(16);
    m2.set_x_scale(-128);
    check_equals(D(m2.get_x_scale()), 128);
    check_equals(D(m2.get_y_scale()), 64);
    check_equals(D(m2.get_rotation()), 0);

    //
    // Test SWFMatrix concatenation
    //
    m1.concatenate_scale(2, 2);
    check_equals(D(m1.get_x_scale()), 2);
    check_equals(D(m1.get_y_scale()), 4);
    check_equals(D(m1.get_rotation()), 2);
    check_equals(m1.get_x_translation(), 5);
    check_equals(m1.get_y_translation(), 6);

    m1.concatenate_scale(3, 3);
    check_equals(D(m1.get_x_scale()), 6);
    check_equals(D(m1.get_y_scale()), 12);
    check_equals(D(m1.get_rotation()), 2);
    check_equals(m1.get_x_translation(), 5);
    check_equals(m1.get_y_translation(), 6);

    m1.concatenate_scale(2, 1);
    check_equals(D(m1.get_x_scale()), 12);
    check_equals(D(m1.get_y_scale()), 12);
    check_equals(D(m1.get_rotation()), 2);
    check_equals(m1.get_x_translation(), 5);
    check_equals(m1.get_y_translation(), 6);

    //
    // Test SWFMatrix transformations
    //
    point p1(0, 0);
    point p2(64, 64);
    point r;

    m1.set_identity();
    // Make a distance of 64 become a distance of 20 .. 
    m1.set_scale(20.0/64, 20.0/64);

    m1.transform(&r, p1);
    check_equals(r.x, 0);
    check_equals(r.y, 0);
   
    m1.transform(&r, p2);
    check_equals(r.x, 20);
    check_equals(r.y, 20);

    // Translate points to have the origin at 32,32
    // (coordinates expressed in prior-to-scaling SWFMatrix)
    m1.concatenate_translation(-32, -32);

    m1.transform(&r, p1);
    check_equals(r.x, -10);
    check_equals(r.y, -10);

    m1.transform(&r, p2);
    check_equals(r.x, 10);
    check_equals(r.y, 10);

    //  Apply a final scaling by 10 keeping the current origin 
    // (reached after translation)
    SWFMatrix final;
    final.set_scale(10, 10);
    final.concatenate(m1);
    m1 = final;

    m1.transform(&r, p1);
    check_equals(r.x, -100);
    check_equals(r.y, -100);

    m1.transform(&r, p2);
    check_equals(r.x, 100);
    check_equals(r.y, 100);

    //
    // Test SWFMatrix invertion
    // 
    m1.set_identity();
    m1.set_translation(50*20, -30*20);
    m1.set_scale(0.5, 2);
    m1.set_rotation(90*3.141593/180);
    
    SWFMatrix m1_inverse = m1;
    m1_inverse.invert();
    // concatenate the inverse SWFMatrix and original SWFMatrix.
    m1_inverse.concatenate(m1); 
    // the result is expected to be an identity SWFMatrix. 
    check_equals(m1_inverse, identity);

    m1 = SWFMatrix(4, 0, 0, 4, 20, 20);
    
    m1_inverse = m1;
    m1_inverse.invert();
    
    check_equals(m1_inverse.a(), 16384 * 65536);
    check_equals(m1_inverse.b(), 0);
    check_equals(m1_inverse.tx(), -16384 * 20);
    check_equals(m1_inverse.c(), 0);
    check_equals(m1_inverse.d(), 16384 * 65536);
    check_equals(m1_inverse.ty(), -16384 * 20);
    
    // concatenate the inverse SWFMatrix and original SWFMatrix.
    m1_inverse.concatenate(m1);
    // the result is expected to be an identity SWFMatrix. 
    check_equals(m1_inverse, identity);
}

