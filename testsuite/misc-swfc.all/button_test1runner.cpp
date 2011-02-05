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

#define INPUT_FILENAME "button_test1.swf"

#include "MovieTester.h"
#include "GnashException.h"
#include "MovieClip.h"
#include "DisplayObject.h"
#include "DisplayList.h"
#include "log.h"
#include "Point2d.h"
#include "VM.h"
#include "as_value.h"

#include "check.h"
#include <string>
#include <cassert>

using namespace gnash;
using namespace std;

int
main(int /*argc*/, char** /*argv*/)
{
	string filename = string(BUILDDIR) + string("/") + string(INPUT_FILENAME);
	auto_ptr<MovieTester> t;

	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity(1);

	try
	{
		t.reset(new MovieTester(filename));
	}
	catch (const GnashException& e)
	{
		std::cerr << "Error initializing MovieTester: " << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
	
	MovieTester& tester = *t;

	// TODO: check why we need this !!
	//       I wouldn't want the first advance to be needed
	tester.advance();

	MovieClip* root = tester.getRootMovie();
	VM& vm = getVM(*getObject(root));

	check_equals(root->get_frame_count(), 1);

	rgba white(255, 255, 255, 255);
	rgba yellow(255, 255, 0, 255);
	rgba green(0,255,0,255);
	rgba red(255,0,0,255);

	// A point on the visible green button
	point visibleGreen(100, 260);

	// A point on the BIG green button, but not on the small one
	point bigGreen(100, 352);

	// A point on the yellow box, overlapping the green button
	point yellowOnGreen(127, 256);

	// A point on the yellow box, not overlapping the green button
	point yellowOffGreen(136, 231);

	// A point on the BIG yellow box, not overlapping the green button,
	// would be on the small red box...
	point bigYellowOffGreen(294,365);

	// A point on the red box, but out of the red button hit area
	point redNoHit(241, 351);

	// A point on the BIG red button, but not on the small one
	point bigRedNoHit(330, 397);

	//----------------------------------------
	// Test starts
	//----------------------------------------

	// check some pixel colors
	check_pixel(visibleGreen.x, visibleGreen.y, 2, green, 1);
	check_pixel(redNoHit.x, redNoHit.y, 2, red, 1);
	check_pixel(yellowOffGreen.x, yellowOffGreen.y, 2, red, 1); // still red, till we move over it
	check_pixel(yellowOnGreen.x, yellowOnGreen.y, 2, red, 1); // still red, till we move over it
	check_pixel(bigGreen.x, bigGreen.y, 2, white, 1); // nothing here, as long as the green is small

	// now move over the red button hit area, so it becomes yellow
	tester.movePointerTo(yellowOnGreen.x, yellowOnGreen.y);
	check_pixel(yellowOffGreen.x, yellowOffGreen.y, 2, yellow, 1); 
	check_pixel(yellowOnGreen.x, yellowOnGreen.y, 2, yellow, 1); 
	tester.movePointerTo(yellowOffGreen.x, yellowOffGreen.y);
	check_pixel(yellowOffGreen.x, yellowOffGreen.y, 2, yellow, 1); 
	check_pixel(yellowOnGreen.x, yellowOnGreen.y, 2, yellow, 1); 

	// 1. Click on the visible part of the green box.
	tester.movePointerTo(visibleGreen.x, visibleGreen.y);
	tester.click();
	tester.advance();

	// check new pixel colors
	check_pixel(visibleGreen.x, visibleGreen.y, 2, green, 1);
	check_pixel(bigGreen.x, bigGreen.y, 2, green, 1); // now green is big
	check_pixel(redNoHit.x, redNoHit.y, 2, red, 1);
	check_pixel(yellowOffGreen.x, yellowOffGreen.y, 2, red, 1); // still red, till we move over it
	check_pixel(yellowOnGreen.x, yellowOnGreen.y, 2, red, 1); // still red, till we move over it

	// 2. Now move your mouse on the top-left area of the red box (the box will become yellow),
	//    and click where it overlaps with the green one.
	tester.movePointerTo(yellowOnGreen.x, yellowOnGreen.y);

	check_pixel(visibleGreen.x, visibleGreen.y, 2, green, 1);
	check_pixel(bigGreen.x, bigGreen.y, 2, green, 1); // now green is big
	check_pixel(redNoHit.x, redNoHit.y, 2, green, 1); // the red button shrinked, showing green underneath
	check_pixel(bigRedNoHit.x, bigRedNoHit.y, 2, green, 1); // the red button is still small, but green is big
	check_pixel(yellowOffGreen.x, yellowOffGreen.y, 2, yellow, 1); // still red, till we move over it
	check_pixel(yellowOnGreen.x, yellowOnGreen.y, 2, yellow, 1); // still red, till we move over it

	tester.click(); 
	tester.advance();

	check_pixel(visibleGreen.x, visibleGreen.y, 2, green, 1);
	check_pixel(bigGreen.x, bigGreen.y, 2, white, 1); // green is small again
	check_pixel(bigRedNoHit.x, bigRedNoHit.y, 2, white, 1); // the red button grew, but pointer is in the hit area...
	check_pixel(bigYellowOffGreen.x, bigYellowOffGreen.y, 2, yellow, 1); // yellow grew
	check_pixel(yellowOffGreen.x, yellowOffGreen.y, 2, yellow, 1); // still red, till we move over it
	check_pixel(yellowOnGreen.x, yellowOnGreen.y, 2, yellow, 1); // still red, till we move over it

	tester.movePointerTo(bigRedNoHit.x, bigRedNoHit.y);
	check_pixel(bigRedNoHit.x, bigRedNoHit.y, 2, red, 1); // the red button grew, but pointer is in the hit area...
	check_pixel(bigYellowOffGreen.x, bigYellowOffGreen.y, 2, red, 1); // yellow grew, but we're not in the hit area

	//----------------------------------------
	// Check test ended completely
	//----------------------------------------

	as_value eot;
        
    getObject(root)->get_member(getURI(vm, "testcompleted"), &eot);
        
	//cerr << "EOT is " << eot.to_debug_string() << endl;
	check(eot.to_bool(8));
}

