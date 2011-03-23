/* 
 *   Copyright (C) 2007, 2009, 2010, 2011 Free Software Foundation, Inc.
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
 * ==case1==
 *
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
 *   (1)mc1 initialized and unloaded twice;
 *   (2)mc2 initialized and unloaded twice;
 *   (3)mc3 initialized only *ONCE*, never unloaded.
 *
 * Deduction:
 *
 *   we need a temporary displaylist when jumping back, otherwise cann't keep mc3 alive
 *   in the current design attempt.
 *
 *
 * ==case2==
 *
 * timeline:
 *
 *   Frame  | 1 | 2 |
 *  --------+---+---+
 *   Event  | P | R |
 *
 * Description:
 *
 *  frame1: place an unnamed sprite 
 *  frame2: remove the sprite
 *
 * Observed behaviour:
 *  
 *  (1)before loop-back, the sprite has a synthesized name instance4
 *  (2)after loop-back, the sprite has a synthesized name instance5
 *
 *
 *  ==case3==
 *
 * timeline:
 *
 *   Frame  | 1 | 2 |
 *  --------+---+---+
 *   Event  | P |   |
 *
 * Description:
 *
 *  frame1: place an unnamed sprite 
 *  frame2: do nothing
 *
 * Observed behaviour:
 *  
 *  (1)before loop-back, the sprite has a synthesized name instance7
 *  (2)after loop-back, the sprite still has the name instance7
 *
 *  (3)but when placing another unnamed sprite after the loop back,
 *     the new instance gets a name instance9. 
 *
 *  Deduction on (3): instance8 was created and discarded during the loop-back.
 */


#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 7
#define OUTPUT_FILENAME "loop_test10.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFDisplayItem it1, it2, it3;
  SWFDisplayItem it41;
  SWFDisplayItem it51;
  SWFMovieClip mc1, mc2, mc3, dejagnuclip;
  SWFMovieClip mc4, mc41;
  SWFMovieClip mc5, mc51;
  SWFShape  sh1, sh2, sh3;

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
  SWFMovie_setRate(mo, 12);

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  add_actions(mo, " haslooped1=false; haslooped2=false; haslooped3=false;"
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

  
  add_actions(mo, "if(! haslooped1) {"
                  "   gotoAndPlay(5);"
                  "   haslooped1 = true;"
                  "}" );
  SWFMovie_nextFrame(mo);  // frame 6
  
  
  check_equals(mo, "mc1Initialized", "2");
  check_equals(mo, "mc2Initialized", "2");
  check_equals(mo, "mc3Initialized", "1");
  check_equals(mo, "mc1Unloaded", "2");
  check_equals(mo, "mc2Unloaded", "2");
  check_equals(mo, "mc3Unloaded", "0");
  
  xcheck_equals(mo, "asOrder", "'0+1+2+3+4+5+1+2+3+5+'");
  SWFMovie_nextFrame(mo);  // frame 7
  
  //
  // ==case 2==
  //
  mc4 = newSWFMovieClip();
    mc41 = newSWFMovieClip(); 
    SWFMovieClip_nextFrame(mc41);
    
    it41 = SWFMovieClip_add(mc4, (SWFBlock)mc41); 
    add_clip_actions(mc4, 
        "_root.check_equals(this._target, '/instance3');"
        "inst = this.getInstanceAtDepth(-16383);"
        "if(! haslooped2) {"
        "   haslooped2 = true;"
        "   _root.check_equals(inst._target, '/instance3/instance4');"
        "} else {"
        "   _root.check_equals(inst._target, '/instance3/instance5');"
        "   stop();"
        "}"
   );
  SWFMovieClip_nextFrame(mc4);
    SWFDisplayItem_remove(it41);
  SWFMovieClip_nextFrame(mc4);  
  
  SWFMovie_add(mo, mc4);
  SWFMovie_nextFrame(mo);  // frame 9
  
  SWFMovie_nextFrame(mo);  // frame 10
  
  //
  // ==case 3==
  //
  mc5 = newSWFMovieClip();
    mc51 = newSWFMovieClip(); 
    SWFMovieClip_nextFrame(mc51);
    
    it51 = SWFMovieClip_add(mc5, (SWFBlock)mc51); 
    add_clip_actions(mc5, 
        "_root.check_equals(this._target, '/instance6');"
        "inst = this.getInstanceAtDepth(-16383);"
        "if(! haslooped3) {"
        "   haslooped3 = true;"
        "   _root.check_equals(inst._target, '/instance6/instance7');"
        "} else {"
        "   _root.check_equals(inst._target, '/instance6/instance7');"
        "   stop();"
        "}"
   );
  SWFMovieClip_nextFrame(mc5);
  SWFMovieClip_nextFrame(mc5); 
  
  SWFMovie_add(mo, mc5);
  SWFMovie_nextFrame(mo);  // frame 11
  
  SWFMovie_nextFrame(mo);  // frame 12
  
  SWFMovie_nextFrame(mo);  // frame 13
  
  mc5 = newSWFMovieClip();
  add_clip_actions(mc5, "_root.check_equals(this._target, '/instance9');");
  SWFMovieClip_nextFrame(mc5);
  
  SWFMovie_add(mo, mc5);
  SWFMovie_nextFrame(mo);  // frame 14
  
  add_actions(mo, "totals(16); stop();");
  SWFMovie_nextFrame(mo);  // frame 15
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
