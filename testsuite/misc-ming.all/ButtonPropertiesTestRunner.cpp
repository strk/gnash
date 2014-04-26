/* 
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
 *   Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 */ 

#define INPUT_FILENAME "ButtonPropertiesTest.swf"

#include "MovieTester.h"
#include "MovieClip.h"
#include "DisplayObject.h"
#include "DisplayList.h"
#include "log.h"
#include "VM.h"

#include "check.h"
#include <string>
#include <cassert>

using namespace gnash;
using namespace std;

TRYMAIN(_runtest);
int
trymain(int /*argc*/, char** /*argv*/)
{
	string filename = string(TGTDIR) + string("/") + string(INPUT_FILENAME);
	MovieTester tester(filename);

	MovieClip* root = tester.getRootMovie();
	assert(root);

	tester.advance();
	check_equals(root->get_current_frame(), 1);
	
    tester.movePointerTo(45, 35);
    tester.click();

    tester.movePointerTo(100, 200);
    tester.movePointerTo(45, 35);
    tester.movePointerTo(100, 200);
    tester.movePointerTo(45, 35);
    tester.movePointerTo(100, 200);
    tester.movePointerTo(45, 35);
    tester.movePointerTo(100, 200);
    tester.movePointerTo(45, 35);
    tester.movePointerTo(100, 200);
    tester.movePointerTo(45, 35);
    tester.click();
    tester.click();
    tester.click();
    return 0;
}

