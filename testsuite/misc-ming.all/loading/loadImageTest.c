/***********************************************************************
 *
 *   Copyright (C) 2007, 2009, 2010, 2011 Free Software Foundation, Inc.
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
 * Test case for the MovieClip.loadMovie actionscript method
 *
 ***********************************************************************/

/*
 * run as ./loadMovieTest <mediadir>
 *
 * mediadir is where lynch.{jpg,swf}, green.{jpg,swf}
 * and offspring.{jpg,swf} are located
 *
 */

#include "ming_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ming.h>

// In version 7 or below PNGs shouldn't work.
#define OUTPUT_VERSION 8
#define OUTPUT_FILENAME "loadImageTest.swf"

void add_clip(SWFMovie mo, char* file, char* name, char* url, int x, int y);
void add_button(SWFMovie mo, int x, int y, const char* label, SWFAction ac);
void add_window(SWFMovie mo, int x, int y);
SWFTextField get_label(const char* label, SWFFont font);


const char* mediadir=".";

void
add_window(SWFMovie mo, int x, int y)
{
	SWFShape sh_window;
	SWFFillStyle fstyle;
	SWFMovieClip mc_window;
	SWFDisplayItem it;

	sh_window = newSWFShape();
	fstyle = SWFShape_addSolidFillStyle(sh_window, 0,0,0,255);
	SWFShape_setRightFillStyle(sh_window, fstyle);
	SWFShape_movePenTo(sh_window, 170, 170);
	SWFShape_drawLine(sh_window, -170, 0);
	SWFShape_drawLine(sh_window, 0, -170);
	SWFShape_drawLine(sh_window, 170, 0);
	SWFShape_drawLine(sh_window, 0, 170);

	mc_window = newSWFMovieClip();
	SWFMovieClip_add(mc_window, (SWFBlock)sh_window);
	SWFMovieClip_add(mc_window, (SWFBlock)newSWFAction(
		"_root.xcheck(getBytesLoaded() < _root.getBytesLoaded());"
		"_root.xcheck(getBytesTotal() < _root.getBytesTotal());"
	));
	SWFMovieClip_nextFrame(mc_window); /* showFrame */

	it = SWFMovie_add(mo, (SWFBlock)mc_window);
	SWFDisplayItem_setName(it, "window"); 
	SWFDisplayItem_moveTo(it, x, y);

    SWFDisplayItem_addAction(it, compileSWFActionCode(
        "_root.note('Click on \"Load PNG\" to load a PNG movie here.');"
        ),
		SWFACTION_ROLLOVER);

	SWFDisplayItem_addAction(it, compileSWFActionCode(
		"delete _level0.window.onMouseDown;"
		),
		SWFACTION_ROLLOUT);

}

SWFTextField
get_label(const char* label, SWFFont font)
{
	SWFTextField tf = newSWFTextField();
	SWFTextField_setFont(tf, (void*)font);
	SWFTextField_addChars(tf, " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ012345689:.,/\\#@?!");
	SWFTextField_addString(tf, label);
	return tf;
}

void
add_button(SWFMovie mo, int x, int y, const char* label, SWFAction ac)
{
	SWFMovieClip btnclip = newSWFMovieClip();
	SWFFont font = get_default_font(mediadir);
	SWFDisplayItem it;
	SWFTextField tf = get_label(label, font);

	SWFMovieClip_add(btnclip, (SWFBlock)tf);
	SWFMovieClip_nextFrame(btnclip);

	it = SWFMovie_add(mo, (SWFBlock)btnclip);
	SWFDisplayItem_moveTo(it, x, y);
	SWFDisplayItem_addAction(it, ac, SWFACTION_PRESS);
}

int
main(int argc, char** argv)
{
	SWFMovie mo;
	SWFMovieClip dejagnuclip;
	SWFDisplayItem it;

	char url_png[512];
    char url_indexpng[512];
    char url_greypng[512];
    char url_gif[512];
    char url_igif[512];

    char png_action[256];
    char indexpng_action[256];
    char greypng_action[256];
    char gif_action[256];
    char igif_action[256];


	/*********************************************
	 *
	 * Initialization
	 *
	 *********************************************/

	if ( argc>1 ) mediadir=argv[1];
	else
	{
		fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
		return 1;
	}

	sprintf(url_png, "%s/png.png", mediadir);
	sprintf(url_indexpng, "%s/indexed.png", mediadir);
	sprintf(url_greypng, "%s/greyscale.png", mediadir);
	sprintf(url_gif, "%s/gif.gif", mediadir);
	sprintf(url_igif, "%s/gif-interlaced.gif", mediadir);

	puts("Setting things up");

	Ming_init();
    Ming_useSWFVersion (OUTPUT_VERSION);
	Ming_setScale(20.0); /* so we talk pixels */
 
	mo = newSWFMovie();
    SWFMovie_setDimension (mo, 800, 600); 
    SWFMovie_setRate (mo, 12.0); 
    SWFMovie_setBackground (mo, 255, 255, 255); 

	/*****************************************************
	 *
	 * Add Dejagnu clip
	 *
	 *****************************************************/

	dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(mediadir), 10, 0, 0, 800, 600);
	it = SWFMovie_add(mo, (SWFBlock)dejagnuclip);
	SWFDisplayItem_moveTo(it, 0, 250);
	SWFMovie_nextFrame(mo); 

	/*****************************************************
	 *
	 * Add the clips
	 *
	 *****************************************************/

    sprintf(png_action, "_root.window.loadMovie(\"%s\");", url_png);
    add_button(mo, 50, 100, "Load PNG", newSWFAction(png_action));

    sprintf(indexpng_action, "_root.window.loadMovie(\"%s\");", url_indexpng);
    add_button(mo, 50, 125, "Load indexed PNG", newSWFAction(indexpng_action));

    sprintf(greypng_action, "_root.window.loadMovie(\"%s\");", url_greypng);
    add_button(mo, 50, 150, "Load greyscale PNG", newSWFAction(greypng_action));

    sprintf(gif_action, "_root.window.loadMovie(\"%s\");", url_gif);
    add_button(mo, 50, 175, "Load GIF", newSWFAction(gif_action));

    sprintf(igif_action, "_root.window.loadMovie(\"%s\");", url_igif);
    add_button(mo, 50, 200, "Load interlaced GIF", newSWFAction(igif_action));


	/*****************************************************
	 *
	 * Add the window clip
	 *
	 *****************************************************/

	puts("Adding window");

	add_window(mo, 600, 100);

	add_actions(mo, "stop();");
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
