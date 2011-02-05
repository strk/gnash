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

/*
 * Zou Lunkai, zoulunkai@gmail.com
 *
 * another testcase for ActionGotoFrame 
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "multi_doactions_and_goto_frame_test.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  dejagnuclip;

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
  SWFMovie_nextFrame(mo); //1st frame
  
  /* Current Gnash will not execute actions in the 3rd doAction tag.
   * Because the m_action_list has been cleared in current goto_frame
   *   function. That why it fails to check x3.
   */
  add_actions(mo, " x1 = \"as_in_doAction_tag1\"; ");  // doAction Tag1    
  add_actions(mo, " gotoAndStop(3); \
                    x2 = \"as_in_doAction_tag2\"; ");  // doAction Tag2
  add_actions(mo, " x3 = \"as_in_doAction_tag3\"; ");  // doAction Tag3          
  SWFMovie_nextFrame(mo); //2nd frame
  

  
  check_equals(mo, "x1", "'as_in_doAction_tag1'");
  check_equals(mo, "x2", "'as_in_doAction_tag2'");
  check_equals(mo, "x3", "'as_in_doAction_tag3'");
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); //3rd frame

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}

