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
 * Test Object.registerClass().
 *
 * 1) Exports a 'redsquare' symbol
 * 2) attach it to main timeline 
 * 3) register a custom class to it
 * 4) attach it again (expected to have the custom class interface)
 * 5) register another custom class to it, this time deriving from MovieClip
 * 6) attach it again (expected to be both instance of custom class and instance of MovieClip)
 * 7) call registerClass again, this time with an *single* argument
 * 8) attach it again (expected to be instance of of MovieClip)
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

	add_actions(mo, "counter = 1; onLoadCalled = new Array();");

	add_actions(mo,
		"var name1 = 'square'+counter;"
		"attachMovie('redsquare', name1, 70+counter);"
		"var clip1 = this[name1];"
		"counter++;"
		);

	SWFMovie_nextFrame(mo); /* end of frame2 */

	add_actions(mo,
		"function CustomClass() { this._x = 80; }"
		"CustomClass.prototype.onLoad = function() { note(this+'.onLoad called'); _root.onLoadCalled.push(this); };"
		"registerClassRet = Object.registerClass('redsquare', CustomClass);"
		);

	check_equals(mo, "typeof(registerClassRet)", "'boolean'");
	check_equals(mo, "registerClassRet", "true");

	add_actions(mo,
		"var name2 = 'square'+counter;"
		"attachMovie('redsquare', name2, 70+counter);"
		"var clip2 = this[name2];"
		"counter++;"
		);


	SWFMovie_nextFrame(mo); /* end of frame3 */

	add_actions(mo,
		"function CustomClass2() { this._x = 160; check_equals(typeof(super.lineTo), 'function'); }"
		"CustomClass2.prototype = new MovieClip;"
		"registerClassRet = Object.registerClass('redsquare', CustomClass2);"
		);

	check_equals(mo, "_root.onLoadCalled.length", "1");
	check_equals(mo, "_root.onLoadCalled[0]", "_level0.square2");
	check_equals(mo, "typeof(registerClassRet)", "'boolean'");
	check_equals(mo, "registerClassRet", "true");

	add_actions(mo,
		"var name3 = 'square'+counter;"
		"attachMovie('redsquare', name3, 70+counter);"
		"var clip3 = this[name3];"
		"counter++;"
		);

	SWFMovie_nextFrame(mo); /* end of frame4 */

	add_actions(mo,
		"registerClassRet = Object.registerClass('redsquare');"
		);

	check_equals(mo, "typeof(registerClassRet)", "'boolean'");
	check_equals(mo, "registerClassRet", "false");

	add_actions(mo,
		"var name4 = 'square'+counter;"
		"attachMovie('redsquare', name4, 70+counter);"
		"var clip4 = this[name4];"
		"clip4._x = 240;"
		"counter++;"
		);

	SWFMovie_nextFrame(mo); /* end of frame5 */

	check_equals(mo, "typeof(clip1)", "'movieclip'");
	check(mo, "clip1 instanceOf MovieClip");
	check_equals(mo, "clip1._x", "0");
	check(mo, "! clip1 instanceOf CustomClass");
	
	// Check that non-enumerable properties (unnamed instances,
	// constructor, __constructor__) are not enumerated.
	add_actions(mo, "var s = ''; for (i in clip1) { s += i + ','; };");
	check_equals(mo, "s", "'onRollOver,'");

	check_equals(mo, "typeof(clip2)", "'movieclip'");
	check(mo, "clip2 instanceOf CustomClass");
	check(mo, "clip2.hasOwnProperty('__constructor__')");
	check(mo, "clip2.hasOwnProperty('constructor')");
	check(mo, "clip2.hasOwnProperty('__proto__')");
	check_equals(mo, "clip2.__proto__", "CustomClass.prototype");
	check_equals(mo, "clip2.__constructor__", "CustomClass");
	check_equals(mo, "clip2.constructor", "CustomClass");
	check_equals(mo, "clip2._x", "80");
	check_equals(mo, "typeof(clip2.lineTo)", "'undefined'");
	check(mo, "! clip2 instanceOf MovieClip");
	
	// Check that non-enumerable properties (unnamed instances,
	// constructor, __constructor__) are not enumerated.
	add_actions(mo, "var s = ''; for (i in clip2) { s += i + ','; };");
	check_equals(mo, "s", "'onLoad,onRollOver,'");

	check(mo, "clip3.hasOwnProperty('__constructor__')");
	check(mo, "clip3.hasOwnProperty('constructor')");
	check(mo, "clip3.hasOwnProperty('__proto__')");
	check_equals(mo, "clip3.__proto__", "CustomClass2.prototype");
	check_equals(mo, "clip3.__constructor__", "CustomClass2");
	check_equals(mo, "clip3.constructor", "CustomClass2");
	check_equals(mo, "typeof(clip3)", "'movieclip'");
	check_equals(mo, "clip3._x", "160");
	check(mo, "clip3 instanceOf CustomClass2");
	check(mo, "clip3 instanceOf MovieClip");
	
	// Check that non-enumerable properties (unnamed instances,
	// constructor, __constructor__) are not enumerated.
	add_actions(mo, "var s = ''; for (i in clip3) { s += i + ','; };");
	check_equals(mo, "s", "'onRollOver,'");

	check(mo, "clip4.hasOwnProperty('__constructor__')");
	check(mo, "clip4.hasOwnProperty('constructor')");
	check(mo, "clip4.hasOwnProperty('__proto__')");
	check_equals(mo, "clip4.__proto__", "CustomClass2.prototype");
	check_equals(mo, "clip4.__constructor__", "CustomClass2");
	check_equals(mo, "clip4.constructor", "CustomClass2");
	check_equals(mo, "typeof(clip4)", "'movieclip'");
	check_equals(mo, "clip4._x", "240");
	check(mo, "clip4 instanceOf MovieClip");

	add_actions(mo,
		"totals(47);"
		"stop();"
		);

	SWFMovie_nextFrame(mo); /* end of frame5 */

	/*****************************************************
	 *
	 * Output movie
	 *
	 *****************************************************/

	puts("Saving " OUTPUT_FILENAME );

	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
