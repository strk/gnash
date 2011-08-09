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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 ***********************************************************************
 *
 * Test case for the RemoveObject2 tag
 * It places and removes 3 squares every 5 frames.
 *
 ***********************************************************************/

#include <stdio.h>
#include <ming.h>

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "RemoveObject2Test.swf"

static SWFShape
make_square(int x, int y, int width, int height, byte r, byte g, byte b)
{
	SWFShape sh = newSWFShape();
	SWFShape_setLineStyle(sh, 1, r, g, b, 255);
	SWFShape_movePenTo(sh, x, y);
	SWFShape_drawLineTo(sh, x, y+height);
	SWFShape_drawLineTo(sh, x+width, y+height);
	SWFShape_drawLineTo(sh, x+width, y);
	SWFShape_drawLineTo(sh, x, y);

	return sh;
}

int main(void)
{
	SWFMovie mo;
	SWFDisplayItem it1, it2, it3;
	SWFShape sh1, sh2, sh3;
	int framenum;

	/*********************************************
	 *
	 * Initialization
	 *
	 *********************************************/

	puts("Setting things up");

	Ming_init();
        Ming_useSWFVersion (OUTPUT_VERSION);
	Ming_setScale(20.0); 
 
	mo = newSWFMovie();

	/*****************************************************
	 *
	 * Add the square named 
	 *
	 *****************************************************/

	SWFMovie_setDimension(mo, 100, 100);

#define FRAMESGAP 5

	sh1 = make_square(10, 10, 20, 20, 255, 0, 0);
	it1 = SWFMovie_add(mo, (SWFBlock)sh1);
	SWFDisplayItem_setDepth(it1, 1);
	SWFDisplayItem_setName(it1, "Name1");

	for (framenum=0; framenum<FRAMESGAP; framenum++) {
		SWFMovie_nextFrame(mo); 
	}

	sh2 = make_square(35, 10, 20, 20, 0, 255, 0);
	it2 = SWFMovie_add(mo, (SWFBlock)sh2);
	SWFDisplayItem_setDepth(it2, 2);
	SWFDisplayItem_setName(it2, "Name2");

	for (framenum=0; framenum<FRAMESGAP; framenum++) {
		SWFMovie_nextFrame(mo); 
	}

	sh3 = make_square(10, 35, 45, 20, 0, 0, 255);
	it3 = SWFMovie_add(mo, (SWFBlock)sh3);
	SWFDisplayItem_setDepth(it3, 3);
	SWFDisplayItem_setName(it3, "Name3");

	for (framenum=0; framenum<FRAMESGAP; framenum++) {
		SWFMovie_nextFrame(mo); 
	}

	SWFMovie_remove(mo, it1);

	for (framenum=0; framenum<FRAMESGAP; framenum++) {
		SWFMovie_nextFrame(mo); 
	}

	SWFMovie_remove(mo, it2);

	for (framenum=0; framenum<FRAMESGAP; framenum++) {
		SWFMovie_nextFrame(mo); 
	}

	SWFMovie_remove(mo, it3);

	puts("Saving " OUTPUT_FILENAME );

	SWFMovie_nextFrame(mo); /* showFrame */

	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
