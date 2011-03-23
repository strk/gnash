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
 *
 * expected behaviour:
 * (1) Within the same frame, if PlaceObject(mc1) is before PlaceObject(mc2), 
 *     then frame0 actions of mc1 should be executed before frame0 actions of mc2.
 * (2) Within the same timeline, frame actions(frameNum>0) of fisrt placed sprites executed last.
 *
 * The actual order of tags are dependent on compiler, so you need to 
 * verify first if the order of tags is what you expect.
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "action_execution_order_test2.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc_red1, mc_red2, mc_red3, mc_red4, mc_red5, dejagnuclip;
  SWFDisplayItem it_red1, it_red2, it_red3, it_red4, it_red5;
  SWFShape  sh_red;

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

  
  mc_red1 = newSWFMovieClip();
  sh_red = make_fill_square (0, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc_red1, (SWFBlock)sh_red);  
  add_clip_actions(mc_red1, " _root.x1 += \"depth10+\"; ");
  SWFMovieClip_nextFrame(mc_red1); /* mc_red1, 1st frame */
  add_clip_actions(mc_red1, " _root.x2 += \"depth10+\"; stop(); ");
  SWFMovieClip_nextFrame(mc_red1); /* mc_red1, 2nd frame */
 
  mc_red2 = newSWFMovieClip();
  sh_red = make_fill_square (80, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc_red2, (SWFBlock)sh_red); 
  add_clip_actions(mc_red2, " _root.x1 += \"depth12+\"; "); 
  SWFMovieClip_nextFrame(mc_red2); /* mc_red2, 1st frame */
  add_clip_actions(mc_red2, " _root.x2 += \"depth12+\"; stop(); "); 
  SWFMovieClip_nextFrame(mc_red2); /* mc_red2, 2nd frame */
  
  mc_red3 = newSWFMovieClip();
  sh_red = make_fill_square (160, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc_red3, (SWFBlock)sh_red);  
  add_clip_actions(mc_red3, " _root.x1 += \"depth11+\"; "); 
  SWFMovieClip_nextFrame(mc_red3); /* mc_red3, 1st frame */
  add_clip_actions(mc_red3, " _root.x2 += \"depth11+\";  stop();"); 
  SWFMovieClip_nextFrame(mc_red3); /* mc_red3, 2nd frame */
  
  
  /* add mc_red1 to _root and name it as "mc_red1" */
  it_red1 = SWFMovie_add(mo, (SWFBlock)mc_red1);  
  SWFDisplayItem_setDepth(it_red1, 10); 
  SWFDisplayItem_setName(it_red1, "mc_red1"); 
  
  /* add mc_red2 to _root and name it as "mc_red2" */
  it_red2 = SWFMovie_add(mo, (SWFBlock)mc_red2);  
  SWFDisplayItem_setDepth(it_red2, 12); 
  SWFDisplayItem_setName(it_red2, "mc_red2"); 
  
  /* add mc_red3 to _root and name it as "mc_red3" */
  it_red3 = SWFMovie_add(mo, (SWFBlock)mc_red3);  
  SWFDisplayItem_setDepth(it_red3, 11); 
  SWFDisplayItem_setName(it_red3, "mc_red3"); 
  
  SWFMovie_nextFrame(mo); /* 2nd frame */

  /* Action order is not dependent on DisplayList depth here! */
  check_equals(mo, "_root.x1", "'depth10+depth12+depth11+'");
  SWFMovie_nextFrame(mo); /* 3rd frame */

  mc_red4 = newSWFMovieClip();
  sh_red = make_fill_square (240, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc_red4, (SWFBlock)sh_red);  
  add_clip_actions(mc_red4, " _root.x2 += \"depth9+\"; "); 
  SWFMovieClip_nextFrame(mc_red4); /* mc_red4, 1st frame */
  
   /* add mc_red4 to _root and name it as "mc_red4" */
  it_red4 = SWFMovie_add(mo, (SWFBlock)mc_red4);  
  SWFDisplayItem_setDepth(it_red4, 9); 
  SWFDisplayItem_setName(it_red4, "mc_red4"); 
  
  mc_red5 = newSWFMovieClip();
  sh_red = make_fill_square (240, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc_red5, (SWFBlock)sh_red);  
  add_clip_actions(mc_red5, " _root.x2 += \"depth13+\"; "); 
  SWFMovieClip_nextFrame(mc_red5); /* mc_red4, 1st frame */
  
  /* add mc_red5 to _root and name it as "mc_red5" */
  it_red5 = SWFMovie_add(mo, (SWFBlock)mc_red5);  
  SWFDisplayItem_setDepth(it_red5, 13); 
  SWFDisplayItem_setName(it_red5, "mc_red4"); 
  
  SWFMovie_nextFrame(mo); /* 4th frame */


  check_equals(mo, "_root.x2", "'depth11+depth12+depth10+depth9+depth13+'");
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); /* 5th frame */
  
  
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}



