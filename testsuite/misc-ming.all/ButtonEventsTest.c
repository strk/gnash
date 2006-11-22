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
 *
 ***********************************************************************
 *
 * Test case for mouse events.
 *
 * In a movie of 120x120 pixels, it places a movieclip containing a squared
 * button in the middle of the stage, and a text area on top.
 *
 * The movie has 3 frames, with the second adding a shape at a lower depth
 * and the third one at an higher depth respect to the button.
 *
 * The following events print the event name in the text area
 * (called _root.textfield) and change the color of the button:
 *
 * MouseOut  : red button (initial state)
 * MouseOver : yellow button
 * MouseDown : green button
 * MouseUp   : yellow button (same as MouseOver, but the label on top changes)
 *
 ***********************************************************************/

#include "ming_utils.h"

#include <ming.h>
#include <stdio.h>
#include <stdlib.h>

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "ButtonEventsTest.swf"

SWFFont font;

void add_event(SWFMovie mo, const char* name, const char* event, const char* action);
void add_code(SWFMovie mo, const char* code);
void add_text_field(SWFMovie mo, const char* name, int depth);
void set_text(SWFMovie mo, const char* text);
SWFDisplayItem add_square(SWFMovie mo, byte r, byte g, byte b, int depth);

void
add_event(SWFMovie mo, const char* name, const char* event, const char* action)
{
	SWFAction ac;
	char buf[1024];

	sprintf(buf,
	"event=undefined;"
	"%s.on%s=function() { %s; };"
	, name, event, action
	);
	ac = compileSWFActionCode(buf);

	SWFMovie_add(mo, (SWFBlock)ac);
}

void
add_code(SWFMovie mo, const char* code)
{
	SWFAction ac;

	ac = compileSWFActionCode(code);

	SWFMovie_add(mo, (SWFBlock)ac);
}

SWFDisplayItem add_button(SWFMovie mo);
SWFDisplayItem
add_button(SWFMovie mo)
{
	SWFDisplayItem it;
	SWFMovieClip mc;
	SWFShape sh1, sh2, sh3, sh4;
	SWFButton bu = newSWFButton();
	mc = newSWFMovieClip();

	sh1 = make_fill_square(0, 0, 40, 40, 0, 0, 0, 0, 0, 0);
	sh2 = make_fill_square(0, 0, 40, 40, 255, 0, 0, 255, 0, 0);
	sh3 = make_fill_square(0, 0, 40, 40, 0, 255, 0, 0, 255, 0);
	sh4 = make_fill_square(0, 0, 40, 40, 255, 255, 0, 255, 255, 0);

	SWFButton_addShape(bu, (SWFCharacter)sh1, SWFBUTTON_HIT);
	SWFButton_addShape(bu, (SWFCharacter)sh2, SWFBUTTON_UP );
	SWFButton_addShape(bu, (SWFCharacter)sh3, SWFBUTTON_DOWN );
	SWFButton_addShape(bu, (SWFCharacter)sh4, SWFBUTTON_OVER );

	SWFButton_addAction(bu, compileSWFActionCode("_root.msg=\"MouseOut\";"), SWFBUTTON_MOUSEOUT);
	SWFButton_addAction(bu, compileSWFActionCode("_root.msg=\"MouseOver\";"), SWFBUTTON_MOUSEOVER);
	SWFButton_addAction(bu, compileSWFActionCode("_root.msg=\"MouseDown\";"), SWFBUTTON_MOUSEDOWN);
	SWFButton_addAction(bu, compileSWFActionCode("_root.msg=\"MouseUp\";"), SWFBUTTON_MOUSEUP);

	SWFMovieClip_add(mc, (SWFBlock)bu);
	SWFMovieClip_nextFrame(mc); /* showFrame */

	it = SWFMovie_add(mo, (SWFBlock)mc);
	return it;
}

void
add_text_field(SWFMovie mo, const char* name, int depth)
{
	SWFDisplayItem it;
	SWFTextField tf = newSWFTextField();
	/*SWFTextField_setFlags(tf, SWFTEXTFIELD_DRAWBOX);*/

	SWFTextField_setFont(tf, (void*)font);
	SWFTextField_addChars(tf, " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ012345689:.,/\\#@?!");
	SWFTextField_addString(tf, "Play with the button");
	SWFTextField_setVariableName(tf, name);

	it = SWFMovie_add(mo, (SWFBlock)tf);
	//SWFDisplayItem_scale(it, 0.3, 0.3);
	SWFDisplayItem_moveTo(it, 0, 10);
	//SWFTextField_setBounds(tf, 120*(1/0.3), 10*(1/0.3));
	SWFDisplayItem_setDepth(it, depth);
	SWFDisplayItem_setName(it, "textfield");
}

void
set_text(SWFMovie mo, const char* text)
{
	char buf[1024];
	sprintf(buf, "_root.msg=\"%s\";", text);
	add_code(mo, buf);
}

int
main(int argc, char **argv)
{
	SWFMovie mo;
	SWFDisplayItem it;
	const char *srcdir=".";
	char fdbfont[256];

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
	SWFMovie_setDimension(mo, 120, 120);
	SWFMovie_setRate(mo, 0.2);

	if ( argc>1 ) srcdir=argv[1];
	else
	{
		fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
		return 1;
	}

	sprintf(fdbfont, "%s/Bitstream Vera Sans.fdb", srcdir);
	FILE *font_file = fopen(fdbfont, "r");
	if ( font_file == NULL )
	{
		perror(fdbfont);
		exit(1);
	}
	/*SWFBrowserFont bfont = newSWFBrowserFont("_sans");*/
	font = loadSWFFontFromFile(font_file);

	add_text_field(mo, "_root.msg", 10);

	/*****************************************************
	 *
	 * Add button
	 *
	 *****************************************************/

	it = add_button(mo);
	SWFDisplayItem_moveTo(it, 40, 40);
	SWFDisplayItem_setName(it, "square1");
	SWFDisplayItem_setDepth(it, 2);

	SWFMovie_nextFrame(mo); /* showFrame */

	/*****************************************************
	 *
	 * On second frame, add a shape at lower depth 
	 *
	 *****************************************************/

	{
		SWFShape sh = make_fill_square(0, 0, 120, 120, 0, 0, 0, 0, 255, 0);
		SWFDisplayItem itsh = SWFMovie_add(mo, (SWFBlock)sh);
		SWFDisplayItem_setDepth(itsh, 1);

		SWFMovie_nextFrame(mo); /* showFrame */
	}

	/*****************************************************
	 *
	 * On third frame, add a shape at higher depth 
	 *
	 *****************************************************/

	{
		SWFShape sh = make_fill_square(0, 0, 120, 120, 0, 0, 0, 0, 255, 0);
		SWFDisplayItem itsh = SWFMovie_add(mo, (SWFBlock)sh);
		SWFDisplayItem_setDepth(itsh, 3);
		SWFDisplayItem_setColorAdd(itsh, 0, 0, 0, -128);

		SWFMovie_nextFrame(mo); /* showFrame */
	}

	/*****************************************************
	 *
	 * Save it...
	 *
	 *****************************************************/

	puts("Saving " OUTPUT_FILENAME );

	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
