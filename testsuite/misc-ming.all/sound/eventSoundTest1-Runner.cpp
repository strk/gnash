/* 
 *   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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
#include "DisplayObject.h"
#include "DisplayList.h"
#include "log.h"
#include "GnashException.h"
#include "VM.h"

#include "check.h"

#include <string>
#include <iostream>
#include <cassert>
#include <memory>

using namespace gnash;
using namespace std;

TRYMAIN(_runtest);
int
trymain(int /*argc*/, char** /*argv*/)
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

	MovieClip* root = tester.getRootMovie();
	assert(root);

	VM& vm = getVM(*getObject(root));

	if ( ! tester.canTestSound() )
	{
		cout << "UNTESTED: sounds can't be tested with this build." << endl;
		return EXIT_SUCCESS; // so testing doesn't abort
	} 

	const int totalFrames = root->get_frame_count();

	// Make sure you adjust this with the test!
	cerr << "Total frames: " <<  totalFrames << endl;
	assert (totalFrames == 26);

	int numSoundsStarted[] = {
		0, 
		4, // Multiple   (+4 sounds started)
		6, // NoMultiple (+2 sounds started)
        9, // Trimmed    (+3 sounds started)
        14 // Attached   (+5 sounds started)
	};

	/// Expected success for each test
	bool testPasses[] = {
		true,
		true,
		true,
		true,
		true
	};

	// Advance and check...
	int frame = root->get_current_frame();
	int test = 0;
	while (frame <= totalFrames) {
		as_value testReady;
		if (getObject(root)->get_member(getURI(vm, "testReady"), &testReady)) {

			getObject(root)->delProperty(getURI(vm, "testReady"));
			
			// When a test is ready, check the result of the previous test.
			if (testPasses[test]) {
				check_equals(tester.soundsStarted(), numSoundsStarted[test]);
			}
			else {
				xcheck_equals(tester.soundsStarted(), numSoundsStarted[test]);
			}

			check_equals(tester.soundsStopped(), tester.soundsStarted());
            check_equals(tester.streamingSound(), false);
			++test;
			tester.click();

		}
		tester.advance();
		frame++;
	}

    if (testPasses[test]) {
        check_equals(tester.soundsStarted(), numSoundsStarted[test]);
    }
    else {
        xcheck_equals(tester.soundsStarted(), numSoundsStarted[test]);
    }

    // Consistency checking
    as_value eot;
    bool endOfTestFound =
        getObject(root)->get_member(getURI(vm, "endoftest"), &eot);
    check(endOfTestFound);

    return EXIT_SUCCESS;
}

