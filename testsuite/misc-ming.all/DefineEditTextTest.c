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
 *
 */ 

/*
 * Test DefineEditText tag.
 * Uses both "embedded" font and device fonts.
 * The text written is 'Hello world' in both cases.
 * Text at the bottom is the one with embedded fonts.
 *
 * run as ./DefineEditTextTest
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#define OUTPUT_VERSION 7
#define OUTPUT_FILENAME "DefineEditTextTest.swf"

SWFDisplayItem add_text_field(SWFMovie mo, SWFBlock font, const char* text);

SWFDisplayItem
add_text_field(SWFMovie mo, SWFBlock font, const char* text)
{
	SWFTextField tf;

	tf = newSWFTextField();

	SWFTextField_setFont(tf, font);

	/* setting flags seem unneeded */
	/*SWFTextField_setFlags(tf, SWFTEXTFIELD_USEFONT|SWFTEXTFIELD_NOEDIT);*/
	SWFTextField_addChars(tf, text);

	SWFTextField_addString(tf, text);

	/*
	 * Bounds computed by Ming (if we omit the setBounds call)
	 * are 2640, 240. This means that we're shrinking the available
	 * space with this explicit setting. Gnash chokes in this case.
	 *
	 * Ref: https://savannah.gnu.org/bugs/?func=detailitem&item_id=16637.
	 */
	SWFTextField_setBounds(tf, 260, 338);
	//SWFTextField_setBounds(tf, 60000, 338);

	/*
	 * The following settings (found in the reported SWF)
	 * are not needed to exploit the bug.
	 */
 
	/*SWFTextField_setHeight(tf, 240);*/
	/*SWFTextField_setColor(tf, 0x00, 0x00, 0x00, 0xff);*/
	/*SWFTextField_setAlignment(tf, SWFTEXTFIELD_ALIGN_LEFT);*/
	/*SWFTextField_setLeftMargin(tf, 0);*/
	/*SWFTextField_setRightMargin(tf, 0);*/
	/*SWFTextField_setIndentation(tf, 0);*/
	/*SWFTextField_setLineSpacing(tf, 40);*/
	/*SWFTextField_setLineSpacing(tf, 40);*/

	return SWFMovie_add(mo, (SWFBlock)tf);
}

int
main(int argc, char** argv)
{
	SWFMovie mo;
	const char *srcdir=".";
	char fdbfont[256];

	/*********************************************
	 *
	 * Initialization
	 *
	 *********************************************/

	if ( argc>1 ) srcdir=argv[1];
	else
	{
		fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
		return 1;
	}

	sprintf(fdbfont, "%s/Bitstream Vera Sans.fdb", srcdir);

	puts("Setting things up");

	Ming_init();
        Ming_useSWFVersion (OUTPUT_VERSION);
	//Ming_setScale(20.0); /* so we talk twips */
 
	mo = newSWFMovie();
	SWFMovie_setRate(mo, 12);
	//SWFMovie_setDimension(mo, 12560, 9020);

	/*********************************************
	 *
	 * Add some textfields
	 *
	 *********************************************/

	/* This is with embedded fonts, not working */
	{
		SWFDisplayItem it;
		FILE *font_file = fopen(fdbfont, "r");
		if ( font_file == NULL )
		{
			perror(fdbfont);
			exit(1);
		}
		SWFBrowserFont bfont = newSWFBrowserFont("_sans");
		SWFFont efont = loadSWFFontFromFile(font_file);

		it = add_text_field(mo, (SWFBlock)bfont, "Hello device _sans font world");
		SWFDisplayItem_setName(it, "dtext");
		SWFDisplayItem_moveTo(it, 60, 60);
		it = add_text_field(mo, (SWFBlock)efont, "Hello embedded font world");
		SWFDisplayItem_setName(it, "etext");
		SWFDisplayItem_moveTo(it, 60, 120);

		SWFBrowserFont bfont2 = newSWFBrowserFont("times");
		it = add_text_field(mo, (SWFBlock)bfont2, "Hello device times font world");
		SWFDisplayItem_setName(it, "dtext2");
		SWFDisplayItem_moveTo(it, 60, 180);
	}

	SWFMovie_add(mo, newSWFAction("offset = 1; count=0;"
			        "onEnterFrame = function() {"
				" if ( ++count > 10 ) { count = 0; offset = -offset; }"
				" etext._y += offset;"
				" etext._x += offset;"
				//" etext._rotation += offset;"
				" dtext._y += offset;"
				" dtext._x += offset;"
				//" dtext._rotation += offset;"
				" dtext2._y += offset;"
				" dtext2._x += offset;"
				//" dtext2._rotation += offset;"
				"};"
				));

	/*****************************************************
	 *
	 * Output movie
	 *
	 *****************************************************/

	puts("Saving " OUTPUT_FILENAME );

	SWFMovie_nextFrame(mo); /* showFrame */

	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
