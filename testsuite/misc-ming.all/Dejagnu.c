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
 *
 */ 

/*
 * Generate a Dejagnu.swf file for import from self-contained
 * SWF testcases
 *
 * run as ./Dejagnu <mediadir>
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 5
#define OUTPUT_FILENAME "Dejagnu.swf"

int
main(int argc, char** argv)
{
	SWFMovie mo;
	SWFMovieClip dejaclip;
	const char *srcdir=".";
	SWFFont bfont; 


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

	puts("Setting things up");

	Ming_init();
        Ming_useSWFVersion (OUTPUT_VERSION);
	Ming_setScale(20.0); /* let's talk pixels */
 
	mo = newSWFMovie();
	SWFMovie_setRate(mo, 1);
	SWFMovie_setDimension(mo, 628, 1024);

	bfont = get_default_font(srcdir);

	/*********************************************
	 *
	 * Body
	 *
	 *********************************************/

	dejaclip = get_dejagnu_clip((SWFBlock)bfont, 3000, 0, 50, 800, 800);

	SWFMovie_add(mo, (SWFBlock)dejaclip);
	SWFMovie_addExport(mo, (SWFBlock)dejaclip, "dejagnu");
	SWFMovie_addExport(mo, (SWFBlock)SWFMovie_addFont(mo, bfont),
		"dejafont");

	SWFMovie_nextFrame(mo); /* showFrame */


	/*****************************************************
	 *
	 * Output movie
	 *
	 *****************************************************/

	puts("Saving " OUTPUT_FILENAME );

	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
