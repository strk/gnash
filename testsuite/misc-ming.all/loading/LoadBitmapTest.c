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
    SWFMovieClip dejagnuclip;
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
        return EXIT_FAILURE;
    }

    // Note that recent ming version have the more convenient
    // newSWFInput_filename() function, but we want to support
    // older versions.
    inp = newSWFInput_file(imgfile);
	bp = newSWFBitmap_fromInput(inp);
	if (!bp) {
		return EXIT_FAILURE;
	}
    SWFMovie_addExport(mo, (SWFBlock)bp, "img1");

    SWFMovie_writeExports(mo);
    
    // For some reason some values are very slightly different, even though
    // the bytes should be taken directly from the embedded DefineBits tag.
    // We'd better not worry about it too much; they're very close.
    add_actions(mo, 
            // 24-bit RGB checker
            "near = function(bitmap, x, y, val) {"
            "   tol = 2;"
            "   col = bitmap.getPixel(x, y);"
            "   col_r = (col & 0xff0000) >> 16;"
            "   col_g = (col & 0xff00) >> 8;"
            "   col_b = (col & 0xff);"
            "   val_r = (val & 0xff0000) >> 16;"
            "   val_g = (val & 0xff00) >> 8;"
            "   val_b = (val & 0xff);"
            "   if (Math.abs(col_r - val_r) > tol) return false;"
            "   if (Math.abs(col_b - val_b) > tol) return false;"
            "   if (Math.abs(col_g - val_g) > tol) return false;"
            "   return true;"
            "};"
            );

    add_actions(mo, 
            "f = flash.display.BitmapData.loadBitmap('img1');");
    check_equals(mo, "typeof(f)", "'object'");
    check_equals(mo, "f.__proto__", "flash.display.BitmapData.prototype");
    check(mo, "f.__proto__ === flash.display.BitmapData.prototype");
    check_equals(mo, "f.transparent", "false");

    // Pixel checking
    check(mo, "near(f, 85, 10, 0x00ff00)");
    check(mo, "near(f, 85, 30, 0x008800)");
    check(mo, "near(f, 85, 50, 0x004400)");
    check(mo, "near(f, 85, 70, 0x002200)");
    check(mo, "near(f, 85, 85, 0x000000)");

    // Now do weird things with the class to see what's called where.
    add_actions(mo, 

            // A static property of an object
            "o = {};"
            "ASSetPropFlags(o, null, 0, 1);"
            "o.func = flash.display.BitmapData.loadBitmap;"

            // Plain function
            "func = flash.display.BitmapData.loadBitmap;"

            // Overwrite flash.display.BitmapData
            "backup = flash.display.BitmapData;"
            "_global.flash.display.BitmapData = 67;"

            // Works
            "c = o.func('img1');"

            // Doesn't work. Not sure why; maybe because 'this' is a level.
            "d = func('img1');"

            "o.prototype = backup.prototype;"
            "e = o.func('img1');"

            "_root.attachBitmap(c, 67);"

            );

    check_equals(mo, "typeof(c)", "'object'");
    check_equals(mo, "c.__proto__", "undefined");
    xcheck_equals(mo, "typeof(d)", "'undefined'");
    
    // The __proto__ member is the prototype of the parent object.
    check_equals(mo, "typeof(e)", "'object'");
    check_equals(mo, "typeof(e.__proto__)", "'object'");
    check_equals(mo, "e.__proto__", "backup.prototype");
    check_equals(mo, "typeof(e.constructor)", "'function'");
    check(mo, "e.constructor != backup.constructor");
    
    add_actions(mo, "_global.flash.display.BitmapData = backup;");

    add_actions(mo, "stop();");
    
    // Output movie
    puts("Saving " OUTPUT_FILENAME);
    SWFMovie_save(mo, OUTPUT_FILENAME);

    fclose(imgfile);

    return EXIT_SUCCESS;
}
