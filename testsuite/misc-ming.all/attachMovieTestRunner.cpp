/* 
 *   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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

#define INPUT_FILENAME "attachMovieTest.swf"

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
	string filename = INPUT_FILENAME;
	MovieTester tester(filename);

	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity(1);

	sprite_instance* root = tester.getRootMovie();
	assert(root);

	check_equals(root->get_frame_count(), 5);
	check_equals(root->get_play_state(), sprite_instance::PLAY);
	check_equals(root->get_current_frame(), 0);

	check(! tester.findDisplayItemByDepth(*root, 70) );
	check(! tester.findDisplayItemByDepth(*root, 71) );
	check(! tester.findDisplayItemByDepth(*root, 72) );
	check(! tester.findDisplayItemByDepth(*root, 73) );

	tester.advance();

	character* ch70 = const_cast<character*>(tester.findDisplayItemByDepth(*root, 70));
	check( ch70 );
	as_value ch70_x;
	check( ch70->get_member("_x", &ch70_x) );
	check_equals( ch70_x, 0 );
	check(! tester.findDisplayItemByDepth(*root, 71) );
	check(! tester.findDisplayItemByDepth(*root, 72) );
	check(! tester.findDisplayItemByDepth(*root, 73) );

	tester.advance();

	check( tester.findDisplayItemByDepth(*root, 70) );
	character* ch71 = const_cast<character*>(tester.findDisplayItemByDepth(*root, 71));
	check( ch71 );
	as_value ch71_x;
	check( ch71->get_member("_x", &ch71_x) );
	check_equals( ch71_x, 70 );
	check(! tester.findDisplayItemByDepth(*root, 72) );
	check(! tester.findDisplayItemByDepth(*root, 73) );

	tester.advance();

	check( tester.findDisplayItemByDepth(*root, 70) );
	check( tester.findDisplayItemByDepth(*root, 71) );
	character* ch72 = const_cast<character*>(tester.findDisplayItemByDepth(*root, 72));
	check( ch72 );
	as_value ch72_x;
	check( ch72->get_member("_x", &ch72_x) );
	check_equals( ch72_x, 140 );
	check(! tester.findDisplayItemByDepth(*root, 73) );

	tester.advance();

	check( tester.findDisplayItemByDepth(*root, 70) );
	check( tester.findDisplayItemByDepth(*root, 71) );
	check( tester.findDisplayItemByDepth(*root, 72) );
	character* ch73 = const_cast<character*>(tester.findDisplayItemByDepth(*root, 73));
	check( ch73 );
	as_value ch73_x;
	check( ch73->get_member("_x", &ch73_x) );
	check_equals( ch73_x, 210 );
}

