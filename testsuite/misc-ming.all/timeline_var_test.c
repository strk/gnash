/***********************************************************************
 *
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
 *
 * You should have received a copy of the GNU General Public License
 *
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 ***********************************************************************
 * zoulunkai,  zoulunkai@gmail.com
 * Test case for variable defined in main timeline;
 *
 * Movie has 4 frames defined as FRAME_COUNT;
 ***********************************************************************/


#include "ming_utils.h"

#include <stdio.h>
#include <ming.h>


#define OUTPUT_FILENAME "timeline_var_test.swf"
const int  FRAME_COUNT  = 4;

SWFAction  action_in_frame1(void);
SWFAction  action_in_frame1()
{
  SWFAction ac;
  ac = compileSWFActionCode(" \
      loop_back = 0; \
  ");
  return ac;
}


SWFAction  action_in_frame2(void);
SWFAction  action_in_frame2()
{
  SWFAction ac;
  ac = compileSWFActionCode(" \
     _root.check(loop_back >= 0); \
     _root.check_equals(loop_back, _root.loop_back); \
     if(loop_back == 0) \
     { \
        _root.check_equals(var_at_frame3, undefined); \
     } \
     else \
     { \
        _root.check_equals(var_at_frame3, \"var_defined_at_frame3\"); \
     } \
  ");
  return ac;
}

SWFAction  action_in_frame3(void);
SWFAction  action_in_frame3()
{
  SWFAction ac;
  ac = compileSWFActionCode(" \
      var var_at_frame3 = \"var_defined_at_frame3\"; \
  ");
  return ac;
}

SWFAction  action_in_frame4(void);
SWFAction  action_in_frame4()
{
  SWFAction ac;
  ac = compileSWFActionCode(" \
    if ( ++loop_back < 2 ) gotoAndPlay(2); \
  " );
  return ac;
}


int main(int argc, char** argv)
{
  SWFMovie  movie;
  SWFMovieClip dejagnuclip;
  SWFAction ac[FRAME_COUNT];

  SWFDisplayItem it;
  SWFMovieClip mc1;

  int i;
  const char *srcdir=".";

  if ( argc>1 ) srcdir=argv[1];
  else
  {
    fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
    return 1;
  }

  Ming_init();
  movie = newSWFMovie();
  SWFMovie_setDimension(movie, 800, 600);

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(movie, (SWFBlock)dejagnuclip);

  // Add frame ActionScipts to frames
  ac[0] = action_in_frame1();
  ac[1] = action_in_frame2();
  ac[2] = action_in_frame3();
  ac[3] = action_in_frame4();
  
  for(i=0; i<FRAME_COUNT; i++)
  {
    SWFMovie_add(movie, (SWFBlock)ac[i]);
    SWFMovie_nextFrame(movie); 
  }

  SWFMovie_add(movie, (SWFBlock)newSWFAction("_level0.ar = [];"));

  // This checks that a change of target in onEnterFrame code does
  // not change the target for other code.
  mc1 = newSWFMovieClip();
  SWFMovieClip_add(mc1, (SWFBlock)newSWFAction("this.g = 'moo'; _level0.ar.push(g);"));
  SWFMovieClip_nextFrame(mc1);
  SWFMovieClip_add(mc1, (SWFBlock)newSWFAction("_level0.ar.push(g);"));
  SWFMovieClip_nextFrame(mc1);

  it = SWFMovie_add(movie, (SWFBlock)mc1);
  SWFDisplayItem_addAction(it,
    compileSWFActionCode(" _root.note('onEnterFrame');"
                         " _level0.ar.push('setTarget');"
                         " asm { push '_level0' settargetexpr }; "),
                         SWFACTION_ENTERFRAME);  
  
  SWFMovie_nextFrame(movie); 

  check_equals(movie, "ar.toString()", "'moo,setTarget,moo'");

  SWFMovie_add(movie, (SWFBlock)newSWFAction("_root.totals(); stop();"));
  SWFMovie_nextFrame(movie); 

  // save files
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(movie, OUTPUT_FILENAME);

  return 0;
}
