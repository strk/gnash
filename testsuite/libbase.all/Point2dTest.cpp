// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010. 2011, 2012
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

#include "check.h"
#include "Point2d.h"
#include <iostream>
#include <sstream>
#include <cassert>

using namespace gnash;
using namespace gnash::geometry;

int
main(int /*argc*/, char** /*argv*/)
{
    point p1(0, 0);
    point p2(10, 0);

    check_equals( p1.distance(p2), 10 );

    point p(p1, p2, 0.5);
    check_equals(p.x, 5);
    check_equals(p.y, 0);

    p.setTo(p1, p2, 0.2);
    check_equals(p.x, 2);
    check_equals(p.y, 0);

    p.setTo(999999, 1000000);
    check_equals(p.x, 999999);
    check_equals(p.y, 1000000);

    std::int64_t square_dist = 0;
    
    square_dist = Point2d::squareDistance(p1, p2);
    check_equals(square_dist, 10 * 10);

    p1.setTo(65537, 0);
    p2.setTo(0, 65536);
    square_dist = Point2d::squareDistance(p1, p2);
    check_equals(square_dist, 65537.0 * 65537.0 + 65536.0 * 65536.0);

    p1.setTo(0x8000000, 0);
    p2.setTo(0, 0);
    square_dist = p1.squareDistance(p2);
    check_equals(square_dist, 1.0 * 0x8000000 * 0x8000000);

    std::int32_t  dist = 0;
    dist = p1.distance(p2);
    check_equals(dist, 0x8000000);
    return 0;
}


