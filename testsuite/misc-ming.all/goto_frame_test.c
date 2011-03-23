/*
 *   Copyright (C) 2005, 2006, 2009, 2010, 2011 Free Software Foundation, Inc.
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
 * Timeline(starts from test2):
 * 
 *   Frame  | 7 | 8 | 9 | 10| 11| 12|
 *  --------+---+---+---+---+---+---+
 *   Event  | J | P | R | P | R | * |
 * 
 *  P = place (by PlaceObject2)
 *  R = remove (by RemoveObject2)
 *  J = jump
 *  * = jump target
 *
 * Description:
 *
 *  frame7:  gotoAndPlay(12);
 *  frame8:  place mc1 at depth 100
 *  frame9:  remove mc1
 *  frame10: place mc2 at depth 100
 *  frame11: remove mc2
 *  frame12: checks
 *
 * Observed behaviour:
 *
 *   (1) both mc1 and mc2 occupys depth -16485 after gotoFrame.
 *   (2) both mc1 and mc2 are visible in frame7 at runtime.
 *
 * Deduction:
 *
 *   (1) different DisplayObjects in the removed depths zone could share the same depth.
 *   (2) DisplayList::testInvariant() probably fails.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "goto_frame_test.swf"


SWFDisplayItem add_static_mc(SWFMovie mo, const char* name, int depth);

SWFDisplayItem
add_static_mc(SWFMovie mo, const char* name, int depth)
{
  SWFMovieClip mc;
  SWFDisplayItem it;

  mc = newSWFMovieClip();
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
  SWFMovieClip  mc_red, dejagnuclip;
  SWFShape  sh_red;
  SWFDisplayItem it_red, it;
  
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
  // Add a ShowFrame here, do all checks at later frames!
  // This will guarantee all the check-functions are defined before we call them.
  add_actions(mo, "asOrder = '0+';");
  SWFMovie_nextFrame(mo); //1st frame
  
  mc_red = newSWFMovieClip();
  sh_red = make_fill_square (0, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc_red, (SWFBlock)sh_red);  
  SWFMovieClip_nextFrame(mc_red);//1st frame
  SWFMovieClip_nextFrame(mc_red);//2st frame
  add_clip_actions(mc_red,  "_root.asOrder += '3+';"
                            "play();");
  SWFMovieClip_nextFrame(mc_red);//3nd frame
  
  add_clip_actions(mc_red, "_root.asOrder += '7+'; stop();");
  SWFMovieClip_nextFrame(mc_red);//4th frame
  
  it_red = SWFMovie_add(mo, (SWFBlock)mc_red);  
  SWFDisplayItem_setDepth(it_red, 3); 
  SWFDisplayItem_setName(it_red, "mc_red");
  
  add_actions(mo, "_root.asOrder += '1+';"
                  "check_equals(mc_red._currentframe, 1); "
                  "mc_red.gotoAndStop(3);"
                  "check_equals(mc_red._currentframe, 3); "
                  "_root.gotoAndStop(3);"
                  "_root.asOrder += '2+';" );               
  SWFMovie_nextFrame(mo); //2nd frame
  
  add_actions(mo, "_root.asOrder += '4+';"
                  "_root.gotoAndStop(4);"
                  "_root.asOrder += '5+';");
  SWFMovie_nextFrame(mo); //3nd frame
  
  add_actions(mo, "_root.asOrder += '6+';"
                  " _root.gotoAndPlay(5);");
  SWFMovie_nextFrame(mo); //4nd frame
  
  //checks
  check_equals(mo, "_root.asOrder", "'0+1+2+3+4+5+6+'");
  SWFMovie_nextFrame(mo); //5th frame

  check_equals(mo, "_root.asOrder", "'0+1+2+3+4+5+6+7+'");
  SWFMovie_nextFrame(mo); //6th frame

  //
  // test2: test forward gotoFrame
  //
  add_actions(mo, "gotoAndPlay(_currentframe + 5);");
  check_equals(mo, "mc1._target", "'/mc1'");
  check_equals(mo, "mc2._target", "'/mc2'");
  SWFMovie_nextFrame(mo); // 7th frame
  
  
  it = add_static_mc(mo, "mc1", 100);
  SWFDisplayItem_addAction(it, newSWFAction(
        "_root.note(this+' unloaded');"
        ), SWFACTION_UNLOAD);
  SWFMovie_nextFrame(mo); // 8th frame
  
  
  SWFDisplayItem_remove(it);
  SWFMovie_nextFrame(mo); // 9th frame
  
  
  it = add_static_mc(mo, "mc2", 100);
  SWFDisplayItem_addAction(it, newSWFAction(
        "_root.note(this+' unloaded');"
        ), SWFACTION_UNLOAD);
  SWFMovie_nextFrame(mo); // 10th frame
  
  
  SWFDisplayItem_remove(it);
  SWFMovie_nextFrame(mo); // 11th frame
  
  
  check_equals(mo, "mc1.getDepth()", "-16485");
  check_equals(mo, "mc1._name", "'mc1'");
  check_equals(mo, "mc2.getDepth()", "-16485");
  check_equals(mo, "mc2._name", "'mc2'");

  add_actions(mo, "totals(); stop();");
  SWFMovie_nextFrame(mo); // 12th frame
    
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
