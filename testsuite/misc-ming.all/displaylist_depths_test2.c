/* 
 *   Copyright (C) 2007 Free Software Foundation, Inc.
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
 */ 

/*
 * Sandro Santilli, strk@keybit.net
 *
 * Test how swapDepth affects DisplayList refresh on gotoAndPlay(current-X).
 * A character is placed at depth 3 (-16381) and moved to the right for a couple of frames
 * using PlaceObject2 tag.
 * After that, the placed instance is moved from depth -16381 to depth 10 (dynamic zone)
 * and a jump-back is issued to the frame containing the right-shift static transform.
 *
 * Expected behaviour is that the gotoFrame() reconstructs the DisplayList so that
 * depth -16381 is taken by the character and transformed exactly as specified by
 * PlaceObject2 tag. This is a *new* instance, altought it uses the same name as the 
 * old one. The old one (now at depth 10) is still there, visible, but can not be
 * accessible by ActionScript anymore as it's name has been overridden by the
 * reconstructed instance.
 *
 * run as ./displaylist_depths_test2
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
	SWFMovieClip mc, mc2;
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
	int i;
	SWFDisplayItem it1,it2,it3,it4;


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

	// Frame 4: move character at depth 3 to position 50,200
	SWFDisplayItem_moveTo(it1, 50, 200); 
	add_actions(mo,
		"check_equals(static3._x, 50);"
		"check_equals(static3.getDepth(), -16381);" 
		);
	SWFMovie_nextFrame(mo); 

	// Frame 5: move character at depth 3 to position 100,200
	SWFDisplayItem_moveTo(it1, 200, 200); 
	add_actions(mo,
		"check_equals(static3.myThing, 'guess');"
		"check_equals(static3._x, 200);"
		"check_equals(static3.getDepth(), -16381);" 
		);
	SWFMovie_nextFrame(mo); 

	// Frame 6: change depth character at depth 3 to depth 10 (dynamic zone)
	add_actions(mo,
		"static3.swapDepths(10);"
		"check_equals(static3.getDepth(), 10);" 
		"static3._rotation = 45;"
		"check_equals(static3.myThing, 'guess');"
		);
	SWFMovie_nextFrame(mo); 

	// Frame 7: go to frame 3 and see if the PlaceObject2 in frame 4
	//          is able to change the position back to 50,200
	//          (it's at 100,200 now!)
	add_actions(mo,

		"check_equals(static3.myThing, 'guess');"

		// Store a reference to the static3 instance
		// before overriding its name
		"dynRef = static3;"

		// this repopulates depth -16381 with a *new* instance 
		"gotoAndStop(4);"

		// static3 doesn't refer to the dynamic object anymore !
		"check_equals(typeof(static3.myThing), 'undefined');"

		// but the reference still does !!
		// Gnash fails here due to it's implementation of "soft references"
		// ... argh ...
		// TODO: use a MovieTester based test runner to check for actual
		//       existance of the old (dynamicized) instance by looking
		//       at the real DisplayList and at the rendered buffer
		//
		"xcheck_equals(dynRef.myThing, 'guess');"
		"xcheck_equals(dynRef.getDepth(), 10);" 

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
