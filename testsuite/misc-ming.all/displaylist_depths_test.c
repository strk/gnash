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
 * Test if dynamic and static objects at the same depth can coexist
 *
 * run as ./displaylist_depths_test
 */

#include "ming_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "displaylist_depths_test.swf"

void add_dynamic_mc(SWFMovie mo, const char* name, int depth, int x, int y, int width, int height);
void add_static_mc(SWFMovie mo, const char* name, int depth, int x, int y, int width, int height);

void
add_dynamic_mc(SWFMovie mo, const char* name, int depth, int x, int y, int width, int height)
{
	SWFAction ac = compile_actions("createEmptyMovieClip('%s', %d);"
			"with (%s) {"
			" lineStyle(1, 0x000000, 100);"	
			" beginFill(0x00FF00, 100);"
			" moveTo(%d, %d);"
			" lineTo(%d, %d);"
			" lineTo(%d, %d);"
			" lineTo(%d, %d);"
			" lineTo(%d, %d);"
			" endFill();"
			"}",
			name, depth, name,
			x, y,
			x, y+height,
			x+width, y+height,
			x+width, y,
			x, y);

	SWFMovie_add(mo, (SWFBlock)ac);
}

void
add_static_mc(SWFMovie mo, const char* name, int depth, int x, int y, int width, int height)
{
	SWFShape sh;
	SWFMovieClip mc;
	SWFDisplayItem it;

	sh = make_fill_square (x, y, width, height, 255, 0, 0, 255, 0, 0);
	mc = newSWFMovieClip();
	SWFMovieClip_add(mc, (SWFBlock)sh);
	SWFMovieClip_nextFrame(mc);
	it = SWFMovie_add(mo, (SWFBlock)mc);
	SWFDisplayItem_setDepth(it, depth); 
	SWFDisplayItem_setName(it, name);

	SWFMovie_add(mo, (SWFBlock)mc);
}


int
main(int argc, char** argv)
{
	SWFMovie mo;
	SWFMovieClip dejagnuclip;

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

	add_actions(mo, "note('Test placement of static and dynamic objects at different depth ranges.');");
	add_actions(mo, "note('Characters placed in *reserved* zones will not be removed');");
	add_actions(mo, "note('');");
	add_actions(mo, "note('WARNING: Adobe flash player 9 seems to fail this test, but flash player 7 succeeds.');");
	add_actions(mo, "note('         Remember this before blaming Gnash for breaking compatibility ;)');");
	add_actions(mo, "note('');");

	// Add a static red square at depth 3 and another one at depth 4
	add_static_mc(mo, "staticmc", 3, 0, 300, 60, 60);

	// Create a dynamic green movieclip at depth 3
	add_dynamic_mc(mo, "dynamicmc", 3, 20, 320, 60, 60);

	// Create a dynamic green movieclip at depth -2000 (just below the static one)
	// The renderer seems to be still rendering this ABOVE the static one !
	add_dynamic_mc(mo, "dynamicmc_2000", -2000, 20, 280, 60, 60);

	// Create a dynamic green movieclip at depth -30000 (below the static range)
	add_dynamic_mc(mo, "dynamicmc_30000", -30000, 20, 280, 60, 60);

	// Create a dynamic green movieclip at depth 0 (first in "dynamic" zone)
	add_dynamic_mc(mo, "dynamicmc0", 0, 90, 320, 60, 60);

	// Create a dynamic green movieclip at depth 1048575 (last in "dynamic" zone)
	add_dynamic_mc(mo, "dynamicmc1048575", 1048575, 160, 320, 60, 60);

	// Create a dynamic green movieclip at depth 1048576 (first in "reserve" zone)
	add_dynamic_mc(mo, "dynamicmc1048576", 1048576, 0, 390, 60, 60);

	// Create a dynamic green movieclip at depth 2130690045 (last in "reserve" zone)
	// This seems to create the empty clip, but NOT render anything !
	add_dynamic_mc(mo, "dynamicmc2130690045", 2130690045, 0, 390, 60, 60);
	
	// Create a static red movieclip at depth 0 
	add_static_mc(mo, "staticmc0", 0, 0, 230, 60, 60);

	// Check what depth has been each char created in
	check_equals(mo, "typeof(staticmc)", "'movieclip'");
	check_equals(mo, "staticmc.getDepth()", "-16381"); // converted at negative depth !
	check_equals(mo, "typeof(dynamicmc)", "'movieclip'");
	check_equals(mo, "dynamicmc.getDepth()", "3");
	check_equals(mo, "typeof(dynamicmc_2000)", "'movieclip'");
	check_equals(mo, "dynamicmc_2000.getDepth()", "-2000");
	check_equals(mo, "typeof(dynamicmc_30000)", "'movieclip'");
	check_equals(mo, "dynamicmc_30000.getDepth()", "-30000");
	check_equals(mo, "typeof(dynamicmc0)", "'movieclip'");
	check_equals(mo, "dynamicmc0.getDepth()", "0");
	check_equals(mo, "typeof(dynamicmc1048575)", "'movieclip'");
	check_equals(mo, "dynamicmc1048575.getDepth()", "1048575");
	check_equals(mo, "typeof(dynamicmc1048576)", "'movieclip'");
	check_equals(mo, "dynamicmc1048576.getDepth()", "1048576");
	check_equals(mo, "typeof(dynamicmc2130690045)", "'movieclip'");
	check_equals(mo, "dynamicmc2130690045.getDepth()", "2130690045");
	check_equals(mo, "typeof(staticmc0)", "'movieclip'");
	check_equals(mo, "staticmc0.getDepth()", "-16384"); // converted at negative depth !

	SWFMovie_nextFrame(mo); 

	// Try removing all characters
	add_actions(mo, "removeMovieClip(staticmc);"
			"removeMovieClip(dynamicmc);"
			"removeMovieClip(dynamicmc_2000);"
			"removeMovieClip(dynamicmc_30000);"
			"removeMovieClip(dynamicmc0);"
			"removeMovieClip(dynamicmc1048575);"
			"removeMovieClip(dynamicmc1048576);"
			"removeMovieClip(dynamicmc2130690045);"
			"removeMovieClip(staticmc0);"
			);

	// Check what gets removed and what not
	check_equals(mo, "typeof(staticmc)", "'movieclip'");
	check_equals(mo, "typeof(dynamicmc)", "'undefined'");
	check_equals(mo, "typeof(dynamicmc_2000)", "'movieclip'"); // clip at negative depth is not removed
	check_equals(mo, "typeof(dynamicmc_30000)", "'movieclip'"); // clip at negative depth is not removed
	check_equals(mo, "typeof(dynamicmc0)", "'undefined'");
	check_equals(mo, "typeof(dynamicmc1048575)", "'undefined'");
	check_equals(mo, "typeof(dynamicmc1048576)", "'movieclip'"); // clip in "reserved" zone not removed
	check_equals(mo, "typeof(dynamicmc2130690045)", "'movieclip'"); // clip in "reserved" zone not removed
	check_equals(mo, "typeof(staticmc0)", "'movieclip'");

	SWFMovie_nextFrame(mo);

	// Move all non-removed chars to the "dynamic" depth range
	// and try removing them again.
	add_actions(mo, "staticmc.swapDepths(1000);"
			"dynamicmc_2000.swapDepths(1001);"
			"dynamicmc1048576.swapDepths(1002);"
			"dynamicmc2130690045.swapDepths(1003);"
			"staticmc0.swapDepths(1004);"
			"dynamicmc_30000.swapDepths(1005);"
			);

	check_equals(mo, "staticmc.getDepth()", "1000"); 
	check_equals(mo, "dynamicmc_2000.getDepth()", "1001");
	check_equals(mo, "dynamicmc1048576.getDepth()", "1002");
	// MM bug: swapDepths aginst this char (in reserved zone) doesn't do
	//         anything with player9. It works fine with player7.
	check_equals(mo, "dynamicmc2130690045.getDepth()", "1003");
	check_equals(mo, "staticmc0.getDepth()", "1004"); 
	// swapDepth doesn't work for the clip at -30000 !!
	check_equals(mo, "dynamicmc_30000.getDepth()", "-30000");

	add_actions(mo, "removeMovieClip(staticmc);"
			"removeMovieClip(dynamicmc_2000);"
			"removeMovieClip(dynamicmc1048576);"
			"removeMovieClip(dynamicmc2130690045);"
			"removeMovieClip(staticmc0);"
			);

	// Check if we cleaned them all now
	check_equals(mo, "typeof(staticmc)", "'undefined'");
	check_equals(mo, "typeof(dynamicmc_2000)", "'undefined'"); 
	check_equals(mo, "typeof(dynamicmc1048576)", "'undefined'"); 
	// MM bug: swapDepths against this char (in reserved zone) didn't do
	//         anything with player9, so the removeMovieClip call against
	//         it still doesn't work. It works fine with player7.
	check_equals(mo, "typeof(dynamicmc2130690045)", "'undefined'"); 
	// MM bug?: altought the staticmc0 results undefined after the call
	//          to removeMovieClip, it is sill rendered !!
	check_equals(mo, "typeof(staticmc0)", "'undefined'");

	add_actions(mo, "_root.totals(); stop();");
	SWFMovie_nextFrame(mo);        

	// TODO:
	// - test the MovieClip and global version of removeMovieClip !

	//Output movie
	puts("Saving " OUTPUT_FILENAME );
	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
