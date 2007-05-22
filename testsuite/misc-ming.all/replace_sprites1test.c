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
 * Test  "Jump backward to start of lifetime after replacement with different sprite"
 *
 * run as ./replace_sprites1test
 *
 * Timeline:
 * 
 *   Frame  | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
 *  --------+---+---+---+---+---+---+---+
 *   Event  |   |P* | X | J |   |   |   |
 * 
 *  P = place (by PlaceObject2)
 *  X = replace (by PlaceObject2)
 *  J = jump
 *  * = jump target
 * 
 * Description:
 * 
 *  frame2: movieclip 1 placed at depth -16381 and position 100,100
 *  frame3: instance at depth -16381 replaced by character 2 at position 110,110
 *  frame4: jump back to frame 2 and stop
 * 
 * Expected behaviour:
 * 
 *  A single "movieclip" instances have been constructed in total.
 *  The instance contains a red shape at (100,100) initially, still a red shape
 *  at (110,110) after the replace, a red shape at (100,100) again on loop-back.
 */

#include "ming_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "replace_sprites1test.swf"

SWFDisplayItem add_static_mc(SWFMovie mo, const char* name, int depth, int x, int y, int width, int height);
SWFMovieClip get_static_mc(int width, int height, int r, int g, int b);

SWFShape
get_shape(int width, int height, int r, int g, int b)
{
  SWFShape sh;

  sh = make_fill_square (-(width/2), -(height/2), width, height, r, g, b, r, g, b);

  return sh;
}

SWFMovieClip
get_static_mc(int width, int height, int r, int g, int b)
{
  SWFShape sh = get_shape(width, height, r, g, b);
  SWFMovieClip mc = newSWFMovieClip();

  SWFMovieClip_add(mc, (SWFBlock)sh);

  SWFMovieClip_nextFrame(mc);

  return mc;

}

SWFDisplayItem
add_static_mc(SWFMovie mo, const char* name, int depth, int x, int y, int width, int height)
{
  SWFMovieClip mc;
  SWFDisplayItem it;

  mc = get_static_mc(width, height, 255, 0, 0);

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
  SWFShape static1, static2;
  int i;
  SWFDisplayItem it1, it2;


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

  static1 = get_static_mc(20, 20, 255, 0, 0);
  static2 = get_static_mc(20, 20, 0, 255, 0);

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFMovie_nextFrame(mo); 

  // Frame 2: place character at depth 3 (-16381)
  it1 = SWFMovie_add(mo, (SWFBlock)static1);
  SWFDisplayItem_setDepth(it1, 3);
  SWFDisplayItem_moveTo(it1, 100, 100);
  SWFDisplayItem_setName(it1, "static1");
  SWFDisplayItem_addAction(it1, newSWFAction(
			"_root.note(this+' onClipConstruct');"
			" _root.check_equals(typeof(_root), 'movieclip');"
		        " if ( isNaN(_root.depth3Constructed) ) {"
			"	_root.depth3Constructed=1; "
			" } else {"
			"	_root.depth3Constructed++;"
			" }"
			" _root.note('_root.depth3Constructed set to '+_root.depth3Constructed);"
			), SWFACTION_CONSTRUCT);
  add_actions(mo, "static1.name='static1';"); 
  SWFMovie_nextFrame(mo); 

  // Frame 3: replace instance at depth -16381 with character 2
  it2 = SWFDisplayItem_replace(it1, (SWFBlock)static2);
  SWFDisplayItem_moveTo(it2, 110, 110);
  SWFDisplayItem_setName(it2, "static2");
  SWFDisplayItem_addAction(it2, newSWFAction(
			"_root.note(this+' onClipConstruct');"
			" _root.check_equals(typeof(_root), 'movieclip');"
		        " if ( isNaN(_root.depth3Constructed) ) {"
			"	_root.depth3Constructed=1; "
			" } else {"
			"	_root.depth3Constructed++;"
			" }"
			" _root.note('_root.depth3Constructed set to '+_root.depth3Constructed);"
			), SWFACTION_CONSTRUCT);


  // Can still reference the old character and it's variables, after replace
  xcheck_equals(mo, "typeof(static1)", "'movieclip'"); 
  xcheck_equals(mo, "static1.name", "'static1'");

  // While we can NOT reference the new character by name
  xcheck_equals(mo, "typeof(static2)", "'undefined'"); // the name wasn't changed

  // Anyway, the old character matrix changed to 110,110 !
  xcheck_equals(mo, "static1._x", "110");

  // We can't check the color in a self-contained testcase unfortunately,
  // we'll need a MovieTester-based runner for this.
  // It is expected the color of the current instance to be RED
  // TODO: implement a MovieTester based runner !!

  SWFMovie_nextFrame(mo); 
 
  // Frame 4: jump to frame 2, stop and check

  add_actions(mo,

    "gotoAndStop(2); " 

    // A single instance has been constructed !!
    "xcheck_equals(_root.depth3Constructed, '1');"

    // Original character name is still referenceable
    "check_equals(typeof(static1), 'movieclip');"

    // And it still has it's user-provided property
    "xcheck_equals(static1.name, 'static1');"

    // The instance have been moved back to its original position (100,100)
    "check_equals(static1._x, 100);"

    // We can't check the color in a self-contained testcase unfortunately,
    // we'll need a MovieTester-based runner for this.
    // It is expected the color of the current instance to be RED
    // TODO: implement a MovieTester based runner !!

    "totals();"
    );
  SWFMovie_nextFrame(mo); 

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
