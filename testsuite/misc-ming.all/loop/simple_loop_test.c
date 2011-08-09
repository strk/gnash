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
 * Simple test for loopback
 *
 * The root movie has 4 frames
 * First frame initializes the dejagnu stuff.
 * Each frame from 2nd to 4th place a DisplayObject on the stage.
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "simple_loop_test.swf"

static SWFShape
get_shape(int r, int g, int b)
{
	SWFShape  sh  = make_fill_square (0, 0, 60, 60, r, g, b, r, g, b);
	return sh;
}

int main(void)
{
  SWFMovie mo;
  SWFShape sh;
  SWFDisplayItem it;

  Ming_init();
  mo = newSWFMovieWithVersion(OUTPUT_VERSION);
  SWFMovie_setDimension(mo, 800, 600);
  SWFMovie_setRate (mo, 2.0);

  /***************************************************************
   * Frame 1 (empty)
   ***************************************************************/

  SWFMovie_nextFrame(mo);

  
  /***************************************************************
   * Frame 2
   ***************************************************************/

  sh = get_shape(255, 0, 0);
  it = SWFMovie_add(mo, (SWFBlock)sh);
  SWFDisplayItem_setDepth(it, 2); 
  SWFMovie_nextFrame(mo); 

  /***************************************************************
   * Frame 3
   ***************************************************************/

  sh = get_shape(0, 255, 0);
  it = SWFMovie_add(mo, (SWFBlock)sh);
  SWFDisplayItem_moveTo(it, 60, 0);
  SWFDisplayItem_setDepth(it, 3); 
  SWFMovie_nextFrame(mo); 

  /***************************************************************
   * Frame 4
   ***************************************************************/

  sh = get_shape(0, 0, 255);
  it = SWFMovie_add(mo, (SWFBlock)sh);
  SWFDisplayItem_moveTo(it, 120, 0);
  SWFDisplayItem_setDepth(it, 4);
  SWFMovie_nextFrame(mo); 


  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}



