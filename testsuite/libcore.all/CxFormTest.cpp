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

#include "log.h"

#include <iostream>
#include <sstream>
#include <cassert>
#include <cmath>
#include <string>

#include "check.h"
#include "SWFCxForm.h"

int main()
{
    gnash::SWFCxForm c;

    // A default constructed CxForm must be:
    // 256, 256, 256, 256, 0, 0, 0, 0.
    check_equals(c.ra, 256);
    check_equals(c.ba, 256);
    check_equals(c.ga, 256);
    check_equals(c.aa, 256);
    check_equals(c.rb, 0);
    check_equals(c.bb, 0);
    check_equals(c.gb, 0);
    check_equals(c.ab, 0);
    
    boost::uint8_t r = 0;
    boost::uint8_t b = 0;
    boost::uint8_t g = 0;
    boost::uint8_t a = 0;

    c.transform(r, b, g, a);
    check_equals(r, 0);
    check_equals(b, 0);
    check_equals(g, 0);
    check_equals(a, 0);

    r = 255;
    b = 255;
    g = 0;
    a = 0;

    c.transform(r, b, g, a);
    check_equals(+r, 255);
    check_equals(+b, 255);
    check_equals(+g, 0);
    check_equals(+a, 0);

    c.rb = 30000;
    c.gb = -30000;

    r = 255;
    b = 255;
    g = 0;
    a = 0;

    c.transform(r, b, g, a);
    check_equals(+r, 255);
    check_equals(+b, 0);
    check_equals(+g, 0);
    check_equals(+a, 0);

    c.ba = 30000;
    c.aa = -30000;

    r = 255;
    b = 100;
    g = 1;
    a = 60;

    c.transform(r, b, g, a);
    check_equals(+r, 255);
    check_equals(+b, 0);
    check_equals(+g, 117);
    check_equals(+a, 0);
    return 0;
}
