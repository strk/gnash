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
 * Test case for hierachical sprites:
 *
 *  Main movie has 1 frames
 *  mc1 has 3 frame
 *
 *  ActionScript code will print current frame and loaded frames
 *
 ***********************************************************************/


#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_FILENAME "spritehier.swf"

static SWFAction frame_print(void);

SWFAction
frame_print(void)
{
	SWFAction ac;
	ac = newSWFAction(
        "trace(\"Current frame: \"+this._currentframe); "
        "trace(\"Frames loaded: \"+this._framesloaded); "
    );

	return ac;
}

int
main(void)
{
	SWFMovie mo;
	SWFShape sh;
	SWFAction ac;
	SWFMovieClip mc1, mc2, mc3;

	Ming_init();
	mo = newSWFMovie();
	mc1 = newSWFMovieClip();
	mc2 = newSWFMovieClip();
	mc3 = newSWFMovieClip();
	sh = newSWFShape();


	// Add frame code to frames
	ac = frame_print();
	SWFMovieClip_add(mc1, (SWFBlock)ac);
	SWFMovieClip_nextFrame(mc1);

	ac = frame_print();
	SWFMovieClip_add(mc1, (SWFBlock)ac);
	SWFMovieClip_nextFrame(mc1);

	ac = frame_print();
	SWFMovieClip_add(mc1, (SWFBlock)ac);
	SWFMovieClip_nextFrame(mc1);

	/* Add mc1 to movie */
	SWFMovie_add(mo, (SWFBlock)mc1);
	SWFMovie_nextFrame(mo); /* showFrame */

	puts("Saving " OUTPUT_FILENAME );
	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
