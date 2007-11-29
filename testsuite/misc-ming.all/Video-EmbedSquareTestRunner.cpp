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

#define INPUT_FILENAME "Video-EmbedSquareTest.swf"

#include "MovieTester.h"
#include "sprite_instance.h"
#include "character.h"
#include "dlist.h"
#include "container.h"
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

	sprite_instance* root = tester.getRootMovie();
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
	
	// Just loop twice, so to catch crashes...
	for (int j = 0; j < 2; ++j) {

		if ( ! tester.canTestVideo() )
		{
			// no pixel checking, but we'd still be interested in crashes...
			tester.advance();
			continue;
		}
		
		// Frame 1

		// Check the color in (1,1) - should be red
		check_pixel(1, 1, 1, red, 5);

		// Check the color in (30,1) - should be yellow
		check_pixel(35*scale_x, 1, 1, yellow, 5);

		// Check the color in (1,30) - should be yellow
		check_pixel(1, 35*scale_y, 1, yellow, 5);

		size_t framecount = root->get_frame_count();
		int frame = 1;
		int i = 0;
		while (frame++ < framecount) {
			// Frame X
			tester.advance();

			// Check the color in (9+i,1) - should be yellow
			check_pixel((5 + i)*scale_x, 1, 1, yellow, 5);

			// Check the color in (25+i,1) - should be red
			check_pixel((25 + i)*scale_x, 1, 1, red, 5);

			// Check the color in (25+i,30) - should be yellow
			check_pixel((25 + i)*scale_x, 35*scale_y, 1, yellow, 5);

			// The video is 128x96 so we don't want to check beyond that
			if (45+i <= 128) {
				// Check the color in (40+i,1) - should be yellow
				check_pixel((45 + i)*scale_x, 1, 1, yellow, 5);
			}

			i += 10;
		}

		tester.advance();
	}

}

