/*
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */ 

/*
 * Zou Lunkai, zoulunkai@gmail.com
 *
 *  Test for nested masks
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "masks_test2.swf"

SWFMovieClip defineMovieclip(int x, int y, int w, int h);

SWFMovieClip defineMovieclip(int x, int y, int w, int h)
{
		SWFMovieClip mc;
		SWFShape  sh;
	  mc = newSWFMovieClip();
  	sh = make_fill_square (x, y, w, h, 255, 0, 0, 255, 0, 0);
 	 	SWFMovieClip_add(mc, (SWFBlock)sh);  
  	SWFMovieClip_nextFrame(mc);
  	return mc;
}

int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc1, mc2, mc3, mc4, mc5, mc6, mc7, dejagnuclip;
  SWFDisplayItem it;
	
  Ming_init();
  mo = newSWFMovieWithVersion(OUTPUT_VERSION);
  SWFMovie_setDimension(mo, 800, 600);
  SWFMovie_setRate (mo, 1.0);
  /*
   *  Don't add dejagnu clip to this file.
   *  Or place it in a safe depth.
   */
  
  mc1 = defineMovieclip(0, 0, 30, 30);
  mc2 = defineMovieclip(0, 0, 60, 60);
  mc3 = defineMovieclip(0, 0, 90, 90);
  mc4 = defineMovieclip(0, 0, 120, 120);
  mc5 = defineMovieclip(0, 0, 150, 150);
  mc6 = defineMovieclip(0, 0, 180, 180);
  mc7 = defineMovieclip(0, 0, 210, 210);
  
  it = SWFMovie_add(mo, (SWFBlock)mc1);  
  SWFDisplayItem_setDepth(it, 1); 
  SWFDisplayItem_setMaskLevel(it, 4);

  it = SWFMovie_add(mo, (SWFBlock)mc2);  
  SWFDisplayItem_setDepth(it, 2); 
  SWFDisplayItem_setMaskLevel(it, 5);
  
  it = SWFMovie_add(mo, (SWFBlock)mc3);  
  SWFDisplayItem_setDepth(it, 3); 
  SWFDisplayItem_setMaskLevel(it, 6);
  
  it = SWFMovie_add(mo, (SWFBlock)mc4);  
  SWFDisplayItem_setDepth(it, 4); 

  it = SWFMovie_add(mo, (SWFBlock)mc5);  
  SWFDisplayItem_setDepth(it, 5); 
  
  it = SWFMovie_add(mo, (SWFBlock)mc6);  
  SWFDisplayItem_setDepth(it, 6); 

	SWFMovie_nextFrame(mo); 

	it = SWFMovie_add(mo, (SWFBlock)mc7);  
  SWFDisplayItem_setDepth(it, 7); 
                    
  SWFMovie_nextFrame(mo); 

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}



