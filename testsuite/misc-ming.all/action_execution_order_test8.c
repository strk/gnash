/*
 *   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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


#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "action_execution_order_test8.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc1, mc2, dejagnuclip;
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
  SWFMovie_nextFrame(mo); /* 1st frame */

  add_actions(mo, " if(check == 1) gotoAndPlay(4); ");
  SWFMovie_nextFrame(mo); /* 2nd frame */
  
  add_actions(mo, " check = 1; gotoAndPlay(2); ");
  SWFMovie_nextFrame(mo); /* 3rd frame */
  
  
  mc1 = newSWFMovieClip();
  add_clip_actions(mc1, " _root.gotoAndStop(6);"
                        " _root.x = 100; " );
  SWFMovieClip_nextFrame(mc1);
  
  /* add mc1 to _root and name it as "mc1" */
  it1 = SWFMovie_add(mo, (SWFBlock)mc1);  
  SWFDisplayItem_setDepth(it1, 3); 
  SWFDisplayItem_setName(it1, "mc1"); 
  check_equals(mo, "typeof(_root.x)", "'undefined'");
  add_actions(mo, " _root.x = 200; ");
  SWFMovie_nextFrame(mo); /* 4th frame */


  SWFDisplayItem_remove(it1);
  
  mc2 = newSWFMovieClip();
  // these actions are expected to be skipped with SWF version higher then 4
  add_clip_actions(mc2, " _root.note(' your player version is lower than  5');"
                        " _root.note(' Or your player is bogus'); "
                        " fail = 0; "
                        //this should not be executed with SWF6
                        " _root.xcheck_equals(fail, 1); " ); 
  it2 = SWFMovie_add(mo, (SWFBlock)mc2);  
  SWFDisplayItem_setDepth(it2, 3); 
  SWFDisplayItem_setName(it2, "mc2"); 
  SWFMovieClip_nextFrame(mc2);
  add_actions(mo, " stop(); ");  
   
  SWFMovie_nextFrame(mo); /* 5th frame */

  SWFDisplayItem_remove(it2);
  check_equals(mo, "_root.x", "200");
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); /* 6th frame */
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}



