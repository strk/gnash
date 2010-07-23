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

#include "ming_utils.h"

#define OUTPUT_VERSION 8
#define OUTPUT_FILENAME "BitmapDataDraw.swf"

const char* mediadir=".";


int
main(int argc, char** argv)
{
    SWFMovie mo;
    SWFMovieClip mc;
    SWFMovieClip dejagnuclip;
    SWFShape sh;
    SWFDisplayItem it;
    SWFFillStyle fill;

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
    SWFShape_drawLine(sh, 100.0, 0.0);
    SWFShape_drawLine(sh, 0.0, 100.0);
    SWFShape_drawLine(sh, -100.0, 0.0);
    SWFShape_drawLine(sh, 0.0, -100.0);
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

    add_actions(mo, "stop();");

    // Output movie
    puts("Saving " OUTPUT_FILENAME);
    SWFMovie_save(mo, OUTPUT_FILENAME);

    return EXIT_SUCCESS;
}
