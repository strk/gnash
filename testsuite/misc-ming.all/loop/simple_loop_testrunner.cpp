/* 
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
 *   2011 Free Software Foundation, Inc.
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

// TODO: fix invalidated bounds, which are clearly bogus !

#define INPUT_FILENAME "simple_loop_test.swf"

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

void testAll(MovieTester& tester);

TRYMAIN(_runtest);
int
trymain(int /*argc*/, char** /*argv*/)
{
	string filename = string(TGTDIR) + string("/") + string(INPUT_FILENAME);
	MovieTester tester(filename);

	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity(1);

	testAll(tester);
	tester.restart();
	testAll(tester);

	return 0;
}


void testAll(MovieTester& tester)
{
	typedef gnash::geometry::SnappingRanges2d<int> Ranges;
	typedef gnash::geometry::Range2d<int> Bounds;

	Ranges invalidated;
	MovieClip* root = tester.getRootMovie();
	assert(root);

	// FRAME 1/4 (start)

	check_equals(root->get_frame_count(), 4);
	check_equals(root->getPlayState(), MovieClip::PLAYSTATE_PLAY);
	check_equals(root->get_current_frame(), 0);
	check_equals(root->getDisplayList().size(), 0); // no chars
	invalidated = tester.getInvalidatedRanges();
	// I think it makes sense for the first frame to have world 
	// inv bounds.
	check( invalidated.isWorld() );

	tester.advance(); // FRAME 2/4
	
	check_equals(root->getPlayState(), MovieClip::PLAYSTATE_PLAY);
	check_equals(root->get_current_frame(), 1);
	check_equals(root->getDisplayList().size(), 1);
	check( tester.findDisplayItemByDepth(*root, 2+DisplayObject::staticDepthOffset) );
	invalidated = tester.getInvalidatedRanges();
	check( invalidated.contains(Bounds(0, 0, 60, 60)) );

	tester.advance(); // FRAME 3/4
	
	check_equals(root->getPlayState(), MovieClip::PLAYSTATE_PLAY);
	check_equals(root->get_current_frame(), 2);
	check_equals(root->getDisplayList().size(), 2);
	check( tester.findDisplayItemByDepth(*root, 2+DisplayObject::staticDepthOffset) );
	check( tester.findDisplayItemByDepth(*root, 3+DisplayObject::staticDepthOffset) );
	invalidated = tester.getInvalidatedRanges();
	check( invalidated.contains(Bounds(60, 0, 120, 60)) );

	tester.advance(); // FRAME 4/4
	
	check_equals(root->getPlayState(), MovieClip::PLAYSTATE_PLAY);
	check_equals(root->get_current_frame(), 3);
	check_equals(root->getDisplayList().size(), 3);
	check( tester.findDisplayItemByDepth(*root, 2+DisplayObject::staticDepthOffset) );
	check( tester.findDisplayItemByDepth(*root, 3+DisplayObject::staticDepthOffset) );
	check( tester.findDisplayItemByDepth(*root, 4+DisplayObject::staticDepthOffset) );
	invalidated = tester.getInvalidatedRanges();
	check( invalidated.contains(Bounds(120, 0, 180, 60)) );

	tester.advance(); // FRAME 1/4 (loop back)
	
	check_equals(root->getPlayState(), MovieClip::PLAYSTATE_PLAY);
	check_equals(root->get_current_frame(), 0);
	check_equals(root->getDisplayList().size(), 0);
	invalidated = tester.getInvalidatedRanges();
	check( invalidated.contains(Bounds(0, 0, 180, 60)) );

	tester.advance(); // FRAME 2/4
	
	check_equals(root->getPlayState(), MovieClip::PLAYSTATE_PLAY);
	check_equals(root->get_current_frame(), 1);
	check_equals(root->getDisplayList().size(), 1);
	check( tester.findDisplayItemByDepth(*root, 2+DisplayObject::staticDepthOffset) );
	invalidated = tester.getInvalidatedRanges();
	check( invalidated.contains(Bounds(0, 0, 60, 60)) );

	tester.advance(); // FRAME 3/4
	
	check_equals(root->getPlayState(), MovieClip::PLAYSTATE_PLAY);
	check_equals(root->get_current_frame(), 2);
	check_equals(root->getDisplayList().size(), 2);
	check( tester.findDisplayItemByDepth(*root, 2+DisplayObject::staticDepthOffset) );
	check( tester.findDisplayItemByDepth(*root, 3+DisplayObject::staticDepthOffset) );
	invalidated = tester.getInvalidatedRanges();
	check( invalidated.contains(Bounds(60, 0, 120, 60)) );

	tester.advance(); // FRAME 4/4
	
	check_equals(root->getPlayState(), MovieClip::PLAYSTATE_PLAY);
	check_equals(root->get_current_frame(), 3);
	check_equals(root->getDisplayList().size(), 3);
	check( tester.findDisplayItemByDepth(*root, 2+DisplayObject::staticDepthOffset) );
	check( tester.findDisplayItemByDepth(*root, 3+DisplayObject::staticDepthOffset) );
	check( tester.findDisplayItemByDepth(*root, 4+DisplayObject::staticDepthOffset) );
	invalidated = tester.getInvalidatedRanges();
	check( invalidated.contains(Bounds(120, 0, 180, 60)) );
}
