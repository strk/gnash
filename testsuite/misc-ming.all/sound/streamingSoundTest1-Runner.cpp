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

#define INPUT_FILENAME "streamingSoundTest1.swf"

#include "MovieTester.h"
#include "MovieClip.h"
#include "DisplayObject.h"
#include "DisplayList.h"
#include "log.h"
#include "GnashException.h"
#include "GnashSleep.h"

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

    gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();

    MovieTester tester(filename);

    dbglogfile.setVerbosity(1);

    MovieClip* root = tester.getRootMovie();
    assert(root);

    check_equals(root->get_frame_count(), 8);

    if (!tester.canTestSound()) {
        cout << "UNTESTED: sounds can't be tested with this build." << endl;
        return EXIT_SUCCESS; // so testing doesn't abort
    } 

    check_equals(tester.streamingSound(), true);
    
    // The Movie contains a sprite with a streaming sound. The sprite
    // has 4 frames, the movie 8. We expect the streaming sound to play
    // twice through. 

    while (root->get_current_frame() < root->get_frame_count() / 2 - 1) {

        // Check twice per frame so we don't get out of sync.
        tester.advanceClock(500);
        tester.advance(false);

        check_equals(tester.streamingSound(), true);
        check_equals(tester.soundsStarted(), 1);
        check_equals(tester.soundsStopped(), 0);
    }

    while (root->get_current_frame() < root->get_frame_count() - 1) {

        // Check twice per frame so we don't get out of sync.
        tester.advanceClock(500);
        tester.advance(false);

        check_equals(tester.streamingSound(), true);
        check_equals(tester.soundsStarted(), 2);
        check_equals(tester.soundsStopped(), 1);
    }

}

