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

#define INPUT_FILENAME "loop_test2.swf"

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
	rgba black(0,0,0,255);

	// Ranges we'll use during the test
	Range2d<int> redRange(300,300,360,360);
	Range2d<int> blackRange(330,270,450,390);

	// Coordinates we'll use during testing
	int x_left = 270; // on the left of any DisplayObject
	int x_red = 310; // on the red square
	int x_int = 340; // on the intersection between the red and black squares
	int x_black = 370; // on black square
	int x_right = 460; // on the right of any DisplayObject
	int y = 330;

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

	tester.advance(); // FRAME 2, place DisplayObjects (black on top)
	invalidated = tester.getInvalidatedRanges();
	
	check_equals(root->getPlayState(), MovieClip::PLAYSTATE_PLAY);
	check_equals(root->get_current_frame(), 1);
	check_equals(root->getDisplayList().size(), 3); // dejagnu + red square + black square

	// check invalidated bounds contain both the red and black square
	check( invalidated.contains(redRange) );
	check( invalidated.contains(blackRange) );
	
	// Check that the black square is over the red square
	check_pixel(x_left, y, 2, white, 2); 
	check_pixel(x_red, y, 2, red, 2); 
	check_pixel(x_int, y, 2, black, 2);  // black is *over* red square
	check_pixel(x_black, y, 2, black, 2);  
	check_pixel(x_right, y, 2, white, 2);  

	tester.advance(); // FRAME 3, depth-swap the two DisplayObjects
	invalidated = tester.getInvalidatedRanges();
	
	check_equals(root->getPlayState(), MovieClip::PLAYSTATE_PLAY);
	check_equals(root->get_current_frame(), 2);
	check_equals(root->getDisplayList().size(), 3); // dejagnu + red square + black square

	// check invalidated bounds to contain the intersection
	// between the two DisplayObjects.
	//
	check( invalidated.contains(Intersection(redRange,blackRange)) );
	
	// Check that the black square is now behind the red square
	check_pixel(x_left, y, 2, white, 2); 
	check_pixel(x_red, y, 2, red, 2); 
	check_pixel(x_int, y, 2, red, 2);  // black is *behind* red square
	check_pixel(x_black, y, 2, black, 2);  
	check_pixel(x_right, y, 2, white, 2);  

	tester.advance(); // FRAME 4, jump to frame 2 and stop
	invalidated = tester.getInvalidatedRanges();

	check_equals(root->getPlayState(), MovieClip::PLAYSTATE_STOP);
	check_equals(root->get_current_frame(), 1);
	check_equals(root->getDisplayList().size(), 3); // dejagnu + red square + black square

	// Invalidated bounds can't be Null because something is printed 
	// in the XTRACE window... Anyway, the squares should be far enoguh
	// to assume the invalidated bounds won't contain their intersection
	//
	// Gnash has an huge invalidated bounds for the whole movie lifetime, btw....
	// 
	xcheck( ! invalidated.intersects(Intersection(redRange,blackRange)) );

	// Check that the black square is still behind the red square
	check_pixel(x_left, y, 2, white, 2); 
	check_pixel(x_red, y, 2, red, 2); 
	check_pixel(x_int, y, 2, red, 2);  // black is *behind* red square
	check_pixel(x_black, y, 2, black, 2);  
	check_pixel(x_right, y, 2, white, 2);  
	return 0;
}

