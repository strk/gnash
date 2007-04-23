/* 
 *   Copyright (C) 2007 Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
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

	check_equals(root->get_frame_count(), 1);
	check_equals(root->get_current_frame(), 0);

	tester.advance();

	rgba white(255, 255, 255, 255);
	rgba blue(0, 0, 255, 255);
	rgba cyan(0, 255, 255, 255);
	rgba green(0, 255, 0, 255);
	rgba red(255, 0, 0, 255);
	rgba yellow(255, 255, 0, 255);
	rgba black(0, 0, 0, 255);
	rgba violet(255, 0, 255, 255);

	// Out of any drawing
	tester.movePointerTo(50, 50);
	check(!tester.isMouseOverMouseEntity());
	check_pixel(50, 50, 2, white, 1);

	// Inside bottom-left blue fill
	tester.movePointerTo(60, 215);
	check(tester.isMouseOverMouseEntity());
	check_pixel(60, 215, 2, blue, 1);

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
	xcheck_pixel(146, 146, 2, red, 2);

	// Over the black "hairlined" line
	tester.movePointerTo(250, 180);
	check(tester.isMouseOverMouseEntity());
	xcheck_pixel(250, 180, 1, black, 2);

	// Inside the yellow line
	tester.movePointerTo(270, 232);
	check(tester.isMouseOverMouseEntity());
	xcheck_pixel(270, 232, 2, yellow, 2);

	// Inside the black vertical line
	tester.movePointerTo(82, 127);
	check(tester.isMouseOverMouseEntity());
	check_pixel(82, 127, 2, black, 2);

	// In the middle of an imaginary line between
	// first and last point of the green curve
	tester.movePointerTo(355, 156);
	xcheck(!tester.isMouseOverMouseEntity()); // fails due to edge::withinSquareDistance bug
	check_pixel(355, 156, 2, white, 2);

	tester.movePointerTo(376, 139);
	xcheck(tester.isMouseOverMouseEntity()); // fails due to edge::withinSquareDistance bug
	xcheck_pixel(376, 139, 2, green, 2); // fails due to bug in AGG
}

