/* 
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#define INPUT_FILENAME "DefineTextTest.swf"

#include "MovieTester.h"
#include "MovieClip.h"
#include "character.h"
#include "DisplayList.h"
#include "log.h"
#include "VM.h"
#include "string_table.h"

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

	check_equals(root->get_frame_count(), 3);
	check_equals(root->get_play_state(), MovieClip::PLAY);
	check_equals(root->get_current_frame(), 0);

	rgba white(255,255,255,255);
	rgba red(255,0,0,255);
	rgba green(0,255,0,255);

	geometry::Point2d cXmm(220,327); // character X center
	geometry::Point2d cXum(220,440); // character X underline/middle

	geometry::Point2d cOmr(135,326); // character O middle/right
	geometry::Point2d cOml(21,330); // character O middle/left
	geometry::Point2d cOmm(77,327); // character O center
	geometry::Point2d cOum(78,440); // character O underline/middle


	tester.advance(); // first frame only contains dejagnu
	check_equals(root->get_current_frame(), 1);

	check_pixel(cXmm.x, cXmm.y, 4, red, 2); // X cross
	check_pixel(cXum.x, cXum.y, 20, white, 2); // X underline (none)

	check_pixel(cOmm.x, cOmm.y, 8, white, 2); // O hole
	check_pixel(cOml.x, cOml.y, 4, green, 2); // O left side
	check_pixel(cOmr.x, cOmr.y, 4, green, 2); // O right side
	check_pixel(cOum.x, cOum.y, 20, white, 2); // O underline (none)

	for (int i=0; i<2; ++i) tester.advance(); // get to the end

	string_table& st = VM::get().getStringTable();
	as_value eot;
	bool endOfTestFound = root->get_member(st.find("endoftest"), &eot);
	check(endOfTestFound);
	check(eot.is_bool());
	check(eot.to_bool());

	// TODO: use check_pixel for checking bacground colors
}

