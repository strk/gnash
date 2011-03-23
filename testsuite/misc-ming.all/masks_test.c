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
 * Test masks
 *
 * run as ./masks_test
 */

#include "ming_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "masks_test.swf"

void add_dynamic_mc(SWFMovie mo, const char* name, int depth, int x, int y, int width, int height, int r, int g, int b);
void add_static_mc(SWFMovie mo, const char* name, int depth, int x, int y, int width, int height, int r, int g, int b);
void add_static_mask(SWFMovie mo, const char* name, int depth, int x, int y, int width, int height, int masklevel);

void
add_dynamic_mc(SWFMovie mo, const char* name, int depth, int x, int y, int width, int height, int r, int g, int b)
{
	SWFAction ac = compile_actions("createEmptyMovieClip('%s', %d);"
			"with (%s) {"
			//" lineStyle(1, 0x000000, 100);"	
			" beginFill(0x%2.2X%2.2X%2.2X, 100);"
			" moveTo(%d, %d);"
			" lineTo(%d, %d);"
			" lineTo(%d, %d);"
			" lineTo(%d, %d);"
			" lineTo(%d, %d);"
			" endFill();"
			"}",
			name, depth, name,
			r,g,b,
			x, y,
			x, y+height,
			x+width, y+height,
			x+width, y,
			x, y
			);

	SWFMovie_add(mo, (SWFBlock)ac);
}

void
add_static_mc(SWFMovie mo, const char* name, int depth, int x, int y, int width, int height, int r, int g, int b)
{
	SWFShape sh;
	SWFMovieClip mc;
	SWFDisplayItem it;

	sh = make_fill_square (0, 0, width, height, r, g, b, r, g, b);
	mc = newSWFMovieClip();
	SWFMovieClip_add(mc, (SWFBlock)sh);

	SWFMovieClip_nextFrame(mc);

	it = SWFMovie_add(mo, (SWFBlock)mc);
	SWFDisplayItem_setDepth(it, depth); 
	SWFDisplayItem_moveTo(it, x, y); 
	SWFDisplayItem_setName(it, name);
}

void
add_static_mask(SWFMovie mo, const char* name, int depth, int x, int y, int width, int height, int masklevel)
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
	SWFDisplayItem_setMaskLevel(it, masklevel);
}


int
main(int argc, char** argv)
{
	SWFMovie mo;
	SWFMovieClip dejagnuclip;
	SWFDisplayItem it;

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
	SWFMovie_setDimension(mo, 1200, 600);
	SWFMovie_setRate (mo, 1);

	dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
	it = SWFMovie_add(mo, (SWFBlock)dejagnuclip);
	SWFDisplayItem_setDepth(it, 1000);
	SWFDisplayItem_moveTo(it, 300, 0);
	SWFMovie_nextFrame(mo);  // FRAME 2 starts here

	add_actions(mo, "note('Test masks and dynamic masks at different depth ranges.');");

	// this one seems to confuse the MM player
	//add_static_mask(mo, "mask1", 1, 0, 150, 200, 100, 20);

	// Red rect (staticmc2) is at 0,200-60,260
	add_static_mc(mo, "staticmc2", 2, 0, 200, 60, 60, 255, 0, 0); // red

	// Yellow rect (staticmc3) is at 30,200-90,260
	add_static_mc(mo, "staticmc3", 3, 30, 200, 60, 60, 255, 255, 0); // yellow

	// Green rect (staticmc4) is at 200,200-260,260
	add_static_mc(mo, "staticmc4", 4, 200, 200, 60, 60, 0, 255, 0); // green

	// Cyan rect (staticmc5) is at 230,200-290,260
	add_static_mc(mo, "staticmc5", 5, 230, 200, 60, 60, 0, 255, 255); // cyan

	// Blue rect (dynamicmc2) is at 0,300-60,360
	add_dynamic_mc(mo, "dynamicmc2", 12, 0, 300, 60, 60, 0, 0, 255); // blue

	// Violet rect (dynamicmc3) is at 30,300-90,360
	add_dynamic_mc(mo, "dynamicmc3", 13, 30, 300, 60, 60, 255, 0, 255); // violet

	// Dark green rect (dynamicmc4) is at 200,300-260,360
	add_dynamic_mc(mo, "dynamicmc4", 14, 200, 300, 60, 60, 0, 128, 0); // dark green

	// Light blue rect (dynamicmc5) is at 230,300-290,360
	add_dynamic_mc(mo, "dynamicmc5", 15, 230, 300, 60, 60, 0, 128, 255); // light blue


	// Red rect
	check_equals(mo, "staticmc2.getDepth()", "-16382");
	check(mo, "staticmc2.hitTest(10, 210, true)");
	check(mo, "staticmc2.hitTest(50, 250, true)");

	// Yellow rect
	check_equals(mo, "staticmc3.getDepth()", "-16381");
	check(mo, "staticmc3.hitTest(40, 210, true)");
	check(mo, "staticmc3.hitTest(80, 250, true)");

	// Green rect
	check_equals(mo, "staticmc4.getDepth()", "-16380"); 
	check(mo, "staticmc4.hitTest(210, 210, true)");
	check(mo, "staticmc4.hitTest(250, 250, true)");

	// Cyan rect
	check_equals(mo, "staticmc5.getDepth()", "-16379"); 
	check(mo, "staticmc5.hitTest(240, 210, true)");
	check(mo, "staticmc5.hitTest(280, 250, true)");

	// Blue rect
	check_equals(mo, "dynamicmc2.getDepth()", "12");
	check(mo, "dynamicmc2.hitTest(10, 310, true)");
	check(mo, "dynamicmc2.hitTest(50, 350, true)");

	// Violet rect
	check_equals(mo, "dynamicmc3.getDepth()", "13");
	check(mo, "dynamicmc3.hitTest(40, 310, true)");
	check(mo, "dynamicmc3.hitTest(80, 350, true)");

	// Dark green rect
	check_equals(mo, "dynamicmc4.getDepth()", "14"); 
	check(mo, "dynamicmc4.hitTest(210, 310, true)");
	check(mo, "dynamicmc4.hitTest(250, 350, true)");

	// Light blue rect
	check_equals(mo, "dynamicmc5.getDepth()", "15"); 
	check(mo, "dynamicmc5.hitTest(240, 310, true)");
	check(mo, "dynamicmc5.hitTest(280, 350, true)");

	add_actions(mo,
		"note('Placed staticmc2 (red), staticmc3 (yellow), staticmc4 (green), staticmc5 (cyan) DisplayObjects');"
		"note('Placed dynamicmc2 (blue), dynamicmc3 (violet), dynamicmc4 (dark green), dynamicmc5 (light blue) DisplayObjects');"
		"note(' - Press any key to continue -');"
		"stop();"
		"l = new Object();"
		"l.onKeyUp = function() { nextFrame(); };"
		"Key.addListener(l);"
	);

	SWFMovie_nextFrame(mo);  // FRAME 3 starts here

	add_actions(mo, 
		"sm23 = staticmc2.setMask(staticmc3);" // red masked by yellow
		"sm54 = staticmc5.setMask(staticmc4);" // cyan masked by green
		"dm23 = dynamicmc2.setMask(dynamicmc3);" // blue masked by violet
		"dm54 = dynamicmc5.setMask(dynamicmc4);" // light blue masked by dark green 
		);

	add_actions(mo,
		"note('staticmc2.setMask(staticmc3) [red masked by yellow]');"
		"note('staticmc5.setMask(staticmc4) [cyan masked by green]');"
		"note('dynamicmc2.setMask(dynamicmc3) [blue masked by violet');"
		"note('dynamicmc5.setMask(dynamicmc4) [light blue masked by dark green');"
	);

	check_equals(mo, "typeof(sm23)", "'boolean'");
	check_equals(mo, "sm23", "true");
	check_equals(mo, "typeof(sm54)", "'boolean'");
	check_equals(mo, "sm54", "true");
	check_equals(mo, "typeof(dm23)", "'boolean'");
	check_equals(mo, "dm23", "true");
	check_equals(mo, "typeof(dm54)", "'boolean'");
	check_equals(mo, "dm54", "true");

	// Red rect is now masked by yellow
	// Red rect (staticmc2) is at 0,200-60,260
	// Yellow rect (staticmc3) is at 30,200-90,260
	check(mo, "!staticmc2.hitTest(10, 210, true)"); 
	check(mo, "staticmc2.hitTest(10, 210, false)"); // bounding box hitTest not affected by masks
	check(mo, "staticmc2.hitTest(50, 250, true)"); 

	// Yellow rect is now a mask
	// hitTest() using 'shape' semantic won't see it,
	// while hitTest() using 'bounding box' semantic will 
	check(mo, "staticmc3.hitTest(40, 210, false)");
	check(mo, "staticmc3.hitTest(80, 250, false)");
	check(mo, "!staticmc3.hitTest(40, 210, true)");
	check(mo, "!staticmc3.hitTest(80, 250, true)");

	// Green rect is now a mask
	// hitTest() using 'shape' semantic won't see it,
	// while hitTest() using 'bounding box' semantic will 
	check(mo, "!staticmc4.hitTest(210, 210, true)");
	check(mo, "!staticmc4.hitTest(250, 250, true)"); 
	check(mo, "staticmc4.hitTest(210, 210, false)");
	check(mo, "staticmc4.hitTest(250, 250, false)");

	// Cyan rect is now masked by green
	// Green rect (staticmc4) is at 200,200-260,260
	// Cyan rect (staticmc5) is at 230,200-290,260
	check(mo, "staticmc5.hitTest(240, 210, true)");
	check(mo, "!staticmc5.hitTest(280, 250, true)");
	check(mo, "staticmc5.hitTest(280, 250, false)");

	// Blue rect now is masked by Violet rect 
	// Violet rect (dynamicmc3) is at 30,300-90,360
	// Blue rect (dynamicmc2) is at 0,300-60,360
	check(mo, "!dynamicmc2.hitTest(10, 310, true)");
	check(mo, "dynamicmc2.hitTest(10, 310, false)");
	check(mo, "dynamicmc2.hitTest(50, 350, true)");

	// Violet rect is now a mask
	// hitTest() using 'shape' semantic won't see it,
	// while hitTest() using 'bounding box' semantic will 
	check(mo, "!dynamicmc3.hitTest(40, 310, true)");
	check(mo, "!dynamicmc3.hitTest(80, 350, true)");
	check(mo, "dynamicmc3.hitTest(40, 310, false)");
	check(mo, "dynamicmc3.hitTest(80, 350, false)");

	// Dark green rect is now a mask
	check(mo, "!dynamicmc4.hitTest(210, 310, true)");
	check(mo, "!dynamicmc4.hitTest(250, 350, true)");
	check(mo, "dynamicmc4.hitTest(210, 310, false)");
	check(mo, "dynamicmc4.hitTest(250, 350, false)");

	// Light blue now masked by Dark green 
	// Light blue rect (dynamicmc5) is at 230,300-290,360
	// Dark green rect (dynamicmc4) is at 200,300-260,360
	check(mo, "dynamicmc5.hitTest(240, 310, true)");
	check(mo, "!dynamicmc5.hitTest(280, 350, true)");
	check(mo, "dynamicmc5.hitTest(280, 350, false)");

	add_actions(mo,
		"note(' - Press any key to continue -');"
    		"stop();"
	);

	SWFMovie_nextFrame(mo);  // FRAME 4 starts here

	add_actions(mo,
		"staticmc2.swapDepths(staticmc3);"
		"staticmc4.swapDepths(staticmc5);"
		"dynamicmc2.swapDepths(dynamicmc3);"
		"dynamicmc4.swapDepths(dynamicmc5);"
		);

	add_actions(mo,
		"note('Swapped depths of chars 2/3 and 4/5 to see if masks are still in effect');"
		"note(' - Press any key to continue -');"
    		"stop();"
	);


	check_equals(mo, "staticmc2.getDepth()", "-16381");
	check_equals(mo, "staticmc3.getDepth()", "-16382");
	check_equals(mo, "staticmc4.getDepth()", "-16379"); 
	check_equals(mo, "staticmc5.getDepth()", "-16380"); 
	check_equals(mo, "dynamicmc2.getDepth()", "13");
	check_equals(mo, "dynamicmc3.getDepth()", "12");
	check_equals(mo, "dynamicmc4.getDepth()", "15"); 
	check_equals(mo, "dynamicmc5.getDepth()", "14"); 

	// Depth swapping didn't change hitTest effects

	// Red rect is now masked by yellow
	// Red rect (staticmc2) is at 0,200-60,260
	// Yellow rect (staticmc3) is at 30,200-90,260
	check(mo, "!staticmc2.hitTest(10, 210, true)"); 
	check(mo, "staticmc2.hitTest(10, 210, false)"); // bounding box hitTest not affected by masks
	check(mo, "staticmc2.hitTest(50, 250, true)"); 

	// Yellow rect is now a mask
	// hitTest() using 'shape' semantic won't see it,
	// while hitTest() using 'bounding box' semantic will 
	check(mo, "staticmc3.hitTest(40, 210, false)");
	check(mo, "staticmc3.hitTest(80, 250, false)");
	check(mo, "!staticmc3.hitTest(40, 210, true)");
	check(mo, "!staticmc3.hitTest(80, 250, true)");

	// Green rect is now a mask
	// hitTest() using 'shape' semantic won't see it,
	// while hitTest() using 'bounding box' semantic will 
	check(mo, "staticmc4.hitTest(210, 210, false)");
	check(mo, "staticmc4.hitTest(250, 250, false)");
	check(mo, "!staticmc4.hitTest(210, 210, true)");
	check(mo, "!staticmc4.hitTest(250, 250, true)"); 

	// Cyan rect is now masked by green
	// Green rect (staticmc4) is at 200,200-260,260
	// Cyan rect (staticmc5) is at 230,200-290,260
	check(mo, "staticmc5.hitTest(240, 210, true)");
	check(mo, "!staticmc5.hitTest(280, 250, true)");
	check(mo, "staticmc5.hitTest(280, 250, false)");

	// Blue rect now is masked by Violet rect 
	// Violet rect (dynamicmc3) is at 30,300-90,360
	// Blue rect (dynamicmc2) is at 0,300-60,360
	check(mo, "!dynamicmc2.hitTest(10, 310, true)");
	check(mo, "dynamicmc2.hitTest(10, 310, false)");
	check(mo, "dynamicmc2.hitTest(50, 350, true)");

	// Violet rect is now a mask
	// hitTest() using 'shape' semantic won't see it,
	// while hitTest() using 'bounding box' semantic will 
	check(mo, "!dynamicmc3.hitTest(40, 310, true)");
	check(mo, "!dynamicmc3.hitTest(80, 350, true)");
	check(mo, "dynamicmc3.hitTest(40, 310, false)");
	check(mo, "dynamicmc3.hitTest(80, 350, false)");

	// Dark green rect is now a mask
	check(mo, "!dynamicmc4.hitTest(210, 310, true)");
	check(mo, "!dynamicmc4.hitTest(250, 350, true)");
	check(mo, "dynamicmc4.hitTest(210, 310, false)");
	check(mo, "dynamicmc4.hitTest(250, 350, false)");

	// Light blue now masked by Dark green 
	// Light blue rect (dynamicmc5) is at 230,300-290,360
	// Dark green rect (dynamicmc4) is at 200,300-260,360
	check(mo, "dynamicmc5.hitTest(240, 310, true)");
	check(mo, "!dynamicmc5.hitTest(280, 350, true)");
	check(mo, "dynamicmc5.hitTest(280, 350, false)");

	SWFMovie_nextFrame(mo);  // FRAME 5 starts here


	add_actions(mo,
		"sm32 = staticmc3.setMask(staticmc2);" // yellow masked by red 
		"sm45 = staticmc4.setMask(staticmc5);" // green masked by cyan
		"dm32 = dynamicmc3.setMask(dynamicmc2);" // violet masked by blue
		"dm45 = dynamicmc4.setMask(dynamicmc5);" // dark green masked by light blue 
		);

	add_actions(mo,
		"note('Swapped mask/maskee:');"
		"note(' staticmc3.setMask(staticmc2) [yellow masked by red]');"
		"note(' staticmc4.setMask(staticmc5) [green masked by cyan]');"
		"note(' dynamicmc3.setMask(dynamicmc4) [violet masked by blue');"
		"note(' dynamicmc4.setMask(dynamicmc5) [dark green masked by light blue');"
	);

	check_equals(mo, "typeof(sm32)", "'boolean'");
	check_equals(mo, "sm32", "true");
	check_equals(mo, "typeof(sm45)", "'boolean'");
	check_equals(mo, "sm45", "true");
	check_equals(mo, "typeof(dm32)", "'boolean'");
	check_equals(mo, "dm32", "true");
	check_equals(mo, "typeof(dm45)", "'boolean'");
	check_equals(mo, "dm45", "true");

	// Red rect is now a mask
	check(mo, "!staticmc2.hitTest(10, 210, true)"); 
	check(mo, "!staticmc2.hitTest(50, 250, true)"); 
	check(mo, "staticmc2.hitTest(10, 210, false)"); 
	check(mo, "staticmc2.hitTest(50, 250, false)"); 

	// Yellow rect is now masked by Red rect
	// Yellow rect (staticmc3) is at 30,200-90,260
	// Red rect (staticmc2) is at 0,200-60,260
	// Intersection is 30,200-60,260
	// TODO: why no hitTest ??
	xcheck(mo, "!staticmc3.hitTest(40, 210, true)"); // I'd think this should be true, why not ?
	check(mo, "!staticmc3.hitTest(80, 250, true)"); // out of masked area
	check(mo, "staticmc3.hitTest(80, 250, false)");
	check(mo, "staticmc3.hitTest(40, 210, false)");

	// Green rect is now masked by Cyan
	// Green rect (staticmc4) is at 200,200-260,260
	// Cyan rect (staticmc5) is at 230,200-290,260
	// Intersection is 230,200-260,260
	// TODO: why no hitTest ??
	check(mo, "!staticmc4.hitTest(210, 210, true)");   // out of masked area
	xcheck(mo, "!staticmc4.hitTest(250, 250, true)");  // I'd think this should be true, why not?
	check(mo, "staticmc4.hitTest(210, 210, false)");
	check(mo, "staticmc4.hitTest(250, 250, false)");

	// Cyan rect is now a mask
	check(mo, "!staticmc5.hitTest(240, 210, true)");
	check(mo, "!staticmc5.hitTest(280, 250, true)");
	check(mo, "staticmc5.hitTest(240, 210, false)");
	check(mo, "staticmc5.hitTest(280, 250, false)");

	// Blue rect is now a mask
	check(mo, "!dynamicmc2.hitTest(10, 310, true)");
	check(mo, "!dynamicmc2.hitTest(50, 350, true)");
	check(mo, "dynamicmc2.hitTest(10, 310, false)");
	check(mo, "dynamicmc2.hitTest(50, 350, false)");

	// Violet rect is now masked by Blue rect
	check(mo, "dynamicmc3.hitTest(40, 310, true)"); 
	check(mo, "!dynamicmc3.hitTest(80, 350, true)");
	check(mo, "dynamicmc3.hitTest(80, 350, false)");

	// Dark green rect is masked by Light blue
	check(mo, "!dynamicmc4.hitTest(210, 310, true)");
	check(mo, "dynamicmc4.hitTest(210, 310, false)");
	check(mo, "dynamicmc4.hitTest(250, 350, true)"); 

	// Light blue is now a mask
	check(mo, "dynamicmc5.hitTest(240, 310, false)");
	check(mo, "dynamicmc5.hitTest(280, 350, false)");
	check(mo, "!dynamicmc5.hitTest(240, 310, true)");
	check(mo, "!dynamicmc5.hitTest(280, 350, true)");

	add_actions(mo,
		"note(' - Press any key to continue -');"
    		"stop();"
	);

	SWFMovie_nextFrame(mo);        

	add_actions(mo,
		"var clips = [staticmc2, staticmc3, staticmc4, staticmc5, dynamicmc2, dynamicmc3, dynamicmc4, dynamicmc5];"
		"for (i=0; i<clips.length; ++i) {"
		" clips[i].onRollOver = function() { this._alpha = 50; };"
		" clips[i].onRollOut = function() { this._alpha = 100; };"
		"}"
		"note('Made all DisplayObjects mouse-sensitive');"
	);

	// Red rect is a mask, but has mouse events !
	check(mo, "staticmc2.hitTest(10, 210, true)"); 
	check(mo, "staticmc2.hitTest(50, 250, true)"); 

	// Yellow rect is now masked by Red rect
	// Yellow rect (staticmc3) is at 30,200-90,260
	// Red rect (staticmc2) is at 0,200-60,260
	// Intersection is 30,200-60,260
	check(mo, "staticmc3.hitTest(40, 210, true)"); 
	check(mo, "!staticmc3.hitTest(80, 250, true)"); // out of masked area
	check(mo, "staticmc3.hitTest(80, 250, false)");

	// Green rect is now masked by Cyan
	// Green rect (staticmc4) is at 200,200-260,260
	// Cyan rect (staticmc5) is at 230,200-290,260
	// Intersection is 230,200-260,260
	check(mo, "!staticmc4.hitTest(210, 210, true)");   // out of masked area
	check(mo, "staticmc4.hitTest(250, 250, true)");  
	check(mo, "staticmc4.hitTest(210, 210, false)");

	// Cyan rect is a mask but has mouse events !
	check(mo, "staticmc5.hitTest(240, 210, true)");
	check(mo, "staticmc5.hitTest(280, 250, true)");

	// Blue rect is a mask but has mouse events !
	check(mo, "dynamicmc2.hitTest(10, 310, true)");
	check(mo, "dynamicmc2.hitTest(50, 350, true)");

	// Violet rect is now masked by Blue rect
	check(mo, "dynamicmc3.hitTest(40, 310, true)"); 
	check(mo, "!dynamicmc3.hitTest(80, 350, true)");
	check(mo, "dynamicmc3.hitTest(80, 350, false)");

	// Dark green rect is masked by Light blue
	check(mo, "!dynamicmc4.hitTest(210, 310, true)");
	check(mo, "dynamicmc4.hitTest(210, 310, false)");
	check(mo, "dynamicmc4.hitTest(250, 350, true)"); 

	// Light blue is a mask but has mouse events !
	check(mo, "dynamicmc5.hitTest(240, 310, true)");
	check(mo, "dynamicmc5.hitTest(280, 350, true)");

	add_actions(mo, "_root.totals(154); stop();");

	SWFMovie_nextFrame(mo);        


	// TODO:
	// - test mask layers !!

	//Output movie
	puts("Saving " OUTPUT_FILENAME );
	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
