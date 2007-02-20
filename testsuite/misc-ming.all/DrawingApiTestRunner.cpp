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

#define INPUT_FILENAME "DrawingApi.swf"

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

	// Out of any drawing
	tester.movePointerTo(50, 50);
	check(!tester.isMouseOverMouseEntity());

	// Inside bottom-left blue fill
	tester.movePointerTo(60, 215);
	check(tester.isMouseOverMouseEntity());

	// Inside cyan clockwise fill
	tester.movePointerTo(190, 112);
	check(tester.isMouseOverMouseEntity());

	// Inside green counterclockwise fill
	tester.movePointerTo(220, 112);
	check(tester.isMouseOverMouseEntity());

	// Inside red "thick" line
	tester.movePointerTo(146, 146);
	xcheck(tester.isMouseOverMouseEntity());

}

