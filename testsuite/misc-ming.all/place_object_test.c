/* 
 *   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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

/*
 * Zou Lunkai, zoulunkai@gmail.com
 *
 * Test for tag PlaceObject2 
 * 
 * To see what happens if placing two shapes at the same depths.
 *
 * Obeserved behaviour:
 *    *both* shapes get rendered!
 *
 * run as ./place_object_test
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "place_object_test.swf"




int
main(int argc, char** argv)
{
	SWFMovie mo;
	SWFMovieClip mc1, mc2, dejagnuclip;
	SWFDisplayItem it;
	SWFShape  sh1,sh2;
	SWFAction ac1, ac2;
	int i;

	const char *srcdir=".";
	if ( argc>1 ) 
		srcdir=argv[1];
	else
	{
   		//fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
   		//return 1;
	}

	Ming_init();
	mo = newSWFMovie();
	SWFMovie_setDimension(mo, 800, 600);
	//SWFMovie_setRate (mo, 1.0);

	dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
	SWFMovie_add(mo, (SWFBlock)dejagnuclip);
	SWFMovie_nextFrame(mo); 

	sh1 = make_fill_square (250, 300, 60, 60, 255, 0, 0, 255, 0, 0);
	sh2 = make_fill_square (300, 300, 60, 60, 255, 0, 0, 0, 0, 0);
	
	it = SWFMovie_add(mo, (SWFBlock)sh1);  
	SWFDisplayItem_setName(it, "sh1"); 
	SWFDisplayItem_setDepth(it, 3); //place the sh1 character at depth 3
	
	it = SWFMovie_add(mo, (SWFBlock)sh2);  
	SWFDisplayItem_setName(it, "sh2"); 
	SWFDisplayItem_setDepth(it, 3); //place the sh2 character at depth 3 again!

	check(mo, "sh1 != undefined");
	xcheck(mo, "sh2 != undefined");

	// we need to allow 2 runs as the first one won't play (correct?)
	add_actions(mo, "_root.totals(); stop();");

	SWFMovie_nextFrame(mo); 


	//Output movie
	puts("Saving " OUTPUT_FILENAME );
	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
