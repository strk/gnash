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

#define INPUT_FILENAME "loop_test.swf"

#include "MovieTester.h"
#include "MovieClip.h"
#include "DisplayObject.h"
#include "DisplayList.h"
#include "log.h"
#include "GnashException.h"

#include "check.h"

#include <string>
#include <iostream>
#include <cassert>
#include <memory>

using namespace gnash;
using namespace std;

int
main(int /*argc*/, char** /*argv*/)
{
	string filename = string(TGTDIR) + string("/") + string(INPUT_FILENAME);
	auto_ptr<MovieTester> t;

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
	//tester.advance();

	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity(1);
	//dbglogfile.setActionDump(1);

	MovieClip* root = tester.getRootMovie();
	assert(root);

	size_t framecount = root->get_frame_count();
	check_equals(framecount, 3);
	check_equals(root->get_current_frame(), 0);

	rgba black(0,0,0,255);
	rgba red(255,0,0,255);
	rgba white(255,255,255,255);

	// Advance till the movie is stopped (or 10 loops are performed)
	bool blackOverRed=false;
	for (size_t i=0; i<=framecount*10; ++i)
	{
		check_equals(root->get_current_frame(), i%framecount);

		// Out of any DisplayObject
		check_pixel(317, 430, 2, white, 2);

		// Fully on the red square: 317,330
		check_pixel(317, 330, 2, red, 2);

		// Fully on the black square: 400,330
		check_pixel(400, 330, 2, black, 2);

		// Intersection of the two squares: 343,330
		// This is black or red depending on loop count
		if ( blackOverRed )
		{
			check_pixel(343, 330, 2, black, 2);
		}
		else
		{
			check_pixel(343, 330, 2, red, 2);
		}

		tester.advance();

		// Let's break if we stopped, as we'll print totals() thus
		// enlarging invalidated bounds too ...
		if (root->getPlayState() == MovieClip::PLAYSTATE_STOP) break;

		// Compute 1-based currentFrame
		size_t currentFrame = root->get_current_frame()+1;

		if ( currentFrame == 3 ) // We swap depths here !
		{
			blackOverRed = !blackOverRed;
			
			// Check the intersection of the two DisplayObjects to
			// be invalidated
			check( tester.getInvalidatedRanges().contains(343, 330) );
		}
		else if ( currentFrame == 1 ) // We restarted here !
		{
			assert(i > 1); // to ensure we looped back

			// Not sure if invalidated ranges should be null here.. 
			// visually, it seems so, but loop-back is too complex
			// to be sure (ie: xtrace window text might be reset or something)
			// I checked that it's not resetDisplayList invalidating it...
			check( tester.getInvalidatedRanges().isNull() );
		}
		else // we did nothing here...
		{
			check( tester.getInvalidatedRanges().isNull() );
		}

	}

}

