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
 *
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 ***********************************************************************
 *
 * Test case for the MovieClipLoader actionscript class
 *
 ***********************************************************************/

/*
 * run as ./movieclip_loader <mediadir>
 *
 * srcdir is where red.jpg, green.jp and offspring.jpg are located
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#define OUTPUT_VERSION 7
#define OUTPUT_FILENAME "moviecliploader_test.swf"

void add_clip(SWFMovie mo, char* file, char* name, char* url, int x, int y);

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

int
main(int argc, char** argv)
{
	SWFMovie mo;
	char file_red[256];
	char file_green[256];
	char file_offspring[256];
	char url_red[256];
	char url_green[256];
	char url_offspring[256];
	const char *srcdir=".";
	SWFShape sh_coverart;
	SWFMovieClip mc_coverart;
	SWFFillStyle fstyle;
	SWFDisplayItem it;
	SWFAction ac;

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

	sprintf(file_red, "%s/red.jpg", srcdir);
	sprintf(file_green, "%s/green.jpg", srcdir);
	sprintf(file_offspring, "%s/offspring.jpg", srcdir);

	/*
	 * Test urls with and w/out 'file://' prefix.
	 * Test both jpeg and swf loading.
	 */
	sprintf(url_red, "file://%s/red.swf", srcdir);
	sprintf(url_green, "file://%s/green.jpg", srcdir);
	sprintf(url_offspring, "%s/offspring.swf", srcdir);


	puts("Setting things up");

	Ming_init();
        Ming_useSWFVersion (OUTPUT_VERSION);
	Ming_setScale(1.0); /* so we talk twips */
 
	mo = newSWFMovie();
        SWFMovie_setDimension (mo, 11000, 8000); 
        SWFMovie_setRate (mo, 12.0); 
        SWFMovie_setBackground (mo, 255, 255, 255); 

	/*****************************************************
	 *
	 * MovieClipLoader class
	 *
	 *****************************************************/

	puts("Compiling MovieClipLoader actionscript");

	/* Action for first frame */
	ac = compileSWFActionCode
(" \
stop();  \
CoverArtLoader = new MovieClipLoader(); \
");
	SWFMovie_add(mo, (SWFBlock)ac);

	/* Add the LYNCH  clip */
	add_clip(mo, file_red, "red", url_red, 200, 4419);

	/* Add the GREEN  clip */
	add_clip(mo, file_green, "green", url_green, 3800, 4419);

	/* Add the OFFSPRING  clip */
	add_clip(mo, file_offspring, "offspring", url_offspring, 7400, 4419);

	/*****************************************************
	 *
	 * Add the coverart clip
	 *
	 *****************************************************/

	puts("Adding coverart");

	sh_coverart = newSWFShape();
	fstyle = SWFShape_addSolidFillStyle(sh_coverart, 0,0,0,255);
	SWFShape_setRightFillStyle(sh_coverart, fstyle);
	SWFShape_movePenTo(sh_coverart, 3400, 3400);
	SWFShape_drawLine(sh_coverart, -3400, 0);
	SWFShape_drawLine(sh_coverart, 0, -3400);
	SWFShape_drawLine(sh_coverart, 3400, 0);
	SWFShape_drawLine(sh_coverart, 0, 3400);

	mc_coverart = newSWFMovieClip();
	SWFMovieClip_add(mc_coverart, (SWFBlock)sh_coverart);
	SWFMovieClip_nextFrame(mc_coverart); /* showFrame */
	it = SWFMovie_add(mo, (SWFBlock)mc_coverart);
	SWFDisplayItem_setName(it, "coverart"); 
	SWFDisplayItem_moveTo(it, 3800, 500);

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
