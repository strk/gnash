/***********************************************************************
 *
 *   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Linking Gnash statically or dynamically with other modules is making a
 * combined work based on Gnash. Thus, the terms and conditions of the GNU
 * General Public License cover the whole combination.
 *
 * As a special exception, the copyright holders of Gnash give you
 * permission to combine Gnash with free software programs or libraries
 * that are released under the GNU LGPL and with code included in any
 * release of Talkback distributed by the Mozilla Foundation. You may
 * copy and distribute such a system following the terms of the GNU GPL
 * for all but the LGPL-covered parts and Talkback, and following the
 * LGPL for the LGPL-covered parts.
 *
 * Note that people who make modified versions of Gnash are not obligated
 * to grant this special exception for their modified versions; it is their
 * choice whether to do so. The GNU General Public License gives permission
 * to release a modified version without this exception; this exception
 * also makes it possible to release a modified version which carries
 * forward this exception.
 *
 ***********************************************************************
 *
 * Test case for the PlaceObject2 tag
 * It places two squares at two different depths with two different
 * names, then places a third at depth of one and with name of the
 * other.
 *
 ***********************************************************************/

#include <stdio.h>
#include <ming.h>

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "PlaceObject2Test.swf"

SWFShape
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

int
main(int argc, char **argv)
{
	SWFMovie mo;
	SWFDisplayItem it;
	SWFShape sh;

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

	sh = make_square(10, 10, 20, 20, 255, 0, 0);
	it = SWFMovie_add(mo, (SWFBlock)sh);
	SWFDisplayItem_setDepth(it, 1);
	SWFDisplayItem_setName(it, "Name1");

	sh = make_square(35, 10, 20, 20, 0, 255, 0);
	it = SWFMovie_add(mo, (SWFBlock)sh);
	SWFDisplayItem_setDepth(it, 2);
	SWFDisplayItem_setName(it, "Name2");

	sh = make_square(10, 35, 45, 20, 0, 0, 255);
	it = SWFMovie_add(mo, (SWFBlock)sh);
	SWFDisplayItem_setDepth(it, 1);
	SWFDisplayItem_setName(it, "Name2");

	puts("Saving " OUTPUT_FILENAME );

	SWFMovie_nextFrame(mo); /* showFrame */

	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
