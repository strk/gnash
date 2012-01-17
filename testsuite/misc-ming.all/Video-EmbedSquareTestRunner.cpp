/* 
 *   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
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

#define INPUT_FILENAME "Video-EmbedSquareTest.swf"

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

int
main(int /*argc*/, char** /*argv*/)
{
	string filename = string(TGTDIR) + string("/") + string(INPUT_FILENAME);
	MovieTester tester(filename);

	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity(1);

	MovieClip* root = tester.getRootMovie();
	assert(root);

	if ( ! tester.canTestRendering() )
	{
		cout << "UNTESTED: rendering of embedded video (testing not possible with this build)." << endl;
		return EXIT_SUCCESS; // so testing doesn't abort
	} 

	rgba red(255,0,0,255);
	rgba yellow(255,255,0,255);

	// TODO: change the scaling of the window/canvas
	int scale_x = 1;
	int scale_y = 1;

	size_t framecount = root->get_frame_count();
	check_equals(framecount, 12);
	
	// Just loop twice, so to catch crashes...
	for (int j = 0; j < 2; ++j) {

		if ( ! tester.canTestVideo() )
		{
			// no pixel checking, but we'd still be interested in crashes...
			tester.advance();
			continue;
		}
		
		// Frame 1

		check_equals(root->get_current_frame(), 0); // 0-based

		// Check the color in (5,5) - should be red
		check_pixel(5, 5, 2, red, 5);

		// Check the color in (35,5) - should be yellow
		check_pixel(35*scale_x, 5, 2, yellow, 5);

		// Check the color in (5,35) - should be yellow
		check_pixel(5, 35*scale_y, 2, yellow, 5);

		while (true)
		{
			// Frame X
			tester.advance();
			size_t framenum = root->get_current_frame();
            assert(framenum > 0);

			cout << "---- Pixel checking in frame " << framenum+1 << " play state " << root->getPlayState() << endl;
			
			size_t i = (framenum-1)*10;

			// Check the color in (5+i,5) - should be yellow
			check_pixel((5 + i)*scale_x, 5, 2, yellow, 5);

			// Check the color in (25+i,5) - should be red
			check_pixel((25 + i)*scale_x, 5, 2, red, 5);

			// Check the color in (25+i,35) - should be yellow
			check_pixel((25 + i)*scale_x, 35*scale_y, 1, yellow, 5);

			// The video is 128x96 so we don't want to check beyond that
			if (45+i <= 128) {
				// Check the color in (45+i,5) - should be yellow
				check_pixel((45 + i)*scale_x, 5, 2, yellow, 5);
			}

			if (framenum + 2 == framecount) {
				// check we're playing, or we'll never get to next loop...
				check_equals(root->getPlayState(), MovieClip::PLAYSTATE_PLAY);
				break;
			}

			tester.click();
		}

		tester.advance();

		// Check the color in (5,5) - should be yellow. Well, anything
		// but white or transparent.
		check_pixel(5, 5, 2, yellow, 5);
		check_equals(root->getPlayState(), MovieClip::PLAYSTATE_STOP);
		tester.click();

		// Sanity check
		check_equals(root->getPlayState(), MovieClip::PLAYSTATE_PLAY);

		tester.advance();

	}

}

