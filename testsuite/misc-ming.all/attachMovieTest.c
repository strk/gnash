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
 *
 * run as ./attachMovieTest
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "attachMovieTest.swf"

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
	add_clip_actions(mc, "onMouseDown = function() { _root.mouseDown++; _root.note(_name+' mouseDown '+_root.mouseDown); };");
	add_clip_actions(mc, "onMouseUp = function() { _root.mouseUp++; _root.note(_name+' mouseUp '+_root.mouseUp); };");
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

    SWFShape sh;
    SWFButton but;
#if MING_VERSION_CODE >= 00040400
    SWFButtonRecord br;
#endif


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

#if MING_VERSION_CODE >= 00040400
    but = newSWFButton();

    sh = make_fill_square (100, 300, 60, 60, 255, 0, 0, 0, 255, 0);
    br = SWFButton_addCharacter(but, (SWFCharacter)sh, SWFBUTTON_UP);
    SWFButtonRecord_setDepth(br, 12);
    
    sh = make_fill_square (100, 300, 60, 60, 255, 0, 0, 255, 0, 0);
    br = SWFButton_addCharacter(but, (SWFCharacter)sh, SWFBUTTON_HIT);
    br = SWFButton_addCharacter(but, (SWFCharacter)sh, SWFBUTTON_OVER);
    SWFButtonRecord_setDepth(br, 10);

    SWFMovie_addExport(mo, (SWFBlock)but, "butexp");
    SWFMovie_writeExports(mo);
#endif
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

	add_actions(mo, "initObj = new Object();");

	add_actions(mo, "counter=0;");

	add_actions(mo,
		"initObj._x = 70*counter;"
		"attachMovie('redsquare', 'square'+counter, 70+counter, initObj);"
		"counter++;"
		);

	check_equals(mo, "square0._x", "0");

	SWFMovie_nextFrame(mo); /* showFrame */

	add_actions(mo,
		"initObj._x = 70*counter;"
		"attachMovie('redsquare', 'square'+counter, 70+counter, initObj);"
		"counter++;"
		);

	check_equals(mo, "square1._x", "70");

	SWFMovie_nextFrame(mo); /* showFrame */

	add_actions(mo,
		"initObj._x = 70*counter;"
		"attachMovie('redsquare', 'square'+counter, 70+counter, initObj);"
		"counter++;"
		);

	check_equals(mo, "square2._x", "140");

	SWFMovie_nextFrame(mo); /* showFrame */

	add_actions(mo,
		"initObj._x = 70*counter;"
		"attachMovie('redsquare', 'square'+counter, 70+counter, initObj);"
		"counter++;"
		);

	check_equals(mo, "square3._x", "210");

	
    SWFMovie_nextFrame(mo); /* showFrame */

#if MING_VERSION_CODE >= 00040400

    add_actions(mo,
            "o = new Object();"
            "o.f = 56;"
            "ar = attachMovie('butexp', 'butatt', 5000, o);"
            "trace(ar);"
            "trace(butatt);"
            "ar = attachMovie('redsquare', 'rs', 5001, o);"
            "ar.t = 34;"
            "butatt.s = 37;"
            );

    /* init object is not used for Buttons */
    check_equals(mo, "butatt.f", "undefined");
    check_equals(mo, "butatt.t", "undefined");
    check_equals(mo, "butatt.s", "37");
#endif
	add_actions(mo, "totals(); stop();");

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
