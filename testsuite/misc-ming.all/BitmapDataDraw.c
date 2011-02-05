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
#include <string.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 8
#define OUTPUT_FILENAME "BitmapDataDraw.swf"

const char* mediadir=".";


/// The test shows the following:
//
/// The MovieClip itself is drawn with no transformation, i.e. identity matrix.
/// Any contained MovieClips keep their transformation.
/// BitmapData.draw draws on top of what's in the BitmapData already.
/// Dynamically loaded images are not drawn.
int
main(int argc, char** argv)
{
    SWFMovie mo;
    SWFMovieClip mc, mc3, mc4, mc5;
    SWFMovieClip dejagnuclip;
    SWFShape sh;
    SWFDisplayItem it;
    SWFFillStyle fill;
    SWFBitmap bp;
    SWFInput inp;
    FILE* imgfile;

    if (argc > 1) mediadir = argv[1];
    else {
        fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    Ming_init();
    Ming_useSWFVersion (OUTPUT_VERSION);
    
    mo = newSWFMovie();
    SWFMovie_setDimension(mo, 800, 600);

    SWFMovie_setRate(mo, 12);
    dejagnuclip = get_dejagnu_clip(
            (SWFBlock)get_default_font(mediadir), 10, 10, 150, 800, 600);
    SWFMovie_add(mo, (SWFBlock)dejagnuclip);
 
    SWFMovie_nextFrame(mo);
    
    mc = newSWFMovieClip();

    // Two shapes.

    // Shape 1
    sh = newSWFShape();
    fill = newSWFSolidFillStyle(0x00, 0xff, 0xff, 0xff);
    SWFShape_setRightFillStyle(sh, fill);
    SWFShape_movePenTo(sh, 10.0, 10.0);
    SWFShape_drawLine(sh, 90.0, 0.0);
    SWFShape_drawLine(sh, 0.0, 90.0);
    SWFShape_drawLine(sh, -90.0, 0.0);
    SWFShape_drawLine(sh, 0.0, -90.0);
    SWFMovieClip_add(mc, (SWFBlock)sh);

    // Shape 2
    sh = newSWFShape();
    fill = newSWFSolidFillStyle(0xff, 0x00, 0xff, 0xff);
    SWFShape_setRightFillStyle(sh, fill);
    SWFShape_movePenTo(sh, 80.0, 80.0);
    SWFShape_drawLine(sh, 50.0, 0.0);
    SWFShape_drawLine(sh, 0.0, 50.0);
    SWFShape_drawLine(sh, -50.0, 0.0);
    SWFShape_drawLine(sh, 0.0, -50.0);
    SWFMovieClip_add(mc, (SWFBlock)sh);

    // Show frame for static MovieClip
    SWFMovieClip_nextFrame(mc);

    it = SWFMovie_add(mo, (SWFBlock)mc);
    SWFDisplayItem_setName(it, "mc1");
    
    SWFMovie_nextFrame(mo);

    add_actions(mo,
            "_root.b = new flash.display.BitmapData(100, 100, false);"
            "b.draw(mc1);"
            "c = _root.createEmptyMovieClip('dynmc', 88);"
            "_root.dynmc._x = 200;"
            "_root.dynmc.attachBitmap(b, 24);"
            );

    // Dimensions are restricted to the BitmapData's size.
    check_equals(mo, "b.width", "100");
    check_equals(mo, "b.height", "100");
    
    // The original MovieClip has:
    // 1. a cyan (0x00ffff) square (10, 10) to (100, 100) and
    // 2. a magenta (0xff00ff) square (80, 80) to (130, 130).

    // Pixel checking
    // Top left corner is white
    check_equals(mo, "b.getPixel(1, 1)", "0xffffff");
    check_equals(mo, "b.getPixel(8, 8)", "0xffffff");
    // Cyan square
    check_equals(mo, "b.getPixel(12, 12)", "0x00ffff");
    check_equals(mo, "b.getPixel(12, 98)", "0x00ffff");
    check_equals(mo, "b.getPixel(98, 12)", "0x00ffff");
    // Magenta square
    check_equals(mo, "b.getPixel(82, 82)", "0xff00ff");

    // Do the same with double width and height.
    add_actions(mo,
            "b = new flash.display.BitmapData(100, 100, false);"
            "b.draw(mc1, new flash.geom.Matrix(2, 0, 0, 2, 34, 34));"
            "_root.createEmptyMovieClip('dynmc2', 89);"
            "_root.dynmc2._x = 200;"
            "_root.dynmc2.attachBitmap(b, 24);"
            );

    // Dimensions are restricted to the BitmapData's size.
    check_equals(mo, "b.width", "100");
    check_equals(mo, "b.height", "100");
    
    // Pixel checking
    // Top left corner is white
    check_equals(mo, "b.getPixel(1, 1)", "0xffffff");
    check_equals(mo, "b.getPixel(8, 8)", "0xffffff");
    // Cyan square is scaled and translated to start at (54, 54)
    // (10 * 2 + 34)
    check_equals(mo, "b.getPixel(12, 12)", "0xffffff");
    check_equals(mo, "b.getPixel(52, 52)", "0xffffff");
    check_equals(mo, "b.getPixel(56, 56)", "0x00ffff");
    // Magenta square isn't there because it doesn't fit.

    // Add with a different matrix
    it = SWFMovie_add(mo, (SWFBlock)mc);
    SWFDisplayItem_setMatrix(it, 0.5f, 0.f, 0.f, 2.0f, 0.f, 200.f);
    SWFDisplayItem_setName(it, "mc2");
    
    // Check that the BitmapData ignores PlaceObject matrix.
    add_actions(mo,
            "b = new flash.display.BitmapData(400, 400, false);"
            "b.draw(mc2);"
            "_root.createEmptyMovieClip('dynmc2', 88);"
            "_root.dynmc2._y = 200;"
            "_root.dynmc2._x = 200;"
            "_root.dynmc2.attachBitmap(b, 28);"
            );
    
    // This is a sanity check more than anything else.
    check_equals(mo, "b.width", "400");
    check_equals(mo, "b.height", "400");
    
    // Pixel checking (Bitmap is now 400x400)
    // Top left corner is white
    check_equals(mo, "b.getPixel(1, 1)", "0xffffff");
    check_equals(mo, "b.getPixel(8, 8)", "0xffffff");
    // Cyan square
    check_equals(mo, "b.getPixel(12, 12)", "0x00ffff");
    check_equals(mo, "b.getPixel(12, 98)", "0x00ffff");
    check_equals(mo, "b.getPixel(98, 12)", "0x00ffff");
    check_equals(mo, "b.getPixel(12, 102)", "0xffffff");
    check_equals(mo, "b.getPixel(102, 12)", "0xffffff");
    // Magenta square
    check_equals(mo, "b.getPixel(82, 82)", "0xff00ff");
    check_equals(mo, "b.getPixel(128, 128)", "0xff00ff");
    check_equals(mo, "b.getPixel(132, 132)", "0xffffff");
    check_equals(mo, "b.getPixel(78, 78)", "0x00ffff");

    SWFMovie_nextFrame(mo);
    
    // Create a nested MovieClip. The clip mc3 contains a
    // copy of mc with a funny translation.
    mc3 = newSWFMovieClip();
    it = SWFMovieClip_add(mc3, (SWFBlock)mc);
    SWFDisplayItem_setName(it, "mc3_mc1");
    SWFDisplayItem_setMatrix(it, 0.5f, 0.1f, -0.1f, 0.5f, 20.f, 20.f);
    SWFMovieClip_nextFrame(mc3);

    it = SWFMovie_add(mo, (SWFBlock)mc3);
    SWFDisplayItem_setName(it, "mc3");

    // This shows that the matrix of sub-clips is used. 
    add_actions(mo,
            "b = new flash.display.BitmapData(400, 400, false);"
            "b.draw(mc3);"

            // Re-use an earlier dynamic clip.
            "_root.dynmc2._y = 200;"
            "_root.dynmc2._x = 200;"
            "_root.dynmc2.attachBitmap(b, 28);"
            );
    
    // Top left corner is white
    check_equals(mo, "b.getPixel(1, 1)", "0xffffff");
    check_equals(mo, "b.getPixel(8, 8)", "0xffffff");
    // Cyan square top left corner
    check_equals(mo, "b.getPixel(25, 28)", "0x00ffff");
    check_equals(mo, "b.getPixel(20, 30)", "0xffffff");
    check_equals(mo, "b.getPixel(25, 25)", "0xffffff");
    // Cyan square bottom left (Check that it's rotated).
    check_equals(mo, "b.getPixel(18, 68)", "0x00ffff");
    // Cyan square top right 
    check_equals(mo, "b.getPixel(64, 36)", "0x00ffff");
    check_equals(mo, "b.getPixel(64, 32)", "0xffffff");
    // Magenta square top left
    check_equals(mo, "b.getPixel(54, 71)", "0xff00ff");
    check_equals(mo, "b.getPixel(54, 67)", "0x00ffff");
    check_equals(mo, "b.getPixel(50, 71)", "0x00ffff");
    // Magenta square bottom right
    check_equals(mo, "b.getPixel(74, 94)", "0xffffff");
    check_equals(mo, "b.getPixel(70, 94)", "0xff00ff");
    
    SWFMovie_nextFrame(mo);

    // Add a MovieClip with an image.

    char file[] = "/green.jpg";
    if (strlen(mediadir) > 1024) {
        fprintf(stderr, "Path to media dir too long! Fix the testcase");
    }
    char path[1024 + sizeof file];
    strcpy(path, mediadir);
    strcat(path, file);

    imgfile = fopen(path, "rb");
    if (!imgfile) {
        fprintf(stderr, "Failed to open bitmap file");
        exit(EXIT_FAILURE);
    }

    // Note that recent ming version have the more convenient
    // newSWFInput_filename() function, but we want to support
    // older versions.
    inp = newSWFInput_file(imgfile);
    bp = (SWFBitmap)newSWFJpegBitmap_fromInput(inp);
    
    // Image clip
    mc5 = newSWFMovieClip();
    SWFMovieClip_add(mc5, (SWFBlock)bp);
    SWFMovieClip_nextFrame(mc5);

    // Container clip for image clip.
    mc4 = newSWFMovieClip();
    it = SWFMovieClip_add(mc4, (SWFBlock)mc5);
    SWFDisplayItem_setMatrix(it, 0.75f, -0.2f, 0.3f, 0.35f, 20, 30);
    SWFMovieClip_nextFrame(mc4);

    it = SWFMovie_add(mo, (SWFBlock)mc4);
    SWFDisplayItem_setName(it, "mc4");

    // Draw on top of the old Bitmap!
    add_actions(mo, "b.draw(mc4);");
    
    check_equals(mo, "b.getPixel(1, 1)", "0xffffff");
    check_equals(mo, "b.getPixel(8, 8)", "0xffffff");
    // Cyan square top left corner
    // Note: The following pixels shouldn't suffer from antialiasing,
    // but not sure how accurate Gnash will be.
    xcheck_equals(mo, "b.getPixel(27, 30)", "0x000010");
    // Cyan square bottom left
    check_equals(mo, "b.getPixel(18, 68)", "0x00ffff");
    // Cyan square top right 
    xcheck_equals(mo, "b.getPixel(65, 36)", "0xfffffd");
    // Magenta square top left
    xcheck_equals(mo, "b.getPixel(62, 71)", "0x1000f");
    check_equals(mo, "b.getPixel(50, 71)", "0x00ffff");
    // Magenta square bottom right
    check_equals(mo, "b.getPixel(74, 94)", "0xffffff");
    check_equals(mo, "b.getPixel(70, 94)", "0xff00ff");
    
    // Test color transform
    add_actions(mo,
            "b = new flash.display.BitmapData(100, 100, false);"
            "cx = new flash.geom.ColorTransform(0.5, 0, 0, 1, -255, 255, 0, 0);"
            "trace(cx);"
            "b.draw(mc1, new flash.geom.Matrix(2, 0, 0, 2, 34, 34), cx);"
            "_root.createEmptyMovieClip('dynmc87', 333);"
            "_root.dynmc87._x = 400;"
            "_root.dynmc87._y = 400;"
            "_root.dynmc87.attachBitmap(b, 1200);"
            );

    // Pixel checking
    // Top left corner is white
    check_equals(mo, "b.getPixel(1, 1)", "0xffffff");
    check_equals(mo, "b.getPixel(8, 8)", "0xffffff");
    // Cyan square is now green.
    check_equals(mo, "b.getPixel(12, 12)", "0xffffff");
    check_equals(mo, "b.getPixel(52, 52)", "0xffffff");
    check_equals(mo, "b.getPixel(56, 56)", "0x00ff00");
    add_actions(mo, "stop();");

    // Output movie
    puts("Saving " OUTPUT_FILENAME);
    SWFMovie_save(mo, OUTPUT_FILENAME);

    fclose(imgfile);

    return EXIT_SUCCESS;
}
