/* 
 *   Copyright (C) 2007 Free Software Foundation, Inc.
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

#define INPUT_FILENAME "loadMovieTest.swf"

#include "MovieTester.h"
#include "sprite_instance.h"
#include "character.h"
#include "dlist.h"
#include "log.h"
#include "URL.h"

#include "check.h"
#include <string>
#include <cassert>
#include <unistd.h>

using namespace gnash;
using namespace std;

std::auto_ptr<MovieTester> tester;
boost::intrusive_ptr<sprite_instance> root;

sprite_instance*
getCoverArt()
{
	character* coverartch = const_cast<character*>(tester->findDisplayItemByName(*root, "coverart"));
	sprite_instance* coverart = coverartch->to_movie();

	//log_msg("Coverart is %p, displaylist is:", coverart);
	//coverart->getDisplayList().dump();

	return coverart;
}

void
checkScribbling()
{
	sprite_instance* coverart = getCoverArt();

	size_t initial_child_count = coverart->getDisplayList().size();

	tester->movePointerTo(73, 204); // the "Scribble" button
	check(tester->isMouseOverMouseEntity());
	for (int i=1; i<=5; ++i) {
		tester->click();
		check_equals(coverart->getDisplayList().size(), initial_child_count+i);
	}

	tester->movePointerTo(59, 225); // the "clear" button
	check(tester->isMouseOverMouseEntity());
	tester->click();
	check_equals(coverart->getDisplayList().size(), initial_child_count);
}

int
main(int /*argc*/, char** /*argv*/)
{
	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity(1);

	string filename = string(TGTDIR) + string("/") + string(INPUT_FILENAME);
	tester.reset(new MovieTester(filename));

	URL baseURL(filename);
	URL mediaURL(MEDIADIR"/");
	URL lynchURL("lynch.swf", mediaURL);
	URL greenURL("green.jpg", mediaURL);
	URL offspringURL("offspring.swf", mediaURL);
	std::string url;

	gnash::RcInitFile& rc = gnash::RcInitFile::getDefaultInstance();
	rc.addLocalSandboxPath(MEDIADIR);



	root = tester->getRootMovie();
	assert(root.get());

	check_equals(root->get_frame_count(), 2);
	check_equals(root->get_current_frame(), 1);

	tester->advance();
	check_equals(root->get_current_frame(), 0);

	// Verify that 'coverart' exists and is empty
	character* coverartch = const_cast<character*>(tester->findDisplayItemByName(*root, "coverart"));
	sprite_instance* coverart = coverartch->to_movie();
	check(coverart);
	url = coverart->get_movie_definition()->get_url();
	check_equals(coverart->get_movie_definition()->get_url(), baseURL.str());

	// Check scribbling on the empty canvas
	checkScribbling();

	// Click on the first (lynch)
	tester->movePointerTo(80, 80);
	check(tester->isMouseOverMouseEntity());
	tester->pressMouseButton();
	sleep(1); // give it some time...
	tester->advance(); // loads (should) happen on next advance
	coverartch = const_cast<character*>(tester->findDisplayItemByName(*root, "coverart"));
	check(coverart != coverartch->to_movie());
	coverart = coverartch->to_movie();
	check_equals(coverart->get_movie_definition()->get_url(), lynchURL.str());
	tester->depressMouseButton();

	// Check scribbling on the lynch
	checkScribbling();

	// Click on the second (green)
	tester->movePointerTo(280, 80);
	check(tester->isMouseOverMouseEntity());
	tester->click();
	sleep(1); // give it some time...
	tester->advance(); // loads (should) happen on next advance
	coverartch = const_cast<character*>(tester->findDisplayItemByName(*root, "coverart"));
	coverart = coverartch->to_movie();
	check_equals(coverart->get_movie_definition()->get_url(), greenURL.str());
	// TODO: find a way to test if the jpeg is really displayed
	//       (like turn it into a mouse-event-handling char and use isMouseOverActiveEntity ?)

	// Check scribbling on the jpeg
	checkScribbling();

	// Click on the third (offspring)
	tester->movePointerTo(480, 80);
	check(tester->isMouseOverMouseEntity());
	tester->click();
	sleep(1); // give it some time to load
	tester->advance(); // loads (should) happen on next advance
	coverartch = const_cast<character*>(tester->findDisplayItemByName(*root, "coverart"));
	coverart = coverartch->to_movie();
	check_equals(coverart->get_movie_definition()->get_url(), offspringURL.str());

	// Check scribbling on the offspring
	checkScribbling();
}

