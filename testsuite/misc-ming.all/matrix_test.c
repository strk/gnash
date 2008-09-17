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
	//SWFMovie_setRate (mo, 2);
	SWFMovie_setRate (mo, 12);

	dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
	SWFMovie_add(mo, (SWFBlock)dejagnuclip);

	add_actions(mo, "printBounds = function(b, roundToDecimal) "
            "{ "
            "   if ( roundToDecimal != undefined ) {"
            "       var round = Math.pow(10, roundToDecimal);"
            //"       trace('rounding to '+round);"
            "       return '' + Math.round(b.xMin*round)/round + ','+ Math.round(b.yMin*round)/round + ' '+ Math.round(b.xMax*round)/round + ',' + Math.round(b.yMax*round)/round; "
            "   } else {"
            "       return '' + b.xMin + ','+ b.yMin + ' '+ b.xMax + ',' + b.yMax; "
            "   }"
            "};"
            );

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

	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'19.95,269.95 80.05,330.05'");

	// X:  19.95 ..  80.05
	// Y: 269.95 .. 330.05

	check(mo, "staticmc.hitTest(21, 270, false)");  // top-left
	check(mo, "staticmc.hitTest(79, 270, false)");  // top-right
	check(mo, "staticmc.hitTest(21, 329, false)");  // bottom-left
	check(mo, "staticmc.hitTest(79, 329, false)");  // bottom-right

	check(mo, "!staticmc.hitTest(18, 300, false)");  // overleft
	check(mo, "!staticmc.hitTest(82, 300, false)");  // overright
	check(mo, "!staticmc.hitTest(50, 260, false)");  // overup
	check(mo, "!staticmc.hitTest(50, 331, false)");  // overdown

	SWFMovie_nextFrame(mo);        

	SWFDisplayItem_rotateTo(it, -45);

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "100");
	check_equals(mo, "Math.round(staticmc._yscale)", "100");
	check_equals(mo, "staticmc._rotation", "45");
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'7.5,257.5 92.5,342.5'");
	check_equals(mo, "staticmc._width", "85");
	check_equals(mo, "staticmc._height", "85");

	// X:   7.5  ..  92.5 
	// Y: 257.5  .. 342.5

	check(mo, "staticmc.hitTest(8, 258, false)");   // top-left
	check(mo, "staticmc.hitTest(90, 258, false)");  // top-right
	check(mo, "staticmc.hitTest(8, 340, false)");   // bottom-left
	check(mo, "staticmc.hitTest(90, 340, false)");  // bottom-right

	check(mo, "!staticmc.hitTest(6, 300, false)");   // overleft
	check(mo, "!staticmc.hitTest(95, 300, false)");  // overright
	check(mo, "!staticmc.hitTest(50, 250, false)");  // overup
	check(mo, "!staticmc.hitTest(50, 345, false)");  // overdown


	SWFMovie_nextFrame(mo);        

	SWFDisplayItem_scaleTo(it, 2, 3);

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "200");
	check_equals(mo, "Math.round(staticmc._yscale)", "300");
	check_equals(mo, "staticmc._rotation", "45");
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-56.25,193.75 156.25,406.25'");
	check_equals(mo, "staticmc._width", "212.5");
	check_equals(mo, "staticmc._height", "212.5");

	// X: -56.25  156.25
	// Y: 193.75  406.25

	check(mo, "staticmc.hitTest(-56, 194, false)");   // top-left
	check(mo, "staticmc.hitTest(156, 194, false)");  // top-right
	check(mo, "staticmc.hitTest(-56, 406, false)");   // bottom-left
	check(mo, "staticmc.hitTest(156, 406, false)");  // bottom-right

	check(mo, "!staticmc.hitTest(-57, 300, false)");   // overleft
	check(mo, "!staticmc.hitTest(157, 300, false)");  // overright
	check(mo, "!staticmc.hitTest(50, 193, false)");  // overup
	check(mo, "!staticmc.hitTest(50, 407, false)");  // overdown

	SWFMovie_nextFrame(mo);        

	SWFDisplayItem_skewXTo(it, 2);

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "447");
	check_equals(mo, "Math.round(staticmc._yscale)", "300");
	check_equals(mo, "Math.round(staticmc._rotation*1000)", "18435"); // 18.43469...
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-141.25,193.75 241.25,406.25'");
	check_equals(mo, "staticmc._width", "382.5");
	check_equals(mo, "staticmc._height", "212.5");

	// X: -141.25  241.25
	// Y:  193.75  406.25

	check(mo, "staticmc.hitTest(-141, 194, false)");   // top-left
	check(mo, "staticmc.hitTest(241, 194, false)");  // top-right
	check(mo, "staticmc.hitTest(-141, 406, false)");   // bottom-left
	check(mo, "staticmc.hitTest(241, 406, false)");  // bottom-right

	check(mo, "!staticmc.hitTest(-142, 300, false)");   // overleft
	check(mo, "!staticmc.hitTest(242, 300, false)");  // overright
	check(mo, "!staticmc.hitTest(50, 193, false)");  // overup
	check(mo, "!staticmc.hitTest(50, 407, false)");  // overdown

	SWFMovie_nextFrame(mo);        

	SWFDisplayItem_scaleTo(it, 2, -3);

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "447");
	check_equals(mo, "Math.round(staticmc._yscale)", "300");
	check_equals(mo, "Math.round(staticmc._rotation*1000)", "18435"); // 18.43469...
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-141.25,193.75 241.25,406.25'");
	check_equals(mo, "staticmc._width", "382.5");
	check_equals(mo, "staticmc._height", "212.5");

	// X: -141.25  241.25
	// Y:  193.75  406.25

	check(mo, "staticmc.hitTest(-141, 194, false)");   // top-left
	check(mo, "staticmc.hitTest(241, 194, false)");  // top-right
	check(mo, "staticmc.hitTest(-141, 406, false)");   // bottom-left
	check(mo, "staticmc.hitTest(241, 406, false)");  // bottom-right

	check(mo, "!staticmc.hitTest(-142, 300, false)");   // overleft
	check(mo, "!staticmc.hitTest(242, 300, false)");  // overright
	check(mo, "!staticmc.hitTest(50, 193, false)");  // overup
	check(mo, "!staticmc.hitTest(50, 407, false)");  // overdown

	SWFMovie_nextFrame(mo);        

	SWFDisplayItem_skewXTo(it, 0);

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "200");
	check_equals(mo, "Math.round(staticmc._yscale)", "300");
	check_equals(mo, "staticmc._rotation", "45");
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-56.25,193.75 156.25,406.25'");
	check_equals(mo, "staticmc._width", "212.5");
	check_equals(mo, "staticmc._height", "212.5");

	// X: -56.25  156.25
	// Y:  193.75  406.25

	check(mo, "staticmc.hitTest(-56, 194, false)");   // top-left
	check(mo, "staticmc.hitTest(156, 194, false)");  // top-right
	check(mo, "staticmc.hitTest(-56, 406, false)");   // bottom-left
	check(mo, "staticmc.hitTest(156, 406, false)");  // bottom-right

	check(mo, "!staticmc.hitTest(-57, 300, false)");   // overleft
	check(mo, "!staticmc.hitTest(157, 300, false)");  // overright
	check(mo, "!staticmc.hitTest(50, 193, false)");  // overup
	check(mo, "!staticmc.hitTest(50, 407, false)");  // overdown


	SWFMovie_nextFrame(mo);        

	SWFDisplayItem_scaleTo(it, -2, 2);

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "200");
	check_equals(mo, "Math.round(staticmc._yscale)", "200");
	check_equals(mo, "staticmc._rotation", "-135");
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-35,215 135,385'");
	check_equals(mo, "staticmc._width", "170");
	check_equals(mo, "staticmc._height", "170");

	// X:  -35     135   
	// Y:  215     385

	check(mo, "staticmc.hitTest(-34, 216, false)");   // top-left
	check(mo, "staticmc.hitTest(134, 216, false)");  // top-right
	check(mo, "staticmc.hitTest(-34, 384, false)");   // bottom-left
	check(mo, "staticmc.hitTest(134, 384, false)");  // bottom-right

	check(mo, "!staticmc.hitTest(-36, 300, false)");   // overleft
	check(mo, "!staticmc.hitTest(136, 300, false)");  // overright
	check(mo, "!staticmc.hitTest(50, 214, false)");  // overup
	check(mo, "!staticmc.hitTest(50, 386, false)");  // overdown

	SWFMovie_nextFrame(mo);        

	SWFDisplayItem_scaleTo(it, 1, 1);
	SWFDisplayItem_skewXTo(it, -2);

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "100"); //  non-rounded value differs with Gnash
	check_equals(mo, "Math.round(staticmc._yscale)", "224"); //  non-rounded value differs with Gnash
	check_equals(mo, "Math.round(staticmc._rotation)", "135"); // non-rounded value differs with Gnash
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-35,257.5 135,342.5'");
	check_equals(mo, "staticmc._width", "170");
	check_equals(mo, "staticmc._height", "85");

	// X:  -35     135   
	// Y:  257.5   342.5

	check(mo, "staticmc.hitTest(-34, 258, false)");   // top-left
	check(mo, "staticmc.hitTest(134, 258, false)");  // top-right
	check(mo, "staticmc.hitTest(-34, 342, false)");   // bottom-left
	check(mo, "staticmc.hitTest(134, 342, false)");  // bottom-right

	check(mo, "!staticmc.hitTest(-36, 300, false)");   // overleft
	check(mo, "!staticmc.hitTest(136, 300, false)");  // overright
	check(mo, "!staticmc.hitTest(50, 257, false)");  // overup
	check(mo, "!staticmc.hitTest(50, 343, false)");  // overdown

	SWFMovie_nextFrame(mo);        
	SWFDisplayItem_setMatrix(it, 1, 0, 0, 1, 50, 300); // reset to near-identity

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "staticmc._rotation", "0");
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'19.95,269.95 80.05,330.05'");
	check_equals(mo, "staticmc._width", "60.1");
	check_equals(mo, "staticmc._height", "60.1");

	// X:   19.95   80.05
	// Y:  269.95  330.05

	check(mo, "staticmc.hitTest(20, 270, false)");   // top-left
	check(mo, "staticmc.hitTest(80, 270, false)");  // top-right
	check(mo, "staticmc.hitTest(20, 330, false)");   // bottom-left
	check(mo, "staticmc.hitTest(80, 330, false)");  // bottom-right

	check(mo, "!staticmc.hitTest(19, 300, false)");   // overleft
	check(mo, "!staticmc.hitTest(81, 300, false)");  // overright
	check(mo, "!staticmc.hitTest(50, 269, false)");  // overup
	check(mo, "!staticmc.hitTest(50, 331, false)");  // overdown

	SWFMovie_nextFrame(mo);        
	SWFDisplayItem_setMatrix(it, -1, 0, 0, 1, 50, 300); // negative x scale gets interpreted as 180 degrees rotation

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "staticmc._rotation", "180"); // x scale -1 gets interpreted as a 180 degree rotation
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'19.95,269.95 80.05,330.05'");
	check_equals(mo, "staticmc._width", "60.1");
	check_equals(mo, "staticmc._height", "60.1");

	// X:   19.95   80.05
	// Y:  269.95  330.05

	check(mo, "staticmc.hitTest(20, 270, false)");   // top-left
	check(mo, "staticmc.hitTest(80, 270, false)");  // top-right
	check(mo, "staticmc.hitTest(20, 330, false)");   // bottom-left
	check(mo, "staticmc.hitTest(80, 330, false)");  // bottom-right

	check(mo, "!staticmc.hitTest(19, 300, false)");   // overleft
	check(mo, "!staticmc.hitTest(81, 300, false)");  // overright
	check(mo, "!staticmc.hitTest(50, 269, false)");  // overup
	check(mo, "!staticmc.hitTest(50, 331, false)");  // overdown

	SWFMovie_nextFrame(mo);        
	SWFDisplayItem_setMatrix(it, 1, 0, 0, -1, 50, 300); // negative y scale (discarded ?)

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "staticmc._rotation", "0"); 
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'19.95,269.95 80.05,330.05'");
	check_equals(mo, "staticmc._width", "60.1");
	check_equals(mo, "staticmc._height", "60.1");

	// X:   19.95   80.05
	// Y:  269.95  330.05

	check(mo, "staticmc.hitTest(20, 270, false)");   // top-left
	check(mo, "staticmc.hitTest(80, 270, false)");  // top-right
	check(mo, "staticmc.hitTest(20, 330, false)");   // bottom-left
	check(mo, "staticmc.hitTest(80, 330, false)");  // bottom-right

	check(mo, "!staticmc.hitTest(19, 300, false)");   // overleft
	check(mo, "!staticmc.hitTest(81, 300, false)");  // overright
	check(mo, "!staticmc.hitTest(50, 269, false)");  // overup
	check(mo, "!staticmc.hitTest(50, 331, false)");  // overdown

	SWFMovie_nextFrame(mo);        
	SWFDisplayItem_setMatrix(it, -1, 0, 0, -1, 50, 300); // negative x and y scales

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "staticmc._rotation", "180"); 
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'19.95,269.95 80.05,330.05'");
	check_equals(mo, "staticmc._width", "60.1");
	check_equals(mo, "staticmc._height", "60.1");

	// X:   19.95   80.05
	// Y:  269.95  330.05

	check(mo, "staticmc.hitTest(20, 270, false)");   // top-left
	check(mo, "staticmc.hitTest(80, 270, false)");  // top-right
	check(mo, "staticmc.hitTest(20, 330, false)");   // bottom-left
	check(mo, "staticmc.hitTest(80, 330, false)");  // bottom-right

	check(mo, "!staticmc.hitTest(19, 300, false)");   // overleft
	check(mo, "!staticmc.hitTest(81, 300, false)");  // overright
	check(mo, "!staticmc.hitTest(50, 269, false)");  // overup
	check(mo, "!staticmc.hitTest(50, 331, false)");  // overdown

	SWFMovie_nextFrame(mo);        
	SWFDisplayItem_setMatrix(it, -1, -45, 0, 1, 50, 300); // negative x scale and some negative skew

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "4501"); // let's tollerate precision for now
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "Math.round(staticmc._rotation)", "-91");  // let's tollerate precision for now
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'19.95,-1082.3 80.05,1682.3'");
	check_equals(mo, "staticmc._width", "60.1");
	check_equals(mo, "staticmc._height", "2764.6");

	// X:    19.95   80.05
	// Y: -1082.3  1682.3

	check(mo, "staticmc.hitTest(20, -1082, false)");   // top-left
	check(mo, "staticmc.hitTest(80, -1082, false)");  // top-right
	check(mo, "staticmc.hitTest(20, 1682, false)");   // bottom-left
	check(mo, "staticmc.hitTest(80, 1682, false)");  // bottom-right

	check(mo, "!staticmc.hitTest(19, 0, false)");   // overleft
	check(mo, "!staticmc.hitTest(81, 0, false)");  // overright
	check(mo, "!staticmc.hitTest(50, -1083, false)");  // overup
	check(mo, "!staticmc.hitTest(50, 1683, false)");  // overdown

	SWFMovie_nextFrame(mo);        
	SWFDisplayItem_setMatrix(it, -1, 45, 0, 1, 50, 300); // negative x scale and some positive skew

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "4501"); // let's tollerate precision for now
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "Math.round(staticmc._rotation)", "91");  // let's tollerate precision for now
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'19.95,-1082.3 80.05,1682.3'");
	check_equals(mo, "staticmc._width", "60.1");
	check_equals(mo, "staticmc._height", "2764.6");

	// X:    19.95   80.05
	// Y: -1082.3  1682.3

	check(mo, "staticmc.hitTest(20, -1082, false)");   // top-left
	check(mo, "staticmc.hitTest(80, -1082, false)");  // top-right
	check(mo, "staticmc.hitTest(20, 1682, false)");   // bottom-left
	check(mo, "staticmc.hitTest(80, 1682, false)");  // bottom-right

	check(mo, "!staticmc.hitTest(19, 0, false)");   // overleft
	check(mo, "!staticmc.hitTest(81, 0, false)");  // overright
	check(mo, "!staticmc.hitTest(50, -1083, false)");  // overup
	check(mo, "!staticmc.hitTest(50, 1683, false)");  // overdown

	SWFMovie_nextFrame(mo);        
	SWFDisplayItem_setMatrix(it, 1, -45, 0, -1, 50, 300); // negative x scale and some negative skew

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "4501"); // let's tollerate precision for now
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "Math.round(staticmc._rotation)", "-89");  // let's tollerate precision for now
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'19.95,-1082.3 80.05,1682.3'");
	check_equals(mo, "staticmc._width", "60.1");
	check_equals(mo, "staticmc._height", "2764.6");

	// X:    19.95   80.05
	// Y: -1082.3  1682.3

	check(mo, "staticmc.hitTest(20, -1082, false)");   // top-left
	check(mo, "staticmc.hitTest(80, -1082, false)");  // top-right
	check(mo, "staticmc.hitTest(20, 1682, false)");   // bottom-left
	check(mo, "staticmc.hitTest(80, 1682, false)");  // bottom-right

	check(mo, "!staticmc.hitTest(19, 0, false)");   // overleft
	check(mo, "!staticmc.hitTest(81, 0, false)");  // overright
	check(mo, "!staticmc.hitTest(50, -1083, false)");  // overup
	check(mo, "!staticmc.hitTest(50, 1683, false)");  // overdown

	SWFMovie_nextFrame(mo);        
	SWFDisplayItem_setMatrix(it, 1, 45, 0, -1, 50, 300); // negative x scale and some positive skew

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "4501"); // let's tollerate precision for now
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "Math.round(staticmc._rotation)", "89");  // let's tollerate precision for now
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'19.95,-1082.3 80.05,1682.3'");
	check_equals(mo, "staticmc._width", "60.1");
	check_equals(mo, "staticmc._height", "2764.6");

	// X:    19.95   80.05
	// Y: -1082.3  1682.3

	check(mo, "staticmc.hitTest(20, -1082, false)");   // top-left
	check(mo, "staticmc.hitTest(80, -1082, false)");  // top-right
	check(mo, "staticmc.hitTest(20, 1682, false)");   // bottom-left
	check(mo, "staticmc.hitTest(80, 1682, false)");  // bottom-right

	check(mo, "!staticmc.hitTest(19, 0, false)");   // overleft
	check(mo, "!staticmc.hitTest(81, 0, false)");  // overright
	check(mo, "!staticmc.hitTest(50, -1083, false)");  // overup
	check(mo, "!staticmc.hitTest(50, 1683, false)");  // overdown

	SWFMovie_nextFrame(mo);        
	SWFDisplayItem_setMatrix(it, -1, 45, 0, -1, 50, 300); // negative x scale and some positive skew

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "4501"); // let's tollerate precision for now
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "Math.round(staticmc._rotation)", "91");  // let's tollerate precision for now
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'19.95,-1082.3 80.05,1682.3'");
	check_equals(mo, "staticmc._width", "60.1");
	check_equals(mo, "staticmc._height", "2764.6");

	// X:    19.95   80.05
	// Y: -1082.3  1682.3

	check(mo, "staticmc.hitTest(20, -1082, false)");   // top-left
	check(mo, "staticmc.hitTest(80, -1082, false)");  // top-right
	check(mo, "staticmc.hitTest(20, 1682, false)");   // bottom-left
	check(mo, "staticmc.hitTest(80, 1682, false)");  // bottom-right

	check(mo, "!staticmc.hitTest(19, 0, false)");   // overleft
	check(mo, "!staticmc.hitTest(81, 0, false)");  // overright
	check(mo, "!staticmc.hitTest(50, -1083, false)");  // overup
	check(mo, "!staticmc.hitTest(50, 1683, false)");  // overdown

	SWFMovie_nextFrame(mo);        
	SWFDisplayItem_setMatrix(it, -1, -45, 0, -1, 50, 300); // negative x scale and some positive skew

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "4501"); // let's tollerate precision for now
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "Math.round(staticmc._rotation)", "-91");  // let's tollerate precision for now
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'19.95,-1082.3 80.05,1682.3'");
	check_equals(mo, "staticmc._width", "60.1");
	check_equals(mo, "staticmc._height", "2764.6");

	// X:    19.95   80.05
	// Y: -1082.3  1682.3

	check(mo, "staticmc.hitTest(20, -1082, false)");   // top-left
	check(mo, "staticmc.hitTest(80, -1082, false)");  // top-right
	check(mo, "staticmc.hitTest(20, 1682, false)");   // bottom-left
	check(mo, "staticmc.hitTest(80, 1682, false)");  // bottom-right

	check(mo, "!staticmc.hitTest(19, 0, false)");   // overleft
	check(mo, "!staticmc.hitTest(81, 0, false)");  // overright
	check(mo, "!staticmc.hitTest(50, -1083, false)");  // overup
	check(mo, "!staticmc.hitTest(50, 1683, false)");  // overdown


	SWFMovie_nextFrame(mo);        
	SWFDisplayItem_setMatrix(it, -1, 0, -45, 1, 50, 300); // negative x scale and some negative skew

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "Math.round(staticmc._yscale)", "4501"); // let's tollerate precision for now
	check_equals(mo, "staticmc._rotation", "180");  
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-1332.3,269.95 1432.3,330.05'");
	check_equals(mo, "staticmc._width", "2764.6");
	check_equals(mo, "staticmc._height", "60.1");

	// X: -1332.3  1432.3 
	// Y:   269.95  330.05

	check(mo, "staticmc.hitTest(-1332, 270, false)");   // top-left
	check(mo, "staticmc.hitTest(1432, 270, false)");  // top-right
	check(mo, "staticmc.hitTest(-1332, 330, false)");   // bottom-left
	check(mo, "staticmc.hitTest(1432, 330, false)");  // bottom-right

	check(mo, "!staticmc.hitTest(-1333, 300, false)");   // overleft
	check(mo, "!staticmc.hitTest(1433, 300, false)");  // overright
	check(mo, "!staticmc.hitTest(0, 269, false)");  // overup
	check(mo, "!staticmc.hitTest(0, 331, false)");  // overdown


	SWFMovie_nextFrame(mo);        
	SWFDisplayItem_setMatrix(it, -1, 0, 45, 1, 50, 300); // negative x scale and some positive skew

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "Math.round(staticmc._yscale)", "4501"); // let's tollerate precision for now
	check_equals(mo, "staticmc._rotation", "180");  
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-1332.3,269.95 1432.3,330.05'");
	check_equals(mo, "staticmc._width", "2764.6");
	check_equals(mo, "staticmc._height", "60.1");

	// X: -1332.3  1432.3 
	// Y:   269.95  330.05

	check(mo, "staticmc.hitTest(-1332, 270, false)");   // top-left
	check(mo, "staticmc.hitTest(1432, 270, false)");  // top-right
	check(mo, "staticmc.hitTest(-1332, 330, false)");   // bottom-left
	check(mo, "staticmc.hitTest(1432, 330, false)");  // bottom-right

	check(mo, "!staticmc.hitTest(-1333, 300, false)");   // overleft
	check(mo, "!staticmc.hitTest(1433, 300, false)");  // overright
	check(mo, "!staticmc.hitTest(0, 269, false)");  // overup
	check(mo, "!staticmc.hitTest(0, 331, false)");  // overdown


	SWFMovie_nextFrame(mo);        
	SWFDisplayItem_setMatrix(it, 1, 0, -45, -1, 50, 300); // negative x scale and some negative skew

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "Math.round(staticmc._yscale)", "4501"); // let's tollerate precision for now
	check_equals(mo, "staticmc._rotation", "0");  
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-1332.3,269.95 1432.3,330.05'");
	check_equals(mo, "staticmc._width", "2764.6");
	check_equals(mo, "staticmc._height", "60.1");

	// X: -1332.3  1432.3 
	// Y:   269.95  330.05

	check(mo, "staticmc.hitTest(-1332, 270, false)");   // top-left
	check(mo, "staticmc.hitTest(1432, 270, false)");  // top-right
	check(mo, "staticmc.hitTest(-1332, 330, false)");   // bottom-left
	check(mo, "staticmc.hitTest(1432, 330, false)");  // bottom-right

	check(mo, "!staticmc.hitTest(-1333, 300, false)");   // overleft
	check(mo, "!staticmc.hitTest(1433, 300, false)");  // overright
	check(mo, "!staticmc.hitTest(0, 269, false)");  // overup
	check(mo, "!staticmc.hitTest(0, 331, false)");  // overdown


	SWFMovie_nextFrame(mo);        
	SWFDisplayItem_setMatrix(it, 1, 0, 45, -1, 50, 300); // negative x scale and some positive skew

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "Math.round(staticmc._yscale)", "4501"); // let's tollerate precision for now
	check_equals(mo, "staticmc._rotation", "0");  
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-1332.3,269.95 1432.3,330.05'");
	check_equals(mo, "staticmc._width", "2764.6");
	check_equals(mo, "staticmc._height", "60.1");

	// X: -1332.3  1432.3 
	// Y:   269.95  330.05

	check(mo, "staticmc.hitTest(-1332, 270, false)");   // top-left
	check(mo, "staticmc.hitTest(1432, 270, false)");  // top-right
	check(mo, "staticmc.hitTest(-1332, 330, false)");   // bottom-left
	check(mo, "staticmc.hitTest(1432, 330, false)");  // bottom-right

	check(mo, "!staticmc.hitTest(-1333, 300, false)");   // overleft
	check(mo, "!staticmc.hitTest(1433, 300, false)");  // overright
	check(mo, "!staticmc.hitTest(0, 269, false)");  // overup
	check(mo, "!staticmc.hitTest(0, 331, false)");  // overdown


	SWFMovie_nextFrame(mo);        
	SWFDisplayItem_setMatrix(it, -1, 0, 45, -1, 50, 300); // negative x scale and some positive skew

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "Math.round(staticmc._yscale)", "4501"); // let's tollerate precision for now
	check_equals(mo, "staticmc._rotation", "180");  
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-1332.3,269.95 1432.3,330.05'");
	check_equals(mo, "staticmc._width", "2764.6");
	check_equals(mo, "staticmc._height", "60.1");

	// X: -1332.3  1432.3 
	// Y:   269.95  330.05

	check(mo, "staticmc.hitTest(-1332, 270, false)");   // top-left
	check(mo, "staticmc.hitTest(1432, 270, false)");  // top-right
	check(mo, "staticmc.hitTest(-1332, 330, false)");   // bottom-left
	check(mo, "staticmc.hitTest(1432, 330, false)");  // bottom-right

	check(mo, "!staticmc.hitTest(-1333, 300, false)");   // overleft
	check(mo, "!staticmc.hitTest(1433, 300, false)");  // overright
	check(mo, "!staticmc.hitTest(0, 269, false)");  // overup
	check(mo, "!staticmc.hitTest(0, 331, false)");  // overdown


	SWFMovie_nextFrame(mo);        
	SWFDisplayItem_setMatrix(it, -1, 0, -45, -1, 50, 300); // negative x scale and some positive skew

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "Math.round(staticmc._yscale)", "4501"); // let's tollerate precision for now
	check_equals(mo, "staticmc._rotation", "180");  
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-1332.3,269.95 1432.3,330.05'");
	check_equals(mo, "staticmc._width", "2764.6");
	check_equals(mo, "staticmc._height", "60.1");

	// X: -1332.3  1432.3 
	// Y:   269.95  330.05

	check(mo, "staticmc.hitTest(-1332, 270, false)");   // top-left
	check(mo, "staticmc.hitTest(1432, 270, false)");  // top-right
	check(mo, "staticmc.hitTest(-1332, 330, false)");   // bottom-left
	check(mo, "staticmc.hitTest(1432, 330, false)");  // bottom-right

	check(mo, "!staticmc.hitTest(-1333, 300, false)");   // overleft
	check(mo, "!staticmc.hitTest(1433, 300, false)");  // overright
	check(mo, "!staticmc.hitTest(0, 269, false)");  // overup
	check(mo, "!staticmc.hitTest(0, 331, false)");  // overdown


	//
	// Now test setting parameters after reading matrix
	//

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, 1, 0, 2, 1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,169.95 290.15,230.05'");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
	xcheck_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.9,170 290.1,230'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, 1, 0, -2, 1, 200, 200); 
    check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,169.95 290.15,230.05'");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
    check_equals(mo, "staticmc._xscale", "-100"); // xscale changed, boundaries don't (not much at least)
	xcheck_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,170 290,230'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, 1, 0, -2, -1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,169.95 290.15,230.05'");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
	xcheck_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,170 290,230'");


	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, -1, 0, 2, 1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,169.95 290.15,230.05'");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
	xcheck_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,170 290,230'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, -1, 0, -2, 1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,169.95 290.15,230.05'");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
	xcheck_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,170 290,230'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, -1, 0, -2, -1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,169.95 290.15,230.05'");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
	xcheck_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,170 290,230'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, 1, 2, 0, 1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'169.95,109.85 230.05,290.15'");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
	xcheck_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'170,110 230,290'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, 1, -2, 0, 1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'169.95,109.85 230.05,290.15'");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
	xcheck_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'170,110 230,290'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, 1, -2, 0, -1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'169.95,109.85 230.05,290.15'");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
	xcheck_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'170,110 230,290'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, -1, 2, 0, 1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'169.95,109.85 230.05,290.15'");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
	xcheck_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'170,110 230,290'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, -1, -2, 0, 1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'169.95,109.85 230.05,290.15'");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
	xcheck_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'170,110 230,290'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, -1, -2, 0, -1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'169.95,109.85 230.05,290.15'");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
	xcheck_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'170,110 230,290'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, -1, -2, 2, -1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,109.85 290.15,290.15'");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
	xcheck_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,110 290,290'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, -1, -2, -2, -1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,109.85 290.15,290.15'");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
	xcheck_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,110 290,290'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, -1, 2, -2, -1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,109.85 290.15,290.15'");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
	xcheck_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,110 290,290'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, -1, 2, -2, 1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,109.85 290.15,290.15'");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
	xcheck_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,110 290,290'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, 1, 2, -2, -1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,109.85 290.15,290.15'");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
	xcheck_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,110 290,290'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, -1, -2, 2, -1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,109.85 290.15,290.15'");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
	xcheck_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,110 290,290'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, -1, -2, 2, 1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,109.85 290.15,290.15'");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
	xcheck_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,110 290,290'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, 1, -2, 2, -1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,109.85 290.15,290.15'");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
	xcheck_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,110 290,290'");

	// TODO:
	// - test more rotations and scales (corner cases too!)
	// - test 'skew' (since Ming supports it)

	SWFMovie_nextFrame(mo);

	add_actions(mo, "_root.totals(450); stop();");
	SWFMovie_nextFrame(mo);        

	//Output movie
	puts("Saving " OUTPUT_FILENAME );
	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
