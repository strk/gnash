/* 
 *   Copyright (C) 2012 Free Software Foundation, Inc.
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
 */ 

#define INPUT_FILENAME "test_15bpp_bitmap.swf"

#include "MovieTester.h"
#include "GnashException.h"
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
	string filename = string(SRCDIR) + string("/") + string(INPUT_FILENAME);
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

	// Colors used for pixel checking
	rgba gray(135, 135, 135, 255); /* bottom-left of the center */
	rgba white(255, 255, 255, 255);
	rgba black(0, 0, 0, 255);
	rgba dark_cyan(0, 135, 135, 255);
	rgba cyan(0, 255, 255, 255);
	rgba dark_magenta(135, 0, 135, 255);
	rgba magenta(255, 0, 255, 255);
	rgba dark_yellow(135, 135, 0, 255);
	rgba yellow(255, 255, 0, 255);
	rgba middle_gray(127, 127, 127, 255); /* top-right of the center */
	rgba dark_gray(71, 71, 71, 255); /* top-left & bottom-right */
	rgba red(255, 0, 0, 255); 
	rgba green(0, 255, 0, 255); 
	rgba blue(0, 0, 255, 255); 
	rgba dark_blue(0, 0, 135, 255); 
	rgba dark_green(0, 135, 0, 255); 
	rgba dark_red(135, 0, 0, 255); 

	// Coordinates used for pixel checking
	struct PT {
		int x;
	       	int y;
		PT(int nx, int ny): x(nx), y(ny) {}
	};

	// from center to out

	PT ul1(     204,      295);     PT ur1(     307, ul1.y);  	
	PT ul2(ul1.x-32, ul1.y-32);     PT ur2(ur1.x+32, ul2.y);
	PT ul3(ul2.x-32, ul2.y-32);     PT ur3(ur2.x+32, ul3.y);
	PT ul4(ul3.x-32, ul3.y-32);     PT ur4(ur3.x+32, ul4.y);
	PT ul5(ul4.x-32, ul4.y-32);     PT ur5(ur4.x+32, ul5.y);
	PT ul6(ul5.x-32, ul5.y-32);     PT ur6(ur5.x+32, ul6.y);
	PT ul7(ul6.x-32, ul6.y-32);     PT ur7(ur6.x+32, ul7.y);

	PT ll1(ul1.x, 314);             PT lr1(ur1.x, ll1.y);  	
	PT ll2(ul2.x, ll1.y+32);        PT lr2(ur2.x, ll2.y);
	PT ll3(ul3.x, ll2.y+32);        PT lr3(ur3.x, ll3.y);
	PT ll4(ul4.x, ll3.y+32);        PT lr4(ur4.x, ll4.y);
	PT ll5(ul5.x, ll4.y+32);        PT lr5(ur5.x, ll5.y);
	PT ll6(ul6.x, ll5.y+32);        PT lr6(ur6.x, ll6.y);
	PT ll7(ul7.x, ll6.y+32);        PT lr7(ur7.x, ll7.y);


	const MovieClip* root = tester.getRootMovie();
	assert(root);

	check_equals(root->get_frame_count(), 1);

	check_pixel(ul1.x, ul1.y, 8, dark_gray, 1);
	check_pixel(ul2.x, ul2.y, 8, dark_yellow, 1);
	check_pixel(ul3.x, ul3.y, 8, dark_magenta, 1);
	check_pixel(ul4.x, ul4.y, 8, dark_cyan, 1);
	check_pixel(ul5.x, ul5.y, 8, black, 1);
	check_pixel(ul6.x, ul6.y, 8, gray, 1);
	check_pixel(ul6.x, ul7.y, 8, white, 1);

	check_pixel(ur1.x, ur1.y, 8, middle_gray, 1);
	check_pixel(ur2.x, ur2.y, 8, yellow, 1);
	check_pixel(ur3.x, ur3.y, 8, magenta, 1);
	check_pixel(ur4.x, ur4.y, 8, cyan, 1);
	check_pixel(ur5.x, ur5.y, 8, black, 1);
	check_pixel(ur6.x, ur6.y, 8, white, 1);
	check_pixel(ur6.x, ur7.y, 8, white, 1);

	check_pixel(ll1.x, ll1.y, 8, gray, 1);
	check_pixel(ll2.x, ll2.y, 8, blue, 1);
	check_pixel(ll3.x, ll3.y, 8, green, 1);
	check_pixel(ll4.x, ll4.y, 8, red, 1);
	check_pixel(ll5.x, ll5.y, 8, white, 1);
	check_pixel(ll6.x, ll6.y, 8, black, 1);
	check_pixel(ll6.x, ll7.y, 8, white, 1);

	check_pixel(lr1.x, lr1.y, 8, dark_gray, 1);
	check_pixel(lr2.x, lr2.y, 8, dark_blue, 1);
	check_pixel(lr3.x, lr3.y, 8, dark_green, 1);
	check_pixel(lr4.x, lr4.y, 8, dark_red, 1);
	check_pixel(lr5.x, lr5.y, 8, gray, 1);
	check_pixel(lr6.x, lr6.y, 8, black, 1);
	check_pixel(lr6.x, lr7.y, 8, white, 1);


	return 0;
}

