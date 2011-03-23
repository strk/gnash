/* 
 *   Copyright (C) 2007, 2009, 2010, 2011 Free Software Foundation, Inc.
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
 */ 

/*
 * Sandro Santilli, strk@keybit.net
 *
 * Test how swapDepth affects DisplayList refresh on gotoAndPlay(current-X).
 *
 * run as ./displaylist_depths_test2
 *
 * Timeline:
 * 
 *   Frame  | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
 *  --------+---+---+---+---+---+---+---+
 *   Event  |   | P |   | T*| T | M | J |
 * 
 *  P = place (by PlaceObject2)
 *  T = transform matrix (by PlaceObject2)
 *  M = move to another depth (by swapDepth)
 *  J = jump
 *  * = jump target
 * 
 * Description:
 * 
 *  frame2: DisplayObject placed at depth -16381 at position (10,200)
 *  frame4: position of instance at depth -16381 shifted to the right (50,200)
 *  frame5: position of instance at depth -16381 shifted to the right (200,200)
 *  frame6: depth of instance changed to 10 (dynamic zone) and stop.
 *  frame7: jump back to frame 4
 * 
 * Expected behaviour on jump back:
 * 
 *  Before the jump we have a single instance at depth 10 and position 100,200.
 *  After the jump we have two instances:
 *         - one at depth 10 and position 100,200
 *           (the same we had before, with its state intact)
 *         - another at depth -16381 and position 50,200
 *           (newly created and placed accordingly to the PlaceObject2 tag on frame4)
 *  Two distinct instances have been constructed in total.
 * 
 */

#include "ming_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

// We need version 7 to use getInstanceAtDepth()
#define OUTPUT_VERSION 7
#define OUTPUT_FILENAME "displaylist_depths_test2.swf"

SWFDisplayItem add_static_mc(SWFMovie mo, const char* name, int depth, int x, int y, int width, int height);

SWFDisplayItem
add_static_mc(SWFMovie mo, const char* name, int depth, int x, int y, int width, int height)
{
	SWFShape sh;
	SWFMovieClip mc;
	SWFDisplayItem it;

	sh = make_fill_square (-(width/2), -(height/2), width, height, 255, 0, 0, 255, 0, 0);
	mc = newSWFMovieClip();
	SWFMovieClip_add(mc, (SWFBlock)sh);

	SWFMovieClip_nextFrame(mc);

	it = SWFMovie_add(mo, (SWFBlock)mc);
	SWFDisplayItem_setDepth(it, depth); 
	SWFDisplayItem_moveTo(it, x, y); 
	SWFDisplayItem_setName(it, name);
	SWFDisplayItem_addAction(it, newSWFAction(
			"_root.note(this+' onClipConstruct');"
			" _root.check_equals(typeof(_root), 'movieclip');"
		        " if ( isNaN(_root.depth3Constructed) ) {"
			"	_root.depth3Constructed=1; "
			" 	_root.note('_root.depth3Constructed set to '+_root.depth3Constructed);"
			" } else {"
			"	_root.depth3Constructed++;"
			" 	_root.note('_root.depth3Constructed set to '+_root.depth3Constructed);"
			" }"
			), SWFACTION_CONSTRUCT);

	return it;
}


int
main(int argc, char** argv)
{
	SWFMovie mo;
	SWFMovieClip dejagnuclip;
	SWFDisplayItem it1;


	const char *srcdir=".";
	if ( argc>1 ) 
		srcdir=argv[1];
	else
	{
   		//fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
   		//return 1;
	}

	Ming_init();
	mo = newSWFMovieWithVersion(OUTPUT_VERSION);
	SWFMovie_setDimension(mo, 800, 600);
	SWFMovie_setRate (mo, 2);

	dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
	SWFMovie_add(mo, (SWFBlock)dejagnuclip);
	SWFMovie_nextFrame(mo); 

	// Frame 2: Add a static movieclip at depth 3 with origin at 10,200
	it1 = add_static_mc(mo, "static3", 3, 10, 200, 20, 20);
	add_actions(mo,
		"static3.myThing = 'guess';"
		"check_equals(static3._x, 10);"
		"check_equals(static3.myThing, 'guess');"
		"check_equals(static3.getDepth(), -16381);" 
		);
	SWFMovie_nextFrame(mo); 

	// Frame 3: nothing new
	SWFMovie_nextFrame(mo); 

	// Frame 4: move DisplayObject at depth 3 to position 50,200
	SWFDisplayItem_moveTo(it1, 50, 200); 
	add_actions(mo,
		"check_equals(static3._x, 50);"
		"check_equals(static3.getDepth(), -16381);" 
		);
	SWFMovie_nextFrame(mo); 

	// Frame 5: move DisplayObject at depth 3 to position 200,200
	SWFDisplayItem_moveTo(it1, 200, 200); 
	add_actions(mo,
		"check_equals(static3.myThing, 'guess');"
		"check_equals(static3._x, 200);"
		"check_equals(static3.getDepth(), -16381);" 
		);
	SWFMovie_nextFrame(mo); 

	// Frame 6: change depth DisplayObject at depth 3 to depth 10 (dynamic zone)
	add_actions(mo,
		"static3.swapDepths(10);"
		"check_equals(static3.getDepth(), 10);" 
		"static3._rotation = 45;"
		"check_equals(static3.myThing, 'guess');"
		);
	SWFMovie_nextFrame(mo); 

	// Frame 7: go to frame 4
	add_actions(mo,

		"check_equals(static3.myThing, 'guess');"

		// Store a reference to the static3 instance
		// before overriding its name
		"dynRef = static3;"

		// this repopulates depth -16381 with a *new* instance 
		"gotoAndStop(4);"

		"note('right after gotoAndStop()');"

		// static3 doesn't refer to the dynamic object anymore !
		"check_equals(typeof(static3.myThing), 'undefined');"

		// but the reference still does !!
		// Gnash fails here due to it's implementation of "soft references"
		// ... argh ...
		// TODO: use a MovieTester based test runner to check for actual
		//       existance of the old (dynamicized) instance by looking
		//       at the real DisplayList and at the rendered buffer
		//
		"check_equals(dynRef.myThing, 'guess');"
		"check_equals(dynRef.getDepth(), 10);" 

		// Luckly we can query for depth chars with getInstanceAtDepth
		"check_equals(typeof(getInstanceAtDepth(-16381)), 'movieclip');"
		"check_equals(typeof(getInstanceAtDepth(10)), 'movieclip');"

		"check_equals(static3.getDepth(), -16381);" 
		"check_equals(static3._x, 50);" 
		"check_equals(depth3Constructed, 2);" 
		"totals();"
		);
	SWFMovie_nextFrame(mo); 

	//Output movie
	puts("Saving " OUTPUT_FILENAME );
	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
