/* 
 *   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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
 * Timeline:
 * 
 *   Frame  | 1 | 2 | 3 | 4 | 5 | 6 |
 *  --------+---+---+---+---+---+---+
 *   Event  |   |P  |RP | RP| * | J |
 * 
 *  P = place (by PlaceObject2)
 *  R = remove (by RemoveObject2)
 *  J = jump
 *  * = jump target
 *
 * Description:
 *
 *  frame2: place mc1 at depth 100 with ratio 0
 *  frame3: remove mc1, place mc2 at depth 100 with ratio 0.1
 *  frame4: remove mc2, place mc3 at depth 100 with ratio 0.2
 *  frame5: do nothing
 *  frame6: jump to frame5
 *
 * Observed behaviour:
 *
 *   (1)mc1 constructed and unloaded twice;
 *   (2)mc2 constructed and unloaded twice;
 *   (3)mc3 constructed only *ONCE*, never unloaded.
 *
 * Deduction:
 *
 *   we need a temporary displaylist when jumping back, otherwise cann't keep mc3 alive
 *   in the current design attempt.
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "loop_test10.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
   SWFDisplayItem it1, it2, it3;
  SWFMovieClip mc1, mc2, mc3, dejagnuclip;
  SWFShape  sh1, sh2, sh3;
  SWFAction ac;
  int i;

  const char *srcdir=".";
  if ( argc>1 ) 
    srcdir=argv[1];
  else
  {
      //fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
      //return 1;
  }

  Ming_init();
  Ming_useSWFVersion (OUTPUT_VERSION);
  
  mo = newSWFMovie();
  SWFMovie_setDimension(mo, 800, 600);
  SWFMovie_setRate(mo, 2);

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  add_actions(mo, " haslooped = false; "
                  " mc1Initialized=0; mc1Unloaded=0;"
                  " mc2Initialized=0; mc2Unloaded=0;"
                  " mc3Initialized=0; mc3Unloaded=0;"
                  " asOrder='0+';");
  SWFMovie_nextFrame(mo); // frame1


  sh1 = make_fill_square (100, 100, 60, 60, 255, 0, 0, 255, 0, 0);
  mc1 = newSWFMovieClip();
  SWFMovieClip_add(mc1, (SWFBlock)sh1); 
  SWFMovieClip_nextFrame(mc1);
  
  sh2 = make_fill_square (200, 200, 60, 60, 0, 0, 0, 0, 0, 0);
  mc2 = newSWFMovieClip();
  SWFMovieClip_add(mc2, (SWFBlock)sh2);  
  SWFMovieClip_nextFrame(mc2); 

  sh3 = make_fill_square (300, 300, 60, 60, 0, 0, 0, 0, 0, 0);
  mc3 = newSWFMovieClip();
  SWFMovieClip_add(mc3, (SWFBlock)sh3);  
  SWFMovieClip_nextFrame(mc3); 
 
  it1 = SWFMovie_add(mo, (SWFBlock)mc1);  
  SWFDisplayItem_setName(it1, "mc1"); 
  SWFDisplayItem_setDepth(it1, 100); 
  SWFDisplayItem_addAction(it1, newSWFAction(
        "_root.note(this+' initialized');"
        "_root.mc1Initialized++;"
        "_root.asOrder += '1+';"
        ), SWFACTION_INIT); 
  SWFDisplayItem_addAction(it1, newSWFAction(
        "_root.note(this+' unloaded');"
        "_root.mc1Unloaded++;"
        "_root.asOrder += '3+';"
        ), SWFACTION_UNLOAD);
  SWFMovie_nextFrame(mo); // frame2
  
    
  SWFDisplayItem_remove(it1);
  it2 = SWFMovie_add(mo, (SWFBlock)mc2); 
  SWFDisplayItem_setName(it2, "mc2"); 
  SWFDisplayItem_setRatio(it2, 0.1); 
  SWFDisplayItem_setDepth(it2, 100); 
  SWFDisplayItem_addAction(it2, newSWFAction(
        "_root.note(this+' initialized');"
        "_root.mc2Initialized++;"
        "_root.asOrder += '2+';"
        ), SWFACTION_INIT);
  SWFDisplayItem_addAction(it2, newSWFAction(
        "_root.note(this+' unloaded');"
        "_root.mc2Unloaded++;"
        "_root.asOrder += '5+';"
        ), SWFACTION_UNLOAD);
  SWFMovie_nextFrame(mo);  // frame3


  SWFDisplayItem_remove(it2);
  it3 = SWFMovie_add(mo, (SWFBlock)mc3); 
  SWFDisplayItem_setName(it3, "mc3"); 
  SWFDisplayItem_setRatio(it3, 0.2); 
  SWFDisplayItem_setDepth(it3, 100); 
  SWFDisplayItem_addAction(it3, newSWFAction(
        "_root.note(this+' initialized');"
        "_root.mc3Initialized++;"
        "_root.asOrder += '4+';"
        ), SWFACTION_INIT);
  SWFDisplayItem_addAction(it3, newSWFAction(
        "_root.note(this+' unloaded');"
        "_root.mc3Unloaded++;"
        ), SWFACTION_UNLOAD);
  SWFMovie_nextFrame(mo);  // frame4


  SWFMovie_nextFrame(mo);  // frame 5

  
  add_actions(mo, "if(! haslooped) {"
                  "   gotoAndPlay(5);"
                  "   haslooped = true;"
                  "}" );
  SWFMovie_nextFrame(mo);  // frame 6
  
  
  check_equals(mo, "mc1Initialized", "2");
  check_equals(mo, "mc2Initialized", "2");
  xcheck_equals(mo, "mc3Initialized", "1");
  check_equals(mo, "mc1Unloaded", "2");
  check_equals(mo, "mc2Unloaded", "2");
  xcheck_equals(mo, "mc3Unloaded", "0");
  
  // Don't bother to fix this order untill timeline control is fixed.
  xcheck_equals(mo, "asOrder", "'0+1+2+3+4+5+1+2+3+5+'");
  add_actions(mo, "totals(); stop();");
  SWFMovie_nextFrame(mo);  // frame 7
   
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
