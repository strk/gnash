// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#include "PropFlags.h"

#include <iostream>
#include <sstream>
#include <cassert>
#include <string>

#include "check.h"

using namespace std;
using namespace gnash;

int
main(int /*argc*/, char** /*argv*/)
{

	PropFlags flags;

	// Check initial state
	check(!flags.get_read_only());
	check(!flags.get_dont_enum());
	check(!flags.get_dont_delete());
	check_equals(flags.get_flags(), 0);

	// Now set some flags and check result

	flags.set_read_only();
	check(flags.get_read_only());

	flags.set_dont_enum();
	check(flags.get_dont_enum());

	flags.set_dont_delete();
	check(flags.get_dont_delete());

	// Now clear the flags and check result

	flags.clear_read_only();
	check(!flags.get_read_only());

	flags.clear_dont_enum();
	check(!flags.get_dont_enum());

	flags.clear_dont_delete();
	check(!flags.get_dont_delete());
}

