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
#include "config.h"
#endif

#include "check.h"
#include "Point2d.h"
#include <iostream>
#include <sstream>
#include <cassert>

using namespace std;
using namespace gnash;
using namespace gnash::geometry;

int
main(int /*argc*/, char** /*argv*/)
{
	Point2d<float> p(0, 0);
	Point2d<float> p1(10, 0);

	check_equals( p.distance(p1), 10 );

	Point2d<float> p2(p, p1, 0.5);
	check_equals(p2.x, 5);
	check_equals(p2.y, 0);

	p2.setTo(p, p1, 0.2);
	check_equals(p2.x, 2);
	check_equals(p2.y, 0);

	p2.setTo(p, p1, 0.7);
	check_equals(p2.x, 7);
	check_equals(p2.y, 0);

	p.setTo(0, 10);
	p2.setTo(p, p1, 0.5);
	check_equals(p2.x, 5);
	check_equals(p2.y, 5);

	p1.setTo(0, 20);
	p2.setTo(p, p1, 0.7);
	check_equals(p2.x, 0);
	check_equals(p2.y, 17); 

}

