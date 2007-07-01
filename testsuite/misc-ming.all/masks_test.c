/* 
 *   Copyright (C) 2007 Free Software Foundation, Inc.
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
	SWFMovieClip mc, mc2;
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
	SWFMovie_setDimension(mo, 800, 600);
	SWFMovie_setRate (mo, 0.3);

	dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
	it = SWFMovie_add(mo, (SWFBlock)dejagnuclip);
	SWFDisplayItem_setDepth(it, 1000);
	SWFMovie_nextFrame(mo); 

	add_actions(mo, "note('Test masks and dynamic masks at different depth ranges.');");

	// this one seems to confuse the MM player
	//add_static_mask(mo, "mask1", 1, 0, 150, 200, 100, 20);

	add_static_mc(mo, "staticmc2", 2, 0, 200, 60, 60, 255, 0, 0);
	add_static_mc(mo, "staticmc3", 3, 30, 200, 60, 60, 255, 255, 0);
	add_static_mc(mo, "staticmc4", 4, 200, 200, 60, 60, 0, 255, 0);
	add_static_mc(mo, "staticmc5", 5, 230, 200, 60, 60, 0, 255, 255);
	add_dynamic_mc(mo, "dynamicmc2", 12, 0, 300, 60, 60, 0, 0, 255);
	add_dynamic_mc(mo, "dynamicmc3", 13, 30, 300, 60, 60, 255, 0, 255);
	add_dynamic_mc(mo, "dynamicmc4", 14, 200, 300, 60, 60, 0, 128, 0);
	add_dynamic_mc(mo, "dynamicmc5", 15, 230, 300, 60, 60, 0, 128, 255);


	check_equals(mo, "staticmc2.getDepth()", "-16382");
	check_equals(mo, "staticmc3.getDepth()", "-16381");
	check_equals(mo, "staticmc4.getDepth()", "-16380"); 
	check_equals(mo, "staticmc5.getDepth()", "-16379"); 
	check_equals(mo, "dynamicmc2.getDepth()", "12");
	check_equals(mo, "dynamicmc3.getDepth()", "13");
	check_equals(mo, "dynamicmc4.getDepth()", "14"); 
	check_equals(mo, "dynamicmc5.getDepth()", "15"); 

	SWFMovie_nextFrame(mo);        

	add_actions(mo, "note('Using setMask on chars in the static depth range 2.mask(3) and 5.mask(5)');");

	add_actions(mo, 
		"staticmc2.setMask(staticmc3);"
		"staticmc5.setMask(staticmc4);"
		"dynamicmc2.setMask(dynamicmc3);"
		"dynamicmc5.setMask(dynamicmc4);"
		);

	SWFMovie_nextFrame(mo);        

	add_actions(mo, "note('Swapping chars 2/3 and 4/5 to see if masks are still in effect');");

	add_actions(mo,
		"staticmc2.swapDepths(staticmc3);"
		"staticmc4.swapDepths(staticmc5);"
		"dynamicmc2.swapDepths(dynamicmc3);"
		"dynamicmc4.swapDepths(dynamicmc5);"
		);

	check_equals(mo, "staticmc2.getDepth()", "-16381");
	check_equals(mo, "staticmc3.getDepth()", "-16382");
	check_equals(mo, "staticmc4.getDepth()", "-16379"); 
	check_equals(mo, "staticmc5.getDepth()", "-16380"); 
	check_equals(mo, "dynamicmc2.getDepth()", "13");
	check_equals(mo, "dynamicmc3.getDepth()", "12");
	check_equals(mo, "dynamicmc4.getDepth()", "15"); 
	check_equals(mo, "dynamicmc5.getDepth()", "14"); 


	add_actions(mo, "_root.totals(); stop();");

	SWFMovie_nextFrame(mo);        

	// TODO:
	// - test mask layers !!

	//Output movie
	puts("Saving " OUTPUT_FILENAME );
	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
