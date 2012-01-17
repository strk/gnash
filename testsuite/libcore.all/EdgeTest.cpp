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

#include <ostream>
#include <sstream>
#include <cassert>
#include <cmath>
#include <algorithm>

#include "Geometry.h" // for edge
#include "check.h"

using gnash::Edge;
using gnash::point;

// for double comparison
struct D {
	double _d;
	double _t; // tolerance

	D(double d) : _d(d), _t(1e-4) {}

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
	// Test distance
	//

	check_equals(Edge::distancePtSeg(point(0,0), point(9, 0), point(9, 0)), 9);
	check_equals(Edge::distancePtSeg(point(0,0), point(0, 0), point(3, 0)), 0);
	check_equals(Edge::distancePtSeg(point(-5,0), point(0, 0), point(3, 0)), 5);
	check_equals(Edge::distancePtSeg(point(5,0), point(0, 0), point(3, 0)), 2);
	check_equals(D(Edge::distancePtSeg(point(0,0), point(-10, 0), point(3, 0))), 0);
	check_equals(Edge::distancePtSeg(point(0,0), point(-10, 0), point(-10, 30)), 10);
	check_equals(Edge::distancePtSeg(point(5,5), point(-10, 0), point(10, 0)), 5);

	//
	// Test pointOnCurve
	//

	//
	// A-----C
	//       |
	//       B
	//
	point A(10, 10);
	point C(20, 10);
	point B(20, 20);
	check_equals(Edge::pointOnCurve(A, C, B, 0), A);
	check_equals(Edge::pointOnCurve(A, C, B, 1), B);
	check_equals(Edge::pointOnCurve(A, C, B, 0.5), point(17.5, 12.5));
	check_equals(std::sqrt((float)Edge::squareDistancePtCurve(A, C, B, B, 1)), 0);
	check_equals(std::sqrt((float)Edge::squareDistancePtCurve(A, C, B, A, 0)), 0);

	//
	// A----B---C
	//
	A.setTo(10, 10);
	C.setTo(40, 10);
	B.setTo(20, 10);
	check_equals(Edge::pointOnCurve(A, C, B, 0), A);
	check_equals(Edge::pointOnCurve(A, C, B, 1), B);
	check_equals(Edge::pointOnCurve(A, C, B, 0.5), point(27.5, 10));
	check_equals(std::sqrt((float)Edge::squareDistancePtCurve(A, C, B, B, 1)), 0);
	check_equals(std::sqrt((float)Edge::squareDistancePtCurve(A, C, B, A, 0)), 0);
}

