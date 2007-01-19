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

/*
 * Test MovieClip.attachMovie().
 *
 * Exports a 'redsquare' symbol and then attach it to main timeline 4 times
 * at depths 70+[0..3] and with xoffset 70*[0..3]
 *
 * run as ./attachMovieTest
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "attachMovieTest.swf"

void
addRedSquareExport(SWFMovie mo)
{
	SWFShape sh;
	SWFMovieClip mc;

	sh = make_fill_square (0, 300, 60, 60, 255, 0, 0, 255, 0, 0);
	mc = newSWFMovieClip();

	SWFMovieClip_add(mc, (SWFBlock)sh);
	SWFMovieClip_nextFrame(mc);

	SWFMovie_addExport(mo, (SWFBlock)mc, "redsquare");

	SWFMovie_writeExports(mo);
}

int
main(int argc, char** argv)
{
	SWFMovie mo;
	SWFMovieClip exportedClip;
	const char *srcdir=".";
	SWFFont bfont; 


	/*********************************************
	 *
	 * Initialization
	 *
	 *********************************************/

	if ( argc>1 ) srcdir=argv[1];
	else
	{
		fprintf(stderr, "Usage: %s\n", argv[0]);
		return 1;
	}

	puts("Setting things up");

	Ming_init();
        Ming_useSWFVersion (OUTPUT_VERSION);
	Ming_setScale(20.0); /* let's talk pixels */
 
	mo = newSWFMovie();
	SWFMovie_setRate(mo, 12);
	//SWFMovie_setDimension(mo, 6400, 4000);
	SWFMovie_setDimension(mo, 640, 400);

	/*********************************************
	 *
	 * Body
	 *
	 *********************************************/

	addRedSquareExport(mo);
	/* it seems we need a SHOWFRAME for this to be effective */
	/* (maybe it's related to loop-back handling ?) */
	SWFMovie_nextFrame(mo); 

	/***********************************************************************
	 * NOTE:
	 *
	 * The following snippet was initially implemented in a *single* frame
	 * (the second one, with first only contained the EXPORTASSET tag).
	 * The code went something like this:
	 *
	 * 	if ( counter < 4 )
	 * 	{
	 * 		attachMovie('redsquare', 'square'+counter, 70+counter);
	 * 		this['square'+counter]._x = 70*counter;
	 * 		counter++;
	 * 	}
	 * 	else
	 * 	{
	 * 		stop();
	 * 	}
	 *
	 * The problem with the above was that Gnash failed due to loop-back
	 * problems.
	 * The "expected" behaviour with the above code is exactly the same
	 * with the current one (ie: number of squares increment up to 4) but
	 * Gnash fails in that it resets the DisplayList to the one generated
	 * after first execution of first frame tags (an empty DisplayList)
	 * at each restart.
	 *
	 * Since we have separate testcases for loop-backs I've preferred
	 * to keep this one focused on MovieClip.attachMovie.
	 *
	 * 	--strk 2007-01-19
	 *
	 ***********************************************************************/

	add_actions(mo,
		"attachMovie('redsquare', 'square'+counter, 70+counter);"
		"this['square'+counter]._x = 70*counter;"
		"counter++;"
		);

	SWFMovie_nextFrame(mo); /* showFrame */

	add_actions(mo,
		"attachMovie('redsquare', 'square'+counter, 70+counter);"
		"this['square'+counter]._x = 70*counter;"
		"counter++;"
		);

	SWFMovie_nextFrame(mo); /* showFrame */

	add_actions(mo,
		"attachMovie('redsquare', 'square'+counter, 70+counter);"
		"this['square'+counter]._x = 70*counter;"
		"counter++;"
		);

	SWFMovie_nextFrame(mo); /* showFrame */

	add_actions(mo,
		"attachMovie('redsquare', 'square'+counter, 70+counter);"
		"this['square'+counter]._x = 70*counter;"
		"counter++;"
		);

	add_actions(mo, "stop();");

	SWFMovie_nextFrame(mo); /* showFrame */


	/*****************************************************
	 *
	 * Output movie
	 *
	 *****************************************************/

	puts("Saving " OUTPUT_FILENAME );

	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
