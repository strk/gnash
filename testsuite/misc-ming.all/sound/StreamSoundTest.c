/* 
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
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
 *
 */ 

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>
#include <errno.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 8
#define OUTPUT_FILENAME "StreamSoundTest.swf"

void addSoundExport(SWFMovie mo);

void
addSoundExport(SWFMovie mo)
{
    SWFSoundStream sounda;
    SWFInput inp;

    FILE* f;

    f = fopen(MEDIADIR"/click.mp3", "r");

    if (!f) {
        perror(MEDIADIR"/click.mp3");
        exit(EXIT_FAILURE);
    }
    
    inp = newSWFInput_file(f);

    sounda = newSWFSoundStream_fromInput(inp);

    SWFMovie_setSoundStream(mo, sounda);
}

int
main(int argc, char** argv)
{
	SWFMovie mo;
	const char *srcdir=".";
	SWFMovieClip  dejagnuclip;
    SWFDisplayItem it;


	/*********************************************
	 *
	 * Initialization
	 *
	 *********************************************/

	if ( argc>1 ) srcdir=argv[1];
	else
	{
		fprintf(stderr, "Usage: %s\n", argv[0]);
		return 1;
	}

	puts("Setting things up");

	Ming_init();
    Ming_useSWFVersion(OUTPUT_VERSION);
	Ming_setScale(20.0); /* let's talk pixels */
 
	mo = newSWFMovie();
	SWFMovie_setRate(mo, 0.5);
	SWFMovie_setDimension(mo, 640, 400);

	/*********************************************
	 *
	 * Body
	 *
	 *********************************************/

	dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10,
            0, 80, 800, 600);
	it = SWFMovie_add(mo, (SWFBlock)dejagnuclip);

    SWFMovie_add(mo, newSWFAction(
            "_root.onEnterFrame = function() { trace('Frame'); };"
            ));

	SWFMovie_nextFrame(mo);  /* end of frame1 */
	addSoundExport(mo);
	SWFMovie_nextFrame(mo);  /* end of frame2 */
	SWFMovie_nextFrame(mo);  /* end of frame3 */
	SWFMovie_nextFrame(mo);  /* end of frame4 */
	SWFMovie_nextFrame(mo);  /* end of frame5 */

	/*****************************************************
	 *
	 * Output movie
	 *
	 *****************************************************/

	puts("Saving " OUTPUT_FILENAME );

	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
