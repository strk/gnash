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
 * Test "Jumping backward to the midle of a DisplayObject's lifetime after static transformation"
 *
 * run as ./displaylist_depths_test4
 *
 * Timeline:
 * 
 *   Frame  | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
 *  --------+---+---+---+---+---+---+---+
 *   Event  |   | P |   | T*| T |   | J |
 * 
 *  P = place (by PlaceObject2)
 *  T = transform matrix (by PlaceObject2)
 *  J = jump
 *  * = jump target
 * 
 * Description:
 * 
 *  frame2: DisplayObject placed at depth -16381 at position (10,200)
 *  frame4: position of instance at depth -16381 shifted to the right (50,200)
 *  frame5: position of instance at depth -16381 shifted to the right (100,200)
 *  frame7: jump back to frame 4
 * 
 * Expected behaviour on jump back:
 * 
 *  Before the jump we have a single instance at depth -16381 and position 100,200.
 *  After the jump we have the same instances at depth -16381, repositioned at 50,200.
 *  A single instance has been constructed in total.
 *  Soft references to the instance created before the jump-back still point to the same instance.
 * 
 */

#include "ming_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

// We need version 7 to use getInstanceAtDepth()
#define OUTPUT_VERSION 7
#define OUTPUT_FILENAME "displaylist_depths_test4.swf"

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

	// Frame 5: move DisplayObject at depth 3 to position 100,200
	SWFDisplayItem_moveTo(it1, 200, 200); 
	add_actions(mo,
		"check_equals(static3.myThing, 'guess');"
		"check_equals(static3._x, 200);"
		"check_equals(static3.getDepth(), -16381);" 
		);
	SWFMovie_nextFrame(mo); 

	// Frame 6: nothing new
	SWFMovie_nextFrame(mo); 

	// Frame 7: go to frame 4
	add_actions(mo,

		"check_equals(static3.myThing, 'guess');"

		// Store a reference to the static3 instance
		// before jumping back
		"dynRef = static3;"

		// this reset char at depth -16381 to be at position 50,200
		"gotoAndStop(4);"

		// Static3 refers to same instance
		"check_equals(static3.myThing, 'guess');" 
		"check_equals(static3.getDepth(), -16381);" 

		// But it has now be reset to position 50,100 as specified
		// by PlaceObject2 tag in frame 4
		"check_equals(static3._x, 50);" 

		// The reference still refers to the same instance
		//  (see http://www.gnashdev.org/wiki/index.php/SoftReferences)
		"check_equals(dynRef.myThing, 'guess');" 
		"check_equals(dynRef.getDepth(), -16381);" 
		"check_equals(typeof(dynRef), 'movieclip');"
		"check_equals(dynRef._x, 50);"
		"check_equals(dynRef, static3);"

		// A single instance is created in total
		"check_equals(depth3Constructed, 1);"  

		"totals();"
		);
	SWFMovie_nextFrame(mo); 

	//Output movie
	puts("Saving " OUTPUT_FILENAME );
	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
