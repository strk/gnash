/* 
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
 *   Free Software Foundation, Inc.
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

#define INPUT_FILENAME "subshapes.swf"

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

TRYMAIN(_runtest);
int
trymain(int /*argc*/, char** /*argv*/)
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
	rgba blue(0, 0, 255, 255);
	rgba gray(204, 204, 204, 255);
	rgba white(255, 255, 255, 255);
	rgba red(255,0,0,255);

	// Coordinates used for pixel checking
	struct PT {
		int x;
	       	int y;
		PT(int nx, int ny): x(nx), y(ny) {}
	};
	PT redBarLL(61,367);  	// lower-left center of red bar rounded cap 
	PT redBarUR(474,38);  	// upper-right center of red bar rounded cap
	PT redBarOvBb(309,171);	// a point on the red bar intersecting the blue box
	PT redBarOvSh(194,262);	// a point on the red bar intersecting the gray shade 
	PT bbIntUL(118,122);	// upper-left corner of blue box interior
	PT bbIntUR(407,122);	// upper-right corner of blue box interior
	PT bbIntLR(404,236);	// lower-right corner of blue box interior
	PT bbIntLL(116,236);	// lower-left corner of blue box interior
	PT bbExtBsh(251,262);	// a point centered below the blue box (Bottom, on the shade)
	PT bbExtT(251,95);	// a point centered above the blue box (Top)
	PT bbExtL(90,185);	// a point centered on the left of the blue box (outside Left)
	PT bbExtRsh(430, 185);	// a point centered on the right of the blue box (outside Right, on the shade)


	// TODO: check why we need this !!
	//       I wouldn't want the first advance to be needed
	//tester.advance();

	const MovieClip* root = tester.getRootMovie();
	assert(root);

	check_equals(root->get_frame_count(), 1);

	// Check lower-left center of red bar rounded cap
	check_pixel(redBarLL.x, redBarLL.y, 2, red, 1);

	// Check upper-right center of red bar rounded cap
	check_pixel(redBarUR.x, redBarUR.y, 2, red, 1);

	// Check that the bar is *below* the blue box
	check_pixel(redBarOvBb.x, redBarOvBb.y, 2, blue, 1);

	// Check that the bar is *above* the gray shade
	check_pixel(redBarOvSh.x, redBarOvSh.y, 2, red, 1);

	// Check the blu box corners
	check_pixel(bbIntUL.x, bbIntUL.y, 2, blue, 1);
	check_pixel(bbIntUR.x, bbIntUR.y, 2, blue, 1);
	check_pixel(bbIntLR.x, bbIntLR.y, 2, blue, 1);
	check_pixel(bbIntLL.x, bbIntLL.y, 2, blue, 1);

	// Check the points outside the blu box
	check_pixel(bbExtBsh.x, bbExtBsh.y, 2, gray, 1);
	check_pixel(bbExtRsh.x, bbExtRsh.y, 2, gray, 1);
	check_pixel(bbExtT.x, bbExtT.y, 2, white, 1);
	check_pixel(bbExtL.x, bbExtL.y, 2, white, 1);

	return 0;
}

