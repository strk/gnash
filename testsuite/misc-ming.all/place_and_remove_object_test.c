/* 
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */ 

/*
 * Zou Lunkai, zoulunkai@gmail.com
 *
 * Test for tag PlaceObject2 and also sprite_instance::advance_sprite(float delta_time)
 *
 * Normally, you will see the both the red square and black square
 *  again and again while looping back
 *
 * run as ./place_and_remove_object_test
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "place_and_remove_object_test.swf"




int
main(int argc, char** argv)
{
	SWFMovie mo;
	SWFMovieClip dejagnuclip;
	SWFShape  sh1,sh2;

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
	SWFMovie_setRate (mo, 1.0);

	dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
	SWFMovie_add(mo, (SWFBlock)dejagnuclip);
	//SWFMovie_nextFrame(mo); 


	sh1 = make_fill_square (0, 300, 60, 60, 255, 0, 0, 255, 0, 0);
	sh2 = make_fill_square (330, 300, 60, 60, 255, 0, 0, 0, 0, 0);

		
	SWFDisplayItem it;
	it = SWFMovie_add(mo, (SWFBlock)sh1);  //add a red square to the 1st frame at depth 3
	SWFDisplayItem_setDepth(it, 3); 
	SWFDisplayItem_setName(it, "sh1");
	check(mo, "_root.sh1 != undefined");
	check_equals(mo, "_root.sh2",  "undefined");
	SWFMovie_nextFrame(mo);        
	
	SWFMovie_remove(mo, it);          //remove the red square at the 2nd frame
	check_equals(mo, "_root.sh1",  "undefined");
	check_equals(mo, "_root.sh2",  "undefined");
	SWFMovie_nextFrame(mo);       
	
	it = SWFMovie_add(mo, (SWFBlock)sh2);  //add a black square to the 3rd frame at depth 3
	SWFDisplayItem_setDepth(it, 3); 
	SWFDisplayItem_setName(it, "sh2");
	check_equals(mo, "_root.sh1",  "undefined");
	check(mo, "_root.sh2 != undefined");
	add_actions(mo, "if ( ++counter > 1 ) { _root.totals(); stop(); }");
	SWFMovie_nextFrame(mo);        

	//Output movie
	puts("Saving " OUTPUT_FILENAME );
	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
