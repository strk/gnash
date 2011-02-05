/* 
 *   Copyright (C) 2007, 2009, 2010, 2011 Free Software Foundation, Inc.
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
 *
 * observed behaviour:
 *   (1) different timelines still share the same vm stack
 *   (2) vm stack should be cleared before next advancement.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "runtime_vm_stack_test.swf"



int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc1, mc11, mc2, dejagnuclip;
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
  add_actions(mo, "testvar1 = 0; testvar2 = 0; testvar3 = 0; ");
  SWFMovie_nextFrame(mo); // frame1
  

  add_actions(mo, 
      "asm{"
      "   push '_root.testvar1'"
      "};");
  mc1 = newSWFMovieClip();
    add_clip_actions(mc1, 
        "asm{"
        "   push 1"
        "   setvariable"
        "   push '_root.testvar2'"
        "   push 2"
        "};");
    
    mc11 = newSWFMovieClip();
    add_clip_actions(mc11, 
        "asm{"
        "   setvariable"
        "   push '_root.testvar3'"
        "   push 3"
        "};");
    SWFMovieClip_nextFrame(mc11);
    SWFMovieClip_add(mc1, (SWFBlock)mc11);   
  SWFMovieClip_nextFrame(mc1);
  
  mc2 = newSWFMovieClip();
  add_clip_actions(mc2, 
    "asm{"
    "   setvariable"
    "};");
  SWFMovieClip_nextFrame(mc2);
  
  // place mc1 and mc2
  it1 = SWFMovie_add(mo, (SWFBlock)mc1); 
  SWFDisplayItem_setDepth(it1, 10);
  it2 = SWFMovie_add(mo, (SWFBlock)mc2); 
  SWFDisplayItem_setDepth(it2, 20);
  SWFMovie_nextFrame(mo); // frame2

  check_equals(mo, "testvar1", "1");
  check_equals(mo, "testvar2", "2");
  check_equals(mo, "testvar3", "3");
  add_actions(mo, 
    "asm{"
    "   push 'testvar1'"
    "   push 4"
    "   push 'testvar2'"
    "   push 5"
    "   push 'testvar3'"
    "   push 6"
    "};");
  SWFMovie_nextFrame(mo); // frame2
  
  add_actions(mo, 
    "asm{"
    "   setvariable"
    "   setvariable"
    "   setvariable"
    "};");
  check_equals(mo, "testvar1", "1");
  check_equals(mo, "testvar2", "2");
  check_equals(mo, "testvar3", "3");
  SWFMovie_nextFrame(mo); // frame3
  
  add_actions(mo, " _root.totals(6); stop(); ");
  SWFMovie_nextFrame(mo); // frame4 
  
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}

