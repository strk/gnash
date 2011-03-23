/*
 *   Copyright (C) 2005, 2006, 2007, 2009, 2010,
 *   2011 Free Software Foundation, Inc.
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
 * Zou Lunkai, zoulunkai@gmail.com
 *
 * This file was intended to test reverse execution of frame tags. 
 * However, the concept of reverse execution may be deprecated someday.
 *
 * frame2: place mc
 * frame3: move mc
 * frame4: move mc
 * frame5: do nothing
 * frame6: remove mc and goto frame5
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "reverse_execute_PlaceObject2_test2.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc, dejagnuclip;
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
  add_actions(mo, " _root.x1 = ''; _root.x2 = ''; ");
  SWFMovie_nextFrame(mo); /* 1st frame */

  
  mc = newSWFMovieClip();
  sh = make_fill_square (0, 0, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc, (SWFBlock)sh);  
  SWFMovieClip_nextFrame(mc);//1st frame
 
  /* add mc to _root and name it as "mc" */
  SWFDisplayItem it;
  it = SWFMovie_add(mo, (SWFBlock)mc);  
  SWFDisplayItem_setDepth(it, 10); 
  
  SWFDisplayItem_addAction(it,
    compileSWFActionCode(" _root.x1 += 'onLoad+'; "),
    SWFACTION_ONLOAD);
  SWFDisplayItem_addAction(it,
    compileSWFActionCode(" _root.x2 += 'onUnload+'; "),
    SWFACTION_UNLOAD);
    
  SWFDisplayItem_setName(it, "mc"); 
  check_equals(mo, "_root.mc._x", "0");
  SWFMovie_nextFrame(mo); /* 2nd frame */

  check_equals(mo, "_root.mc._x", "300");
  SWFDisplayItem_move(it, 300.0, 300.0);
  SWFMovie_nextFrame(mo); /* 3rd frame */ 

  check_equals(mo, "_root.mc._x", "900");
  SWFDisplayItem_move(it, 600.0, 600.0);
  SWFMovie_nextFrame(mo); /* 4th frame */
  
  check_equals(mo, "_root.mc._x", "900");
  SWFMovie_nextFrame(mo); /* 5th frame */
  
  SWFDisplayItem_remove(it);
  add_actions(mo, " if(stopflag != 1)  gotoAndPlay(5);  stopflag = 1; ");
  SWFMovie_nextFrame(mo); /* 6th frame */

  //checks 
  check_equals(mo, "_root.x1", "'onLoad+onLoad+'" );
  check_equals(mo, "_root.x2", "'onUnload+onUnload+'" );
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); /* 7th frame */
  
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}



