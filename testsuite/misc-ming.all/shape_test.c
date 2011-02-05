/* 
 *   Copyright (C) 2005, 2006, 2009, 2010, 2011 Free Software Foundation, Inc.
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
 *  (1) Verify that shapes can not hold variables, but rather evaluates to their
 *      containing sprite when referenced by name.
 *
 *  (2) See effects of inconsistent drawing 
 *
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 7
#define OUTPUT_FILENAME "shape_test.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip dejagnuclip;
  SWFDisplayItem it;
  SWFShape  sh1,sh2;

  const char *srcdir=".";
  if ( argc>1 ) 
    srcdir=argv[1];
  else
  {
      //fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
      //return 1;
  }

  Ming_init();
  mo = newSWFMovie();
  SWFMovie_setDimension(mo, 800, 600);
  //SWFMovie_setRate (mo, 1.0);

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFMovie_nextFrame(mo); 

  sh1 = make_fill_square (100, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  sh2 = make_fill_square (300, 300, 60, 60, 255, 0, 0, 0, 0, 0);
  
  it = SWFMovie_add(mo, (SWFBlock)sh1);  
  SWFDisplayItem_setName(it, "sh1"); 
  SWFDisplayItem_setDepth(it, 3); //place the sh1 DisplayObject at depth 3;
  
  it = SWFMovie_add(mo, (SWFBlock)sh2);  
  SWFDisplayItem_setName(it, "sh2"); 
  SWFDisplayItem_setDepth(it, 4); //place the sh2 DisplayObject at depth 4;

  check(mo, "sh1 != undefined");
  check(mo, "sh2 != undefined");
  
  // Do these checks mean that shapes are movieclips?
  // seems not.
  check_equals(mo, "typeof(sh1)", "'movieclip'");
  check_equals(mo, "typeof(sh2)", "'movieclip'");
  check_equals(mo, "typeof(_root)", "'movieclip'");
  
  add_actions(mo, 
    "sh1.var1 = 10;"
    "sh2.var2 = 20;"
    );

  // Do these checks mean that we can add variables to shapes?
  // seems not, variable are added to the _root, interesting.
  check_equals(mo, "sh1.var1", "10");
  check_equals(mo, "sh2.var2", "20");
  check_equals(mo, "_root.var1", "10");
  check_equals(mo, "_root.var2", "20");
  
  check_equals(mo, "sh1._x", "0");
  check_equals(mo, "sh2._x", "0");

  add_actions(mo, 
    "sh1._x = 0;"
    "sh2._x = 400;"
    );

  check_equals(mo, "sh1._x", "400");
  check_equals(mo, "sh2._x", "400");
  check_equals(mo, "_root._x", "400");

  add_actions(mo, "_root._x = 0;" ); /* cleanup */
    
  // Do these checks mean that shapes are *not* movieclips?
  check_equals(mo, "typeof(sh1.getDepth())", "'undefined'");
  check_equals(mo, "typeof(sh2.getDepth())", "'undefined'");
    
  // Do these checks mean that shapes are *not* movieclips?
  check_equals(mo, "typeof(getInstanceAtDepth(-16381))", "'undefined'");
  check_equals(mo, "typeof(getInstanceAtDepth(-16380))", "'undefined'");

  SWFMovie_nextFrame(mo); 

  /*
   * UdoG's drawing
   *
   * See DrawingApiTest.as (inv8)
   *
   */
  { /*  using left fill, non-closed paths */
	SWFDisplayItem it1, it2;
	SWFShape sh = newSWFShape();
	SWFMovieClip mc = newSWFMovieClip();
	SWFShape_setLineStyle(sh, 1, 0, 0, 0, 255);
	SWFShape_setLeftFillStyle(sh, SWFShape_addSolidFillStyle(sh, 0, 255, 0, 255));
	SWFShape_movePenTo(sh, 20, 10);		/* 0 */
	SWFShape_drawLineTo(sh, 40, 10);	/* 1 */
	SWFShape_drawLineTo(sh, 40, 40);	/* 2 */
	SWFShape_drawLineTo(sh, 20, 40);	/* 3 */
	SWFShape_drawLineTo(sh, 20, 10);	/* 4 */
	SWFShape_drawLineTo(sh, 10, 10);	/* 5 */
	SWFShape_drawLineTo(sh, 10, 20);	/* 6 */
	SWFShape_drawLineTo(sh, 30, 20);	/* 7 */
	SWFShape_drawLineTo(sh, 30, 30);	/* 8 */
	SWFShape_drawLineTo(sh, 20, 30);	/* 9 */
	it1 = SWFMovieClip_add(mc, (SWFBlock)sh);

	// Test that clip events are not invoked for shapes
#if 0 // current Ming HEAD chokes if we add an onClipConstruct event... 
	SWFDisplayItem_addAction(it1, newSWFAction( 
		"_root.check(false && 'clip event for shape should not be executed');"
		SWFACTION_CONSTRUCT);
#endif
	SWFDisplayItem_addAction(it1, newSWFAction( 
		"_root.check(false && 'clip event for shape should not be executed');"
		), SWFACTION_ENTERFRAME);
	SWFDisplayItem_addAction(it1, newSWFAction( 
		"_root.check(false && 'clip event for shape should not be executed');"
		), SWFACTION_ONLOAD);
	SWFDisplayItem_addAction(it1, newSWFAction( 
		"_root.check(false && 'clip event for shape should not be executed');"
		), SWFACTION_UNLOAD);
	SWFDisplayItem_addAction(it1, newSWFAction( 
		"_root.check(false && 'clip event for shape should not be executed');"
		), SWFACTION_MOUSEMOVE);
	SWFDisplayItem_addAction(it1, newSWFAction( 
		"_root.check(false && 'clip event for shape should not be executed');"
		), SWFACTION_MOUSEDOWN);
	// None of these should be executed
	SWFDisplayItem_addAction(it1, newSWFAction(
		"_root.check(false && 'clip event for shape should not be executed');"
		), SWFACTION_ONLOAD);

	SWFDisplayItem_moveTo(it1, 80, 120);
	SWFDisplayItem_scale(it1, 2, 2);
	SWFMovieClip_nextFrame(mc);
	it2 = SWFMovie_add(mo, (SWFBlock)mc);
  }
  { /*  using right fill, non-closed paths */
	SWFDisplayItem it;
	SWFShape sh = newSWFShape();
	SWFMovieClip mc = newSWFMovieClip();
	SWFShape_setLineStyle(sh, 1, 0, 0, 0, 255);
	SWFShape_setLeftFillStyle(sh, SWFShape_addSolidFillStyle(sh, 0, 255, 0, 255));
	SWFShape_movePenTo(sh, 20, 10);		/* 0 */
	SWFShape_drawLineTo(sh, 40, 10);	/* 1 */
	SWFShape_drawLineTo(sh, 40, 40);	/* 2 */
	SWFShape_drawLineTo(sh, 20, 40);	/* 3 */
	SWFShape_drawLineTo(sh, 20, 10);	/* 4 */
	SWFShape_drawLineTo(sh, 10, 10);	/* 5 */
	SWFShape_drawLineTo(sh, 10, 20);	/* 6 */
	SWFShape_drawLineTo(sh, 30, 20);	/* 7 */
	SWFShape_drawLineTo(sh, 30, 30);	/* 8 */
	SWFShape_drawLineTo(sh, 20, 30);	/* 9 */
	it = SWFMovieClip_add(mc, (SWFBlock)sh);
	SWFDisplayItem_moveTo(it, 200, 120);
	SWFDisplayItem_scale(it, 2, 2);
	SWFMovieClip_nextFrame(mc);
	it = SWFMovie_add(mo, (SWFBlock)mc);
  }
  { /*  using left fill, closed paths */
	SWFDisplayItem it;
	SWFShape sh = newSWFShape();
	SWFMovieClip mc = newSWFMovieClip();
	SWFShape_setLineStyle(sh, 1, 0, 0, 0, 255);
	SWFShape_setLeftFillStyle(sh, SWFShape_addSolidFillStyle(sh, 255, 0, 0, 255));
	SWFShape_movePenTo(sh, 20, 10);		/* 0 */
	SWFShape_drawLineTo(sh, 40, 10);	/* 1 */
	SWFShape_drawLineTo(sh, 40, 40);	/* 2 */
	SWFShape_drawLineTo(sh, 20, 40);	/* 3 */
	SWFShape_drawLineTo(sh, 20, 10);	/* 4 */
	SWFShape_drawLineTo(sh, 10, 10);	/* 5 */
	SWFShape_drawLineTo(sh, 10, 20);	/* 6 */
	SWFShape_drawLineTo(sh, 30, 20);	/* 7 */
	SWFShape_drawLineTo(sh, 30, 30);	/* 8 */
	SWFShape_drawLineTo(sh, 20, 30);	/* 9 */
	SWFShape_drawLineTo(sh, 20, 10);	/* 0 */
	it = SWFMovieClip_add(mc, (SWFBlock)sh);
	SWFDisplayItem_moveTo(it, 80, 200);
	SWFDisplayItem_scale(it, 2, 2);
	SWFMovieClip_nextFrame(mc);
	it = SWFMovie_add(mo, (SWFBlock)mc);
  }
  { /*  using right fill, closed paths */
	SWFDisplayItem it;
	SWFShape sh = newSWFShape();
	SWFMovieClip mc = newSWFMovieClip();
	SWFShape_setLineStyle(sh, 1, 0, 0, 0, 255);
	SWFShape_setRightFillStyle(sh, SWFShape_addSolidFillStyle(sh, 255, 0, 0, 255));
	SWFShape_movePenTo(sh, 20, 10);		/* 0 */
	SWFShape_drawLineTo(sh, 40, 10);	/* 1 */
	SWFShape_drawLineTo(sh, 40, 40);	/* 2 */
	SWFShape_drawLineTo(sh, 20, 40);	/* 3 */
	SWFShape_drawLineTo(sh, 20, 10);	/* 4 */
	SWFShape_drawLineTo(sh, 10, 10);	/* 5 */
	SWFShape_drawLineTo(sh, 10, 20);	/* 6 */
	SWFShape_drawLineTo(sh, 30, 20);	/* 7 */
	SWFShape_drawLineTo(sh, 30, 30);	/* 8 */
	SWFShape_drawLineTo(sh, 20, 30);	/* 9 */
	SWFShape_drawLineTo(sh, 20, 10);	/* 0 */
	it = SWFMovieClip_add(mc, (SWFBlock)sh);
	SWFDisplayItem_moveTo(it, 200, 200);
	SWFDisplayItem_scale(it, 2, 2);
	SWFMovieClip_nextFrame(mc);
	it = SWFMovie_add(mo, (SWFBlock)mc);
  }

  add_actions(mo, "_root.totals(); stop();");

  SWFMovie_nextFrame(mo); 

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
