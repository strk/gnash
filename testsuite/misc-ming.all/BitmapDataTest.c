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

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 8
#define OUTPUT_FILENAME "BitmapDataTest.swf"

void addRedSquareExport(SWFMovie mo);

const char* mediadir=".";

void
addRedSquareExport(SWFMovie mo)
{
	SWFShape sh;
	SWFMovieClip mc;

	sh = make_fill_square (0, 0, 60, 60, 255, 0, 0, 255, 0, 0);
	mc = newSWFMovieClip();

	SWFMovieClip_add(mc, (SWFBlock)sh);
	/* This is here just to turn the clip into an active one */
	add_clip_actions(mc, "onRollOver = function() {};");
	SWFMovieClip_nextFrame(mc);

	SWFMovie_addExport(mo, (SWFBlock)mc, "redsquare");

	SWFMovie_writeExports(mo);
}


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip dejagnuclip, staticSquare;
  SWFShape shape;
  SWFDisplayItem it;

  if ( argc>1 ) mediadir=argv[1];
  else
  {
    fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
    return 1;
  }
	
  Ming_init();
  Ming_useSWFVersion (OUTPUT_VERSION);
	
  mo = newSWFMovie();
  SWFMovie_setDimension(mo, 800, 600);

  if (mo == NULL) return -1;

  addRedSquareExport(mo);

  add_actions(mo, "_root.onKeyDown = _root.onMouseUp = function() {"
	                "play(); }; "
                    "Key.addListener(_root);");

  SWFMovie_setRate(mo, 12);
    dejagnuclip = get_dejagnu_clip(
            (SWFBlock)get_default_font(mediadir), 10, 10, 150, 800, 600);
    SWFMovie_add(mo, (SWFBlock)dejagnuclip);
 
  SWFMovie_nextFrame(mo);

  add_actions(mo, "BitmapData = flash.display.BitmapData;"
		  "Rectangle = flash.geom.Rectangle;"
		  "bmp = new BitmapData(150, 150, false);"
		  "rect = new Rectangle(10, 10, 100, 100);"
		  "bmp.fillRect(rect, 0x00ff00);"
		  "mc = _root.createEmptyMovieClip('mc', getNextHighestDepth());"
          "d = mc.getNextHighestDepth();"
          "mc.attachBitmap(bmp, d);"
          "bmp2 = new BitmapData(20, 20, true);"
          "rect2 = new Rectangle (10, 10, 20, 20);"
          "bmp2.fillRect(rect2, 0xffffff00);"
          "d2 = mc.getNextHighestDepth();"
          "mc.attachBitmap(bmp2, d2);"
          "note('1. You should see a small yellow square in the top left "
                "corner of a larger green square. Click to proceed.');"
          "stop();"
          );

    SWFMovie_nextFrame(mo);

    add_actions(mo, "rect = new Rectangle (90, 90, 120, 120);"
		    "bmp.fillRect(rect, 0x0000FF);"
            "note('2. You should see a new blue square covering the "
                "bottom right corner of the large green square. Click"
                " to proceed.');"
            "stop();"
            );

    SWFMovie_nextFrame(mo);

    add_actions(mo, "mc.createEmptyMovieClip('d', d);"
            "note('3. You should see just the small yellow square in the top "
                "left corner. Click to proceed.');");

    add_actions(mo, "stop();");

    SWFMovie_nextFrame(mo);
    
    // Place a dynamic DisplayObject at depth 4
    add_actions(mo, "mc.removeMovieClip();"
            "_root.attachMovie('redsquare', 'rs', 4);");

    // Place a static DisplayObject at depth 3
    staticSquare = newSWFMovieClip();
    shape = make_fill_square (300, 0, 60, 60, 255, 0, 255, 255, 0, 255);
    SWFMovieClip_add(staticSquare, (SWFBlock)shape);  
    SWFMovieClip_nextFrame(staticSquare);
    
    it = SWFMovie_add(mo, (SWFBlock)staticSquare);  
    SWFDisplayItem_setDepth(it, 3); 
    SWFDisplayItem_setName(it, "staticSquare");

    add_actions(mo, 
            "note('4. You should see a red square in the top left and a "
            "purple square in the top right. Click to proceed.');");
    add_actions(mo, "stop();");

    SWFMovie_nextFrame(mo);

    add_actions(mo, "staticSquare.swapDepths(20);"
            "note('5. There should have been no change. Click to proceed.');"
            "stop();"
            );

    SWFMovie_nextFrame(mo);

    add_actions(mo, "_root.attachBitmap(bmp, 2);"
            "note('6. You should see the green and blue squares "
            "under the red square. The purple square should still be there. "
            "Click to proceed.');"
            "stop();"
            );

    SWFMovie_nextFrame(mo);

    add_actions(mo, "_root.attachBitmap(bmp, 3);"
            "note('7. There should have been no change. Click to proceed.');"
            "stop();"
            );

    SWFMovie_nextFrame(mo);

    SWFDisplayItem_remove(it);
    
    add_actions(mo, "_root.attachBitmap(bmp2, 20);"
            "note('8. The purple square should have gone. The small yellow "
            "square should have replaced the top left corner of the red "
            "square. The green and blue squares should still be there. "
            "Click to proceed.');"
            "stop();"
            );

    SWFMovie_nextFrame(mo);    

    add_actions(mo, "bmp.dispose(); bmp2.dispose();"
            "note('9. You should see just the red square. Click to proceed.');"
            "stop();"
            );

    SWFMovie_nextFrame(mo);    

    add_actions(mo, "bmp = new BitmapData(100, 100, false, 0x0000ff);"
            "note('10. There should have been no change. Click to proceed.');"
            "stop();"
            );

    SWFMovie_nextFrame(mo);    
    
    add_actions(mo, "bmp3 = new BitmapData(100, 100, false);"
            "rect3 = new Rectangle(20, 20, 90, 90);"
            "bmp3.fillRect(rect3, 0x0000ff);"
            "ch = _root.createEmptyMovieClip('original', 40);"
            "original.attachBitmap(bmp3, getNextHighestDepth());"
            "ch._name = 'duplicate';"
            "newch = _root.createEmptyMovieClip('original', "
                    "getNextHighestDepth());"
            "note('11. You should see a large blue square only. "
            "Click to proceed.');"
            "stop();"
            );

    SWFMovie_nextFrame(mo);    
    
    add_actions(mo, 
            "original.removeMovieClip();"
            "note('12. There should have been no change. Click to proceed.');"
            "stop();"
            );

    SWFMovie_nextFrame(mo);    
    
    add_actions(mo, 
            "duplicate.removeMovieClip();"
            "note('13. You should see the red square again. Click to "
            "proceed.');"
            "stop();"
            );

    SWFMovie_nextFrame(mo);

    add_actions(mo,
            "_root.createEmptyMovieClip('mcLeft', getNextHighestDepth());"
            "mcLeft.duplicateMovieClip('mcMiddle', getNextHighestDepth(),"
                        " { _x: 300, _y: 0 } );"
            "mcLeft.attachBitmap(bmp3, getNextHighestDepth());"
            "mcMiddle.attachBitmap(bmp3, getNextHighestDepth());"
            "mcMiddle.duplicateMovieClip('mcRight', getNextHighestDepth(),"
                        " { _x: 600, _y: 0 } );"
            "note('14. You should see two blue squares. Click to proceed.');"
            "stop();"
            );

    SWFMovie_nextFrame(mo);

    add_actions(mo,
            "bmp3.noise(293);"
            "note('15. You should see two noise patterns where the two "
            "squares were. Click to proceed.');"
            "stop();"
            );

    add_actions(mo, "_root.onKeyDown = _root.onMouseUp = undefined;"
            "_root.eof = true;" // hook for test runner...
            "note(' - END OF TEST - thanks for flying with us ! ');"
            //"totals(6);" // no AS based tests...
            );

    SWFMovie_nextFrame(mo);

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
