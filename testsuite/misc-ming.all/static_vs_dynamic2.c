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
 * create(by tag) "mc1" in static depth;
 * create(by AS) "dup" in dynamic depth;
 * swap them;
 * try to move and remove the DisplayObject in static depth;
 *
 * TODO: add test for REPLACE.
 * Question: how to REPLACE a DisplayObject?
 */
  

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "static_vs_dynamic2.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc1, mc2, dejagnuclip;
  SWFDisplayItem it;
  SWFShape  sh;

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
  SWFMovie_nextFrame(mo); 

  mc1 = newSWFMovieClip();
  sh = make_fill_square (0, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc1, (SWFBlock)sh);  
  SWFMovieClip_nextFrame(mc1);
    
  it = SWFMovie_add(mo, (SWFBlock)mc1);  
  SWFDisplayItem_setDepth(it, 2); 
  SWFDisplayItem_setName(it, "mc1");
  
  add_actions(mo, "duplicateMovieClip('mc1', 'dup', 1);"
                  "check_equals(mc1.getDepth(), -16382);"
                  "check_equals(dup.getDepth(), 1);"
                  "mc1.swapDepths(dup);"
                  "check_equals(mc1.getDepth(), 1);"
                  "check_equals(dup.getDepth(), -16382);"
                  "check_equals(mc1._x, 0);"
                  "check_equals(dup._x, 0);");
  SWFMovie_nextFrame(mo);        

  /* PlaceObject2 can not move an as-created movieClip */
  SWFDisplayItem_move(it, 100, 100);
  check_equals(mo, "mc1._x", "0");
  check_equals(mo, "dup._x", "0");
  SWFMovie_nextFrame(mo); 
  
  /* PlaceObject2 can not move an as-created movieClip */
  SWFDisplayItem_moveTo(it, 300, 300);
  check_equals(mo, "mc1._x", "0");
  check_equals(mo, "dup._x", "0");
  SWFMovie_nextFrame(mo); 
  
  /* RemoveObject2 */
  SWFDisplayItem_remove(it);
  /* "mc1" keeps alive as it has been swapped to a dynamic depth */
  check_equals(mo, "typeof(mc1)", "'movieclip'");
  /* "dup" got removed by the RemoveObject2 tag */
  check_equals(mo, "typeof(dup)", "'undefined'");
  SWFMovie_nextFrame(mo); 
  
  
  /*
   * some more tests about removing a movieClip
  */
  mc2 = newSWFMovieClip();
  sh = make_fill_square (300, 300, 30, 30, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc2, (SWFBlock)sh);  
  SWFMovieClip_nextFrame(mc2);
    
  it = SWFMovie_add(mo, (SWFBlock)mc2);  
  SWFDisplayItem_setDepth(it, 3); 
  SWFDisplayItem_setName(it, "mc2");
  SWFMovie_nextFrame(mo); 
  
  check_equals(mo, "typeof(mc2)", "'movieclip'");
  add_actions(mo, "removeMovieClip(mc2);");
  /* can not remove a tag-created movieclip in static depth */
  check_equals(mo, "typeof(mc2)", "'movieclip'");
  add_actions(mo, "mc2.swapDepths(3);");
  add_actions(mo, "removeMovieClip(mc2);");
  /* tag-created movieclip can be removed in dynamic depth */
  check_equals(mo, "typeof(mc2)", "'undefined'");
  SWFMovie_nextFrame(mo); 
  
  add_actions(mo, " _root.totals(); stop();" );
  SWFMovie_nextFrame(mo);        
  
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
