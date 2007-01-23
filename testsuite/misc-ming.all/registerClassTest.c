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
 * Test Object.registerClass().
 *
 * 1) Exports a 'redsquare' symbol
 * 2) attach it to main timeline 
 * 3) register a custom class to it
 * 4) attach it again (expected to have the custom class interface)
 *
 * run as ./registerClass
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "registerClassTest.swf"

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
	SWFMovieClip  dejagnuclip;


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
	SWFMovie_nextFrame(mo);  /* end of frame1 */

	add_actions(mo, "counter = 1;");

	add_actions(mo,
		"var name1 = 'square'+counter;"
		"attachMovie('redsquare', name1, 70+counter);"
		"var clip1 = this[name1];"
		"counter++;"
		);

	SWFMovie_nextFrame(mo); /* end of frame2 */

	add_actions(mo,
		"function CustomClass() { "
		"}"
		"CustomClass.prototype.moveRight = function(pix) {"
		"	this._x += pix;"
		"};"
		"CustomClass.prototype.sayHello = function() {"
		"	note('Hello from CustomClass instance');"
		"};"
		"MovieClip.prototype.sayHello = function() {"
		"	note('Hello from MovieClip instance');"
		"};"
		"Object.registerClass('redsquare', CustomClass);"
		);

	add_actions(mo,
		"var name2 = 'square'+counter;"
		"attachMovie('redsquare', name2, 70+counter);"
		"var clip2 = this[name2];"
		"clip2.moveRight(80);" // TODO:  test this!
		"counter++;"
		);


	check_equals(mo, "typeof(clip1)", "'movieclip'");
	check(mo, "clip1 instanceOf MovieClip");
	check(mo, "! clip1 instanceOf CustomClass");

	check_equals(mo, "typeof(clip2)", "'movieclip'");
	/**/ check(mo, "clip2 instanceOf CustomClass");
	/**/ check(mo, "! clip2 instanceOf MovieClip");

	add_actions(mo,
		"totals();"
		"stop();"
		);

	SWFMovie_nextFrame(mo); /* end of frame3 */


	/*****************************************************
	 *
	 * Output movie
	 *
	 *****************************************************/

	puts("Saving " OUTPUT_FILENAME );

	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
