/***********************************************************************
 *
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


#include <stdio.h>
#include <ming.h>


#define OUTPUT_FILENAME "timeline_var_test.swf"
const int  FRAME_COUNT  = 4;

SWFAction  action_in_frame1()
{
  SWFAction ac;
  ac = compileSWFActionCode(" \
      loop_back = 0; \
  ");
  return ac;
}


SWFAction  action_in_frame2()
{
  SWFAction ac;
  ac = compileSWFActionCode(" \
     if(loop_back == 0) \
     { \
        if ( var_at_frame3 == undefined ) \
            trace(\"PASSED: var_at_frame3 == undefined \" ); \
        else \
            trace(\"FAILED: var_at_frame3 == undefined \" ); \
     } \
     else \
     { \
         if ( var_at_frame3 == \"var_defined_at_frame3\" ) \
             trace(\"XPASSED: var_at_frame3 == var_defined_at_frame3\" ); \
         else \
             trace(\"XFAILED: var_at_frame3 == var_defined_at_frame3\" ); \
     } \
  ");
  return ac;
}

SWFAction  action_in_frame3()
{
  SWFAction ac;
  ac = compileSWFActionCode(" \
      var var_at_frame3 = \"var_defined_at_frame3\"; \
  ");
  return ac;
}

SWFAction  action_in_frame4()
{
  SWFAction ac;
  ac = compileSWFActionCode(" \
    if ( ++loop_back < 2 ) gotoAndPlay(2); \
  " );
  return ac;
}


int main()
{
  SWFMovie  movie;
  SWFAction ac[FRAME_COUNT];
  int i;

  Ming_init();
  movie = newSWFMovie();

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
  
  // save files
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(movie, OUTPUT_FILENAME);

  return 0;
}
