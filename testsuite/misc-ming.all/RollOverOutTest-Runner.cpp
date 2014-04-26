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

#define INPUT_FILENAME "RollOverOutTest.swf"

#include "MovieTester.h"
#include "MovieClip.h"
#include "DisplayObject.h"
#include "DisplayList.h"
#include "log.h"

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

	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity(1);

	MovieClip* root = tester.getRootMovie();
	assert(root);

	check_equals(root->get_frame_count(), 4);
	check_equals(root->getPlayState(), MovieClip::PLAYSTATE_PLAY);
	check_equals(root->get_current_frame(), 0);

	tester.advance();  // advance to the second frame.

	const DisplayObject* mc1 = tester.findDisplayItemByName(*root, "square1");
	check(mc1);
	const DisplayObject* mc2 = tester.findDisplayItemByName(*root, "square2");
	check(mc2);

	check_equals(mc1->visible(), true);
	check_equals(mc2->visible(), false);
	check_equals(root->getPlayState(), MovieClip::PLAYSTATE_STOP);
	check_equals(root->get_current_frame(), 1);

	// we're in stop mode, so advance should not advance anything
	tester.advance();
	check_equals(root->get_current_frame(), 1);
	tester.advance();
	check_equals(root->get_current_frame(), 1);

	// roll over the middle of the square, this should trigger
	// the addition of a goto_frame(3) action, which is executed
	// at advance() time.
	tester.movePointerTo(60, 60);
	tester.advance();
	check_equals(root->get_current_frame(), 2);
	check_equals(root->getPlayState(), MovieClip::PLAYSTATE_STOP);
	tester.advance();
	check_equals(root->get_current_frame(), 2);

	// keep the pointer inside
	tester.movePointerTo(41, 60);
	tester.advance();
	check_equals(root->get_current_frame(), 2);

	// pointer on the border (corner case)
	tester.movePointerTo(40, 60);
	tester.advance();
	check_equals(root->get_current_frame(), 2);
	
	// pointer out of the square.
	tester.movePointerTo(300, 60);
	//tester.advance();  // mouse event handler should drive the movie
	check_equals(root->get_current_frame(), 1);
	
	// pointer back to the square again.
	tester.movePointerTo(60, 60);
	tester.click();
	//tester.advance(); // mouse event handler should drive the movie
	check_equals(root->get_current_frame(), 3);
	return 0;
}



