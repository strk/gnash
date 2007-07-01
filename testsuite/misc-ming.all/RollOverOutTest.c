/***********************************************************************
 *
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
 * shape in the middle of the stage.
 *
 * The movie has 3 frames.
 *
 *	- frame1: initialization
 *	- frame2: the square is red
 *	- frame3: the square is green
 *
 * In frame2, rollOver event moves the playhead to frame3.
 * In frame3, rollOut  event moves the playhead to frame2.
 *
 ***********************************************************************/

#include "ming_utils.h"

#include <ming.h>
#include <stdio.h>
#include <stdlib.h>

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "RollOverOutTest.swf"

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

SWFDisplayItem add_square(SWFMovie mo, byte r, byte g, byte b, int depth)
{
	SWFDisplayItem it;
	SWFMovieClip mc;
	SWFShape sh;
	mc = newSWFMovieClip();
	sh = make_fill_square(0, 0, 40, 40, r, g, b, r, g, b);
	SWFMovieClip_add(mc, (SWFBlock)sh);
	SWFMovieClip_nextFrame(mc); /* showFrame */
	it = SWFMovie_add(mo, (SWFBlock)mc);
	SWFDisplayItem_setDepth(it, depth);
	return it;
}

void
add_text_field(SWFMovie mo, const char* name, int depth)
{
	SWFDisplayItem it;
	SWFTextField tf = newSWFTextField();
	SWFTextField_setFlags(tf, SWFTEXTFIELD_DRAWBOX);

	SWFTextField_setFont(tf, (void*)font);
	SWFTextField_addChars(tf, " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ012345689:.,/\\#@?!");
	SWFTextField_addString(tf, " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ012345689:.,/\\#@?!");
	SWFTextField_setVariableName(tf, name);

	it = SWFMovie_add(mo, (SWFBlock)tf);
	SWFDisplayItem_scale(it, 0.3, 0.3);
	SWFDisplayItem_moveTo(it, 0, 10);
	SWFTextField_setBounds(tf, 120*(1/0.3), 10*(1/0.3));
	SWFDisplayItem_setDepth(it, depth);
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

	add_text_field(mo, "_root.msg", 1);

	/*****************************************************
	 *
	 * Add squares
	 *
	 *****************************************************/

	it = add_square(mo, 255, 0, 0, 2);
	SWFDisplayItem_moveTo(it, 40, 40);
	SWFDisplayItem_setName(it, "square1");

	it = add_square(mo, 0, 255, 0, 3);
	SWFDisplayItem_moveTo(it, 40, 40);
	SWFDisplayItem_setName(it, "square2");

	add_code(mo, "square1._visible = false; square2._visible=false;");

	SWFMovie_nextFrame(mo); /* showFrame */


	/*****************************************************
	 *
	 * Frame 1: display a red square.
	 *          onRollOver: goto frame2
	 *
	 *****************************************************/

	set_text(mo, "Frame1: move the mouse on the square");
	add_code(mo, "square1._visible = true; square2._visible=false;");

	add_event(mo, "square1", "RollOver", "gotoAndPlay(2)");
	//add_event(mo, "square", "RollOver", "");
	//add_event(mo, "square", "RollOut", "");
	//add_event(mo, "square", "MouseDown", "");
	//add_event(mo, "square", "MouseUp", "");

	add_code(mo, "stop();");
	SWFMovie_nextFrame(mo); /* showFrame */

	/*****************************************************
	 *
	 * Frame 2: display a green square.
	 *          onRollOut: goto frame1
	 *
	 *****************************************************/

	set_text(mo, "Frame2: move the mouse off the square");
	add_code(mo, "square2._visible = true; square1._visible=false;");

	//add_event(mo, "square", "RollOver", "gotoAndPlay(2)");
	//add_event(mo, "square", "RollOver", "");
	add_event(mo, "square2", "RollOut", "gotoAndPlay(1)");
	//add_event(mo, "square", "MouseDown", "");
	//add_event(mo, "square", "MouseUp", "");

	add_code(mo, "stop();");
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
