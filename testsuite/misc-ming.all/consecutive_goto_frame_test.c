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
 * From this testcase, we can see that, the execution order of the 
 *   actions are like this:
 * actions in 1st frame of _root
 * actions in 1st frame of mc_red
 * actions in 1st target frame of _root
 * actions in 1st target frame of mc_red
 * actions in 2nd target frame of _root
 * actions in 2nd target frame of mc_red
 * actions in 3rd target frame of _root
 * actions in 3rd target frame of mc_red
 *
 * another testcase for ActionGotoFrame 
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "consecutive_goto_frame_test.swf"


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
  SWFMovie_setRate (mo, 1.0);

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFMovie_nextFrame(mo); /* 1st frame -  so we can use _root.note */
  
  mc_red = newSWFMovieClip();
  sh_red = make_fill_square (0, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc_red, (SWFBlock)sh_red);  
  add_clip_actions(mc_red, " _root.note('frm1 of mc_red - gotoAndStop(2)'); "
		  	"x = 'as_in_frm1_of_mc_red'; "
                        "gotoAndStop(2); ");
  SWFMovieClip_nextFrame(mc_red); /* 1st frame */
  add_clip_actions(mc_red, " _root.note('frm2 of mc_red - gotoAndStop(3)'); "
		  	"x = 'as_in_frm2_of_mc_red'; "
                        "gotoAndStop(3); ");
  SWFMovieClip_nextFrame(mc_red); /* 2nd frame */
  add_clip_actions(mc_red, " _root.note('frm3 of mc_red - gotoAndStop(4)'); "
		  	"x = 'as_in_frm3_of_mc_red'; "
                        "gotoAndStop(4); ");           
  SWFMovieClip_nextFrame(mc_red); /* 3rd frame */
  SWFMovieClip_nextFrame(mc_red); /* 4th frame */
  
  
  SWFDisplayItem it_red;
  it_red = SWFMovie_add(mo, (SWFBlock)mc_red); 
  SWFDisplayItem_setDepth(it_red, 3); 
  SWFDisplayItem_setName(it_red, "mc_red");
  
  add_actions(mo, " _root.note('frm2 of root - gotoAndStop(3)'); "
		  "mc_red.x = 'as_in_frm2_of_root'; "
                  "gotoAndStop(3); ");
  SWFMovie_nextFrame(mo); /* 2nd frame */
  
  /* mc_red.x has been set after playing the 1st frame, check it here */
  add_actions(mo, " check_equals(mc_red.x, 'as_in_frm1_of_mc_red'); "
		  " _root.note('frm3 of root - gotoAndStop(4)');"
                  "mc_red.x = 'as_in_frm3_of_root'; "
                  "gotoAndStop(4); ");               
  SWFMovie_nextFrame(mo); /* 3rd frame */
  
  /* mc_red.x has been set again after playing the 2nd frame, check it again */
  add_actions(mo, " check_equals(mc_red.x, 'as_in_frm2_of_mc_red'); "
		  " _root.note('frm4 of root - gotoAndStop(5)');"
                  " mc_red.x = \"as_in_frm4_of_root\"; "
                  " gotoAndStop(5); ");
  SWFMovie_nextFrame(mo); /* 4th frame */
  
  /* mc_red.x has been set again after playing the 3rd frame, check it again */
  check_equals(mo, "mc_red.x", "'as_in_frm3_of_mc_red'");
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); /* 5th frame */

  /* Output movie */
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
