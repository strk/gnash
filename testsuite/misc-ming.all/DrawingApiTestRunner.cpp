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

#define INPUT_FILENAME "DrawingApiTest.swf"

#include "MovieTester.h"
#include "MovieClip.h"
#include "log.h"
#include "GnashKey.h" // for gnash::key::code

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

	MovieClip* root = tester.getRootMovie();
	assert(root);

	check_equals(root->get_frame_count(), 2);
	check_equals(root->get_current_frame(), 0);
	
	// advance to the second frame, first frame is an empty frame.
	//
	// Don't use the Dejagnuclip for this test, the EditText box in it
	// would violates many checks(invalidatedBound...) here.
	//
	tester.advance();

	rgba white(255, 255, 255, 255);
	rgba blue(0, 0, 255, 255);
	rgba cyan(0, 255, 255, 255);
	rgba green(0, 255, 0, 255);
	rgba green_alpha80(120, 248, 120, 255);
	rgba light_green(128, 255, 128, 255); 
	// Two greens at 50% alpha overlapping
	rgba overlapping_light_green(64, 255, 64, 255); 
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
	// this fails since Udo's rewrite of Shape_def::point_test
	// won't turn it into an 'expected' change as reverting the point_test 
	// would fix it.
	check(tester.isMouseOverMouseEntity());
	check_pixel(190, 112, 2, cyan, 1);

	// Inside green counterclockwise fill
	tester.movePointerTo(220, 112);
	check(tester.isMouseOverMouseEntity());
	check_pixel(220, 112, 2, green_alpha80, 1);

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
	check(!tester.isMouseOverMouseEntity()); // failed due to edge::withinSquareDistance bug
	check_pixel(376, 180, 2, white, 2);

	// Over the green curve
	tester.movePointerTo(376, 139);
	check(tester.isMouseOverMouseEntity()); // failed due to edge::withinSquareDistance bug
	check_pixel(376, 139, 2, green, 2); // failed due to bug in AGG

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

	struct IntPoint {
        int x;
        int y;
        IntPoint(int nx, int ny)
            :
            x(nx), y(ny)
        {}
    };

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
	check_pixel(146, 146, 2, white, 2); // gnash failed drawing invisible dynamic maskees !

	tester.pressKey(gnash::key::h); // make it visible again
	tester.advance(); // commit 
	check_pixel(146, 146, 2, red, 2);

	tester.click(); // disable the mask
	tester.advance(); // commit


	//--------------------------------------------------------------
	// Go to drawing #2 (hit the '2' ascii key)
	// and test rendering of those invalid shapes.
	//--------------------------------------------------------------

	tester.pressKey(gnash::key::_2); // go to second drawing
	tester.advance(); // commit

	//--------------------------------------------------------------
	// The double "EL"s case
	//--------------------------------------------------------------

	// In the right green 'el' shape (not explicitly closed fill)
	check_pixel(80, 170, 2, green, 2);

	// Outside the right green 'el' shape
	// (but close to the auto-closing edge)
	check_pixel(90, 144, 2, white, 2);

	// In the left green horizontally flipped 'el' shape
	// (not explicitly closed fill)
	check_pixel(30, 170, 2, green, 2);

	// Outside the left green 'el' shape
	// (but close to the auto-closing edge)
	check_pixel(20, 144, 2, white, 2);

	// Between the two 'el' shapes
	check_pixel(56, 170, 2, white, 2);

	//--------------------------------------------------------------
	// The red crossing edges case 
	//--------------------------------------------------------------

	// In the left niche
	check_pixel(46, 60, 2, white, 2);

	// In the right niche
	check_pixel(74, 60, 2, white, 2);

	// In the upper niche
	check_pixel(60, 48, 2, red, 2);

	// In the lower niche
	check_pixel(60, 72, 2, red, 2);

	//--------------------------------------------------------------
	// The four-in-a-row case (there should be no visible fill)
	//--------------------------------------------------------------

	// upper-left
	check_pixel(136, 35, 2, white, 2);
	// center-left
	check_pixel(136, 56, 2, white, 2);
	// lower-left
	check_pixel(136, 75, 2, white, 2);
	// lower-center
	check_pixel(156, 75, 2, white, 2);
	// lower-right
	check_pixel(175, 75, 2, white, 2);
	// center-right
	check_pixel(175, 56, 2, white, 2);
	// upper-right
	check_pixel(175, 35, 2, white, 2);
	// upper-center
	check_pixel(156, 35, 2, white, 2);
	// center
	check_pixel(156, 57, 2, white, 2);

	//--------------------------------------------------------------
	// The nested squares cases 
	//--------------------------------------------------------------


	//--------------------------------------------------------------
	// First nested squares case (hole)
	//--------------------------------------------------------------

	// X (145..160..175)
	// Y (145..160..175)

	// center-left
	check_pixel(145, 160, 2, green, 2);
	// center-right
	check_pixel(175, 160, 2, green, 2);
	// upper-center
	check_pixel(160, 145, 2, green, 2);
	// lower-center
	check_pixel(160, 175, 2, green, 2);
	// center-center (hole)
	check_pixel(160, 160, 2, white, 2);

	//--------------------------------------------------------------
	// Second nested squares case (hole)
	//--------------------------------------------------------------

	// X (194..210..226)
	// Y (145..160..175)

	// center-left
	check_pixel(194, 160, 2, green, 2);
	// center-right
	check_pixel(226, 160, 2, green, 2);
	// upper-center
	check_pixel(210, 145, 2, green, 2);
	// lower-center
	check_pixel(210, 175, 2, green, 2);
	// center-center (hole)
	check_pixel(210, 160, 2, white, 2);

	//--------------------------------------------------------------
	// Third nested squares case (subshape)
	//--------------------------------------------------------------

	// X (244..260..276)
	// Y (145..160..175)

	// center-left
	check_pixel(244, 160, 2, green, 2);
	// center-right
	check_pixel(276, 160, 2, green, 2);
	// upper-center
	check_pixel(260, 145, 2, green, 2);
	// lower-center
	check_pixel(260, 175, 2, green, 2);
	// center-center (two overlapping subshapes)
	check_pixel(260, 160, 2, green, 2);

	tester.pressKey(gnash::key::MINUS); // alpha goes down to 75
	tester.pressKey(gnash::key::MINUS); // alpha goes down to 50
	tester.advance(); // commit

	// center-left
	check_pixel(244, 160, 2, light_green, 2);
	// center-right
	check_pixel(276, 160, 2, light_green, 2);
	// upper-center
	check_pixel(260, 145, 2, light_green, 2);
	// lower-center
	check_pixel(260, 175, 2, light_green, 2);
	// center-center (two overlapping subshapes)
	check_pixel(260, 160, 2, overlapping_light_green, 2);

	tester.pressKey(gnash::key::PLUS); // alpha goes up to 75
	tester.pressKey(gnash::key::PLUS); // alpha goes up to 100
	tester.advance(); // commit

	//--------------------------------------------------------------
	// Fourth nested squares case (subshape)
	//--------------------------------------------------------------

	// X (293..309..325)
	// Y (145..160..175)

	// center-left
	check_pixel(294, 160, 2, green, 2);
	// center-right
	check_pixel(325, 160, 2, green, 2);
	// upper-center
	check_pixel(309, 145, 2, green, 2);
	// lower-center
	check_pixel(309, 175, 2, green, 2);
	// center-center (two overlapping subshapes)
	check_pixel(309, 160, 2, green, 2);

	tester.pressKey(gnash::key::MINUS); // alpha goes down to 75
	tester.pressKey(gnash::key::MINUS); // alpha goes down to 50
	tester.advance(); // commit

	// center-left
	check_pixel(294, 160, 2, light_green, 2);
	// center-right
	check_pixel(325, 160, 2, light_green, 2);
	// upper-center
	check_pixel(309, 145, 2, light_green, 2);
	// lower-center
	check_pixel(309, 175, 2, light_green, 2);
	// center-center (two overlapping subshapes)
	check_pixel(309, 160, 2, overlapping_light_green, 2);

	tester.pressKey(gnash::key::PLUS); // alpha goes up to 75
	tester.pressKey(gnash::key::PLUS); // alpha goes up to 100
	tester.advance(); // commit

	//--------------------------------------------------------------
	// Complex single-path crossing:
	// 
	//  10     5----4,0----------1
	//         |#####|###########|
	//         |#####|###########|
	//  20     6-----+----7######|
	//               |    |######|
	//               |    |######|
	//  30           9----8######|
	//               |###########|
	//  40           3-----------2
	//
	//         10   20    30     40
	//
	//  {X,Y} Scale : 200
	//  X offset    : 200
	//--------------------------------------------------------------

	int scale = 2;
	int xo = 200;
	int yo = 0;

	// Upper-Left
	check_pixel(xo + (15*scale), yo + (15*scale), 2, red, 2);
	// Upper-Center
	check_pixel(xo + (25*scale), yo + (15*scale), 2, red, 2);
	// Upper-On_09_stroke
	// AGG fails rendering a white stroke on the red background.
	// Cairo succeeds.
	check_pixel(xo + (20*scale), yo + (15*scale), 2, red, 2);
	// Upper-Right
	check_pixel(xo + (35*scale), yo + (15*scale), 2, red, 2);

	// Center-Left
	check_pixel(xo + (15*scale), yo + (25*scale), 2, white, 2);
	// Center-Center
	check_pixel(xo + (25*scale), yo + (25*scale), 2, white, 2);
	// Center-Right
	check_pixel(xo + (35*scale), yo + (25*scale), 2, red, 2);

	// Lower-Left
	check_pixel(xo + (15*scale), yo + (35*scale), 2, white, 2);
	// Lower-Lower
	check_pixel(xo + (25*scale), yo + (35*scale), 2, red, 2);
	// Lower-Right
	check_pixel(xo + (35*scale), yo + (35*scale), 2, red, 2);

	// On the 0-9 stroke, out of fill 
	// AGG fails rendering a white stroke on the red background.
	// Cairo succeeds.
	check_pixel(xo + (20*scale), yo + (25*scale), 3, white, 2);

    // Test picture 3
	tester.pressKey(gnash::key::_3); tester.advance();

    const int w = 100, h = 100;

    int x = 20, y = 20;

    // Shape 1

    // Bottom left corner (green line).
    check_pixel(x, y + h, 2, green, 2);
    // Bottom left fill (red)
    check_pixel(x + 20, y + 60, 2, red, 2);
    // Top right fill (red)
    check_pixel(x + 80, y + 20, 2, red, 2);
    // Dead centre fill (red)
    xcheck_pixel(x + w / 2, y + h / 2, 2, red, 2);
    // Top right corner (blue line)
    check_pixel(x + w, y, 2, blue, 2);

    // Shape 2
    
    x += 200;

    // Bottom left corner (green line).
    check_pixel(x, y + h, 2, green, 2);
    // Bottom left fill (red)
    check_pixel(x + 20, y + 60, 2, red, 2);
    // Top right fill (red)
    xcheck_pixel(x + 80, y + 20, 2, red, 2);
    // Dead centre fill (red)
    xcheck_pixel(x + w / 2, y + h / 2, 2, red, 2);
    // Top right corner (blue line, is correct to be over black line ending)
    check_pixel(x + w, y, 2, blue, 2);
    // Top centre (black line)
    check_pixel(x + w / 2, y, 2, black, 2);

    // Shape 3
    
    x += 200;

    // Bottom left corner (black line).
    check_pixel(x, y + h, 2, black, 2);
    // Bottom left fill (none)
    check_pixel(x + 20, y + 60, 2, white, 2);
    // Top right fill (none)
    check_pixel(x + 80, y + 20, 2, white, 2);
    // Dead centre fill (none)
    check_pixel(x + w / 2, y + h / 2, 2, white, 2);
    // Top right corner (nothing)
    check_pixel(x + w, y, 2, white, 2);

    // Shape 4
    
    x = 20;
    y += 150;

    // Should look the same as Shape 3

    // Bottom left corner (black line).
    check_pixel(x, y + h, 2, black, 2);
    // Bottom left fill (none)
    check_pixel(x + 20, y + 60, 2, white, 2);
    // Top right fill (none)
    check_pixel(x + 80, y + 20, 2, white, 2);
    // Dead centre fill (none)
    check_pixel(x + w / 2, y + h / 2, 2, white, 2);
    // Top right corner (nothing)
    check_pixel(x + w, y, 2, white, 2);

    // Shape 5

    x += 200;

    // Bottom left corner (black line).
    check_pixel(x, y + h, 2, black, 2);
    // Bottom left fill (black)
    check_pixel(x + 20, y + 60, 2, black, 2);
    // Top right fill (none)
    check_pixel(x + 80, y + 20, 2, white, 2);
    // Dead centre (black line)
    check_pixel(x + w / 2, y + h / 2, 2, black, 2);
    // Top right corner (nothing)
    check_pixel(x + w, y, 2, white, 2);
    
    // Shape 6

    x += 200;

    // NB: the rendering of this shape is not consistent
    // across different versions of the pp. It doesn't seem
    // like a sane case, so there is probably no need
    // to worry about compatibility.

    // Bottom left corner (yellow line).
    check_pixel(x, y + h, 2, yellow, 2);
    // Bottom left fill (green)
    check_pixel(x + 20, y + 60, 2, green, 2);

    // The following tests are possibly meaningless:
    
    // Top right fill (blue)
    check_pixel(x + 80, y + 20, 2, blue, 2);
    // Dead centre (yellow line)
    check_pixel(x + w / 2, y + h / 2, 2, yellow, 2);
    // Top right corner (yellow line)
    check_pixel(x + w, y, 2, yellow, 2);

    // Test picture 4
	tester.pressKey(gnash::key::_4); tester.advance();

    // The shapes are 90x90, spaced in a 100x100 pixel grid with 6 shapes in
    // each row.

    x = 0;
    y = 0;

    // Shape 1
    // Check each corner. The gradient is much too spread out, so should be
    // practically the same blue-white everywhere.
    rgba lightblue(120, 129, 248, 255);
    check_pixel(x + 2, y + 2, 2, lightblue, 2);
    check_pixel(x + 2, y + 90 - 2, 2, lightblue, 2);
    check_pixel(x + 90 - 2, y + 2, 2, lightblue, 2);
    check_pixel(x + 90 - 2, y + 90 - 2, 2, lightblue, 2);

    // Shape 2
    // Check each corner
    x += 100;
    check_pixel(x + 2, y + 2, 2, blue, 2);
    check_pixel(x + 2, y + 90 - 2, 2, blue, 2);
    check_pixel(x + 90 - 2, y + 2, 2, white, 2);
    check_pixel(x + 90 - 2, y + 90 - 2, 2, white, 2);
    
    // Shape 3
    // Check each corner
    x += 100;
    check_pixel(x + 2, y + 2, 2, blue, 2);
    check_pixel(x + 2, y + 90 - 2, 2, white, 2);
    check_pixel(x + 90 - 2, y + 2, 2, white, 2);
    check_pixel(x + 90 - 2, y + 90 - 2, 2, violet, 2);
    
    // Shape 4
    // Check each corner
    x += 100;
    rgba whiteblue(70,70,248,255);
    check_pixel(x + 2, y + 2, 2, blue, 2);
    check_pixel(x + 2, y + 90 - 2, 2, whiteblue, 2);
    check_pixel(x + 90 - 2, y + 2, 2, whiteblue, 2);
    check_pixel(x + 90 - 2, y + 90 - 2, 2, white, 2);

    // Shape 5
    // Check each corner
    x += 100;
    rgba whitegreen(80,248,80,255);
    check_pixel(x + 2, y + 2, 2, white, 2);
    check_pixel(x + 2, y + 90 - 2, 2, whitegreen, 2);
    check_pixel(x + 90 - 2, y + 2, 2, whiteblue, 2);
    check_pixel(x + 90 - 2, y + 90 - 2, 2, white, 2);

    // Shape 6
    // Check each row
    x += 100;
    int row = 90 / 5;
    check_pixel(x + 45, y + 1, 2, blue, 2);
    check_pixel(x + 45, y + row, 2, white, 2);
    check_pixel(x + 45, y + row * 2, 2, green, 2);

    // Note that these two have a lower alpha value and the rgba value we
    // expect is that combined with the white background and surrounding
    // fill colours..
    check_pixel(x + 45, y + row * 3, 2, rgba(240,120,244,255), 2);
    check_pixel(x + 45, y + row * 4 , 2, rgba(184,240,240,255), 2);
    
    // I'm fairly sure this should be plain yellow, but we render it with
    // alpha.
    xcheck_pixel(x + 45, y + 89, 2, yellow, 2);

    y += 100;
    x = 0;

    // Shape 7
    // Check each corner and centre. The gradient is much too spread out,
    // so should be almost the same blue-white everywhere.
    rgba otherblue(24,24,248,255);
    rgba otherblue2(56,56,248,255);
    check_pixel(x + 2, y + 2, 2, otherblue, 2);
    check_pixel(x + 2, y + 90 - 2, 2, otherblue2, 2);
    check_pixel(x + 90 - 2, y + 2, 2, otherblue, 2);
    check_pixel(x + 90 - 2, y + 90 - 2, 2, otherblue2, 2);
    check_pixel(x + 45, y + 45, 2, otherblue, 2);

    // Shape 8
    x += 100;
    check_pixel(x + 2, y + 2, 2, white, 2);
    check_pixel(x + 2, y + 90 - 2, 2, white, 2);
    check_pixel(x + 90 - 2, y + 2, 2, white, 2);
    check_pixel(x + 90 - 2, y + 90 - 2, 2, white, 2);
    check_pixel(x + 45, y + 45, 2, blue, 2);
    
    // Shape 9
    x += 100;
    check_pixel(x + 2, y + 2, 2, violet, 2);
    check_pixel(x + 2, y + 90 - 2, 2, violet, 2);
    check_pixel(x + 90 - 2, y + 2, 2, violet, 2);
    check_pixel(x + 90 - 2, y + 90 - 2, 2, violet, 2);
    check_pixel(x + 45, y + 45, 2, blue, 2);
    // Inner white circle
    check_pixel(x + 45, y + 45 / 2, 2, white, 2)

    // Shape 10
    x += 100;
    check_pixel(x + 2, y + 2, 2, violet, 2);
    check_pixel(x + 2, y + 90 - 2, 2, violet, 2);
    check_pixel(x + 90 - 2, y + 2, 2, violet, 2);
    check_pixel(x + 90 - 2, y + 90 - 2, 2, blue, 2);
    
    check_pixel(x + 90, y + 45, 2, white, 2);
    check_pixel(x + 45, y + 90, 2, white, 2);

    
    // Shape 11
    x += 100;
    check_pixel(x + 2, y + 2, 2, blue, 2);
    check_pixel(x + 2, y + 90 - 2, 2, green, 2);
    check_pixel(x + 90 - 2, y + 2, 2, green, 2);
    check_pixel(x + 90 - 2, y + 90 - 2, 2, green, 2);
    
    check_pixel(x + 45, y + 1, 2, white, 2);
    check_pixel(x + 1, y + 45, 2, white, 2);
    
    // Shape 12
    x += 100;
    check_pixel(x + 2, y + 2, 2, yellow, 2);
    check_pixel(x + 2, y + 90 - 2, 2, yellow, 2);
    check_pixel(x + 90 - 2, y + 2, 2, yellow, 2);
    check_pixel(x + 90 - 2, y + 90 - 2, 2, yellow, 2);
    check_pixel(x + 45, y + 45, 2, blue, 2);

    // Shape 13
    y += 100;
    x = 0;
    check_pixel(x + 7, y + 2, 2, green, 2);
    check_pixel(x + 7, y + 90 - 2, 2, green, 2);
    check_pixel(x + 90 - 2, y + 2, 2, green, 2);
    check_pixel(x + 90 - 2, y + 90 - 2, 2, green, 2);

    // Shape 14
    x += 100;
    check_pixel(x + 7, y + 2, 2, red, 2);
    check_pixel(x + 7, y + 90 - 2, 2, red, 2);
    check_pixel(x + 90 - 2, y + 2, 2, red, 2);
    check_pixel(x + 90 - 2, y + 90 - 2, 2, red, 2);

    // Shape 15
    x += 100;
    check_pixel(x + 7, y + 2, 2, green, 2);
    check_pixel(x + 7, y + 90 - 2, 2, green, 2);
    check_pixel(x + 90 - 2, y + 2, 2, green, 2);
    check_pixel(x + 90 - 2, y + 90 - 2, 2, green, 2);

    // 3 invalid fills follow

    // Shape 16
    x += 100;
    check_pixel(x + 7, y + 2, 2, white, 2);
    check_pixel(x + 7, y + 90 - 2, 2, white, 2);
    check_pixel(x + 90 - 2, y + 2, 2, white, 2);
    check_pixel(x + 90 - 2, y + 90 - 2, 2, white, 2);

    // Shape 17
    x += 100;
    check_pixel(x + 7, y + 2, 2, white, 2);
    check_pixel(x + 7, y + 90 - 2, 2, white, 2);
    check_pixel(x + 90 - 2, y + 2, 2, white, 2);
    check_pixel(x + 90 - 2, y + 90 - 2, 2, white, 2);

    x = 0;
    y += 100;

    // Shape 18
    check_pixel(x + 7, y + 2, 2, white, 2);
    check_pixel(x + 7, y + 90 - 2, 2, white, 2);
    check_pixel(x + 90 - 2, y + 2, 2, white, 2);
    check_pixel(x + 90 - 2, y + 90 - 2, 2, white, 2);

    //----------------------------------------------------------
	// TODO: check startDrag/stopDrag on the hit detector
	// (hit 'd' key to toggle)
	//----------------------------------------------------------
	//


}

