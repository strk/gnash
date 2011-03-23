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
 * movieclips hierarchy:
 *  _root
 *     |---mc1
 *          |---mc11
 *
 * file description:
 *  Testing variables are initialized to zero, and get set to 1 when related function
 *  call succeeds.
 *
 * Obeserved behaviour:
 *  ActionCallFunction supports path prefix before a function name.
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME  "callFunction_test.swf"


int
main(int argc, char** argv)
{
  
  SWFMovie mo;
  SWFMovieClip dejagnuclip;
  SWFMovieClip mc1, mc11;
  SWFDisplayItem it1, it11;


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

  // _root.frame1
  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  add_actions(mo, " x1=0; x2=0; x3=0; x4=0; x5=0; x6=0; x7=0; x8=0; x9=0; x10=0; ");
  SWFMovie_nextFrame(mo); 

  //
  // _root.frame2: define movieclips and some functions.
  //
  // Define mc11
  mc11 = newSWFMovieClip();
  add_clip_actions(mc11, " func1 =  function () { _root.x1=1;  _root.x2=this; }; "
                          "stop();");
  SWFMovieClip_nextFrame(mc11); 
  add_clip_actions(mc11, "_root.x3 = 1; stop();");
  SWFMovieClip_nextFrame(mc11); 
  add_clip_actions(mc11, "_root.x4 = 1; stop();");
  SWFMovieClip_nextFrame(mc11); 
  
  // Define mc1, add mc11 to mc1   
  mc1 = newSWFMovieClip();
  it11 = SWFMovieClip_add(mc1, (SWFBlock)mc11);  
  SWFDisplayItem_setDepth(it11, 3); 
  SWFDisplayItem_setName(it11, "mc11"); 
  add_clip_actions(mc1, " func2 =  function () { _root.x5=1;  _root.x6=this; }; "
                        "stop();");
  SWFMovieClip_nextFrame(mc1); 
  add_clip_actions(mc1, "_root.x7 = 1; stop();");
  SWFMovieClip_nextFrame(mc1); 
  add_clip_actions(mc1, "_root.x8 = 1; stop();");
  SWFMovieClip_nextFrame(mc1);
  
  // place mc1 
  it1 = SWFMovie_add(mo, (SWFBlock)mc1); 
  SWFDisplayItem_setDepth(it1, 4); 
  SWFDisplayItem_setName(it1, "mc1"); 
  SWFMovie_nextFrame(mo); 
  
  //
  // _root.frame3: invoke function calls
  //
  add_actions(mo,  "asm{"
                   "push 0 "
                   "push  '_root.mc1.mc11.func1' " // valid format
                   "callfunction " 
                   "};");
  add_actions(mo,  "asm{"
                   "push 2 "
                   "push 1 "
                   "push  '_root.mc1.mc11.gotoAndStop' " // valid format
                   "callfunction " 
                   "};");
  add_actions(mo,  "asm{"
                   "push 0 "
                   "push  '_root.mc1:func2' " // valid format
                   "callfunction " 
                   "};");
  add_actions(mo,  "asm{"
                   "push 2 "
                   "push 1 "
                   "push  '/mc1/:gotoAndStop' " // valid format
                   "callfunction " 
                   "};");
  add_actions(mo,  "asm{"
                   "push 3 "
                   "push 1 "
                   "push  '/_root/mc1/mc11/gotoAndStop' " // *invalid* format
                   "callfunction " 
                   "};");
 add_actions(mo,  "asm{"
                   "push 3 "
                   "push 1 "
                   "push  '_root:mc1:gotoAndStop' " // valid format
                   "callfunction " 
                   "};");
  SWFMovie_nextFrame(mo); 
  
  //
  // _root.frame4: check if the function call succeeded
  //
  check_equals(mo, "_root.x1", "1");
  check_equals(mo, "_root.x2", "_root.mc1.mc11");
  check_equals(mo, "_root.x3", "1");
  check_equals(mo, "_root.x4", "0");
  check_equals(mo, "_root.x5", "1");
  check_equals(mo, "_root.x6", "_root.mc1");
  check_equals(mo, "_root.x7", "1");
  check_equals(mo, "_root.x8", "1");
  
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); 
  
  
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}

