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

#define INPUT_FILENAME "StreamSoundTest.swf"

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

int
main(int /*argc*/, char** /*argv*/)
{
	string filename = string(TGTDIR) + string("/") + string(INPUT_FILENAME);
	MovieTester tester(filename);

	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity(1);

	MovieClip* root = tester.getRootMovie();
	assert(root);

    VM& vm = getVM(*getObject(root));

    const size_t framecount = root->get_frame_count();

    // Sanity.
    check_equals(framecount, 5);

	if (!tester.canTestSound()) {
		cout << "UNTESTED: sounds can't be tested with this build." << endl;
		return EXIT_SUCCESS; // so testing doesn't abort
	} 

    // Shouldn't be streaming yet.
    check_equals(tester.streamingSound(), false);

    // 20 x 0.75 seconds = 15 in total. The sound should last
    // 13.74 seconds.
    for (size_t i = 0; i < 20; ++i) {
        if (root->get_current_frame() + 1 == framecount) break;
        tester.advance();
        check_equals(tester.streamingSound(), true);
    }

}

