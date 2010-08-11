/* 
 *   Copyright (C) 2007, 2009, 2010 Free Software Foundation, Inc.
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
#define OUTPUT_FILENAME "LoadBitmapTest.swf"

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
    
    // Add a MovieClip with an image.

    char file[] = "/green.jpg";
    if (strlen(mediadir) > 1024) {
        fprintf(stderr, "Path to media dir too long! Fix the testcase");
    }
    char path[1024 + sizeof file];
    strcpy(path, mediadir);
    strcat(path, file);

    inp = newSWFInput_filename(path);
    bp = (SWFBitmap)newSWFJpegBitmap_fromInput(inp);

    SWFMovie_addExport(mo, (SWFBlock)bp, "img1");

    add_actions(mo, 
            "b = flash.display.BitmapData.loadBitmap('img1');"
            "_root.attachBitmap(b, 67);"
            "trace(b);"
            "stop();"
            );
    
#if 0

    // Pixel checking
    // Top left corner is white
    check_equals(mo, "b.getPixel(1, 1)", "0xffffff");
    check_equals(mo, "b.getPixel(8, 8)", "0xffffff");
    // Cyan square is now green.
    check_equals(mo, "b.getPixel(12, 12)", "0xffffff");
    check_equals(mo, "b.getPixel(52, 52)", "0xffffff");
    check_equals(mo, "b.getPixel(56, 56)", "0x00ff00");
    add_actions(mo, "stop();");
#endif

    // Output movie
    puts("Saving " OUTPUT_FILENAME);
    SWFMovie_save(mo, OUTPUT_FILENAME);

    return EXIT_SUCCESS;
}
