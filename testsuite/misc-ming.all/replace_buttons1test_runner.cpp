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

#define INPUT_FILENAME "replace_buttons1test.swf"

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
using namespace gnash::geometry;

TRYMAIN(_runtest);
int
trymain(int /*argc*/, char** /*argv*/)
{
	typedef gnash::geometry::SnappingRanges2d<int> Ranges;
	typedef gnash::geometry::Range2d<int> Bounds;

	string filename = string(TGTDIR) + string("/") + string(INPUT_FILENAME);
	MovieTester tester(filename);

	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity(1);

	// Colors we'll use during the test
	rgba red(255,0,0,255);
	rgba white(255,255,255,255);

	// Ranges we'll use during the test
	Range2d<int> redRange1(100,300,160,360);
	Range2d<int> redRange2(130,330,190,390);

	Ranges invalidated;
	MovieClip* root = tester.getRootMovie();
	assert(root);

	// FRAME 1 (start)

	check_equals(root->get_frame_count(), 4);
	check_equals(root->getPlayState(), MovieClip::PLAYSTATE_PLAY);
	check_equals(root->get_current_frame(), 0);
	check_equals(root->getDisplayList().size(), 1);  // dejagnu clip
	invalidated = tester.getInvalidatedRanges();
	check( invalidated.contains(76, 4) ); // the "-xtrace enabled-" label...

	tester.advance(); // FRAME 2, place DisplayObject
	invalidated = tester.getInvalidatedRanges();
	
	check_equals(root->getPlayState(), MovieClip::PLAYSTATE_PLAY);
	check_equals(root->get_current_frame(), 1);
	check_equals(root->getDisplayList().size(), 2); // dejagnu + red square

	// check invalidated bounds contain the red square at (100,300 - 160,360)
	check( invalidated.contains(redRange1) );
	
	// check that we have a red square at (100,300 - 160,360)
	check_pixel(104, 304, 2, red, 2); // UL
	check_pixel(156, 304, 2, red, 2); // UR
	check_pixel(156, 356, 2, red, 2); // LL
	check_pixel(104, 356, 2, red, 2); // LR

	// and nothing around it...
	check_pixel( 96, 330, 2, white, 2); // Left
	check_pixel(164, 330, 2, white, 2); // Right
	check_pixel(130, 296, 2, white, 2); // Top
	check_pixel(130, 364, 2, white, 2); // Bottom

	tester.advance(); // FRAME 3, replace DisplayObject
	invalidated = tester.getInvalidatedRanges();
	
	check_equals(root->getPlayState(), MovieClip::PLAYSTATE_PLAY);
	check_equals(root->get_current_frame(), 2);
	check_equals(root->getDisplayList().size(), 2); // dejagnu + red square

	// check invalidated bounds to contain:
	// 	- the red square (moved)
	// 	- the red square (original)
	//
	check( invalidated.contains(redRange1) );
	check( invalidated.contains(redRange2) );
	
	// check that we have a red square at (130,330 - 190,390)
	// Gnash fails here becase it does a *real* replace, while it
	// seems we're not supposed to replace, just to move (who knows why?!)
	check_pixel(134, 334, 2, red, 2); // UL
	check_pixel(186, 334, 2, red, 2); // UR
	check_pixel(186, 386, 2, red, 2); // LL
	check_pixel(134, 386, 2, red, 2); // LR

	// and nothing around it...
	check_pixel(126, 360, 2, white, 2); // Left
	check_pixel(194, 360, 2, white, 2); // Right
	check_pixel(160, 326, 2, white, 2); // Top
	check_pixel(160, 394, 2, white, 2); // Bottom

	tester.advance(); // FRAME 4, jump to frame 2 and stop
	invalidated = tester.getInvalidatedRanges();

	check_equals(root->getPlayState(), MovieClip::PLAYSTATE_STOP);
	check_equals(root->get_current_frame(), 1);
	check_equals(root->getDisplayList().size(), 2); // dejagnu + red square

	// check invalidated bounds to contain:
	// 	- the red square (moved)
	// 	- the red square (original)
	//
	check( invalidated.contains(redRange1) );
	check( invalidated.contains(redRange2) );

	// check that we have a red square at (100,300 - 160,360)
	check_pixel(104, 304, 2, red, 2); // UL
	check_pixel(156, 304, 2, red, 2); // UR
	check_pixel(156, 356, 2, red, 2); // LL
	check_pixel(104, 356, 2, red, 2); // LR

	// and nothing around it...
	check_pixel( 96, 330, 2, white, 2); // Left
	check_pixel(164, 330, 2, white, 2); // Right
	check_pixel(130, 296, 2, white, 2); // Top
	check_pixel(130, 364, 2, white, 2); // Bottom

	return 0;
}

