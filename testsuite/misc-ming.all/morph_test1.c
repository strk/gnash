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
 *  zou lunkai  zoulunkai@gmail.com
 *
 *  test morphs 
*/

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME  "morph_test1.swf"

void make_shape(SWFShape sh, int x, int y, int width, int height, byte r, byte g, byte b);

void
make_shape(SWFShape sh, int x, int y, int width, int height, byte r, byte g, byte b)
{
  SWFFillStyle fs = SWFShape_addSolidFillStyle(sh, r, g, b, 255);
  SWFShape_setLineStyle(sh, 1, r, g, b, 255);
  SWFShape_setLeftFillStyle(sh, fs);
  SWFShape_movePenTo(sh, x, y);
  SWFShape_drawLineTo(sh, x, y+height);
  SWFShape_drawLineTo(sh, x+width, y+height);
  SWFShape_drawLineTo(sh, x+width, y);
  SWFShape_drawLineTo(sh, x, y);
}

int
main(int argc, char** argv)
{
  
  SWFMovie mo;
  SWFMovieClip dejagnuclip;
  SWFMorph morph;
  SWFShape startShape, endShape;
  SWFDisplayItem it;
  float ratio;


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

  // _root.frame1
  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFMovie_nextFrame(mo); 

  // _root.frame2, define and place a morph
  morph =  newSWFMorphShape();
  startShape = SWFMorph_getShape1(morph);
  make_shape(startShape, 0, 0, 100, 100, 255, 0 ,0);
  endShape = SWFMorph_getShape2(morph);
  make_shape(endShape, 700, 500, 100, 100, 0, 255 ,0);

  it = SWFMovie_add(mo, (SWFBlock)morph); 
  SWFMovie_nextFrame(mo); 
  
  // update the morph with different ratios
  for(ratio=0.2; ratio<1.01; ratio+=0.2)
  {
     SWFDisplayItem_remove(it);
     it = SWFMovie_add(mo, (SWFBlock)morph);
     SWFDisplayItem_setRatio(it, ratio);
     SWFMovie_nextFrame(mo);     
  }  
  
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}

