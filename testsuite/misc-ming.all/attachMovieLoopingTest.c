/* 
 *   Copyright (C) 2005, 2006, 2009, 2010, 2011 Free Software Foundation, Inc.
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
 * The attachMovie() calls all happen in the same frame, the second one.
 * It is expected that all of them will persist (not be cleared by loopback).
 *
 * run as ./attachMovieLoopingTest
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 7
#define OUTPUT_FILENAME "attachMovieLoopingTest.swf"

void addRedSquareExport(SWFMovie mo);
void
addRedSquareExport(SWFMovie mo)
{
	SWFShape sh;
	SWFMovieClip mc;

	sh = make_fill_square (0, 0, 60, 60, 255, 0, 0, 255, 0, 0);
	mc = newSWFMovieClip();

	SWFMovieClip_add(mc, (SWFBlock)sh);
	/* This is here just to turn the clip into an active one */
	add_clip_actions(mc, "onRollOver = function() {};");
	SWFMovieClip_nextFrame(mc);

	SWFMovie_addExport(mo, (SWFBlock)mc, "redsquare");

	SWFMovie_writeExports(mo);
}

int
main(int argc, char** argv)
{
	SWFMovie mo;
	const char *srcdir=".";
	SWFMovieClip dejagnuclip;


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

	dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 80, 800, 600);
	SWFMovie_add(mo, (SWFBlock)dejagnuclip);

	addRedSquareExport(mo);
	/* it seems we need a SHOWFRAME for this to be effective */
	/* (maybe it's related to loop-back handling ?) */
	SWFMovie_nextFrame(mo); 

    // This should run for four frames. The counter should only be reset
    // on the first frame, i.e. when start is undefined. This should
    // work for all swf versions, unlike "if (undefined < 4);"

	add_actions(mo, "initObj = new Object();");
	add_actions(mo, "if (!started) { counter = 0; started = true; }");
	add_actions(mo, "redsquare = function() { "
            "           trace('hello redsquare'); "
            "           if (counter > 0) {"
            "               check_equals(this._x, counter * 70);"
            "               check_equals(Math.round(this._xscale), 99);"
            "               check_equals(Math.round(this._yscale), "
            "                       Math.round((10 * counter +5) / 60 * 100));"
            "		        check_equals(this._height, 10 * counter + 5); "
            "		        check_equals(this.aProperty, 6); "
            "           } else {"
            "               check_equals(this._x, 0);"
            "               check_equals(this._xscale, 100);"
            "               check_equals(this._height, 60.1);"
            "		        check_equals(this.aProperty, undefined); "
            "           };"
            "       };"
            "redsquare.prototype = new MovieClip();"
            "Object.registerClass('redsquare', redsquare);"
            );

	add_actions(mo,
		"if ( counter < 4 ) {"
		"	if ( counter > 0 ) { "
		"		initObj.aProperty = 6;"
		"		initObj._xscale = 99;"
		"		initObj._x = 70*counter;"
		"		initObj._height = 10*counter + 5; "
		"		attachMovie('redsquare', "
		"			'square'+counter, 70+counter, initObj);"
		"	} else {"
		/* We don't use an initObject for the first attachMovie call
		 * to verify that the DisplayObject will be kept in DisplayList
		 * at loopback time anyway
		 */
		"		attachMovie('redsquare', "
		"			'square'+counter, 70+counter);"
		"	}"
		" 	check_equals(this['square'+counter]._x, 70*counter);"
        "   if (counter > 0) {"
		" 	    check_equals(this['square'+counter]._height, 10 * counter + 5);"
		"   };"
        "	note('Depth is '+70*counter);"
		"	counter++;"
		"	note('Next counter is '+counter);"
		"} else {"
		"	totals(26); stop();"
		"}"
		);

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
