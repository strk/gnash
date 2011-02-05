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


#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "action_execution_order_test7.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc, dejagnuclip;

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

  mc = newSWFMovieClip();
  add_clip_actions(mc, "_root.x = 1;"
                       "_root.check_equals(_root.x, 1);");
  SWFMovieClip_nextFrame(mc);
 
 
  add_actions(mo, " gotoAndPlay(4); ");
  SWFMovie_nextFrame(mo); /* 2nd frame */
  
  SWFMovie_nextFrame(mo); /* 3rd frame */
  
   
  /* add mc to _root and name it as "mc" */
  SWFDisplayItem it;
  it = SWFMovie_add(mo, (SWFBlock)mc);  
  SWFDisplayItem_setDepth(it, 3); 
  SWFDisplayItem_setName(it, "mc"); 
  add_actions(mo, " _root.x = 2; "
                  " _root.check_equals(_root.x, 2); ");
  SWFMovie_nextFrame(mo); /* 4th frame */

	SWFDisplayItem_remove(it);
  check_equals(mo, "_root.x", "1");
  check_equals(mo, "typeof(mc)", "'undefined'");
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); /* 5th frame */

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}



