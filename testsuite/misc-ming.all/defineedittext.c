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
 * Currently only uses "device" (browser) font
 *
 * run as ./defineedittext
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#define OUTPUT_VERSION 7
#define OUTPUT_FILENAME "defineedittext.swf"


void
add_clip(SWFMovie mo, char* file, char* name,
		char* url, int x, int y)
{
	FILE *fd;
	SWFJpegBitmap bm;
	SWFShape sh;
	SWFMovieClip mc;
	SWFDisplayItem it;
	SWFAction ac;
	char action[1024];

	printf("Adding %s\n", file);

	fd = fopen(file, "r");
	if ( ! fd ) {
		perror(file);
		exit(1);
	}
	bm = newSWFJpegBitmap(fd);
	sh = newSWFShapeFromBitmap((SWFBitmap)bm, SWFFILL_CLIPPED_BITMAP);
	mc = newSWFMovieClip();
	SWFMovieClip_add(mc, (SWFBlock)sh);
	SWFMovieClip_nextFrame(mc); /* showFrame */
	it = SWFMovie_add(mo, (SWFBlock)mc);
	SWFDisplayItem_setName(it, name);
	SWFDisplayItem_moveTo(it, x, y);

	/* "Click" handler */
	sprintf(action, " \
%s.onPress = function () { \
	_root.CoverArtLoader.loadClip('%s', coverart); \
}; \
", name, url);

	ac = compileSWFActionCode(action);

	SWFMovie_add(mo, (SWFBlock)ac);
}

void
add_text_field(SWFMovie mo, SWFBlock font, const char* text)
{
	SWFTextField tf;

	tf = newSWFTextField();

	SWFTextField_addString(tf, text);

	SWFTextField_setFont(tf, (SWFBlock)font);

	SWFMovie_add(mo, (SWFBlock)tf);
}

int
main(int argc, char** argv)
{
	SWFMovie mo;

	/*********************************************
	 *
	 * Initialization
	 *
	 *********************************************/

	puts("Setting things up");

	Ming_init();
        Ming_useSWFVersion (OUTPUT_VERSION);
	Ming_setScale(1.0); /* so we talk twips */
 
	mo = newSWFMovie();
        SWFMovie_setDimension (mo, 11000, 8000); 
        SWFMovie_setRate (mo, 12.0); 
        SWFMovie_setBackground (mo, 255, 255, 255); 

	/*********************************************
	 *
	 * Add some textfields
	 *
	 *********************************************/

	/* This is with browser font */
	{
		SWFBrowserFont bfont = newSWFBrowserFont("_sans");
		add_text_field(mo, (SWFBlock)bfont, "Test");
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
