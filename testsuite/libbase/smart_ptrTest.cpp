// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "check.h"
#include "smart_ptr.h"
#include "ref_counted.h"

#include <iostream>
#include <sstream>
#include <cassert>

using namespace std;
using namespace gnash;

int
main(int /*argc*/, char** /*argv*/)
{
	ref_counted* obj = new ref_counted();
	check_equals(obj->get_ref_count(), 0);

	smart_ptr<ref_counted> ptr1(obj);
	check_equals(obj->get_ref_count(), 1);

	smart_ptr<ref_counted> ptr2(obj);
	check_equals(obj->get_ref_count(), 2);

	smart_ptr<ref_counted> ptr3(ptr1);
	check_equals(obj->get_ref_count(), 3);

	smart_ptr<ref_counted> ptr4(ptr2.get_ptr());
	check_equals(obj->get_ref_count(), 4);

	ptr1 = ptr3;
	check_equals(obj->get_ref_count(), 4);

	ptr1 = ptr3.get_ptr();
	check_equals(obj->get_ref_count(), 4);

	smart_ptr<ref_counted> ptr5;
	ptr5 = ptr3;
	check_equals(obj->get_ref_count(), 5);

	{
		smart_ptr<ref_counted> ptr6(obj);
		check_equals(obj->get_ref_count(), 6);
		smart_ptr<ref_counted> ptr7(ptr6);
		check_equals(obj->get_ref_count(), 7);
		smart_ptr<ref_counted> ptr8(ptr7);
		check_equals(obj->get_ref_count(), 8);
	}

	check_equals(obj->get_ref_count(), 5);
}

