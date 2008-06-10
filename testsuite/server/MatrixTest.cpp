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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "matrix.h"
#include <iostream>
#include <sstream>
#include <cassert>

#include "check.h"

using namespace std;
using namespace gnash;

// for double comparison
struct D {
	double _d;
	double _t; // tolerance

	D(double d) : _d(d), _t(1e-6) {}

	// Set tolerance
	D(double d, double t) : _d(d), _t(t) {}

	// Return true if the difference between the two
	// doubles is below the minimum tolerance defined for the two
	bool operator==(const D& d)
	{
		double tol = std::min(_t, d._t);
		double delta = fabs(_d - d._d);
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
#if 0
    std::string label;

	// Check attributes of the identity
	matrix identity; 
	check(identity.is_valid());
	identity.set_identity();
	check_equals(identity.get_x_scale(), 1);
	check_equals(identity.get_y_scale(), 1);
	check_equals(identity.get_rotation(), 0);
	check_equals(identity.get_x_translation(), 0);
	check_equals(identity.get_y_translation(), 0);

	// The inverse of identity is still the identity
	matrix invert;
	invert.set_inverse(identity);
	check_equals(invert, identity);

	//---------------------------------------------
	// Test canonic parameter setting and getting
	//---------------------------------------------

	matrix m1;
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

	//---------------------------------------------
	// Test concatenation
	//---------------------------------------------

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

	//---------------------------------------------
	// Test transformations
	//---------------------------------------------

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
	// (coordinates expressed in prior-to-scaling matrix)
	m1.concatenate_translation(-32, -32);

	m1.transform(&r, p1);
	check_equals(r.x, -10);
	check_equals(r.y, -10);

	m1.transform(&r, p2);
	check_equals(r.x, 10);
	check_equals(r.y, 10);

	// Apply a final scaling by 10 keeping
	// the current origin (reached after
	// translation)

	matrix final;
	final.set_scale(10, 10);
	final.concatenate(m1);
	m1 = final;

	m1.transform(&r, p1);
	check_equals(r.x, -100);
	check_equals(r.y, -100);

	m1.transform(&r, p2);
	check_equals(r.x, 100);
	check_equals(r.y, 100);
#endif
}

