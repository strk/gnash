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
 * Test case for the DefineBitsJPEG2 tag
 *
 ***********************************************************************/

#include <stdio.h>
#include <ming.h>

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "definebitsjpeg2.swf"

#define URL_LYNCH "lynch.jpg"

int
main(int argc, char **argv)
{
	SWFMovie mo;
	const char *jpeg_filename="lynch.jpg";
	FILE *jpeg_fd;
	SWFJpegBitmap jpeg_bm;
	SWFShape jpeg_sh;
	SWFMovieClip jpeg_mc;

	/*********************************************
	 *
	 * Initialization
	 *
	 *********************************************/

	if ( argc > 1 ) jpeg_filename=argv[1];
	else
	{
		fprintf(stderr, "Usage: %s <jpegfile>\n", argv[0]);
		return 1;
	}

	puts("Setting things up");

	Ming_init();
        Ming_useSWFVersion (OUTPUT_VERSION);
	Ming_setScale(1.0); /* so we talk twips */
 
	mo = newSWFMovie();

	/*****************************************************
	 *
	 * Add the LYNCH  clip
	 *
	 *****************************************************/

	puts("Adding lynch");

	jpeg_fd = fopen(jpeg_filename, "r");
	if ( ! jpeg_fd ) {
		perror(jpeg_filename);
		return 1;
	}
	jpeg_bm = newSWFJpegBitmap(jpeg_fd);
	jpeg_sh = newSWFShapeFromBitmap((SWFBitmap)jpeg_bm, SWFFILL_CLIPPED_BITMAP);
	jpeg_mc = newSWFMovieClip();
	SWFMovieClip_add(jpeg_mc, (SWFBlock)jpeg_sh);
	SWFMovieClip_nextFrame(jpeg_mc); /* showFrame */

	SWFMovie_add(mo, (SWFBlock)jpeg_mc);


	puts("Saving " OUTPUT_FILENAME );

	SWFMovie_nextFrame(mo); /* showFrame */

	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
