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

#define INPUT_FILENAME "DrawingApiTest.swf"

#include "MovieTester.h"
#include "sprite_instance.h"
#include "character.h"
#include "dlist.h"
#include "container.h"
#include "log.h"
#include "gnash.h" // for gnash::key::code

#include "check.h"
#include <string>
#include <cassert>
#include <sstream>

using namespace gnash;
using namespace gnash::geometry;
using namespace std;

/// Return a Range2d<int> defining the square inscribed in a circle
//
/// @param radius
///	Radius in pixels
///
Range2d<int>
inscribedRect(int x, int y, int radius)
{
	Range2d<int> ret;

	int side = int(round((float)radius * sqrt(2.0f))); 
	int halfside = int(side/2.0); // round toward zero

	// Simply constructing a stringstream fixes an optimization
	// bug with GCC-4.1.2 resulting in absurd values !
	// See https://savannah.gnu.org/bugs/?20853
	std::stringstream work_around_GCC_412_bug;

	// upper-left corner
	int ULx = x-halfside;
	int ULy = y-halfside;

	// lower-right corner
	int LRx = x+halfside;
	int LRy = y+halfside;

	ret.expandTo(ULx, ULy);
	ret.expandTo(LRx, LRy);

	return ret;
}

int
main(int /*argc*/, char** /*argv*/)
{
	string filename = string(TGTDIR) + string("/") + string(INPUT_FILENAME);
	MovieTester tester(filename);

	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity(1);

	sprite_instance* root = tester.getRootMovie();
	assert(root);

	check_equals(root->get_frame_count(), 1);
	check_equals(root->get_current_frame(), 0);

	//tester.advance();

	rgba white(255, 255, 255, 255);
	rgba blue(0, 0, 255, 255);
	rgba cyan(0, 255, 255, 255);
	rgba green(0, 255, 0, 255);
	rgba red(255, 0, 0, 255);
	rgba yellow(255, 255, 0, 255);
	rgba black(0, 0, 0, 255);
	rgba gray(127, 127, 127, 255);
	rgba violet(255, 0, 255, 255);
	rgba halftrans_violet(255, 128, 255, 255);

	// Out of any drawing
	tester.movePointerTo(50, 50);
	check(!tester.isMouseOverMouseEntity());
	check_pixel(50, 50, 2, white, 1);

	// Inside bottom-left blue fill
	tester.movePointerTo(60, 215);
	check(tester.isMouseOverMouseEntity());
	check_pixel(60, 215, 2, blue, 1);

	// Inside bottom-left blue fill but in the 
	// curve internal to check if the point_test
	// works for filled curves
	tester.movePointerTo(40, 205);
	check(tester.isMouseOverMouseEntity());
	check_pixel(40, 205, 2, blue, 1);

	// Inside cyan clockwise fill
	tester.movePointerTo(190, 112);
	check(tester.isMouseOverMouseEntity());
	check_pixel(190, 112, 2, cyan, 1);

	// Inside green counterclockwise fill
	tester.movePointerTo(220, 112);
	check(tester.isMouseOverMouseEntity());
	check_pixel(220, 112, 2, green, 1);

	// Inside violet fill
	tester.movePointerTo(250, 112);
	check(tester.isMouseOverMouseEntity());
	check_pixel(250, 112, 2, violet, 1);

	// Inside red "thick" line
	tester.movePointerTo(146, 146);
	check(tester.isMouseOverMouseEntity());
	check_pixel(146, 146, 2, red, 2);

	// Over the black "hairlined" line
	tester.movePointerTo(250, 180);
	check(tester.isMouseOverMouseEntity());
	// pixel at 250,180 is black
	check_pixel(250, 180, 1, black, 2);
	// pixels above and below 180 are white
	check_pixel(250, 179, 1, white, 2);
	check_pixel(250, 181, 1, white, 2);

	// Over the transparent line (150,100)
	tester.movePointerTo(150, 100);
	check(!tester.isMouseOverMouseEntity()); 
	check_pixel(150, 100, 2, white, 2); 

	// Over the violet line (146,224)
	tester.movePointerTo(146, 224);
	check(tester.isMouseOverMouseEntity());
	check_pixel(146, 224, 1, halftrans_violet, 2);

	// Inside the yellow line
	tester.movePointerTo(270, 232);
	check(tester.isMouseOverMouseEntity());
	check_pixel(270, 232, 2, yellow, 2);

	// Inside the black vertical line
	tester.movePointerTo(82, 127);
	check(tester.isMouseOverMouseEntity());
	check_pixel(82, 127, 2, black, 2);

	// In the middle of an imaginary line between
	// first and last point of the green curve
	tester.movePointerTo(376, 180);
	xcheck(!tester.isMouseOverMouseEntity()); // fails due to edge::withinSquareDistance bug
	check_pixel(376, 180, 2, white, 2);

	// Over the green curve
	tester.movePointerTo(376, 139);
	check(tester.isMouseOverMouseEntity()); // failed due to edge::withinSquareDistance bug
	check_pixel(376, 139, 2, green, 2); // fails due to bug in AGG

	// Over the center of the green circle fill
	tester.movePointerTo(330, 160);
	check(tester.isMouseOverMouseEntity()); 
	check_pixel(330, 160, 2, green, 2); 

	// Over the boundary of the green circle fill
	tester.movePointerTo(363, 174);
	check(tester.isMouseOverMouseEntity());  // failed due to edge::withinSquareDistance bug
	check_pixel(363, 174, 2, black, 2); 

	// Check that nothing is drawin in the bottom line
	check_pixel(47, 280, 10, white, 2); 
	check_pixel(101, 280, 10, white, 2); 
	check_pixel(151, 280, 10, white, 2); 
	check_pixel(201, 280, 10, white, 2); 
	check_pixel(250, 280, 10, white, 2); 
	check_pixel(303, 280, 10, white, 2); 
	check_pixel(351, 280, 10, white, 2); 
	check_pixel(400, 280, 10, white, 2); 

	tester.advance();

	// TODO: check bounds of the child, and hitTest

	// Check that the invalidated bounds to contain the first circle bounds
	check(tester.getInvalidatedRanges().contains(inscribedRect(47, 280, 10)));

	// Check that first circle has been drawn
	check_pixel(47, 280, 3, yellow, 2); 
	check_pixel(101, 280, 10, white, 2); 
	check_pixel(151, 280, 10, white, 2); 
	check_pixel(201, 280, 10, white, 2); 
	check_pixel(250, 280, 10, white, 2); 
	check_pixel(303, 280, 10, white, 2); 
	check_pixel(351, 280, 10, white, 2); 
	check_pixel(400, 280, 10, white, 2); 

	tester.advance();

	// TODO: check bounds of the child, and hitTest

	// Check invalidated bounds to contain the first and second circle bounds
	check(tester.getInvalidatedRanges().contains(inscribedRect(47, 280, 10)));
	check(tester.getInvalidatedRanges().contains(inscribedRect(101, 280, 10)));

	// Check that only the second circle is visible
	check_pixel(47, 280, 10, white, 2); 
	check_pixel(101, 280, 3, yellow, 2); 
	check_pixel(151, 280, 10, white, 2); 
	check_pixel(201, 280, 10, white, 2); 
	check_pixel(250, 280, 10, white, 2); 
	check_pixel(303, 280, 10, white, 2); 
	check_pixel(351, 280, 10, white, 2); 
	check_pixel(400, 280, 10, white, 2); 

	tester.advance();

	// TODO: check bounds of the child, and hitTest

	// Check invalidated bounds to contain the second and third circle bounds
	check(tester.getInvalidatedRanges().contains(inscribedRect(101, 280, 10)));
	check(tester.getInvalidatedRanges().contains(inscribedRect(151, 280, 10)));

	// Check that only the third circle is visible
	check_pixel(47, 280, 10, white, 2); 
	check_pixel(101, 280, 10, white, 2); 
	check_pixel(151, 280, 3, yellow, 2); 
	check_pixel(201, 280, 10, white, 2); 
	check_pixel(250, 280, 10, white, 2); 
	check_pixel(303, 280, 10, white, 2); 
	check_pixel(351, 280, 10, white, 2); 
	check_pixel(400, 280, 10, white, 2); 

	tester.advance();

	// TODO: check bounds of the child, and hitTest

	// Check invalidated bounds to contain the third and fourth circle bounds
	check(tester.getInvalidatedRanges().contains(inscribedRect(151, 280, 10)));
	check(tester.getInvalidatedRanges().contains(inscribedRect(201, 280, 10)));

	// Check that only the fourth circle is visible
	check_pixel(47, 280, 10, white, 2); 
	check_pixel(101, 280, 10, white, 2); 
	check_pixel(151, 280, 10, white, 2); 
	check_pixel(201, 280, 3, yellow, 2); 
	check_pixel(250, 280, 10, white, 2); 
	check_pixel(303, 280, 10, white, 2); 
	check_pixel(351, 280, 10, white, 2); 
	check_pixel(400, 280, 10, white, 2); 

	tester.advance();

	// TODO: check bounds of the child, and hitTest

	// Check invalidated bounds to contain the fourth and fifth circle bounds
	check(tester.getInvalidatedRanges().contains(inscribedRect(201, 280, 10)));
	check(tester.getInvalidatedRanges().contains(inscribedRect(250, 280, 10)));

	// Check that only the fifth circle is visible
	check_pixel(47, 280, 10, white, 2); 
	check_pixel(101, 280, 10, white, 2); 
	check_pixel(151, 280, 10, white, 2); 
	check_pixel(201, 280, 10, white, 2); 
	check_pixel(250, 280, 3, yellow, 2); 
	check_pixel(303, 280, 10, white, 2); 
	check_pixel(351, 280, 10, white, 2); 
	check_pixel(400, 280, 10, white, 2); 

	tester.advance();

	// TODO: check bounds of the child, and hitTest

	// Check invalidated bounds to contain the fifth and sixth circle bounds
	check(tester.getInvalidatedRanges().contains(inscribedRect(250, 280, 10)));
	check(tester.getInvalidatedRanges().contains(inscribedRect(303, 280, 10)));

	// Check that only the sixth circle is visible
	check_pixel(47, 280, 10, white, 2); 
	check_pixel(101, 280, 10, white, 2); 
	check_pixel(151, 280, 10, white, 2); 
	check_pixel(201, 280, 10, white, 2); 
	check_pixel(250, 280, 10, white, 2); 
	check_pixel(303, 280, 3, yellow, 2); 
	check_pixel(351, 280, 10, white, 2); 
	check_pixel(400, 280, 10, white, 2); 

	tester.advance();

	// TODO: check bounds of the child, and hitTest

	// Check invalidated bounds to contain the sixth and seventh circle bounds
	check(tester.getInvalidatedRanges().contains(inscribedRect(303, 280, 10)));
	check(tester.getInvalidatedRanges().contains(inscribedRect(351, 280, 10)));

	// Check that only the seventh circle is visible
	check_pixel(47, 280, 10, white, 2); 
	check_pixel(101, 280, 10, white, 2); 
	check_pixel(151, 280, 10, white, 2); 
	check_pixel(201, 280, 10, white, 2); 
	check_pixel(250, 280, 10, white, 2); 
	check_pixel(303, 280, 10, white, 2); 
	check_pixel(351, 280, 3, yellow, 2); 
	check_pixel(400, 280, 10, white, 2); 

	tester.advance();

	// TODO: check bounds of the child, and hitTest

	// Check invalidated bounds to contain the seventh and eighth circle bounds
	check(tester.getInvalidatedRanges().contains(inscribedRect(351, 280, 10)));
	check(tester.getInvalidatedRanges().contains(inscribedRect(400, 280, 10)));

	// Check that only the eighth circle is visible
	check_pixel(47, 280, 10, white, 2); 
	check_pixel(101, 280, 10, white, 2); 
	check_pixel(151, 280, 10, white, 2); 
	check_pixel(201, 280, 10, white, 2); 
	check_pixel(250, 280, 10, white, 2); 
	check_pixel(303, 280, 10, white, 2); 
	check_pixel(351, 280, 10, white, 2); 
	check_pixel(400, 280, 3, yellow, 2); 

	tester.advance();

	// TODO: check bounds of the child, and hitTest

	// Check that no bounds have been invalidated 
	check(tester.getInvalidatedRanges().isNull());

	// Check that only the eighth circle is visible
	check_pixel(47, 280, 10, white, 2); 
	check_pixel(101, 280, 10, white, 2); 
	check_pixel(151, 280, 10, white, 2); 
	check_pixel(201, 280, 10, white, 2); 
	check_pixel(250, 280, 10, white, 2); 
	check_pixel(303, 280, 10, white, 2); 
	check_pixel(351, 280, 10, white, 2); 
	check_pixel(400, 280, 3, yellow, 2); 

	//--------------------------------------------------------------
	// Check hitdetector bounds and reactions on mouse movement 
	//--------------------------------------------------------------

	typedef struct IntPoint { int x; int y; IntPoint(int nx, int ny) : x(nx), y(ny) {} };

	IntPoint c1s(6, 346); // top-right of first yellow circle (in when small)
	IntPoint c1b(16, 329); // top-right of first yellow circle (in when big, out when small)
	IntPoint c2s(66, 340); // top-right of second yellow circle (in when small)
	IntPoint c2b(75, 332); // top-right of second yellow circle (in when big, out when small)
	IntPoint c3s(129, 340); // top-right of third yellow circle (in when small)
	IntPoint c3b(133, 330); // top-right of third yellow circle (in when big, out when small)

	// Disjoint cursor's and drawing's bounding boxes 
	tester.movePointerTo(50, 50);
	tester.advance(); // hitTest runs onEnterFrame

	check_pixel(c1s.x, c1s.y, 1, yellow, 2); 
	check_pixel(c1b.x, c1b.y, 1, white, 2); 

	check_pixel(c2s.x, c2s.y, 1, yellow, 2); 
	check_pixel(c2b.x, c2b.y, 1, white, 2); 

	check_pixel(c3s.x, c3s.y, 1, yellow, 2); 
	check_pixel(c3b.x, c3b.y, 1, white, 2); 

	// Cursor's and drawing's bounding box intersection
	//
	// NOTE: shapes don't intersect, the cursor is a circle,
	//       and only the top-left corner of it's bounding box
	//       intersects the drawing's bounding box
	// NOTE: one pixel down or one pixel right and they would not intersect anymore
	//
	tester.movePointerTo(424, 304);
	tester.advance(); // hitTest runs onEnterFrame

	check_pixel(c1s.x, c1s.y, 1, yellow, 2); 
	check(tester.getInvalidatedRanges().contains(c1b.x, c1b.y));
	check_pixel(c1b.x, c1b.y, 1, yellow, 2); 

	check(!tester.getInvalidatedRanges().contains(c2s.x, c2s.y)); // failure won't impact correctness, only performance
	check(!tester.getInvalidatedRanges().contains(c2b.x, c2b.y)); // failure won't impact correctness, only performance
	check_pixel(c2s.x, c2s.y, 1, yellow, 2); 
	check_pixel(c2b.x, c2b.y, 1, white, 2); 

	check(!tester.getInvalidatedRanges().contains(c3s.x, c3s.y)); // failure won't impact correctness, only performance
	check(!tester.getInvalidatedRanges().contains(c3b.x, c3b.y)); // failure won't impact correctness, only performance
	check_pixel(c3s.x, c3s.y, 1, yellow, 2); 
	check_pixel(c3b.x, c3b.y, 1, white, 2); 

	// Pointer inside drawing's bounding box 
	// (pointer itself, not the cursor)
	tester.movePointerTo(221, 84);
	tester.advance(); // hitTest runs onEnterFrame

	check(!tester.getInvalidatedRanges().contains(c1s.x, c1s.y)); // failure won't impact correctness, only performance
	check(!tester.getInvalidatedRanges().contains(c1b.x, c1b.y)); // failure won't impact correctness, only performance
	check_pixel(c1s.x, c1s.y, 1, yellow, 2); 
	check_pixel(c1b.x, c1b.y, 1, yellow, 2); 

	check_pixel(c2s.x, c2s.y, 1, yellow, 2); 
	check(tester.getInvalidatedRanges().contains(c2b.x, c2b.y));
	check_pixel(c2b.x, c2b.y, 1, yellow, 2); 

	check(!tester.getInvalidatedRanges().contains(c3s.x, c3s.y)); // failure won't impact correctness, only performance
	check(!tester.getInvalidatedRanges().contains(c3b.x, c3b.y)); // failure won't impact correctness, only performance
	check_pixel(c3s.x, c3s.y, 1, yellow, 2); 
	check_pixel(c3b.x, c3b.y, 1, white, 2); 

	// Pointer inside drawing's shape
	tester.movePointerTo(167, 170); // the thik red line
	tester.advance(); // hitTest runs onEnterFrame

	check(!tester.getInvalidatedRanges().contains(c1s.x, c1s.y)); // failure won't impact correctness, only performance
	check(!tester.getInvalidatedRanges().contains(c1b.x, c1b.y)); // failure won't impact correctness, only performance
	check_pixel(c1s.x, c1s.y, 1, yellow, 2); 
	check_pixel(c1b.x, c1b.y, 1, yellow, 2); 

	check(!tester.getInvalidatedRanges().contains(c2s.x, c2s.y)); // failure won't impact correctness, only performance
	check(!tester.getInvalidatedRanges().contains(c2b.x, c2b.y)); // failure won't impact correctness, only performance
	check_pixel(c2s.x, c2s.y, 1, yellow, 2); 
	check_pixel(c2b.x, c2b.y, 1, yellow, 2); 

	check_pixel(c3s.x, c3s.y, 1, yellow, 2); 
	check(tester.getInvalidatedRanges().contains(c3b.x, c3b.y));
	check_pixel(c3b.x, c3b.y, 1, yellow, 2); 

	//--------------------------------------------------------------
	// Check _visible toggling effects
	// (triggered onKeyDown 'h', effects visible after advance)
	//--------------------------------------------------------------

	check_pixel(330, 160, 2, green, 2); 
	tester.pressKey(gnash::key::h); tester.advance();
	check_pixel(330, 160, 2, white, 2); 
	tester.pressKey(gnash::key::h); tester.advance();
	check_pixel(330, 160, 2, green, 2); 

	//--------------------------------------------------------------
	// Check setMask effect
	// (triggered onMouseDown, effects visible after advance)
	//--------------------------------------------------------------

	tester.click(); // this should enable cursor shape masking drawing

	tester.movePointerTo(50, 50); // out of any drawing
	tester.advance();

        // Inside bottom-left blue fill
	check_pixel(60, 215, 2, white, 2); 
	tester.movePointerTo(60, 215); tester.advance(); // move the mask over it...
	check_pixel(60, 215, 2, blue, 2); 

	// Inside violet fill
	check_pixel(250, 112, 2, white, 2);
	tester.movePointerTo(250, 112); tester.advance(); // move the mask over it
	check_pixel(250, 112, 2, violet, 2);

	// Inside red "thick" line
	check_pixel(146, 146, 2, white, 2);
	tester.movePointerTo(146, 146); tester.advance(); // move the mask over it
	check_pixel(146, 146, 2, red, 2);

	// Inside the violet fill
	check_pixel(250, 112, 2, white, 2);
	tester.click(); // this should disable the mask
	check_pixel(250, 112, 2, violet, 2);

	//--------------------------------------------------------------
	// Check setMask on invisible shape 
	//--------------------------------------------------------------

	tester.movePointerTo(146, 146); // move pointer over red shape
	tester.click(); // enable the mask
	tester.advance(); // commit all of the above
	check_pixel(146, 146, 2, red, 2);

	tester.pressKey(gnash::key::h); // make it invisible
	tester.advance(); // commit 
	xcheck_pixel(146, 146, 2, white, 2); // gnash still draws invisible dynamic maskees !

	tester.pressKey(gnash::key::h); // make it visible again
	tester.advance(); // commit 
	check_pixel(146, 146, 2, red, 2);


	//--------------------------------------------------------------
	// TODO: go to drawing #2 (hit the '2' ascii key)
	//       and test rendering of those invalid shapes.
	//--------------------------------------------------------------

}

