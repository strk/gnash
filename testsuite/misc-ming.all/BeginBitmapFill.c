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
#define OUTPUT_FILENAME "BeginBitmapFill.swf"


/// The beginBitmapFill test shows that:
//
/// 1. The fill always starts at 0, 0 of the MovieClip unless a matrix argument
///    is passed.
/// 2. It repeats by default.
/// 3. If repeat is false, the edge colours are used for the fill.
/// 4. Changes to the BitmapData affect all fills.
const char* mediadir=".";

int
main(int argc, char** argv)
{
    SWFMovie mo;
    SWFMovieClip dejagnuclip;
  
    if (argc > 1) mediadir=argv[1];
    else {
        fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
        return EXIT_FAILURE;
    }
  	
    Ming_init();
    Ming_useSWFVersion(OUTPUT_VERSION);
  	
    mo = newSWFMovie();
    SWFMovie_setDimension(mo, 800, 600);
  
    SWFMovie_setRate(mo, 12);
    dejagnuclip = get_dejagnu_clip(
              (SWFBlock)get_default_font(mediadir), 10, 10, 150, 800, 600);
      SWFMovie_add(mo, (SWFBlock)dejagnuclip);
   
    SWFMovie_nextFrame(mo);

    add_actions(mo,
            "b = new flash.display.BitmapData(150, 150, false);"
            "b.fillRect(new flash.geom.Rectangle(10, 10, 10, 130), 0xff0000);"
            "b.fillRect(new flash.geom.Rectangle(25, 10, 10, 130), 0x00ff00);"
            "b.fillRect(new flash.geom.Rectangle(40, 10, 10, 130), 0x0000ff);"
            "mc = _root.createEmptyMovieClip('mc1', 55);"
            "with(mc) {"

            // Shape 1
            "   x = 0;"
            "   y = 0;"
            "   moveTo(x + 0, y + 0);"
            "   beginBitmapFill(b);"
            "   lineTo(x + 0, y + 100);"
            "   lineTo(x + 100, y + 100);"
            "   lineTo(x + 100, y + 0);"

            // Shape 2
            "   x = 100;"
            "   y = 0;"
            "   moveTo(x + 0, y + 0);"
            "   beginBitmapFill(b);"
            "   lineTo(x + 100, y + 0);"
            "   lineTo(x + 100, y + 100);"
            "   lineTo(x + 0, y + 100);"

            // Shape 3
            "   x = 300;"
            "   y = 0;"
            "   moveTo(x + 0, y + 0);"
            "   beginBitmapFill(b);"
            "   lineTo(x + 100, y + 100);"
            "   lineTo(x + 0, y + 100);"
            "};"
            
            // Shape 4
            "b = new flash.display.BitmapData(150, 150, false);"
            "b.fillRect(new flash.geom.Rectangle(10, 10, 10, 130), 0xff00ff);"
            "b.fillRect(new flash.geom.Rectangle(25, 10, 10, 130), 0xffff00);"
            "b.fillRect(new flash.geom.Rectangle(40, 10, 10, 130), 0x00ffff);"
            "mc = _root.createEmptyMovieClip('mc2', 66);"
            "with(mc) {"
            "   x = 0;"
            "   y = 0;"
            "   moveTo(x + 0, y + 0);"
            "   beginBitmapFill(b);"
            "   lineTo(x + 0, y + 100);"
            "   lineTo(x + 100, y + 100);"
            "   lineTo(x + 100, y + 0);"
            "   transform.matrix = new flash.geom.Matrix(2, -1.3, 2.4, 1, 20, 200);"
            "};"
        );
    
    /// Now with matrix argument. Repeat is true by default
    add_actions(mo,
            // Shape 5
            "b = new flash.display.BitmapData(150, 150, false);"
            "b.fillRect(new flash.geom.Rectangle(10, 10, 10, 130), 0x000000);"
            "b.fillRect(new flash.geom.Rectangle(25, 10, 10, 130), 0xaaff00);"
            "b.fillRect(new flash.geom.Rectangle(40, 10, 10, 130), 0x00ccff);"
            "mc = _root.createEmptyMovieClip('mc3', 77);"
            "with(mc) {"
            "   x = 0;"
            "   y = 300;"
            "   moveTo(x + 0, y + 0);"
            "   matrix = new flash.geom.Matrix();"
            "   matrix.rotate(Math.PI / 2);"
            "   beginBitmapFill(b, matrix, true);"
            "   lineTo(x + 0, y + 100);"
            "   lineTo(x + 200, y + 100);"
            "   lineTo(x + 200, y + 0);"
            "};"
        );

    // Now with repeat set to false
    add_actions(mo,
            // Shape 6
            "b = new flash.display.BitmapData(20, 20, false);"
            "b.fillRect(new flash.geom.Rectangle(0, 0, 10, 10), 0x000000);"
            "b.fillRect(new flash.geom.Rectangle(10, 10, 10, 10), 0xaaff00);"
            "b.fillRect(new flash.geom.Rectangle(10, 0, 10, 10), 0xaaff00);"
            "b.fillRect(new flash.geom.Rectangle(0, 10, 10, 10), 0xaaffaa);"
            "mc = _root.createEmptyMovieClip('mc4', 88);"
            "with(mc) {"
            "   x = 300;"
            "   y = 300;"
            "   moveTo(x + 0, y + 0);"
            "   m = new flash.geom.Matrix();"
            "   m.tx = 350;"
            "   m.ty = 350;"
            "   beginBitmapFill(b, m, false);"
            "   lineTo(x + 0, y + 100);"
            "   lineTo(x + 150, y + 100);"
            "   lineTo(x + 150, y + 0);"
            "};"
            
            // Shape 7
            "mc = _root.createEmptyMovieClip('mc5', 99);"
            "with(mc) {"
            "   x = 500;"
            "   y = 300;"
            "   moveTo(x + 0, y + 0);"
            "   m = new flash.geom.Matrix();"
            "   m.tx = 550;"
            "   m.ty = 550;"
            "   beginBitmapFill(b, m, true);"
            "   lineTo(x + 0, y + 100);"
            "   lineTo(x + 150, y + 100);"
            "   lineTo(x + 150, y + 0);"
            "};"
        );

    // Change the Bitmap afterwards
    add_actions(mo,
            // Shape 8
            "b = new flash.display.BitmapData(20, 20, false);"
            "b.fillRect(new flash.geom.Rectangle(0, 0, 10, 10), 0x000000);"
            "b.fillRect(new flash.geom.Rectangle(10, 10, 10, 10), 0xaaff00);"
            "b.fillRect(new flash.geom.Rectangle(10, 0, 10, 10), 0xaaff00);"
            "b.fillRect(new flash.geom.Rectangle(0, 10, 10, 10), 0xaaffaa);"
            "mc = _root.createEmptyMovieClip('mc4', 111);"
            "with(mc) {"
            "   x = 10;"
            "   y = 450;"
            "   moveTo(x + 0, y + 0);"
            "   m = new flash.geom.Matrix();"
            "   m.tx = 0;"
            "   m.ty = 450;"
            "   beginBitmapFill(b, m, false);"
            "   lineTo(x + 0, y + 100);"
            "   lineTo(x + 150, y + 100);"
            "   lineTo(x + 150, y + 0);"
            "};"
            "b.fillRect(new flash.geom.Rectangle(0, 0, 20, 20), 0xff0000);"
            );
    
    // Dispose of the bitmap afterwards
    add_actions(mo,
            // Shape 9
            "b = new flash.display.BitmapData(20, 20, false);"
            "b.fillRect(new flash.geom.Rectangle(0, 0, 10, 10), 0x000000);"
            "b.fillRect(new flash.geom.Rectangle(10, 10, 10, 10), 0xaaff00);"
            "b.fillRect(new flash.geom.Rectangle(10, 0, 10, 10), 0xaaff00);"
            "b.fillRect(new flash.geom.Rectangle(0, 10, 10, 10), 0xaaffaa);"
            "mc = _root.createEmptyMovieClip('mc9', 222);"
            "with(mc) {"
            "   x = 450;"
            "   y = 450;"
            "   moveTo(x + 0, y + 0);"
            "   m = new flash.geom.Matrix();"
            "   m.tx = 0;"
            "   m.ty = 450;"
            "   beginBitmapFill(b, m, false);"
            "   lineTo(x + 0, y + 100);"
            "   lineTo(x + 150, y + 100);"
            "   lineTo(x + 150, y + 0);"
            "};"
            "b.dispose();"
            );
    check_equals(mo, "_root.mc9._width", "150");

    add_actions(mo, "stop();");
    SWFMovie_nextFrame(mo);

    //Output movie
    puts("Saving "OUTPUT_FILENAME);
    SWFMovie_save(mo, OUTPUT_FILENAME);
  
    return 0;
}
