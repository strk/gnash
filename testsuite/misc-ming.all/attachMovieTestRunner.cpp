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

#define INPUT_FILENAME "attachMovieTest.swf"

#include "MovieTester.h"
#include "MovieClip.h"
#include "DisplayObject.h"
#include "DisplayList.h"
#include "log.h"
#include "VM.h"

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

	as_value tmp;

	check_equals(root->get_frame_count(), 6);
	check_equals(root->getPlayState(), MovieClip::PLAYSTATE_PLAY);
	check_equals(root->get_current_frame(), 0);

	check(! tester.findDisplayItemByDepth(*root, 70) );
	check(! tester.findDisplayItemByDepth(*root, 71) );
	check(! tester.findDisplayItemByDepth(*root, 72) );
	check(! tester.findDisplayItemByDepth(*root, 73) );

	tester.movePointerTo(30, 30);
	check(!tester.isMouseOverMouseEntity());
	// check that the pixel under the mouse is white
	check_pixel(30, 30, 2, rgba(255,255,255,255), 2);

	tester.advance();

	check(tester.findDisplayItemByDepth(*root, 70) );
	check(! tester.findDisplayItemByDepth(*root, 71) );
	check(! tester.findDisplayItemByDepth(*root, 72) );
	check(! tester.findDisplayItemByDepth(*root, 73) );

	tester.movePointerTo(30, 30);
	check(tester.isMouseOverMouseEntity());
	// check that the pixel under the mouse is red
	check_pixel(30, 30, 2, rgba(255,0,0,255), 2);

	tester.movePointerTo(100, 30);
	check(!tester.isMouseOverMouseEntity());
	// check that the pixel under the mouse is white
	check_pixel(100, 30, 2, rgba(255,255,255,255), 2);

	VM& vm = tester.vm();
	const ObjectURI& mousedown = getURI(vm, "mousedown");
	const ObjectURI& mouseup = getURI(vm, "mouseup");

	getObject(root)->get_member(mousedown, &tmp);
	check(tmp.is_undefined());
	getObject(root)->get_member(mouseup, &tmp);
	check(tmp.is_undefined());

	// Note that we are *not* on an active entity !
	tester.pressMouseButton();

	getObject(root)->get_member(mousedown, &tmp);
	check_equals(toNumber(tmp, vm), 1);
	check ( ! getObject(root)->get_member(mouseup, &tmp) );

	tester.depressMouseButton();

	getObject(root)->get_member(mousedown, &tmp);
	check_equals(toNumber(tmp, vm), 1);
	getObject(root)->get_member(mouseup, &tmp);
	check_equals(toNumber(tmp, vm), 1);

	tester.advance();

	check( tester.findDisplayItemByDepth(*root, 70) );
	check( tester.findDisplayItemByDepth(*root, 71) );
	check(! tester.findDisplayItemByDepth(*root, 72) );
	check(! tester.findDisplayItemByDepth(*root, 73) );

	tester.movePointerTo(100, 30);
	check(tester.isMouseOverMouseEntity());
	// check that the pixel under the mouse is red
	check_pixel(100, 30, 2, rgba(255,0,0,255), 2);

	tester.movePointerTo(170, 30);
	check(!tester.isMouseOverMouseEntity());
	// check that the pixel under the mouse is white
	check_pixel(170, 30, 2, rgba(255,255,255,255), 2);

	tester.advance();

	check( tester.findDisplayItemByDepth(*root, 70) );
	check( tester.findDisplayItemByDepth(*root, 71) );
	check( tester.findDisplayItemByDepth(*root, 72) );
	check(! tester.findDisplayItemByDepth(*root, 73) );

	tester.movePointerTo(170, 30);
	check(tester.isMouseOverMouseEntity());
	// check that the pixel under the mouse is red
	check_pixel(170, 30, 2, rgba(255,0,0,255), 2);

	tester.movePointerTo(240, 30);
	check(!tester.isMouseOverMouseEntity());
	// check that the pixel under the mouse is white
	check_pixel(240, 30, 2, rgba(255,255,255,255), 2);

	tester.advance();

	check( tester.findDisplayItemByDepth(*root, 70) );
	check( tester.findDisplayItemByDepth(*root, 71) );
	check( tester.findDisplayItemByDepth(*root, 72) );
	check( tester.findDisplayItemByDepth(*root, 73) );

	tester.movePointerTo(240, 30);
	check(tester.isMouseOverMouseEntity());
	// check that the pixel under the mouse is red
	check_pixel(240, 30, 2, rgba(255,0,0,255), 2);

	tester.movePointerTo(340, 30);
	check(! tester.isMouseOverMouseEntity());
	// check that the pixel under the mouse is white
	check_pixel(340, 30, 2, rgba(255,255,255,255), 2);

	// Note that we are *not* on an active entity !
	tester.pressMouseButton();

	getObject(root)->get_member(mousedown, &tmp);
	check_equals(toNumber(tmp, vm), 5);

}

