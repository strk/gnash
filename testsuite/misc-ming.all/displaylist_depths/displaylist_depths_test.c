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
			"}"
			"%s.createEmptyMovieClip('child', 1);"
			"with (%s.child) {"
			" lineStyle(3, 0x000000, 100);"	
			" moveTo(%d, %d);"
			" lineTo(%d, %d);"
			" lineTo(%d, %d);"
			" lineTo(%d, %d);"
			" lineTo(%d, %d);"
			"}",
			name, depth, name,
			x, y,
			x, y+height,
			x+width, y+height,
			x+width, y,
			x, y,
			name, name,
			x+5, y+5,
			x+5, y+height-5,
			x+width-5, y+height-5,
			x+width-5, y+5,
			x+5, y+5
			);

	SWFMovie_add(mo, (SWFBlock)ac);
}

void
add_static_mc(SWFMovie mo, const char* name, int depth, int x, int y, int width, int height)
{
	SWFShape sh;
	SWFMovieClip mc, mc2;
	SWFDisplayItem it;

	sh = make_fill_square (-(width/2), -(height/2), width, height, 255, 0, 0, 255, 0, 0);
	mc = newSWFMovieClip();
	SWFMovieClip_add(mc, (SWFBlock)sh);

	sh = make_square (-(width/2)+5, -(height/2)+5, width-10, height-10, 0, 0, 0);
	mc2 = newSWFMovieClip(); // child
	SWFMovieClip_add(mc2, (SWFBlock)sh);
	SWFMovieClip_nextFrame(mc2);

	it = SWFMovieClip_add(mc, (SWFBlock)mc2);
	SWFDisplayItem_setName(it, "child");
	SWFMovieClip_nextFrame(mc);

	it = SWFMovie_add(mo, (SWFBlock)mc);
	SWFDisplayItem_setDepth(it, depth); 
	SWFDisplayItem_moveTo(it, x, y); 
	//SWFDisplayItem_moveTo(it, -(width/2), -(height/2)); 
	SWFDisplayItem_setName(it, name);
	SWFDisplayItem_addAction(it, newSWFAction("this._rotation+=2;"), SWFACTION_ENTERFRAME);
	SWFDisplayItem_addAction(it, newSWFAction("this._y+=5;"), SWFACTION_ENTERFRAME);

	//SWFMovie_add(mo, (SWFBlock)mc);
}


int
main(int argc, char** argv)
{
	SWFMovie mo;
	SWFMovieClip dejagnuclip;
	int i;

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

	for (i=0; i<10; ++i) SWFMovie_nextFrame(mo); 

	// Duplicate all sprites
	add_actions(mo,
		"duplicateMovieClip(staticmc, 'staticmc_dup', 2000); staticmc_dup._x += 300;"
		"duplicateMovieClip(dynamicmc, 'dynamicmc_dup', -2001); dynamicmc_dup._x += 300;"
		"duplicateMovieClip(dynamicmc_2000, 'dynamicmc_2000_dup', 2002); dynamicmc_2000_dup._x += 300;"
		"duplicateMovieClip(dynamicmc_30000, 'dynamicmc_30000_dup', 2003); dynamicmc_30000_dup._x += 300;"
		"duplicateMovieClip(dynamicmc0, 'dynamicmc0_dup', -2004); dynamicmc0_dup._x += 300;"
		"duplicateMovieClip(dynamicmc1048575, 'dynamicmc1048575_dup', -2005); dynamicmc1048575_dup._x += 300;"
		"duplicateMovieClip(dynamicmc1048576, 'dynamicmc1048576_dup', 2006); dynamicmc1048576_dup._x += 300;"
		"duplicateMovieClip(dynamicmc2130690045, 'dynamicmc2130690045_dup', 2007); dynamicmc2130690045_dup._x += 300;"
		// This is just to test that the next placement on depth 2008 will override it
		"duplicateMovieClip(staticmc0, 'staticmc0_dup_fake', -2008);"
		"duplicateMovieClip(staticmc0, 'staticmc0_dup', -2008); staticmc0_dup._x += 300;");

	// Check that all depths have been duplicated 
	check_equals(mo, "typeof(staticmc_dup)", "'movieclip'");
	check_equals(mo, "staticmc_dup.getDepth()", "2000"); 
	check_equals(mo, "typeof(dynamicmc_dup)", "'movieclip'");
	check_equals(mo, "dynamicmc_dup.getDepth()", "-2001");
	check_equals(mo, "typeof(dynamicmc_2000_dup)", "'movieclip'");
	check_equals(mo, "dynamicmc_2000_dup.getDepth()", "2002");
	check_equals(mo, "typeof(dynamicmc_30000_dup)", "'movieclip'");
	check_equals(mo, "dynamicmc_30000_dup.getDepth()", "2003");
	check_equals(mo, "typeof(dynamicmc0_dup)", "'movieclip'");
	check_equals(mo, "dynamicmc0_dup.getDepth()", "-2004");
	check_equals(mo, "typeof(dynamicmc1048575_dup)", "'movieclip'");
	check_equals(mo, "dynamicmc1048575_dup.getDepth()", "-2005");
	check_equals(mo, "typeof(dynamicmc1048576_dup)", "'movieclip'");
	check_equals(mo, "dynamicmc1048576_dup.getDepth()", "2006");
	check_equals(mo, "typeof(dynamicmc2130690045_dup)", "'movieclip'");
	check_equals(mo, "dynamicmc2130690045_dup.getDepth()", "2007");
	check_equals(mo, "typeof(staticmc0_dup_fake)", "'undefined'");
	check_equals(mo, "typeof(staticmc0_dup)", "'movieclip'");
	check_equals(mo, "staticmc0_dup.getDepth()", "-2008"); 

	// Check that duplicated stuff got also drawing api duplication
	// And (but only for the static case) child duplication

	check_equals(mo, "staticmc_dup._width", "staticmc._width"); 
	check_equals(mo, "parseInt(staticmc_dup._width/10)", "7"); 
	check_equals(mo, "typeof(staticmc.child)", "'movieclip'"); 
	check_equals(mo, "typeof(staticmc_dup.child)", "'movieclip'"); 

	// Note that dynamicmc_dup is at negative depth
	check_equals(mo, "dynamicmc_dup._width", "dynamicmc._width"); 
	check_equals(mo, "parseInt(dynamicmc_dup._width/10)", "6"); 
	check_equals(mo, "typeof(dynamicmc.child)", "'movieclip'"); 
	check_equals(mo, "typeof(dynamicmc_dup.child)", "'undefined'"); 

	check_equals(mo, "dynamicmc_2000_dup._width", "dynamicmc_2000._width"); 
	check_equals(mo, "parseInt(dynamicmc_2000_dup._width/10)", "6"); 
	check_equals(mo, "typeof(dynamicmc_2000.child)", "'movieclip'"); 
	check_equals(mo, "typeof(dynamicmc_2000_dup.child)", "'undefined'"); 

	check_equals(mo, "dynamicmc_30000_dup._width", "dynamicmc_30000._width"); 
	check_equals(mo, "parseInt(dynamicmc_30000_dup._width/10)", "6"); 
	check_equals(mo, "typeof(dynamicmc_30000.child)", "'movieclip'"); 
	check_equals(mo, "typeof(dynamicmc_30000_dup.child)", "'undefined'"); 

	// Note that dynamicmc0_dup is at negative depth
	check_equals(mo, "dynamicmc0_dup._width", "dynamicmc0._width"); 
	check_equals(mo, "parseInt(dynamicmc0_dup._width/10)", "6"); 
	check_equals(mo, "typeof(dynamicmc0.child)", "'movieclip'"); 
	check_equals(mo, "typeof(dynamicmc0_dup.child)", "'undefined'"); 

	// Note that dynamicmc1048575_dup is at negative depth
	check_equals(mo, "dynamicmc1048575_dup._width", "dynamicmc1048575._width"); 
	check_equals(mo, "parseInt(dynamicmc1048575_dup._width/10)", "6"); 
	check_equals(mo, "typeof(dynamicmc1048575.child)", "'movieclip'"); 
	check_equals(mo, "typeof(dynamicmc1048575_dup.child)", "'undefined'"); 

	check_equals(mo, "dynamicmc1048576_dup._width", "dynamicmc1048576._width"); 
	check_equals(mo, "parseInt(dynamicmc1048576._width/10)", "6"); 
	check_equals(mo, "typeof(dynamicmc1048576.child)", "'movieclip'"); 
	check_equals(mo, "typeof(dynamicmc1048576_dup.child)", "'undefined'"); 

	check_equals(mo, "dynamicmc2130690045_dup._width", "dynamicmc2130690045._width"); 
	check_equals(mo, "parseInt(dynamicmc2130690045._width/10)", "6"); 
	check_equals(mo, "typeof(dynamicmc2130690045.child)", "'movieclip'"); 
	check_equals(mo, "typeof(dynamicmc2130690045_dup.child)", "'undefined'"); 

	// Note that staticmc0_dup is at negative depth
	check_equals(mo, "staticmc0_dup._width", "staticmc0._width"); 
	check_equals(mo, "parseInt(staticmc0._width/10)", "7"); 
	check_equals(mo, "typeof(staticmc0.child)", "'movieclip'"); 
	check_equals(mo, "typeof(staticmc0_dup.child)", "'movieclip'"); 

	for (i=0; i<10; ++i) SWFMovie_nextFrame(mo); 

	// Try removing all DisplayObjects
	add_actions(mo, "removeMovieClip(staticmc);"
			"removeMovieClip(dynamicmc);"
			"removeMovieClip(dynamicmc_2000);"
			"removeMovieClip(dynamicmc_30000);"
			"removeMovieClip(dynamicmc0);"
			"removeMovieClip(dynamicmc1048575);"
			"removeMovieClip(dynamicmc1048576);"
			"removeMovieClip(dynamicmc2130690045);"
			"removeMovieClip(staticmc0);"
			"removeMovieClip(staticmc_dup);"
			"removeMovieClip(dynamicmc_dup);"
			"removeMovieClip(dynamicmc_2000_dup);"
			"removeMovieClip(dynamicmc_30000_dup);"
			"removeMovieClip(dynamicmc0_dup);"
			"removeMovieClip(dynamicmc1048575_dup);"
			"removeMovieClip(dynamicmc1048576_dup);"
			"removeMovieClip(dynamicmc2130690045_dup);"
			"removeMovieClip(staticmc0_dup);"
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

	// These are duplicated clips
	check_equals(mo, "typeof(staticmc_dup)", "'undefined'");
	check_equals(mo, "typeof(dynamicmc_dup)", "'movieclip'");
	check_equals(mo, "typeof(dynamicmc_2000_dup)", "'undefined'"); 
	check_equals(mo, "typeof(dynamicmc_30000_dup)", "'undefined'"); 
	check_equals(mo, "typeof(dynamicmc0_dup)", "'movieclip'");
	check_equals(mo, "typeof(dynamicmc1048575_dup)", "'movieclip'");
	check_equals(mo, "typeof(dynamicmc1048576_dup)", "'undefined'"); 
	check_equals(mo, "typeof(dynamicmc2130690045_dup)", "'undefined'"); 
	check_equals(mo, "typeof(staticmc0_dup)", "'movieclip'");

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
	// - test the MovieClip version of removeMovieClip !

	//Output movie
	puts("Saving " OUTPUT_FILENAME );
	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
