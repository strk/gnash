/* 
 *   Copyright (C) 2007 Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

/*
 * Sandro Santilli, strk@keybit.net
 *
 * Test if dynamic and static objects at the same depth can coexist
 *
 * run as ./displaylist_depths_test
 */

#include "ming_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "displaylist_depths_test.swf"

static SWFMovieClip get_static_mc(void);

SWFMovieClip
get_static_mc()
{
	SWFShape sh;
	SWFMovieClip mc;

	sh = make_fill_square (0, 300, 60, 60, 255, 0, 0, 255, 0, 0);
	mc = newSWFMovieClip();
	SWFMovieClip_add(mc, (SWFBlock)sh);
	SWFMovieClip_nextFrame(mc);

	return mc;
}


int
main(int argc, char** argv)
{
	SWFMovie mo;
	SWFMovieClip dejagnuclip;
	SWFMovieClip mc1;
	SWFDisplayItem it;

	const char *srcdir=".";
	if ( argc>1 ) 
		srcdir=argv[1];
	else
	{
   		//fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
   		//return 1;
	}

	Ming_init();
	mo = newSWFMovieWithVersion(OUTPUT_VERSION);
	SWFMovie_setDimension(mo, 800, 600);
	SWFMovie_setRate (mo, 12);

	dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
	SWFMovie_add(mo, (SWFBlock)dejagnuclip);
	SWFMovie_nextFrame(mo);  // end of frame 1


	// Add a static red square at depth 3
	mc1 = get_static_mc();
	it = SWFMovie_add(mo, (SWFBlock)mc1); 
	SWFDisplayItem_setDepth(it, 3); 
	SWFDisplayItem_setName(it, "staticmc");
	SWFMovie_nextFrame(mo); // end of frame 2

	// Create an (dynamic) green movieclip at depth 3
	add_actions(mo, "createEmptyMovieClip('dynamicmc', 3);"
			"with (dynamicmc) {"
			" lineStyle(1, 0x000000, 100);"	
			" beginFill(0x00FF00, 100);"
			" moveTo(20, 320);"
			" lineTo(80, 320);"
			" lineTo(80, 380);"
			" lineTo(20, 380);"
			" lineTo(20, 320);"
			"}");
	SWFMovie_nextFrame(mo); // end of frame 3
	
	// Check that both exist
	check_equals(mo, "typeof(staticmc)", "'movieclip'");
	check_equals(mo, "typeof(dynamicmc)", "'movieclip'");

	// Check that the static one had been moved at negative depth !
	check_equals(mo, "staticmc.getDepth()", "-16381");
	check_equals(mo, "dynamicmc.getDepth()", "3");

	add_actions(mo, "_root.totals(); stop();");
	SWFMovie_nextFrame(mo);        

	//Output movie
	puts("Saving " OUTPUT_FILENAME );
	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
