/* 
 *   Copyright (C) 2007 Free Software Foundation, Inc.
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

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 8
#define OUTPUT_FILENAME "BitmapDataTest.swf"

const char* mediadir=".";

int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip mc;
  SWFMovieClip dejagnuclip;

  if ( argc>1 ) mediadir=argv[1];
  else
  {
    fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
    return 1;
  }
	
  Ming_init();
  Ming_useSWFVersion (OUTPUT_VERSION);

	
  mo = newSWFMovie();
  SWFMovie_setDimension(mo, 640, 480);

  if (mo == NULL) return -1;

  add_actions(mo, "_root.onKeyDown = function() {"
	                "play(); }; "
                    "Key.addListener(_root);");

  SWFMovie_setRate(mo, 2);
    dejagnuclip = get_dejagnu_clip(
            (SWFBlock)get_default_font(mediadir), 10, 10, 200, 800, 600);
    SWFMovie_add(mo, (SWFBlock)dejagnuclip);
 
  SWFMovie_nextFrame(mo);

  add_actions(mo, "BitmapData = flash.display.BitmapData;"
		  "Rectangle = flash.geom.Rectangle;"
		  "bmp = new BitmapData(150, 150, false);"
		  "rect = new Rectangle(10, 10, 100, 100);"
		  "bmp.fillRect(rect, 0x223322);"
		  "mc = _root.createEmptyMovieClip('mc', getNextHighestDepth());"
          "d = mc.getNextHighestDepth();"
          "mc.attachBitmap(bmp, d);"
          "bmp2 = new BitmapData(20, 20, true);"
          "rect2 = new Rectangle (10, 10, 20, 20);"
          "bmp2.fillRect(rect2, 0xffffff22);"
          "d2 = mc.getNextHighestDepth();"
          "mc.attachBitmap(bmp2, d2);"
          "note('1. You should see a small yellow square in the top left\n"
                "corner of a larger green square. Press a key.');"
          "stop();"
          );

    SWFMovie_nextFrame(mo);

    add_actions(mo, "rect = new Rectangle (90, 90, 120, 120);"
		    "bmp.fillRect(rect, 0xffee66);"
            "note('2. You should see a new pale yellow square covering the \n"
                "bottom right corner of the large green square. Press a key.');"
            "stop();"
            );

    SWFMovie_nextFrame(mo);

    add_actions(mo, "mc.createEmptyMovieClip('d', d);"
            "note('3. You should see just the small yellow square in the top\n"
                "left corner. Press a key.');");

    add_actions(mo, "stop();");

    SWFMovie_nextFrame(mo);
    
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
