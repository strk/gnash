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

#define INPUT_FILENAME "ResolveEventsTest.swf"

#include "MovieTester.h"
#include "MovieClip.h"
#include "DisplayObject.h"
#include "DisplayList.h"
#include "TextField.h"
#include "log.h"

#include "check.h"
#include <string>
#include <cassert>

using namespace gnash;
using namespace std;

int
main(int /*argc*/, char** /*argv*/)
{
	//string filename = INPUT_FILENAME;
	string filename = string(TGTDIR) + string("/") + string(INPUT_FILENAME);
	MovieTester tester(filename);

	MovieClip* root = tester.getRootMovie();
	assert(root);

	check_equals(root->get_frame_count(), 7);
	check_equals(root->get_current_frame(), 0);

    tester.advance();
	
    check_equals(root->get_current_frame(), 1);
    
    tester.advance();
    check_equals(root->get_current_frame(), 2);

    tester.movePointerTo(150, 150);
    tester.click();

    tester.advance();
    check_equals(root->get_current_frame(), 3);
    
    tester.movePointerTo(250, 250);
    tester.click();
    tester.advance();
    check_equals(root->get_current_frame(), 4);
    
    tester.movePointerTo(251, 251);
    tester.movePointerTo(252, 251);
    tester.movePointerTo(251, 252);
    tester.click();
    tester.advance();
    check_equals(root->get_current_frame(), 5);
    
    tester.advance();

	// last advance should not restart the loop (it's in STOP mode)
    check_equals(root->getPlayState(), MovieClip::PLAYSTATE_STOP);
	check_equals(root->get_current_frame(), 6);

}

