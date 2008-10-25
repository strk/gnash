/* 
 *   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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

#define INPUT_FILENAME "eventSoundTest1.swf"

#include "MovieTester.h"
#include "MovieClip.h"
#include "character.h"
#include "DisplayList.h"
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
	string filename = string(TGTDIR) + string("/") + string(INPUT_FILENAME);
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

	tester.advance();

	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity(1);
	dbglogfile.setActionDump(1);

	MovieClip* root = tester.getRootMovie();
	assert(root);

	check_equals(root->get_frame_count(), 20);

	if ( ! tester.canTestSound() )
	{
		cout << "UNTESTED: sounds can't be tested with this build." << endl;
		return EXIT_SUCCESS; // so testing doesn't abort
	} 

	// Advance and check...
	int frame = root->get_current_frame(); // 1
	int sounds = 1;
	while (frame < 21) {
//		check_equals(root->get_current_frame(), frame);
		check_equals(tester.soundsStarted(), sounds);
		tester.advance();
		frame++;

//		check_equals(root->get_current_frame(), frame);
		check_equals(tester.soundsStopped(), sounds);
		tester.advance();
		frame++;
		sounds++;		
	}

	check_equals(tester.soundsStopped(), tester.soundsStarted());

}

