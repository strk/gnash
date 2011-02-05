/*
 *   Copyright (C) 2005, 2006, 2007, 2009, 2010, 2011 Free Software Foundation, Inc.
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
 * frame2: gotoAndPlay(4);
 * frame4: place "mc1" at depth3; gotoAndPlay(6);
 * frame5: place "mc2" at depth4;
 * frame6: stop;
 *
 * expected behaviour:
 *  first frame actions in "mc1" and "mc2" should not be executed.
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "action_execution_order_test9.swf"

SWFDisplayItem add_static_mc(SWFMovie mo, const char* name,
        const char* scripts, int depth);

SWFDisplayItem
add_static_mc(SWFMovie mo, const char* name, const char* scripts, int depth)
{
  SWFMovieClip mc;
  SWFDisplayItem it;

  mc = newSWFMovieClip();
  add_clip_actions(mc, scripts);
  SWFMovieClip_nextFrame(mc);

  it = SWFMovie_add(mo, (SWFBlock)mc);
  SWFDisplayItem_setDepth(it, depth); 
  SWFDisplayItem_setName(it, name);

  return it;
}

int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  dejagnuclip;
  SWFDisplayItem it1, it2;
  
  const char *srcdir=".";
  if ( argc>1 ) 
    srcdir=argv[1];
  else
  {
      fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
      return 1;
  }

  Ming_init();
  mo = newSWFMovieWithVersion(OUTPUT_VERSION);
  SWFMovie_setDimension(mo, 800, 600);
  SWFMovie_setRate (mo, 12.0);

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFMovie_nextFrame(mo); // 1st frame 

  add_actions(mo, " gotoAndPlay(4); ");
  SWFMovie_nextFrame(mo); // 2nd frame
  
  SWFMovie_nextFrame(mo); // 3rd frame 
    
  it1 = add_static_mc(mo, "mc1", "_root.x = 100;", 3);
  add_actions(mo, " gotoAndPlay(6); ");
  SWFMovie_nextFrame(mo); // 4th frame 

  SWFDisplayItem_remove(it1);
  it2 = add_static_mc(mo, "mc2", "_root.x = 200;", 4); 
  SWFMovie_nextFrame(mo); // 5th frame 

  SWFDisplayItem_remove(it2);
  // Gnash fails because actions in "mc1" got executed, see expected behaviour
  check_equals(mo, "typeof(_root.x)", "'undefined'");
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); // 6th frame 
  
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}


