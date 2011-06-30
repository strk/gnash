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
 */

#include "ming_utils.h"

#include <ming.h>
#include <stdio.h>
#include <stdlib.h>

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "ButtonPropertiesTest.swf"

SWFFont font;

void add_event(SWFMovie mo, const char* name, const char* event, const char* action);
SWFDisplayItem add_button(SWFMovie mo);

SWFDisplayItem
add_button(SWFMovie mo)
{
	SWFDisplayItem it;
	SWFMovieClip mc, mc1;
	SWFButtonRecord br;
	SWFShape sh1, sh2, sh3, sh4, sh1a, sh2a, sh3a, sh4a;
	SWFButton bu = newSWFButton();
	mc = newSWFMovieClip();

	sh1 = make_fill_square(0, 0, 40, 40, 0, 0, 0, 0, 0, 0);
	sh1a = make_fill_square(30, 30, 5, 5, 128, 128, 128, 128, 128, 128);
	sh2 = make_fill_square(0, 0, 40, 40, 255, 0, 0, 255, 0, 0);
	sh2a = make_fill_square(30, 30, 5, 5, 128, 0, 0, 128, 0, 0);
	sh3 = make_fill_square(0, 0, 40, 40, 0, 255, 0, 0, 255, 0);
	sh3a = make_fill_square(30, 30, 5, 5, 0, 128, 0, 0, 128, 0);
	sh4 = make_fill_square(0, 0, 40, 40, 255, 255, 0, 255, 255, 0);
	sh4a = make_fill_square(30, 30, 5, 5, 128, 128, 0, 128, 128, 0);

	/* Higher depth DisplayObject is intentionally added before lower depth one */
	br = SWFButton_addCharacter(bu, (SWFCharacter)sh1a, SWFBUTTON_HIT);
	SWFButtonRecord_setDepth(br, 2);
	br = SWFButton_addCharacter(bu, (SWFCharacter)sh1, SWFBUTTON_HIT);
	SWFButtonRecord_setDepth(br, 1);

	/* Higher depth DisplayObject is intentionally added before lower depth one */
	br = SWFButton_addCharacter(bu, (SWFCharacter)sh2a, SWFBUTTON_UP );
	SWFButtonRecord_setDepth(br, 2);
	br = SWFButton_addCharacter(bu, (SWFCharacter)sh2, SWFBUTTON_UP );
	SWFButtonRecord_setDepth(br, 1);

    mc1 = newSWFMovieClip();
    br = SWFButton_addCharacter(bu, (SWFCharacter)mc1, SWFBUTTON_UP);
	SWFButtonRecord_setDepth(br, 8);

    br = SWFButton_addCharacter(bu, (SWFCharacter)mc1, SWFBUTTON_DOWN);
	SWFButtonRecord_setDepth(br, 7);

    br = SWFButton_addCharacter(bu, (SWFCharacter)mc1, SWFBUTTON_HIT);
	SWFButtonRecord_setDepth(br, 9);

	it = SWFMovieClip_add(mc, (SWFBlock)bu);
	SWFDisplayItem_setName(it, "button");
	SWFMovieClip_nextFrame(mc); /* showFrame */


	it = SWFMovie_add(mo, (SWFBlock)mc);
	return it;
}

int
main(int argc, char **argv)
{
	SWFMovie mo;
	SWFDisplayItem it;
	const char *srcdir=".";
	SWFMovieClip dejagnuclip;

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
	SWFMovie_setDimension(mo, 800, 600);
	SWFMovie_setRate(mo, 12);

	if ( argc>1 ) srcdir=argv[1];
	else
	{
		fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
		return 1;
	}

	font = get_default_font(srcdir); 

	/* Dejagnu equipment */
	dejagnuclip = get_dejagnu_clip((SWFBlock)font, 10, 0, 0, 800, 600);
	it = SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  	SWFDisplayItem_setDepth(it, 200); 
  	SWFDisplayItem_move(it, 200, 0); 

	SWFMovie_nextFrame(mo); /* showFrame */

	/*****************************************************
	 *
	 * Add button
	 *
	 *****************************************************/

	it = add_button(mo);
	SWFDisplayItem_moveTo(it, 40, 30);
	SWFDisplayItem_setName(it, "square1");
	SWFDisplayItem_setDepth(it, 2);

    /* This button has one character per state. It shows that each state
     * except HIT generates one new instance and deletes the old one.
     * HIT deletes the old instance property and does not add a new one.
     */

    add_actions(mo, "note('This is a very simple test. Do anything you like "
            "with the buttons and you should get no failures');");

    add_actions(mo, "var c = 2;");
    add_actions(mo,
            "props = function() {"
            "   s=''; "
            "   for (i in square1.button) { "
            "       if (i.substr(0, 8) == 'instance') { s += i; }; "
            "   };"
            "   return s;"
            "};");
    add_actions(mo, "check_equals(props(), 'instance' + c++);");
    add_actions(mo,
            "square1.button.onRollOver = function() {"
            "   check_equals(props(), '');"
            "};"
            "square1.button.onRollOut = function() {"
            "   check_equals(props(), 'instance' + c++);"
            "};"
            "square1.button.onMouseDown = function() {"
            "   check_equals(props(), 'instance' + c++);"
            "};"
            "square1.button.onPress = function() {"
            "   check_equals(props(), 'instance' + c++);"
            "};"
            "square1.button.onRelease = function() {"
            "   check_equals(props(), '');"
            "};"
            "square1.button.onReleaseOutside = function() {"
            "   check_equals(props(), 'instance' + c++);"
            "};"
            );
             
    check_equals(mo, "square1.button.getDepth()", "-16383");
    add_actions(mo, "stop();");
	SWFMovie_nextFrame(mo); /* showFrame */

	/*****************************************************
	 *
	 * Save it...
	 *
	 *****************************************************/

	puts("Saving " OUTPUT_FILENAME );

	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
