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
 * Test extracting _width, _height, _xscale, _yscale, _x, _y and _rotation
 * from hard-coded matrices.
 *
 * run as ./matrix_test
 */

#include "ming_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "matrix_test.swf"

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
	SWFDisplayItem_setName(it, name);

	return it;
}


int
main(int argc, char** argv)
{
	SWFMovie mo;
	SWFMovieClip dejagnuclip;
	SWFDisplayItem it;
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

	add_actions(mo, "printBounds = function(b) { return '' + b.xMin + ','+ b.yMin + ' '+ b.xMax + ',' + b.yMax; };");

	SWFMovie_nextFrame(mo); 


	// Static 60x60 movieclip 
	it = add_static_mc(mo, "staticmc", 3, 50, 300, 60, 60);

	// Check if we cleaned them all now
	check_equals(mo, "typeof(staticmc)", "'movieclip'");
	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "staticmc._rotation", "0");
	check_equals(mo, "staticmc._width", "60.1");
	check_equals(mo, "staticmc._height", "60.1");

	SWFMovie_nextFrame(mo);        

	SWFDisplayItem_rotateTo(it, -45);

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "100");
	check_equals(mo, "Math.round(staticmc._yscale)", "100");
	check_equals(mo, "Math.round(staticmc._rotation*100)", "4500");
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'7.5,257.5 92.5,342.5'");
	check_equals(mo, "staticmc._width", "85");
	check_equals(mo, "staticmc._height", "85");

	SWFMovie_nextFrame(mo);        

	SWFDisplayItem_scaleTo(it, 2, 3);

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "200");
	check_equals(mo, "Math.round(staticmc._yscale)", "300");
	check_equals(mo, "Math.round(staticmc._rotation*100)", "4500");
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-56.25,193.75 156.25,406.25'");
	check_equals(mo, "staticmc._width", "212.5");
	check_equals(mo, "staticmc._height", "212.5");

	SWFMovie_nextFrame(mo);        

	SWFDisplayItem_skewXTo(it, 2);

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "447");
	check_equals(mo, "Math.round(staticmc._yscale)", "300");
	check_equals(mo, "Math.round(staticmc._rotation*1000)", "18435");
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-141.25,193.75 241.25,406.25'");
	check_equals(mo, "staticmc._width", "382.5");
	check_equals(mo, "staticmc._height", "212.5");

	SWFMovie_nextFrame(mo);        

	SWFDisplayItem_scaleTo(it, 2, -3);

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "447");
	check_equals(mo, "Math.round(staticmc._yscale)", "300");
	check_equals(mo, "Math.round(staticmc._rotation*1000)", "18435");
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-141.25,193.75 241.25,406.25'");
	check_equals(mo, "staticmc._width", "382.5");
	check_equals(mo, "staticmc._height", "212.5");

	SWFMovie_nextFrame(mo);        

	SWFDisplayItem_skewXTo(it, 0);

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "200");
	check_equals(mo, "Math.round(staticmc._yscale)", "300");
	check_equals(mo, "Math.round(staticmc._rotation*1000)", "45000");
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-56.25,193.75 156.25,406.25'");
	check_equals(mo, "staticmc._width", "212.5");
	check_equals(mo, "staticmc._height", "212.5");

	SWFMovie_nextFrame(mo);        

	SWFDisplayItem_scaleTo(it, -2, 2);

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "200");
	check_equals(mo, "Math.round(staticmc._yscale)", "200");
	check_equals(mo, "Math.round(staticmc._rotation*1000)", "-135000");
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-35,215 135,385'");
	check_equals(mo, "staticmc._width", "170");
	check_equals(mo, "staticmc._height", "170");

	SWFMovie_nextFrame(mo);        

	SWFDisplayItem_scaleTo(it, 1, 1);
	SWFDisplayItem_skewXTo(it, -2);

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "100");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	xcheck_equals(mo, "Math.round(staticmc._rotation*1000)", "135000"); // gnash gives 135001
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-35,257.5 135,342.5'");
	check_equals(mo, "staticmc._width", "170");
	check_equals(mo, "staticmc._height", "85");

	SWFMovie_nextFrame(mo);        
	SWFDisplayItem_setMatrix(it, 1, 0, 0, 1, 50, 300); // reset to near-identity

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "100");
	check_equals(mo, "Math.round(staticmc._yscale)", "100");
	check_equals(mo, "Math.round(staticmc._rotation*1000)", "0");
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'19.95,269.95 80.05,330.05'");
	check_equals(mo, "staticmc._width", "60.1");
	check_equals(mo, "staticmc._height", "60.1");

	SWFMovie_nextFrame(mo);        
	SWFDisplayItem_setMatrix(it, -1, 0, 0, 1, 50, 300); // negative x scale gets interpreted as 180 degrees rotation

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "100");
	check_equals(mo, "Math.round(staticmc._yscale)", "100");
	check_equals(mo, "Math.round(staticmc._rotation*1000)", "180000"); // x scale -1 gets interpreted as a 180 degree rotation
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'19.95,269.95 80.05,330.05'");
	check_equals(mo, "staticmc._width", "60.1");
	check_equals(mo, "staticmc._height", "60.1");

	SWFMovie_nextFrame(mo);        
	SWFDisplayItem_setMatrix(it, 1, 0, 0, -1, 50, 300); // negative y scale (discarded ?)

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "100");
	check_equals(mo, "Math.round(staticmc._yscale)", "100");
	check_equals(mo, "Math.round(staticmc._rotation*1000)", "0"); 
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'19.95,269.95 80.05,330.05'");
	check_equals(mo, "staticmc._width", "60.1");
	check_equals(mo, "staticmc._height", "60.1");


	// TODO:
	// - test more rotations and scales (corner cases too!)
	// - test 'skew' (since Ming supports it)

	SWFMovie_nextFrame(mo);

	add_actions(mo, "_root.totals(98); stop();");
	SWFMovie_nextFrame(mo);        

	//Output movie
	puts("Saving " OUTPUT_FILENAME );
	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
