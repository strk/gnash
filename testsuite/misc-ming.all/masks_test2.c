/*
 *   Copyright (C) 2007, 2009, 2010, 2011 Free Software Foundation, Inc.
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
 *  Zou Lunkai, zoulunkai@gmail.com
 *
 *  Test for nested layer (static) masks
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

  const char *srcdir=".";
  if ( argc>1 ) srcdir=argv[1];
  
  Ming_init();
  mo = newSWFMovieWithVersion(OUTPUT_VERSION);
  SWFMovie_setDimension(mo, 800, 600);
  SWFMovie_setRate (mo, 1.0);
  /*
   *  Don't add dejagnu clip to this file.
   *  Or place it in a safe depth.
   */
  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  it = SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFDisplayItem_setDepth(it, 1000);
  SWFDisplayItem_moveTo(it, 0, 220);

  SWFMovie_nextFrame(mo); 
  
  mc1 = defineMovieclip(0, 0, 30, 30);   // placed at depth 1, masking up to level 4
  mc2 = defineMovieclip(0, 0, 60, 60);   // placed at depth 2, masking up to level 5
  mc3 = defineMovieclip(0, 0, 90, 90);   // placed at depth 3, masking up to level 6
  mc4 = defineMovieclip(0, 0, 120, 120); // placed at depth 4
  mc5 = defineMovieclip(0, 0, 150, 150); // placed at depth 5
  mc6 = defineMovieclip(0, 0, 180, 180); // placed at depth 6
  mc7 = defineMovieclip(0, 0, 210, 210); // placed at depth 7
  
  it = SWFMovie_add(mo, (SWFBlock)mc1);  
  SWFDisplayItem_setDepth(it, 1); 
  SWFDisplayItem_setMaskLevel(it, 4);
  SWFDisplayItem_setName(it, "mc1"); 

  it = SWFMovie_add(mo, (SWFBlock)mc2);  
  SWFDisplayItem_setDepth(it, 2); 
  SWFDisplayItem_setMaskLevel(it, 5);
  SWFDisplayItem_setName(it, "mc2"); 
  
  it = SWFMovie_add(mo, (SWFBlock)mc3);  
  SWFDisplayItem_setDepth(it, 3); 
  SWFDisplayItem_setMaskLevel(it, 6);
  SWFDisplayItem_setName(it, "mc3"); 
  
  it = SWFMovie_add(mo, (SWFBlock)mc4);  
  SWFDisplayItem_setDepth(it, 4); 
  SWFDisplayItem_setName(it, "mc4"); 

  it = SWFMovie_add(mo, (SWFBlock)mc5);  
  SWFDisplayItem_setDepth(it, 5); 
  SWFDisplayItem_setName(it, "mc5"); 
  
  it = SWFMovie_add(mo, (SWFBlock)mc6);  
  SWFDisplayItem_setDepth(it, 6); 
  SWFDisplayItem_setName(it, "mc6"); 

  check(mo, "mc1.hitTest(28, 28, true)");
  check(mo, "mc2.hitTest(58, 58, true)");
  check(mo, "mc3.hitTest(88, 88, true)");
  check(mo, "mc4.hitTest(118, 118, true)");
  check(mo, "mc5.hitTest(148, 148, true)");
  check(mo, "mc6.hitTest(178, 178, true)");

  add_actions(mo,
	"mc4.onRollOver = function() { this._alpha = 50; };"
	"mc4.onRollOut = function() { this._alpha = 100; };"
  );

  SWFMovie_nextFrame(mo); 

  it = SWFMovie_add(mo, (SWFBlock)mc7);  
  SWFDisplayItem_setDepth(it, 7); 
  SWFDisplayItem_setName(it, "mc7"); 

  check(mo, "mc7.hitTest(208, 208, true)");

  add_actions(mo, "_root.totals(7); stop();");

  SWFMovie_nextFrame(mo); 

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}



