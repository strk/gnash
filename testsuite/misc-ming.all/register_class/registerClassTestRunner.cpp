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

#define INPUT_FILENAME "registerClassTest.swf"

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

	check_equals(root->get_frame_count(), 6);
	check_equals(root->getPlayState(), MovieClip::PLAYSTATE_PLAY);
	check_equals(root->get_current_frame(), 0);

	check(! tester.findDisplayItemByDepth(*root, 71) );
	check(! tester.findDisplayItemByDepth(*root, 72) );

	tester.movePointerTo(30, 30);
	check(!tester.isMouseOverMouseEntity());

	tester.advance();
	check_equals(root->get_current_frame(), 1);

	check( tester.findDisplayItemByDepth(*root, 71) );
	check(! tester.findDisplayItemByDepth(*root, 72) );

	tester.movePointerTo(30, 30);
	check(tester.isMouseOverMouseEntity());
	tester.movePointerTo(100, 30);
	check(!tester.isMouseOverMouseEntity());

	tester.advance();
	check_equals(root->get_current_frame(), 2);

	check(tester.findDisplayItemByDepth(*root, 71) );
	check(tester.findDisplayItemByDepth(*root, 72) );

	tester.movePointerTo(30, 30);
	check(tester.isMouseOverMouseEntity());
	tester.movePointerTo(100, 30);
	check(tester.isMouseOverMouseEntity());

	tester.advance();
	check_equals(root->get_current_frame(), 3);

	check(tester.findDisplayItemByDepth(*root, 71) );
	check(tester.findDisplayItemByDepth(*root, 72) );
	check(tester.findDisplayItemByDepth(*root, 73) );

	tester.movePointerTo(30, 30);
	check(tester.isMouseOverMouseEntity());
	tester.movePointerTo(100, 30);
	check(tester.isMouseOverMouseEntity());
	tester.movePointerTo(170, 30);
	check(tester.isMouseOverMouseEntity());

	tester.advance();
	check_equals(root->get_current_frame(), 4);

	tester.movePointerTo(30, 30);
	check(tester.isMouseOverMouseEntity());
	tester.movePointerTo(100, 30);
	check(tester.isMouseOverMouseEntity());
	tester.movePointerTo(170, 30);
	check(tester.isMouseOverMouseEntity());
	tester.movePointerTo(240, 30);
	check(tester.isMouseOverMouseEntity());

	tester.advance();
	check_equals(root->get_current_frame(), 5);

	tester.advance();
	check_equals(root->get_current_frame(), 5); // we should be in stop mode
}

