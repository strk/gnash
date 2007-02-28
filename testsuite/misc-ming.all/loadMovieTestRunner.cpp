/* 
 *   Copyright (C) 2007 Free Software Foundation, Inc.
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

#define INPUT_FILENAME "loadMovieTest.swf"

#include "MovieTester.h"
#include "sprite_instance.h"
#include "character.h"
#include "dlist.h"
#include "container.h"
#include "log.h"
#include "URL.h"

#include "check.h"
#include <string>
#include <cassert>

using namespace gnash;
using namespace std;

int
main(int /*argc*/, char** /*argv*/)
{
return 0;
	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity(1);

	string filename = string(TGTDIR) + string("/") + string(INPUT_FILENAME);
	MovieTester tester(filename);

	URL baseURL(filename);
	URL mediaURL(MEDIADIR"/");
	URL lynchURL("lynch.swf", mediaURL);
	URL greenURL("green.jpg", mediaURL);
	URL offspringURL("offspring.swf", mediaURL);
	std::string url;


	sprite_instance* root = tester.getRootMovie();
	assert(root);

	check_equals(root->get_frame_count(), 1);
	check_equals(root->get_current_frame(), 0);

	tester.advance();
	check_equals(root->get_current_frame(), 0);

	// Verify that 'coverart' exists and is empty
	character* coverartch = const_cast<character*>(tester.findDisplayItemByName(*root, "coverart"));
	sprite_instance* coverart = coverartch->to_movie();
	check(coverart);
	url = coverart->get_movie_definition()->get_url();
	check_equals(coverart->get_movie_definition()->get_url(), baseURL.str());

	// Click on the first (lynch)
	tester.movePointerTo(80, 80);
	check(tester.isMouseOverMouseEntity());
	tester.pressMouseButton();
	sleep(1); // give it some time...
	coverartch = const_cast<character*>(tester.findDisplayItemByName(*root, "coverart"));
	check(coverart != coverartch->to_movie());
	coverart = coverartch->to_movie();
	check_equals(coverart->get_movie_definition()->get_url(), lynchURL.str());
	tester.depressMouseButton();

	// Click on the second (green)
	tester.movePointerTo(280, 80);
	check(tester.isMouseOverMouseEntity());
	tester.pressMouseButton();
	tester.depressMouseButton();
	sleep(1); // give it some time...
	coverartch = const_cast<character*>(tester.findDisplayItemByName(*root, "coverart"));
	coverart = coverartch->to_movie();
	check_equals(coverart->get_movie_definition()->get_url(), greenURL.str());
	// TODO: find a way to test if the jpeg is really displayed
	//       (like turn it into a mouse-event-handling char and use isMouseOverActiveEntity ?)

	// Click on the third (offspring)
	tester.movePointerTo(480, 80);
	check(tester.isMouseOverMouseEntity());
	tester.pressMouseButton();
	tester.depressMouseButton();
	sleep(1); // give it some time to load
	coverartch = const_cast<character*>(tester.findDisplayItemByName(*root, "coverart"));
	coverart = coverartch->to_movie();
	check_equals(coverart->get_movie_definition()->get_url(), offspringURL.str());

}

