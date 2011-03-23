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
 *  frame2: movieclip 1 placed at depth -16381 and position 100,300
 *  frame3: instance at depth -16381 replaced by DisplayObject 2 at position 130,330
 *  frame4: jump back to frame 2 and stop
 * 
 * Expected behaviour:
 * 
 *  In frame 2 we have a red square at (100,300), in frame 3 we have a red square at (130,330),
 *  after the jump-back we have a red square at (100,300) again.
 *  The name specified in the PlaceObject2 tag in frame 2 always point to the same instance
 */

#include "ming_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "replace_sprites1test.swf"

SWFDisplayItem add_static_mc(SWFMovie mo, const char* name, int depth, int x, int y, int width, int height);
SWFMovieClip get_static_mc(int width, int height, int r, int g, int b);
SWFShape get_shape(int width, int height, int r, int g, int b);

SWFShape
get_shape(int width, int height, int r, int g, int b)
{
  SWFShape sh;

  sh = make_fill_square (0, 0, width, height, r, g, b, r, g, b);

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
  SWFMovieClip static1, static2;
  SWFDisplayItem it1;


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

  static1 = get_static_mc(60, 60, 255, 0, 0);
  static2 = get_static_mc(60, 60, 0, 255, 0);

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFMovie_nextFrame(mo); 

  // Frame 2: place DisplayObject at depth 3 (-16381)
  it1 = SWFMovie_add(mo, (SWFBlock)static1);
  SWFDisplayItem_setDepth(it1, 3);
  SWFDisplayItem_moveTo(it1, 100, 300);
  SWFDisplayItem_setName(it1, "static1");
  SWFDisplayItem_addAction(it1, newSWFAction(
			"_root.note(this+' onClipConstruct (place)');"
		        " if ( isNaN(_root.depth3Constructed) ) {"
			"	_root.depth3Constructed=1; "
			" } else {"
			"	_root.depth3Constructed++;"
			" }"
			" _root.note('_root.depth3Constructed set to '+_root.depth3Constructed);"
			), SWFACTION_CONSTRUCT);
  SWFDisplayItem_addAction(it1, newSWFAction(
			"_root.note(this+' onClipUnload (place)');"
		        " if ( isNaN(_root.depth3Unloaded) ) {"
			"	_root.depth3Unloaded=1; "
			" } else {"
			"	_root.depth3Unloaded++;"
			" }"
			" _root.note('_root.depth3Unloaded set to '+_root.depth3Unloaded);"
			), SWFACTION_UNLOAD);
  add_actions(mo, "static1.name='static1';"); 

  check_equals(mo, "typeof(static1)", "'movieclip'"); 

  // This is important to verify, see next check for it after REPLACE
  check_equals(mo, "static1.name", "'static1'");

  check_equals(mo, "static1._target", "'/static1'");

  SWFMovie_nextFrame(mo); 

  // Frame 3: replace instance at depth -16381 with DisplayObject 2
  if ( SWFMovie_replace(mo, it1, (SWFBlock)static2) )
  {
	  abort(); // grace and beauty...
  }
  SWFDisplayItem_moveTo(it1, 130, 330);
  SWFDisplayItem_setName(it1, "static2");
  SWFDisplayItem_addAction(it1, newSWFAction(
			"_root.note(this+' onClipConstruct (replace)');"
			" _root.check_equals(typeof(_root), 'movieclip');"
		        " if ( isNaN(_root.depth3Constructed) ) {"
			"	_root.depth3Constructed=1; "
			" } else {"
			"	_root.depth3Constructed++;"
			" }"
			" _root.note('_root.depth3Constructed set to '+_root.depth3Constructed);"
			), SWFACTION_CONSTRUCT);
  SWFDisplayItem_addAction(it1, newSWFAction(
			"_root.note(this+' onClipUnload (replace)');"
		        " if ( isNaN(_root.depth3Unloaded) ) {"
			"	_root.depth3Unloaded=1; "
			" } else {"
			"	_root.depth3Unloaded++;"
			" }"
			" _root.note('_root.depth3Unloaded set to '+_root.depth3Unloaded);"
			), SWFACTION_UNLOAD);


  // Can still reference the old DisplayObject and it's variables, after replace
  check_equals(mo, "typeof(static1)", "'movieclip'"); 
  check_equals(mo, "static1.name", "'static1'");
  check_equals(mo, "static1._target", "'/static1'");
  check_equals(mo, "static1._x", "130");

  // While the new name results undefined...
  check_equals(mo, "typeof(static2)", "'undefined'"); 

  // Everything suggests that a new instance is NOT created on replace !!!
  // Gnash here fails because it creates a NEW instance

  // Anyway, the old DisplayObject matrix changed to 130,330 !
  check_equals(mo, "static1._x", "130");

  // We can't check the color in a self-contained testcase unfortunately,
  // we'll need a MovieTester-based runner for this.
  // It is expected the color of the current instance to be RED
  // TODO: implement a MovieTester based runner !!

  SWFMovie_nextFrame(mo); 
 
  // Frame 4: jump to frame 2, stop and check

  add_actions(mo,

    "gotoAndStop(2); " 

    // A single instance has been constructed !!
    "check_equals(_root.depth3Constructed, '1');"

    // Original DisplayObject name is still referenceable
    "check_equals(typeof(static1), 'movieclip');"

    // And it still has it's user-provided property
    "check_equals(static1.name, 'static1');"

    // The instance have been moved back to its original position (100,300)
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
