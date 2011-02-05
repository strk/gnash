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
 * Test MovieClip.attachMovie().
 *
 * Exports a 'redsquare' symbol and then attach it to main timeline 4 times
 * at depths 70+[0..3] and with xoffset 70*[0..3]
 *
 * run as ./attachMovieTest
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "DepthLimitsTest.swf"

void addRedSquareExport(SWFMovie mo);

void
addRedSquareExport(SWFMovie mo)
{
	SWFShape sh;
	SWFMovieClip mc;

	sh = make_fill_square (0, 0, 60, 60, 255, 0, 0, 255, 0, 0);
	mc = newSWFMovieClip();

	SWFMovieClip_add(mc, (SWFBlock)sh);
	/* This is here just to turn the clip into an active one */
	add_clip_actions(mc, "onRollOver = function() {};");
	add_clip_actions(mc, "onMouseDown = function() { _root.mouseDown++; _root.note(_name+' mouseDown '+_root.mouseDown); };");
	add_clip_actions(mc, "onMouseUp = function() { _root.mouseUp++; _root.note(_name+' mouseUp '+_root.mouseUp); };");
	SWFMovieClip_nextFrame(mc);

	SWFMovie_addExport(mo, (SWFBlock)mc, "redsquare");

	SWFMovie_writeExports(mo);
}

int
main(int argc, char** argv)
{
	SWFMovie mo;
	const char *srcdir=".";
	SWFMovieClip dejagnuclip;


	/*********************************************
	 *
	 * Initialization
	 *
	 *********************************************/

	if ( argc>1 ) srcdir=argv[1];
	else
	{
		fprintf(stderr, "Usage: %s\n", argv[0]);
		return 1;
	}

	puts("Setting things up");

	Ming_init();
    Ming_useSWFVersion (OUTPUT_VERSION);
	Ming_setScale(20.0); /* let's talk pixels */
 
	mo = newSWFMovie();
	SWFMovie_setRate(mo, 12);
	//SWFMovie_setDimension(mo, 6400, 4000);
	SWFMovie_setDimension(mo, 640, 400);

	/*********************************************
	 *
	 * Body
	 *
	 *********************************************/

	dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 80, 800, 600);
	SWFMovie_add(mo, (SWFBlock)dejagnuclip);

	addRedSquareExport(mo);
	/* it seems we need a SHOWFRAME for this to be effective */
	/* (maybe it's related to loop-back handling ?) */
	SWFMovie_nextFrame(mo); 

	/* A load of tests for depth */
	
	add_actions(mo, "attachMovie('redsquare', 'depthtest', -16, initObj);"
	                "d = depthtest.getDepth();");
	check_equals(mo, "d", "-16");


	add_actions(mo, "attachMovie('redsquare', 'depthtest', -16384, initObj);"
	                "d = depthtest.getDepth();");
	check_equals(mo, "d", "-16384");

    /* Less than -16384 fails */
	add_actions(mo, "attachMovie('redsquare', 'depthtest2', -20000, initObj);"
	                "d = depthtest2.getDepth();");
	check_equals(mo, "d", "undefined");

    /* It really does */
	add_actions(mo, "attachMovie('redsquare', 'depthtest2', -16385, initObj);"
	                "d = depthtest2.getDepth();");
	check_equals(mo, "d", "undefined");

    /* Up to 2130690044 works */
	add_actions(mo, "attachMovie('redsquare', 'depthtest2', 1147483648, initObj);"
	                "d = depthtest2.getDepth();");
	check_equals(mo, "d", "1147483648");

    /* Up to 2130690044 works */
	add_actions(mo, "attachMovie('redsquare', 'depthtest3', 2130690044, initObj);"
	                "d = depthtest3.getDepth();");
	check_equals(mo, "d", "2130690044");

    /* 2130690045 doesn't work */
	add_actions(mo, "attachMovie('redsquare', 'depthtest4', 2130690045, initObj);"
	                "d = depthtest4.getDepth();");
	check_equals(mo, "d", "undefined");

    /* duplicateMovieClip */
    /* Same limits...     */

    add_actions(mo, "createEmptyMovieClip('original', 10);");

	add_actions(mo, "duplicateMovieClip('original', 'dup1', -1);"
	                "d = dup1.getDepth();");
	check_equals(mo, "d", "-1");

	add_actions(mo, "original.duplicateMovieClip('odup1', -1);"
	                "d = odup1.getDepth();");
	check_equals(mo, "d", "-1");

	add_actions(mo, "duplicateMovieClip('original', 'dup2', -16384);"
	                "d = dup2.getDepth();");
	check_equals(mo, "d", "-16384");

	add_actions(mo, "original.duplicateMovieClip('odup2', -16384);"
	                "d = odup2.getDepth();");
	check_equals(mo, "d", "-16384");

	add_actions(mo, "duplicateMovieClip('original', 'dup3', -16385);"
	                "d = dup3.getDepth();");
	check_equals(mo, "d", "undefined");

	add_actions(mo, "original.duplicateMovieClip('odup3', -16385);"
	                "d = odup3.getDepth();");
	check_equals(mo, "d", "undefined");

	add_actions(mo, "duplicateMovieClip('original', 'dup4', 2130690044);"
	                "d = dup4.getDepth();");
	check_equals(mo, "d", "2130690044");

	add_actions(mo, "original.duplicateMovieClip('odup4', 2130690044);"
	                "d = odup4.getDepth();");
	check_equals(mo, "d", "2130690044");

	add_actions(mo, "duplicateMovieClip('original', 'dup5', 2130690045);"
	                "d = dup5.getDepth();");
	check_equals(mo, "d", "undefined");

	add_actions(mo, "original.duplicateMovieClip('odup5', 2130690045);"
	                "d = odup5.getDepth();");
	check_equals(mo, "d", "undefined");
    
	add_actions(mo, "totals(); stop();");

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
