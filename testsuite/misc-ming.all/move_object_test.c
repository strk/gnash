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

/*
 * Test for tag PlaceObject2 used to move chars
 * 
 * Moves an "unnamed" movieclip square.
 * Expect the clip to get back to its original position
 * on loop-back.
 *
 * It is important for the clip to be unnamed, as 
 * we're testing for a bug not occurring if a name
 * is specified in PLACEOBJECT2 tag.
 * Luckly, this won't prevent us from referencing it,
 * as the player is expected to assign syntetized
 * instance names to unnamed objects.
 *
 * run as ./move_object_test
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "move_object_test.swf"




int
main(int argc, char** argv)
{
	SWFMovie mo;
	SWFMovieClip mc1, dejagnuclip;
	SWFDisplayItem it1;
	SWFShape  sh1;

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
	SWFMovie_setRate (mo, 1.0);

	sh1 = make_fill_square (0, 0, 60, 60, 255, 0, 0, 255, 0, 0);
	mc1 = newSWFMovieClip();
	SWFMovieClip_add(mc1, (SWFBlock)sh1);
	add_clip_actions(mc1, "onRollOver = function() {};");
	SWFMovieClip_nextFrame(mc1);

	dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 70, 800, 500);
	SWFMovie_add(mo, (SWFBlock)dejagnuclip);
	it1 = SWFMovie_add(mo, (SWFBlock)mc1);  
	check_equals(mo, "typeof(instance2)", "'movieclip'");
	check_equals(mo, "instance2._x", "0");

	SWFMovie_nextFrame(mo);  // frame 1

	SWFDisplayItem_moveTo(it1, 700, 1);

	// instance1 is automatically assigned to the dejagnu clip
	check_equals(mo, "typeof(instance1)", "'movieclip'");

	// instance2 is automatically assigned to the square clip
	check_equals(mo, "typeof(instance2)", "'movieclip'");
	check_equals(mo, "instance2._x", "700");

	add_actions(mo, "if ( ++counter > 1 ) { totals(); stop(); }");
	SWFMovie_nextFrame(mo);  // frame 2


	//Output movie
	puts("Saving " OUTPUT_FILENAME );
	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
