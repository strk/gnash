/***********************************************************************
 *
 *   Copyright (C) 2005, 2006, 2009, 2010, 2011 Free Software Foundation, Inc.
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
 *
 * You should have received a copy of the GNU General Public License
 *
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 ***********************************************************************
 *
 * Test case for the Bitmap smoothing
 * Requires: MING-0.4.3 (00040300)
 *
 ***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ming.h>

#include "ming_utils.h"

int
main(int argc, char **argv)
{
	SWFMovie mo;
	SWFInput in;
	SWFBitmap bitmap;
	SWFShape shpSmt;
	SWFShape shpHrd;
	SWFMovieClip mc;
	SWFDisplayItem it;
	int swfversion;
	SWFFont font;
	SWFMovieClip dejagnuclip;
	char outputFilename[256];
    FILE* imgfile;

	if ( argc < 2 ) {
		fprintf(stderr, "Usage: %s <swf_version>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	swfversion = atoi(argv[1]);
	sprintf(outputFilename, "BitmapSmoothingTest-v%d.swf", swfversion);

	/*********************************************
	 *
	 * Initialization
	 *
	 *********************************************/


	puts("Setting things up");

	Ming_init();
        Ming_useSWFVersion (swfversion);
 
	mo = newSWFMovieWithVersion(swfversion);

	/****************************************************
	* Create filled shapes mc
	****************************************************/
    imgfile = fopen(MEDIADIR"/vstroke.png", "rb");
    if (!imgfile) {
        fprintf(stderr, "Failed to open bitmap file");
        return EXIT_FAILURE;
    }

    // Note that recent ming version have the more convenient
    // newSWFInput_filename() function, but we want to support
    // older versions.
    in = newSWFInput_file(imgfile);
	bitmap = newSWFBitmap_fromInput(in);
	if (!bitmap) {
		return EXIT_FAILURE;
	}

	shpSmt = newSWFShapeFromBitmap(bitmap,
		SWFFILL_CLIPPED_BITMAP);

	shpHrd = newSWFShapeFromBitmap(bitmap,
		SWFFILL_NONSMOOTHED_CLIPPED_BITMAP);

	mc = newSWFMovieClip();
	SWFMovieClip_add(mc, (SWFBlock)shpSmt);
	it = SWFMovieClip_add(mc, (SWFBlock)shpHrd);
	SWFDisplayItem_moveTo(it, 0, 5);
	SWFMovieClip_nextFrame(mc); 

	/****************************************************
	* Create filled shapes mc, and scale it
	****************************************************/

	it = SWFMovie_add(mo, (SWFBlock)mc);
	SWFDisplayItem_scaleTo(it, 30, 10);

	SWFMovie_setDimension(mo, SWFBitmap_getWidth(bitmap)*30, 500);

	/****************************************************
	* Add dejagnu clip
	****************************************************/

	font = get_default_font(MEDIADIR);
	dejagnuclip = get_dejagnu_clip((SWFBlock)font, 10, 0, 0, 200, 200);
	it = SWFMovie_add(mo, (SWFBlock)dejagnuclip);
	SWFDisplayItem_setDepth(it, 200); 
	SWFDisplayItem_move(it, 0, 100); 

	/****************************************************
	* TODO: Add actions
	****************************************************/


	/****************************************************
	* Save things up
	****************************************************/

	printf("Saving %s\n", outputFilename);

	SWFMovie_nextFrame(mo); /* showFrame */

	SWFMovie_save(mo, outputFilename);

    fclose(imgfile);

	return EXIT_SUCCESS;
}

