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

#define INPUT_FILENAME "background.swf"

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
	string filename = string(TGTDIR) + string("/") + string(INPUT_FILENAME);
	unique_ptr<MovieTester> t;

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
	rgba red(255,0,0,255);

	MovieClip* root = tester.getRootMovie();
	assert(root);

	check_equals(root->get_frame_count(), 2);
	check_equals(root->get_current_frame(), 0);

	// error tolerance
	// WARNING ! AGG_RGB555 reports red as 240,0,0 !!!!
	// it's really annoying ... (UdoG: help ?)
	int tol = 15;

	check_pixel(60, 60, 50, red, tol);
	check_pixel(60, 400, 50, red, tol);
	check_pixel(600, 400, 50, red, tol);
	check_pixel(600, 60, 50, red, tol);
	check_pixel(320, 240, 50, red, tol);

	tester.advance();

	check_equals(root->get_current_frame(), 1);

	check_pixel(60, 60, 50, red, tol);
	check_pixel(60, 400, 50, red, tol);
	check_pixel(600, 400, 50, red, tol);
	check_pixel(600, 60, 50, red, tol);
	check_pixel(320, 240, 50, red, tol);
	return 0;
}

