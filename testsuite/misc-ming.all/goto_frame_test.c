/*
 *   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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
 * Test for ActionGotoFrame 
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "goto_frame_test.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc_red, dejagnuclip;
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
  // Add a ShowFrame here, do all checks at later frames!
  // This will guarantee all the check-functions are defined before we call them.
  SWFMovie_nextFrame(mo); //1st frame
  
  mc_red = newSWFMovieClip();
  sh_red = make_fill_square (0, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc_red, (SWFBlock)sh_red);  
  SWFMovieClip_nextFrame(mc_red);//1st frame
  SWFMovieClip_nextFrame(mc_red);//2st frame
  add_clip_actions(mc_red, "var flag = \"action_executed\"; \
                            var x = \"mc_red.frame3\";\
                            play();");
  SWFMovieClip_nextFrame(mc_red);//3nd frame
  
  add_clip_actions(mc_red, "x = \"mc_red.frame4\"; \
                            stop();");
  SWFMovieClip_nextFrame(mc_red);//4th frame
  
  SWFDisplayItem it_red;
  it_red = SWFMovie_add(mo, (SWFBlock)mc_red);  
  SWFDisplayItem_setDepth(it_red, 3); 
  SWFDisplayItem_setName(it_red, "mc_red");
  add_actions(mo, " check_equals(mc_red._currentframe, 1);  \
                    mc_red.gotoAndStop(3); \
                    check_equals(mc_red._currentframe, 3); \
                    _root.gotoAndStop(3); ");               
  SWFMovie_nextFrame(mo); //2nd frame
  
  add_actions(mo, " _root.gotoAndStop(4); \
                    mc_red.x = \"_root.frame3\"; ");
  SWFMovie_nextFrame(mo); //3nd frame
  
  add_actions(mo, " _root.gotoAndStop(5); \
                    mc_red.x = \"_root.frame4\"; ");
  SWFMovie_nextFrame(mo); //4nd frame
  
  //checks
  check_equals(mo, "_root.mc_red.flag", "'action_executed'");
  check_equals(mo, "_root.mc_red.x", "'_root.frame4'");
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); //5th frame

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
