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
 * Test extracting _width, _height, _xscale, _yscale, _x, _y and _rotation
 * from hard-coded matrices.
 *
 * run as ./matrix_test
 */

#include "ming_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#define OUTPUT_VERSION 8
#define OUTPUT_FILENAME "matrix_test.swf"

SWFDisplayItem add_static_mc(SWFMovie mo, const char* name, int depth, int x, int y, int width, int height);

SWFDisplayItem
add_static_mc(SWFMovie mo, const char* name, int depth, int x, int y, int width, int height)
{
	SWFShape sh;
	SWFMovieClip mc, mc2, mc3;
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

	sh = make_fill_square ((width/4), -(height/8), (width/4), (height/4), 0, 255, 0, 0, 255, 0);
	mc3 = newSWFMovieClip(); // child
	SWFMovieClip_add(mc3, (SWFBlock)sh);
	SWFMovieClip_nextFrame(mc3);
	it = SWFMovieClip_add(mc, (SWFBlock)mc3);
	SWFDisplayItem_setName(it, "face");

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
	//SWFMovie_setRate (mo, 0.2);
	//SWFMovie_setRate (mo, 2);
	SWFMovie_setRate (mo, 12);

	dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
	SWFMovie_add(mo, (SWFBlock)dejagnuclip);

	add_actions(mo,
            "printBounds = function(b, roundToDecimal) "
            "{ "
            "   if ( roundToDecimal != undefined ) {"
            "       var round = Math.pow(10, roundToDecimal);"
            //"       trace('rounding to '+round);"
            "       return '' + Math.round(b.xMin*round)/round + ','+ Math.round(b.yMin*round)/round + ' '+ Math.round(b.xMax*round)/round + ',' + Math.round(b.yMax*round)/round; "
            "   } else {"
            "       return '' + b.xMin + ','+ b.yMin + ' '+ b.xMax + ',' + b.yMax; "
            "   }"
            "};"
            "printMatrix = function(m, roundToDecimal) "
            "{ "
            "   if ( roundToDecimal != undefined ) {"
            "       var round = Math.pow(10, roundToDecimal);"
            //"       trace('rounding to '+round);"
            "       return '(a=' + Math.round(m.a*round)/round + ', b='+ Math.round(m.b*round)/round + ', c='+ Math.round(m.c*round)/round + ', d=' + Math.round(m.d*round)/round + ', tx='+Math.round(m.tx*round)/round+', ty='+Math.round(m.ty*round)/round+')'; "
            "   } else {"
            "       return m.toString();"
            "   }"
            "};"
            "onMouseDown = function() {"
            " if ( playing ) { playing=false; stop(); }"
            " else { playing=true; play(); }"
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
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=1, b=0, c=0, d=1, tx=50, ty=300)'");

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
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=-1, b=0, c=0, d=1, tx=50, ty=300)'");

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
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=1, b=0, c=0, d=-1, tx=50, ty=300)'");

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
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=-1, b=0, c=0, d=-1, tx=50, ty=300)'");

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
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=-1, b=-45, c=0, d=1, tx=50, ty=300)'");

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "4501"); // let's tollerate precision for now
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "Math.round(staticmc._rotation)", "-91");  // let's tollerate precision for now
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'19.95,-1082.3 80.05,1682.3'");
	check_equals(mo, "staticmc._width", "60.1");
	check_equals(mo, "Math.round(staticmc._height*10)", "27646");

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
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=-1, b=45, c=0, d=1, tx=50, ty=300)'");

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "4501"); // let's tollerate precision for now
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "Math.round(staticmc._rotation)", "91");  // let's tollerate precision for now
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'19.95,-1082.3 80.05,1682.3'");
	check_equals(mo, "staticmc._width", "60.1");
	check_equals(mo, "Math.round(staticmc._height*10)", "27646");

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
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=1, b=-45, c=0, d=-1, tx=50, ty=300)'");

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "4501"); // let's tollerate precision for now
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "Math.round(staticmc._rotation)", "-89");  // let's tollerate precision for now
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'19.95,-1082.3 80.05,1682.3'");
	check_equals(mo, "staticmc._width", "60.1");
	check_equals(mo, "Math.round(staticmc._height*10)", "27646");

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
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=1, b=45, c=0, d=-1, tx=50, ty=300)'");

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "4501"); // let's tollerate precision for now
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "Math.round(staticmc._rotation)", "89");  // let's tollerate precision for now
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'19.95,-1082.3 80.05,1682.3'");
	check_equals(mo, "staticmc._width", "60.1");
	check_equals(mo, "Math.round(staticmc._height*10)", "27646");

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
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=-1, b=45, c=0, d=-1, tx=50, ty=300)'");

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "4501"); // let's tollerate precision for now
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "Math.round(staticmc._rotation)", "91");  // let's tollerate precision for now
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'19.95,-1082.3 80.05,1682.3'");
	check_equals(mo, "staticmc._width", "60.1");
	check_equals(mo, "Math.round(staticmc._height*10)", "27646");

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
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=-1, b=-45, c=0, d=-1, tx=50, ty=300)'");

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "Math.round(staticmc._xscale)", "4501"); // let's tollerate precision for now
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "Math.round(staticmc._rotation)", "-91");  // let's tollerate precision for now
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'19.95,-1082.3 80.05,1682.3'");
	check_equals(mo, "staticmc._width", "60.1");
	check_equals(mo, "Math.round(staticmc._height*10)", "27646");

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
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=-1, b=0, c=-45, d=1, tx=50, ty=300)'");

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "Math.round(staticmc._yscale)", "4501"); // let's tollerate precision for now
	check_equals(mo, "staticmc._rotation", "180");  
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-1332.3,269.95 1432.3,330.05'");
	check_equals(mo, "Math.round(staticmc._width*10)", "27646");
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
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=-1, b=0, c=45, d=1, tx=50, ty=300)'");

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "Math.round(staticmc._yscale)", "4501"); // let's tollerate precision for now
	check_equals(mo, "staticmc._rotation", "180");  
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-1332.3,269.95 1432.3,330.05'");
	check_equals(mo, "Math.round(staticmc._width*10)", "27646");
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
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=1, b=0, c=-45, d=-1, tx=50, ty=300)'");

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "Math.round(staticmc._yscale)", "4501"); // let's tollerate precision for now
	check_equals(mo, "staticmc._rotation", "0");  
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-1332.3,269.95 1432.3,330.05'");
	check_equals(mo, "Math.round(staticmc._width*10)", "27646");
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
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=1, b=0, c=45, d=-1, tx=50, ty=300)'");

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "Math.round(staticmc._yscale)", "4501"); // let's tollerate precision for now
	check_equals(mo, "staticmc._rotation", "0");  
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-1332.3,269.95 1432.3,330.05'");
	check_equals(mo, "Math.round(staticmc._width*10)", "27646");
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
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=-1, b=0, c=45, d=-1, tx=50, ty=300)'");

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "Math.round(staticmc._yscale)", "4501"); // let's tollerate precision for now
	check_equals(mo, "staticmc._rotation", "180");  
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-1332.3,269.95 1432.3,330.05'");
	check_equals(mo, "Math.round(staticmc._width*10)", "27646");
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
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=-1, b=0, c=-45, d=-1, tx=50, ty=300)'");

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "Math.round(staticmc._yscale)", "4501"); // let's tollerate precision for now
	check_equals(mo, "staticmc._rotation", "180");  
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-1332.3,269.95 1432.3,330.05'");
	check_equals(mo, "Math.round(staticmc._width*10)", "27646");
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
	SWFDisplayItem_setMatrix(it, -2, 0, -45, -0.5, 50, 300); // negative x scale and some positive skew
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=-2, b=0, c=-45, d=-0.5, tx=50, ty=300)'");

	check_equals(mo, "staticmc._x", "50");
	check_equals(mo, "staticmc._y", "300");
	check_equals(mo, "staticmc._xscale", "200");
	check_equals(mo, "Math.round(staticmc._yscale)", "4500"); // let's tollerate precision for now
	check_equals(mo, "staticmc._rotation", "180");  
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root),0)", "'-1362,285 1462,315'"); // when not rounded, gives different results in SWF6 and SWF8
	check_equals(mo, "Math.round(staticmc._width*10)", "28247");
	check_equals(mo, "Math.round(staticmc._height)", "30"); // when not rounded, gives different results in SWF6 and SWF8

	// X: -1362.35  1462.35
	// Y:   285.00  315.05

	check(mo, "staticmc.hitTest(-1362, 286, false)");   // top-left
	check(mo, "staticmc.hitTest(1462, 286, false)");  // top-right
	check(mo, "staticmc.hitTest(-1362, 315, false)");   // bottom-left
	check(mo, "staticmc.hitTest(1462, 315, false)");  // bottom-right

	check(mo, "!staticmc.hitTest(-1363, 300, false)");   // overleft
	check(mo, "!staticmc.hitTest(1463, 300, false)");  // overright
	check(mo, "!staticmc.hitTest(0, 284, false)");  // overup
	check(mo, "!staticmc.hitTest(0, 316, false)");  // overdown

	SWFMovie_nextFrame(mo);        
	SWFDisplayItem_setMatrix(it, 1, 1, 1, 1, 2, 3); // not-invertible matrix (I'd think)
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=1, b=1, c=1, d=1, tx=2, ty=3)'");

	check_equals(mo, "staticmc._x", "2");
	check_equals(mo, "staticmc._y", "3");
	check_equals(mo, "Math.round(staticmc._xscale)", "141");
	check_equals(mo, "Math.round(staticmc._yscale)", "141");
	check_equals(mo, "staticmc._rotation", "45");  
	check_equals(mo, "printBounds(staticmc.getBounds())", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-58.1,-57.1 62.1,63.1'");
	add_actions(mo, "o = {x:1, y:1}; staticmc.localToGlobal(o);");
	check_equals(mo, "o.x", "4");
	check_equals(mo, "o.y", "5");
	add_actions(mo, "o = {x:1, y:1}; staticmc.globalToLocal(o);"); // this triggers matrix inversion 
	check_equals(mo, "o.x", "1");
	check_equals(mo, "o.y", "1");
	add_actions(mo, "o = {x:4, y:5}; staticmc.globalToLocal(o);"); // this triggers matrix inversion 
	check_equals(mo, "o.x", "4");
	check_equals(mo, "o.y", "5");
	check_equals(mo, "Math.round(staticmc._width*10)", "1202");
	check_equals(mo, "staticmc._height", "120.2");

	// This is a matrix found in the mario.swf movie (bug #24280)
	//
	//     Matrix:
	//      ScaleX 0.000000   ScaleY 0.000000
	//      RotateSkew0 -1.000000   RotateSkew1 1.167969
	//      TranslateX    -15   TranslateY   2700
	//
	SWFMovie_nextFrame(mo);        
	SWFDisplayItem_setMatrix(it, 0, -1, 1.167969, 0, -.75, 135); 
	check_equals(mo, "staticmc._x", "-0.75");
	check_equals(mo, "staticmc._y", "135");

// Ming 0.4.2 (aka 0.4.0-rc2) omits the scales rather then set them to zero 
#if MING_VERSION_CODE <= 00040200
	check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=1, b=-1, c=1.16796875, d=1, tx=-0.75, ty=135)'");
	check_equals(mo, "Math.round(staticmc._xscale)", "141");
	check_equals(mo, "Math.round(staticmc._yscale)", "154");
	check_equals(mo, "staticmc._rotation", "-45");  
#else
	check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=0, b=-1, c=1.16796875, d=0, tx=-0.75, ty=135)'");
	check_equals(mo, "Math.round(staticmc._xscale)", "100");
	check_equals(mo, "Math.round(staticmc._yscale)", "117");
	check_equals(mo, "staticmc._rotation", "-90");  
#endif

	// This is a matrix found in the mario.swf movie (bug #24280)
	//
	//    Matrix:
	//     ScaleX 0.000000   ScaleY 0.000000
	//     RotateSkew0 0.972519   RotateSkew1 -1.000000
	//     TranslateX    279   TranslateY   4296
	//
	SWFMovie_nextFrame(mo);        
	SWFDisplayItem_setMatrix(it, 0, 0.972519, -1, 0, 13.950, 214.80); 
	check_equals(mo, "Math.round(staticmc._x*100)/100", "13.95"); 
	check_equals(mo, "staticmc._y", "214.80");

// Ming 0.4.2 (aka 0.4.0-rc2) omits the scales rather then set them to zero 
#if MING_VERSION_CODE <= 00040200
	check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=1, b=0.972518920898438, c=-1, d=1, tx=13.95, ty=214.8)'");
	check_equals(mo, "Math.round(staticmc._xscale)", "139");
	check_equals(mo, "Math.round(staticmc._yscale)", "141");
	check_equals(mo, "Math.round(staticmc._rotation)", "44");  
#else
	check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=0, b=0.972518920898438, c=-1, d=0, tx=13.95, ty=214.8)'");
	check_equals(mo, "Math.round(staticmc._xscale)", "97");
	check_equals(mo, "Math.round(staticmc._yscale)", "100");
	check_equals(mo, "Math.round(staticmc._rotation)", "90");  
#endif


	//
	// Now test setting parameters after reading matrix
	//

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, 1, 0, 0, -2, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'169.95,139.9 230.05,260.1'");
	check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=1, b=0, c=0, d=-2, tx=200, ty=200)'");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "staticmc._rotation", "0");
	check_equals(mo, "staticmc._yscale", "200");

	add_actions(mo, "staticmc._yscale = 100;"); // swap _yscale sign using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=0, c=0, d=-1, tx=200, ty=200)'");
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "staticmc._rotation", "0");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'170,170 230,230'");

	add_actions(mo, "staticmc._yscale = 0 - staticmc._yscale;"); // swap _yscale sign using ActionScript
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'169.95,169.95 230.05,230.05'");
	check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=1, b=0, c=0, d=1, tx=200, ty=200)'");
	check_equals(mo, "staticmc._yscale", "-100");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "staticmc._rotation", "0");

	add_actions(mo, "staticmc._yscale = 0 - staticmc._yscale;"); // swap _yscale sign using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=0, c=0, d=-1, tx=200, ty=200)'");
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'169.95,169.95 230.05,230.05'");
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "staticmc._rotation", "0");

	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=0, c=0, d=-1, tx=200, ty=200)'");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "staticmc._rotation", "0");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'170,170 230,230'");

	add_actions(mo, "staticmc._rotation = 2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=-0.03, c=0.03, d=-1, tx=200, ty=200)'");
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "staticmc._rotation", "2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'169,169 231,231'");

	add_actions(mo, "staticmc._rotation = -2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=0.03, c=-0.03, d=-1, tx=200, ty=200)'");
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "staticmc._rotation", "-2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'169,169 231,231'");

	add_actions(mo, "staticmc._rotation = -90;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=0, b=1, c=-1, d=0, tx=200, ty=200)'");
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "staticmc._rotation", "-90");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'170,170 230,230'");

	add_actions(mo, "staticmc._yscale = -100;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=0, b=1, c=1, d=0, tx=200, ty=200)'");
	check_equals(mo, "staticmc._yscale", "-100");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "staticmc._rotation", "-90");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'170,170 230,230'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, 1, 0, 2, 1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,169.95 290.15,230.05'");
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=1, b=0, c=2, d=1, tx=200, ty=200)'");

	check_equals(mo, "staticmc._rotation", "0");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	check_equals(mo, "staticmc._xscale", "100");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=0, c=2, d=1, tx=200, ty=200)'");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,170 290,230'");

	check_equals(mo, "staticmc._rotation", "0");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	add_actions(mo, "staticmc._yscale = 0 - staticmc._yscale;"); // swap _yscale sign using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=0, c=-2, d=-1, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "staticmc._rotation", "0");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,170 290,230'");

	add_actions(mo, "staticmc._rotation = 2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=-0.03, c=-1.96, d=-1.07, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "staticmc._rotation", "2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'111,167 289,233'");

	add_actions(mo, "staticmc._rotation = -2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=0.03, c=-2.03, d=-0.93, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "staticmc._rotation", "-2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'109,171 291,229'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, 1, 0, -2, 1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,169.95 290.15,230.05'");
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=1, b=0, c=-2, d=1, tx=200, ty=200)'");

	check_equals(mo, "staticmc._rotation", "0");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	check_equals(mo, "staticmc._xscale", "100");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=0, c=-2, d=1, tx=200, ty=200)'");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,170 290,230'");

	check_equals(mo, "staticmc._rotation", "0");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	add_actions(mo, "staticmc._yscale = 0 - staticmc._yscale;"); // swap _yscale sign using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=0, c=2, d=-1, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "staticmc._rotation", "0");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,170 290,230'");

	add_actions(mo, "staticmc._rotation = 2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=-0.03, c=2.03, d=-0.93, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "staticmc._rotation", "2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'109,171 291,229'");

	add_actions(mo, "staticmc._rotation = -2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=0.03, c=1.96, d=-1.07, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "staticmc._rotation", "-2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'111,167 289,233'");


	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, 1, 0, -2, -1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,169.95 290.15,230.05'");
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=1, b=0, c=-2, d=-1, tx=200, ty=200)'");

	check_equals(mo, "staticmc._rotation", "0");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	check_equals(mo, "staticmc._xscale", "100");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=0, c=-2, d=-1, tx=200, ty=200)'");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,170 290,230'");

	check_equals(mo, "staticmc._rotation", "0");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	add_actions(mo, "staticmc._yscale = 0 - staticmc._yscale;"); // swap _yscale sign using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=0, c=2, d=1, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "staticmc._rotation", "0");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,170 290,230'");

	add_actions(mo, "staticmc._rotation = 2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=-0.03, c=1.96, d=1.07, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "staticmc._rotation", "2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'111,167 289,233'");

	add_actions(mo, "staticmc._rotation = -2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=0.03, c=2.03, d=0.93, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "staticmc._rotation", "-2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'109,171 291,229'");


	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, -1, 0, 2, 1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,169.95 290.15,230.05'");
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=-1, b=0, c=2, d=1, tx=200, ty=200)'");

	check_equals(mo, "staticmc._rotation", "180");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	check_equals(mo, "staticmc._xscale", "100");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=0, c=2, d=1, tx=200, ty=200)'");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,170 290,230'");

	check_equals(mo, "staticmc._rotation", "180");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	add_actions(mo, "staticmc._yscale = 0 - staticmc._yscale;"); // swap _yscale sign using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=0, c=-2, d=-1, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "staticmc._rotation", "180");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,170 290,230'");

	add_actions(mo, "staticmc._rotation = 2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=-0.03, c=1.96, d=1.07, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "staticmc._rotation", "2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'111,167 289,233'");

	add_actions(mo, "staticmc._rotation = -2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=0.03, c=2.03, d=0.93, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "staticmc._rotation", "-2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'109,171 291,229'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, -1, 0, -2, 1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,169.95 290.15,230.05'");
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=-1, b=0, c=-2, d=1, tx=200, ty=200)'");

	check_equals(mo, "staticmc._rotation", "180");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	check_equals(mo, "staticmc._xscale", "100");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=0, c=-2, d=1, tx=200, ty=200)'");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,170 290,230'");

	check_equals(mo, "staticmc._rotation", "180");
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=0, c=-2, d=1, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	add_actions(mo, "staticmc._yscale = 0 - staticmc._yscale;"); // swap _yscale sign using ActionScript
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "staticmc._rotation", "180");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,170 290,230'");

	add_actions(mo, "staticmc._rotation = 2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=-0.03, c=-2.03, d=0.93, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "staticmc._rotation", "2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'109,171 291,229'");

	add_actions(mo, "staticmc._rotation = -2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=0.03, c=-1.96, d=1.07, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "staticmc._rotation", "-2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'111,167 289,233'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, -1, 0, -2, -1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,169.95 290.15,230.05'");
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=-1, b=0, c=-2, d=-1, tx=200, ty=200)'");

	check_equals(mo, "staticmc._rotation", "180");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	check_equals(mo, "staticmc._xscale", "100");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=0, c=-2, d=-1, tx=200, ty=200)'");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,170 290,230'");

	check_equals(mo, "staticmc._rotation", "180");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	add_actions(mo, "staticmc._yscale = 0 - staticmc._yscale;"); // swap _yscale sign using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=0, c=2, d=1, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "staticmc._rotation", "180");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,170 290,230'");

	add_actions(mo, "staticmc._rotation = 2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=-0.03, c=-1.96, d=-1.07, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "staticmc._rotation", "2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'111,167 289,233'");

	add_actions(mo, "staticmc._rotation = -2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=0.03, c=-2.03, d=-0.93, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "staticmc._xscale", "-100");
	check_equals(mo, "staticmc._rotation", "-2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'109,171 291,229'");


	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, 1, 2, 0, 1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'169.95,109.85 230.05,290.15'");
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=1, b=2, c=0, d=1, tx=200, ty=200)'");

	check_equals(mo, "Math.round(staticmc._rotation)", "63");
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "Math.round(staticmc._xscale)", "224");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=-2, c=0, d=1, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'170,110 230,290'");

	check_equals(mo, "Math.round(staticmc._rotation)", "63");
	check_equals(mo, "staticmc._yscale", "100");
	add_actions(mo, "staticmc._yscale = 0 - staticmc._yscale;"); // swap _yscale sign using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=-2, c=0, d=-1, tx=200, ty=200)'");
	check_equals(mo, "staticmc._yscale", "-100");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "Math.round(staticmc._rotation)", "63");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'170,110 230,290'");

	add_actions(mo, "staticmc._rotation = 2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-2.23, b=-0.08, c=-0.88, d=-0.48, tx=200, ty=200)'");
	check_equals(mo, "staticmc._yscale", "-100");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), -1)", "'110,180 290,220'"); 

	add_actions(mo, "staticmc._rotation = -2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 1)", "'(a=-2.2, b=0.1, c=-0.9, d=-0.4, tx=200, ty=200)'");
	check_equals(mo, "staticmc._yscale", "-100");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "-2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), -1)", "'110,190 290,210'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, 1, -2, 0, 1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'169.95,109.85 230.05,290.15'");
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=1, b=-2, c=0, d=1, tx=200, ty=200)'");

	check_equals(mo, "Math.round(staticmc._rotation)", "-63");
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "Math.round(staticmc._xscale)", "224");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=2, c=0, d=1, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'170,110 230,290'");

	check_equals(mo, "Math.round(staticmc._rotation)", "-63");
	check_equals(mo, "staticmc._yscale", "100");
	add_actions(mo, "staticmc._yscale = 0 - staticmc._yscale;"); // swap _yscale sign using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=2, c=0, d=-1, tx=200, ty=200)'");
	check_equals(mo, "staticmc._yscale", "-100");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "Math.round(staticmc._rotation)", "-63");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'170,110 230,290'");

	add_actions(mo, "staticmc._rotation = 2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 1)", "'(a=-2.2, b=-0.1, c=0.9, d=-0.4, tx=200, ty=200)'");
	check_equals(mo, "staticmc._yscale", "-100");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), -1)", "'110,190 290,210'");

	add_actions(mo, "staticmc._rotation = -2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-2.23, b=0.08, c=0.88, d=-0.48, tx=200, ty=200)'");
	check_equals(mo, "staticmc._yscale", "-100");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "-2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), -1)", "'110,180 290,220'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, 1, -2, 0, -1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'169.95,109.85 230.05,290.15'");
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=1, b=-2, c=0, d=-1, tx=200, ty=200)'");

	check_equals(mo, "Math.round(staticmc._rotation)", "-63");
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "Math.round(staticmc._xscale)", "224");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=2, c=0, d=-1, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'170,110 230,290'");

	check_equals(mo, "Math.round(staticmc._rotation)", "-63");
	check_equals(mo, "staticmc._yscale", "100");
	add_actions(mo, "staticmc._yscale = 0 - staticmc._yscale;"); // swap _yscale sign using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=2, c=0, d=1, tx=200, ty=200)'");
	check_equals(mo, "staticmc._yscale", "-100");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "Math.round(staticmc._rotation)", "-63");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'170,110 230,290'");

	add_actions(mo, "staticmc._rotation = 2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 1)", "'(a=-2.2, b=-0.1, c=-0.9, d=0.4, tx=200, ty=200)'");
	check_equals(mo, "staticmc._yscale", "-100");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), -1)", "'110,190 290,210'");

	add_actions(mo, "staticmc._rotation = -2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-2.23, b=0.08, c=-0.88, d=0.48, tx=200, ty=200)'");
	check_equals(mo, "staticmc._yscale", "-100");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "-2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), -1)", "'110,180 290,220'");


	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, -1, 2, 0, 1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'169.95,109.85 230.05,290.15'");
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=-1, b=2, c=0, d=1, tx=200, ty=200)'");

	check_equals(mo, "Math.round(staticmc._rotation)", "117");
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "Math.round(staticmc._xscale)", "224");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=-2, c=0, d=1, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'170,110 230,290'");

	check_equals(mo, "Math.round(staticmc._rotation)", "117");
	check_equals(mo, "staticmc._yscale", "100");
	add_actions(mo, "staticmc._yscale = 0 - staticmc._yscale;"); // swap _yscale sign using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=-2, c=0, d=-1, tx=200, ty=200)'");
	check_equals(mo, "staticmc._yscale", "-100");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "Math.round(staticmc._rotation)", "117");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'170,110 230,290'");

	add_actions(mo, "staticmc._rotation = 2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 1)", "'(a=-2.2, b=-0.1, c=-0.9, d=0.4, tx=200, ty=200)'");
	check_equals(mo, "staticmc._yscale", "-100");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), -1)", "'110,190 290,210'");

	add_actions(mo, "staticmc._rotation = -2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-2.23, b=0.08, c=-0.88, d=0.48, tx=200, ty=200)'");
	check_equals(mo, "staticmc._yscale", "-100");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "-2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), -1)", "'110,180 290,220'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, -1, -2, 0, 1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'169.95,109.85 230.05,290.15'");
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=-1, b=-2, c=0, d=1, tx=200, ty=200)'");

	check_equals(mo, "Math.round(staticmc._rotation)", "-117");
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "Math.round(staticmc._xscale)", "224");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=2, c=0, d=1, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'170,110 230,290'");

	check_equals(mo, "Math.round(staticmc._rotation)", "-117");
	check_equals(mo, "staticmc._yscale", "100");
	add_actions(mo, "staticmc._yscale = 0 - staticmc._yscale;"); // swap _yscale sign using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=2, c=0, d=-1, tx=200, ty=200)'");
	check_equals(mo, "staticmc._yscale", "-100");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "Math.round(staticmc._rotation)", "-117");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'170,110 230,290'");

	add_actions(mo, "staticmc._rotation = 2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-2.23, b=-0.08, c=0.88, d=0.48, tx=200, ty=200)'");
	check_equals(mo, "staticmc._yscale", "-100");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), -1)", "'110,180 290,220'");

	add_actions(mo, "staticmc._rotation = -2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 1)", "'(a=-2.2, b=0.1, c=0.9, d=0.4, tx=200, ty=200)'");
	check_equals(mo, "staticmc._yscale", "-100");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "-2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), -1)", "'110,190 290,210'");


	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, -1, -2, 0, -1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'169.95,109.85 230.05,290.15'");
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=-1, b=-2, c=0, d=-1, tx=200, ty=200)'");

	check_equals(mo, "Math.round(staticmc._rotation)", "-117");
	check_equals(mo, "staticmc._yscale", "100");
	check_equals(mo, "Math.round(staticmc._xscale)", "224");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=2, c=0, d=-1, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'170,110 230,290'");

	check_equals(mo, "Math.round(staticmc._rotation)", "-117");
	check_equals(mo, "staticmc._yscale", "100");
	add_actions(mo, "staticmc._yscale = 0 - staticmc._yscale;"); // swap _yscale sign using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=2, c=0, d=1, tx=200, ty=200)'");
	check_equals(mo, "staticmc._yscale", "-100");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "Math.round(staticmc._rotation)", "-117");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'170,110 230,290'");

	add_actions(mo, "staticmc._rotation = 2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-2.23, b=-0.08, c=-0.88, d=-0.48, tx=200, ty=200)'");
	check_equals(mo, "staticmc._yscale", "-100");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), -1)", "'110,180 290,220'");

	add_actions(mo, "staticmc._rotation = -2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 1)", "'(a=-2.2, b=0.1, c=-0.9, d=-0.4, tx=200, ty=200)'");
	check_equals(mo, "staticmc._yscale", "-100");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "-2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), -1)", "'110,190 290,210'");


	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, -1, -2, 2, -1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,109.85 290.15,290.15'");
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=-1, b=-2, c=2, d=-1, tx=200, ty=200)'");

	check_equals(mo, "Math.round(staticmc._rotation)", "-117");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	check_equals(mo, "Math.round(staticmc._xscale)", "224");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=2, c=2, d=-1, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,110 290,290'");

	check_equals(mo, "Math.round(staticmc._rotation)", "-117");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	add_actions(mo, "staticmc._yscale = 0 - staticmc._yscale;"); // swap _yscale sign using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=2, c=-2, d=1, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "Math.round(staticmc._rotation)", "-117");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,110 290,290'");

	add_actions(mo, "staticmc._rotation = 2;"); // change _rotation using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-2.23, b=-0.08, c=0.08, d=-2.23, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'131,131 270,270'");

	add_actions(mo, "staticmc._rotation = -2;"); // change _rotation using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-2.23, b=0.08, c=-0.08, d=-2.23, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "-2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'131,131 270,270'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, -1, -2, -2, -1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,109.85 290.15,290.15'");
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=-1, b=-2, c=-2, d=-1, tx=200, ty=200)'");

	check_equals(mo, "Math.round(staticmc._rotation)", "-117");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	check_equals(mo, "Math.round(staticmc._xscale)", "224");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,110 290,290'");
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=2, c=-2, d=-1, tx=200, ty=200)'");

	check_equals(mo, "Math.round(staticmc._rotation)", "-117");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	add_actions(mo, "staticmc._yscale = 0 - staticmc._yscale;"); // swap _yscale sign using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=2, c=2, d=1, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "Math.round(staticmc._rotation)", "-117");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,110 290,290'");

	add_actions(mo, "staticmc._rotation = 2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-2.23, b=-0.08, c=-1.83, d=1.28, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'78,159 322,241'");

	add_actions(mo, "staticmc._rotation = -2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-2.23, b=0.08, c=-1.74, d=1.4, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "-2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), -1)", "'80,160 320,240'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, -1, 2, -2, -1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,109.85 290.15,290.15'");
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=-1, b=2, c=-2, d=-1, tx=200, ty=200)'");

	check_equals(mo, "Math.round(staticmc._rotation)", "117");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	check_equals(mo, "Math.round(staticmc._xscale)", "224");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=-2, c=-2, d=-1, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,110 290,290'");

	check_equals(mo, "Math.round(staticmc._rotation)", "117");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	add_actions(mo, "staticmc._yscale = 0 - staticmc._yscale;"); // swap _yscale sign using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=-2, c=2, d=1, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "Math.round(staticmc._rotation)", "117");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,110 290,290'");

	add_actions(mo, "staticmc._rotation = 2;"); // change _rotation using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-2.23, b=-0.08, c=0.08, d=-2.23, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'131,131 270,270'");

	add_actions(mo, "staticmc._rotation = -2;"); // change _rotation using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-2.23, b=0.08, c=-0.08, d=-2.23, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "-2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'131,131 270,270'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, -1, 2, -2, 1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,109.85 290.15,290.15'");
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=-1, b=2, c=-2, d=1, tx=200, ty=200)'");

	check_equals(mo, "Math.round(staticmc._rotation)", "117");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	check_equals(mo, "Math.round(staticmc._xscale)", "224");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=-2, c=-2, d=1, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,110 290,290'");

	check_equals(mo, "Math.round(staticmc._rotation)", "117");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	add_actions(mo, "staticmc._yscale = 0 - staticmc._yscale;"); // swap _yscale sign using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=-2, c=2, d=-1, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "Math.round(staticmc._rotation)", "117");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,110 290,290'");

	add_actions(mo, "staticmc._rotation = 2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-2.23, b=-0.08, c=-1.74, d=-1.4, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), -1)", "'80,160 320,240'");

	add_actions(mo, "staticmc._rotation = -2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-2.23, b=0.08, c=-1.83, d=-1.28, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "-2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'78,159 322,241'");


	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, 1, 2, -2, -1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,109.85 290.15,290.15'");
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=1, b=2, c=-2, d=-1, tx=200, ty=200)'");

	check_equals(mo, "Math.round(staticmc._rotation)", "63");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	check_equals(mo, "Math.round(staticmc._xscale)", "224");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=-2, c=-2, d=-1, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,110 290,290'");

	check_equals(mo, "Math.round(staticmc._rotation)", "63");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	add_actions(mo, "staticmc._yscale = 0 - staticmc._yscale;"); // swap _yscale sign using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=-2, c=2, d=1, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "Math.round(staticmc._rotation)", "63");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,110 290,290'");

	add_actions(mo, "staticmc._rotation = 2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-2.23, b=-0.08, c=1.83, d=-1.28, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'78,159 322,241'");

	add_actions(mo, "staticmc._rotation = -2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-2.23, b=0.08, c=1.74, d=-1.4, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "-2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), -1)", "'80,160 320,240'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, -1, -2, 2, -1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,109.85 290.15,290.15'");
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=-1, b=-2, c=2, d=-1, tx=200, ty=200)'");

	check_equals(mo, "Math.round(staticmc._rotation)", "-117");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	check_equals(mo, "Math.round(staticmc._xscale)", "224");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=2, c=2, d=-1, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,110 290,290'");

	check_equals(mo, "Math.round(staticmc._rotation)", "-117");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	add_actions(mo, "staticmc._yscale = 0 - staticmc._yscale;"); // swap _yscale sign using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=2, c=-2, d=1, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "Math.round(staticmc._rotation)", "-117");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,110 290,290'");

	add_actions(mo, "staticmc._rotation = 2;"); // change _rotation using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-2.23, b=-0.08, c=0.08, d=-2.23, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'131,131 270,270'");

	add_actions(mo, "staticmc._rotation = -2;"); // change _rotation using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-2.23, b=0.08, c=-0.08, d=-2.23, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "-2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'131,131 270,270'");


	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, -1, -2, 2, 1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,109.85 290.15,290.15'");
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=-1, b=-2, c=2, d=1, tx=200, ty=200)'");

	check_equals(mo, "Math.round(staticmc._rotation)", "-117");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	check_equals(mo, "Math.round(staticmc._xscale)", "224");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=2, c=2, d=1, tx=200, ty=200)'"); // swaps a,b signs
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,110 290,290'");

	check_equals(mo, "Math.round(staticmc._rotation)", "-117");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	add_actions(mo, "staticmc._yscale = 0 - staticmc._yscale;"); // swap _yscale sign using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=2, c=-2, d=-1, tx=200, ty=200)'"); // swaps c,d signs
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "Math.round(staticmc._rotation)", "-117");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,110 290,290'");

	add_actions(mo, "staticmc._rotation = 2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-2.23, b=-0.08, c=1.83, d=-1.28, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'78,159 322,241'");

	add_actions(mo, "staticmc._rotation = -2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-2.23, b=0.08, c=1.74, d=-1.4, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "-2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), -1)", "'80,160 320,240'");

	SWFMovie_nextFrame(mo);

	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, 1, -2, 2, -1, 200, 200); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'109.85,109.85 290.15,290.15'");
    check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=1, b=-2, c=2, d=-1, tx=200, ty=200)'");

	check_equals(mo, "Math.round(staticmc._rotation)", "-63");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	check_equals(mo, "Math.round(staticmc._xscale)", "224");
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;"); // swap _xscale sign using ActionScript
    check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=2, c=2, d=-1, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,110 290,290'");

	check_equals(mo, "Math.round(staticmc._rotation)", "-63");
	check_equals(mo, "Math.round(staticmc._yscale)", "224");
	add_actions(mo, "staticmc._yscale = 0 - staticmc._yscale;"); // swap _yscale sign using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=2, c=-2, d=1, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "Math.round(staticmc._rotation)", "-63");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'110,110 290,290'");

	add_actions(mo, "staticmc._rotation = 2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-2.23, b=-0.08, c=-1.74, d=-1.4, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), -1)", "'80,160 320,240'");

	add_actions(mo, "staticmc._rotation = -2;"); // change _rotation using ActionScript
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-2.23, b=0.08, c=-1.83, d=-1.28, tx=200, ty=200)'");
	check_equals(mo, "Math.round(staticmc._yscale)", "-224");
	check_equals(mo, "Math.round(staticmc._xscale)", "-224");
	check_equals(mo, "staticmc._rotation", "-2");
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'78,159 322,241'");

	SWFMovie_nextFrame(mo);

	// This is a matrix found in the mario.swf movie (bug #24280)
	//
	//    Matrix:
	//     ScaleX 0.000000   ScaleY 0.000000
	//     RotateSkew0 0.972519   RotateSkew1 -1.000000
	//     TranslateX    279   TranslateY   4296
	//
	// Actually this ming version omits ScaleX and ScaleY
	// (hasScale flag clear). I hope it doesn't count!
	//
	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFMovie_nextFrame(mo);        
	SWFDisplayItem_setMatrix(it, 0, 0.972519, -1, 0, 13.950, 214.80); 
	check_equals(mo, "Math.round(staticmc._x*100)/100", "13.95"); 
	check_equals(mo, "staticmc._y", "214.80");

// Ming 0.4.2 (aka 0.4.0-rc2) omits the scales rather then set them to zero 
#if MING_VERSION_CODE <= 00040200
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=1, b=0.97, c=-1, d=1, tx=13.95, ty=214.8)'");
	check_equals(mo, "Math.round(staticmc._xscale)", "139");
	check_equals(mo, "Math.round(staticmc._yscale)", "141");
	check_equals(mo, "Math.round(staticmc._rotation)", "44");  
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'-46,156 74,274'");
#else
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=0, b=0.97, c=-1, d=0, tx=13.95, ty=214.8)'");
	check_equals(mo, "Math.round(staticmc._xscale)", "97");
	check_equals(mo, "Math.round(staticmc._yscale)", "100");
	check_equals(mo, "Math.round(staticmc._rotation)", "90");  
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'-16,186 44,244'");
#endif

	// swap _xscale sign using ActionScript
	add_actions(mo, "staticmc._xscale = 0 - staticmc._xscale;");
	check_equals(mo, "Math.round(staticmc._x*100)/100", "13.95"); 
	check_equals(mo, "staticmc._y", "214.80");

// Ming 0.4.2 (aka 0.4.0-rc2) omits the scales rather then set them to zero 
#if MING_VERSION_CODE <= 00040200
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=-1, b=-0.97, c=-1, d=1, tx=13.95, ty=214.8)'");
	check_equals(mo, "Math.round(staticmc._xscale)", "-139");
	check_equals(mo, "Math.round(staticmc._yscale)", "141");
	check_equals(mo, "Math.round(staticmc._rotation)", "44");  
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'-46,156 74,274'");
#else
	check_equals(mo, "printMatrix(staticmc.transform.matrix, 2)", "'(a=0, b=-0.97, c=-1, d=0, tx=13.95, ty=214.8)'");
	check_equals(mo, "Math.round(staticmc._xscale)", "-97");
	check_equals(mo, "Math.round(staticmc._yscale)", "100");
	check_equals(mo, "Math.round(staticmc._rotation)", "90");  
	check_equals(mo, "printBounds(staticmc.getBounds(_root), 0)", "'-16,186 44,244'");
#endif

	SWFMovie_nextFrame(mo);
	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, 1, 0, 0, 1, 0, 0); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=1, b=0, c=0, d=1, tx=0, ty=0)'");
	check_equals(mo, "staticmc._rotation", "0");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "staticmc._yscale", "100");

	add_actions(mo, "staticmc._yscale = -200;"); // change _yscale using ActionScript
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-30.05,-60.1 30.05,60.1'");
	check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=1, b=0, c=0, d=-2, tx=0, ty=0)'");
	check_equals(mo, "staticmc._rotation", "0");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "staticmc._yscale", "-200");

	add_actions(mo, "staticmc._rotation = -90;"); // change _rotation using ActionScript
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-60.1,-30.05 60.1,30.05'");
	check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=0, b=-1, c=-2, d=0, tx=0, ty=0)'");
	check_equals(mo, "staticmc._rotation", "-90");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "staticmc._yscale", "-200");

	add_actions(mo, "staticmc._rotation = 90;"); // change _rotation using ActionScript
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-60.1,-30.05 60.1,30.05'");
	check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=0, b=1, c=2, d=0, tx=0, ty=0)'");
	check_equals(mo, "staticmc._rotation", "90");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "staticmc._yscale", "-200");

	SWFMovie_nextFrame(mo);
	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, 1, 0, 0, -2, 0, 0); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-30.05,-60.1 30.05,60.1'");
	check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=1, b=0, c=0, d=-2, tx=0, ty=0)'");
	check_equals(mo, "staticmc._rotation", "0");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "staticmc._yscale", "200");

	add_actions(mo, "staticmc._rotation = -90;"); // change _rotation using ActionScript
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-60.1,-30.05 60.1,30.05'");
	check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=0, b=-1, c=-2, d=0, tx=0, ty=0)'");
	check_equals(mo, "staticmc._rotation", "-90");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "staticmc._yscale", "200");

	add_actions(mo, "staticmc._rotation = 90;"); // change _rotation using ActionScript
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-60.1,-30.05 60.1,30.05'");
	check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=0, b=1, c=2, d=0, tx=0, ty=0)'");
	check_equals(mo, "staticmc._rotation", "90");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "staticmc._yscale", "200");

	SWFMovie_nextFrame(mo);
	SWFDisplayItem_remove(it);
	it = add_static_mc(mo, "staticmc", 4, 0, 0, 60, 60);
	SWFDisplayItem_setMatrix(it, 1, 0, 0, 1, 0, 0); 
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-30.05,-30.05 30.05,30.05'");
	check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=1, b=0, c=0, d=1, tx=0, ty=0)'");
	check_equals(mo, "staticmc._rotation", "0");
	check_equals(mo, "staticmc._xscale", "100");
	check_equals(mo, "staticmc._yscale", "100");

	add_actions(mo, "staticmc._xscale = -200;"); // change _yscale using ActionScript
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-60.1,-30.05 60.1,30.05'");
	check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=-2, b=0, c=0, d=1, tx=0, ty=0)'");
	check_equals(mo, "staticmc._rotation", "0");
	check_equals(mo, "staticmc._xscale", "-200");
	check_equals(mo, "staticmc._yscale", "100");

	add_actions(mo, "staticmc._rotation = -90;"); // change _rotation using ActionScript
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-30.05,-60.1 30.05,60.1'");
	check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=0, b=2, c=1, d=0, tx=0, ty=0)'");
	check_equals(mo, "staticmc._rotation", "-90");
	check_equals(mo, "staticmc._xscale", "-200");
	check_equals(mo, "staticmc._yscale", "100");

	add_actions(mo, "staticmc._rotation = 90;"); // change _rotation using ActionScript
	check_equals(mo, "printBounds(staticmc.getBounds(_root))", "'-30.05,-60.1 30.05,60.1'");
	check_equals(mo, "staticmc.transform.matrix.toString()", "'(a=0, b=-2, c=-1, d=0, tx=0, ty=0)'");
	check_equals(mo, "staticmc._rotation", "90");
	check_equals(mo, "staticmc._xscale", "-200");
	check_equals(mo, "staticmc._yscale", "100");

	SWFMovie_nextFrame(mo);

	add_actions(mo, "_root.totals(1083); stop();");
	SWFMovie_nextFrame(mo);        

	//Output movie
	puts("Saving " OUTPUT_FILENAME );
	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
