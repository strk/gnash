// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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
#include "as_object.h"
#include "Property.h"
#include "PropertyList.h"
#include "MovieClip.h"
#include "character.h"
#include "RGBA.h"
#include "movie_root.h"

#include <iostream>
#include <sstream>
#include <cassert>
#include <cmath>
#include <string>
#include <memory>
#include <boost/scoped_ptr.hpp>

#include "check.h"

using namespace gnash;
using namespace std;
using namespace boost;

int
main(int /*argc*/, char** /*argv*/)
{
	std::cout << "sizeof(int): " << (sizeof(int)) << std::endl;
	std::cout << "sizeof(float): " << (sizeof(float)) << std::endl;

	std::cout << "sizeof(long): " << (sizeof(long)) << std::endl;
	std::cout << "sizeof(double): " << (sizeof(double)) << std::endl;

	std::cout << "sizeof(as_value): " << (sizeof(as_value)) << std::endl;

	std::cout << "sizeof(Property): " << (sizeof(Property)) << std::endl;
	std::cout << "sizeof(Property*): " << (sizeof(Property*)) << std::endl;
	std::cout << "sizeof(PropertyList): " << (sizeof(PropertyList)) << std::endl;
	std::cout << "sizeof(auto_ptr<PropertyList>): " << (sizeof(auto_ptr<PropertyList>)) << std::endl;
	std::cout << "sizeof(scoped_ptr<PropertyList>): " << (sizeof(scoped_ptr<PropertyList>)) << std::endl;

	std::cout << "sizeof(GcResource): " << (sizeof(GcResource)) << std::endl;
	std::cout << "sizeof(as_object): " << (sizeof(as_object)) << std::endl;
	std::cout << "sizeof(character): " << (sizeof(character)) << std::endl;
	std::cout << "sizeof(MovieClip): " << (sizeof(MovieClip)) << std::endl;

	std::cout << "sizeof(rgba): " << (sizeof(rgba)) << std::endl;
	std::cout << "sizeof(line_style): " << (sizeof(line_style)) << std::endl;
	std::cout << "sizeof(fill_style): " << (sizeof(fill_style)) << std::endl;
	std::cout << "sizeof(SWFMatrix): " << (sizeof(SWFMatrix)) << std::endl;
	std::cout << "sizeof(movie_root): " << (sizeof(movie_root)) << std::endl;
}

