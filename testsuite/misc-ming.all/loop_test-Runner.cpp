/* 
 *   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 */ 

#define INPUT_FILENAME "loop_test.swf"

#include "MovieTester.h"
#include "sprite_instance.h"
#include "character.h"
#include "dlist.h"
#include "container.h"
#include "log.h"
#include "GnashException.h"

#include "check.h"

#include <string>
#include <iostream>
#include <cassert>
#include <memory>

using namespace gnash;
using namespace std;

int
main(int /*argc*/, char** /*argv*/)
{
	string filename = INPUT_FILENAME;
	auto_ptr<MovieTester> t;

	try
	{
		t.reset(new MovieTester(filename));
	}
	catch (const GnashException& e)
	{
		std::cerr << "Error initializing MovieTester: " << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}

	MovieTester& tester = *t;

	// TODO: check why we need this !!
	//       I wouldn't want the first advance to be needed
	tester.advance();

	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity(1);
	dbglogfile.setActionDump(1);

	sprite_instance* root = tester.getRootMovie();
	assert(root);

	check_equals(root->get_frame_count(), 30);

	const character* movieClip1 = tester.findDisplayItemByName(*root, "movieClip1");
	check(movieClip1);
	check_equals(movieClip1->get_depth(), 1);

	const character* movieClip2 = tester.findDisplayItemByName(*root, "movieClip2");
	check(movieClip2);
	check_equals(movieClip2->get_depth(), 2);

	// Advance to frame 27
	for (int i=root->get_current_frame(); i<28; ++i) {
		tester.advance();
	}

	// In frame 29, a swapDepth() action will
	// change the characters depth
	tester.advance();
	check_equals(root->get_current_frame(), 29);
	check_equals(movieClip1->get_depth(), 2);
	check_equals(movieClip2->get_depth(), 1);

	// Now keep advancing until last frame is reached
	// (29, as framecount is 0-based)
	for (int i=root->get_current_frame(); i<29; ++i) {
		tester.advance();
	}

	check_equals(root->get_current_frame(), 29);
	check_equals(movieClip1->get_depth(), 2);
	check_equals(movieClip2->get_depth(), 1);


	// Next advance will make the movie restart
	tester.advance();
	check_equals(root->get_current_frame(), 0);

	// We expect the depth to be kept on restart
	check_equals(movieClip1->get_depth(), 2);
	check_equals(movieClip2->get_depth(), 1);

	// ... until next SwapDepth ...

	for (int i=root->get_current_frame(); i<28; ++i) {
		tester.advance();
	}

	check_equals(movieClip1->get_depth(), 2);
	check_equals(movieClip2->get_depth(), 1);

	// we expect depths to be swapped again 
	tester.advance();
	check_equals(root->get_current_frame(), 29);
	xcheck_equals(movieClip1->get_depth(), 1);
	xcheck_equals(movieClip2->get_depth(), 2);

	// .. and the new deptsh to be kept at restart
	tester.advance();
	check_equals(root->get_current_frame(), 0);
	xcheck_equals(movieClip1->get_depth(), 1);
	xcheck_equals(movieClip2->get_depth(), 2);
}

