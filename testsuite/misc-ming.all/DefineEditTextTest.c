/* 
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
 */ 

/*
 * Test DefineEditText tag.
 * Uses "embedded" font and defines a smaller rectangle then required.
 *
 * run as ./DefineEditTextTest
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#define OUTPUT_VERSION 7
#define OUTPUT_FILENAME "DefineEditTextTest.swf"

void add_text_field(SWFMovie mo, SWFBlock font, const char* text);

void
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
	SWFTextField_setBounds(tf, 160, 338);

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

	SWFMovie_add(mo, (SWFBlock)tf);
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
	Ming_setScale(1.0); /* so we talk twips */
 
	mo = newSWFMovie();
	SWFMovie_setRate(mo, 24);
	SWFMovie_setDimension(mo, 12560, 9020);

	/*********************************************
	 *
	 * Add some textfields
	 *
	 *********************************************/

	/* This is with embedded fonts, not working */
	{
		FILE *font_file = fopen(fdbfont, "r");
		if ( font_file == NULL )
		{
			perror(fdbfont);
			exit(1);
		}
		/*SWFBrowserFont bfont = newSWFBrowserFont("_sans");*/
		SWFFont bfont = loadSWFFontFromFile(font_file);
		add_text_field(mo, (SWFBlock)bfont, "Hello world");
	}

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
