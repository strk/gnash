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

#define INPUT_FILENAME "NetStream-SquareTest.swf"

#include "MovieTester.h"
#include "sprite_instance.h"
#include "character.h"
#include "dlist.h"
#include "container.h"
#include "log.h"

#include "check.h"
#include <string>
#include <cassert>
#include <unistd.h>

using namespace gnash;
using namespace std;

int
main(int /*argc*/, char** /*argv*/)
{
	string filename = string(TGTDIR) + string("/") + string(INPUT_FILENAME);
	MovieTester tester(filename);

	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity(1);

	sprite_instance* root = tester.getRootMovie();
	assert(root);

	if ( ! tester.canTestVideo() )
	{
		cout << "UNTESTED: NetStream video (not supported by this build)." << endl;
		return EXIT_SUCCESS;
	}

	// On NetStatus.Play.Stop we jump to last frame and stop,
	// so this loop is about equivalent to running a
	// generic self-contained test.
	//
	// When all possible tests are implemented as self-contained, we'll
	// add tests that can't be self-contained.
	//
	while (root->get_current_frame() < 2)
	{
		tester.advance();

		// sleep to give the NetStream a chance to load data and trigger notifications
		// needs more analisys to find a good way for doing this..
		sleep(1);
	}


}

