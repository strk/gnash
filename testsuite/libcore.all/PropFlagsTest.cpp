// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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
	check(!flags.test<PropFlags::readOnly>());
	check(!flags.test<PropFlags::dontEnum>());
	check(!flags.test<PropFlags::dontDelete>());
	check_equals(flags.get_flags(), 0);

    PropFlags flags2(PropFlags::dontDelete | PropFlags::dontEnum);
    check(flags2.test<PropFlags::dontEnum>());
    check(flags2.test<PropFlags::dontDelete>());
    check(!flags2.test<PropFlags::readOnly>());

    PropFlags i;
    check(i.get_visible(5));
    check(i.get_visible(6));
    check(i.get_visible(7));
    check(i.get_visible(8));
    check(i.get_visible(9));

    i.set_flags(PropFlags::onlySWF6Up);
    check(!i.get_visible(5));
    check(i.get_visible(6));
    check(i.get_visible(7));
    check(i.get_visible(8));
    check(i.get_visible(9));

    i.set_flags(PropFlags::onlySWF7Up);
    check(!i.get_visible(5));
    check(!i.get_visible(6));
    check(i.get_visible(7));
    check(i.get_visible(8));
    check(i.get_visible(9));

    i.set_flags(PropFlags::onlySWF8Up);
    check(!i.get_visible(5));
    check(!i.get_visible(6));
    check(!i.get_visible(7));
    check(i.get_visible(8));
    check(i.get_visible(9));

    i.set_flags(PropFlags::onlySWF9Up);
    check(!i.get_visible(5));
    check(!i.get_visible(6));
    check(!i.get_visible(7));
    check(!i.get_visible(8));
    check(i.get_visible(9));

    PropFlags i2;
    i2.set_flags(PropFlags::onlySWF8Up);
    check(!i2.get_visible(5));
    check(!i2.get_visible(6));
    check(!i2.get_visible(7));
    check(i2.get_visible(8));
    check(i2.get_visible(9));

    PropFlags i3;
    i3.set_flags(PropFlags::ignoreSWF6);
    check(i3.get_visible(5));
    check(!i3.get_visible(6));
    check(i3.get_visible(7));
    check(i3.get_visible(8));
    check(i3.get_visible(9));
}

