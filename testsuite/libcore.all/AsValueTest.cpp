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

#include "as_value.h"

#include <iostream>
#include <sstream>
#include <cassert>
#include <cmath>
#include <string>

#include "check.h"

#include "utility.h"

using namespace std;
using namespace gnash;

int
main(int /*argc*/, char** /*argv*/)
{
	float num = 0;

	std::cout << "sizeof(as_value): " << (sizeof(as_value)) << std::endl;

	check(!isnan(num));

	num /= 9999999;

	check(!isnan(num));
        check(utility::isFinite(num));

	num = std::numeric_limits<float>::quiet_NaN();

	check(isnan(num));
	check(!utility::isFinite(num));

	num = std::numeric_limits<float>::infinity();
	
	check(!isnan(num));
	check(!utility::isFinite(num));

	num = 1.0 / 0.0;

	check(!utility::isFinite(num));
	check(!isnan(num));

	int intgr = num;

	num = intgr;

	check(!isnan(num));
	check(utility::isFinite(num));

}

