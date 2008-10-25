/* 
 *   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>
#include <errno.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 8
#define OUTPUT_FILENAME "LoadVarsTest.swf"

int
main(int argc, char** argv)
{
	SWFMovie mo;
	const char *srcdir=".";
	SWFMovieClip  dejagnuclip;

	char loadvars[1048];

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


	sprintf(loadvars, "e = l.load('%svars.txt');", srcdir);
	puts("Setting things up");

	Ming_init();
	Ming_useSWFVersion (OUTPUT_VERSION);
	Ming_setScale(20.0); /* let's talk pixels */
 
	mo = newSWFMovie();
	SWFMovie_setRate(mo, 12);
	SWFMovie_setDimension(mo, 640, 400);

	/*********************************************
	 *
	 * Body
	 *
	 *********************************************/

	dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir),
			10, 0, 80, 800, 600);
	SWFMovie_add(mo, (SWFBlock)dejagnuclip);

	/// Construct LoadVars and backup methods.
	add_actions(mo, "l = new LoadVars;"
			"odatB = l.onData;"
			"olB = l.onLoad;"
			"odecB = l.decode;"
			"loadString = '';"
			"decodeString = '';"
			"dataString = '';");

	add_actions(mo, "ourLoad = function(arg) {"
			"loadString += 'onLoad called with ' + typeof(arg)" 
			"+ ' argument ' + arg;"
			"play();"
			"};");

	add_actions(mo, "ourData = function(arg) {"
			"dataString += 'onData called with ' + typeof(arg) "
			"+ ' argument ' + arg;"
			"play();"
			"};");

	
	add_actions(mo, "decodeCalled = 0;");
	add_actions(mo, "ourDecode = function(arg) {"
			"decodeString += 'onDecode called with ' + typeof(arg)"
			"+ ' argument ' + arg;"
			"decodeCalled++;"
			"};");

	/// The onDecode method is stays overwritten to see where it gets
	/// called from. We don't call it ourselves. Don't forget to 
	/// overwrite it again when the LoadVars object is construct again.
	add_actions(mo, "l.onDecode = ourDecode;");

	/// What happens when load fails?
	//
	/// Both onData and onLoad are called *independently*. That is,
	/// neither is called from the default implementation of the other.
	
	SWFMovie_nextFrame(mo);

	/// onData
	add_actions(mo, "l.onData = ourData;"
			"dataString = '';"
			"e = l.load('fail');");
	check_equals(mo, "e", "true");
	add_actions(mo, "stop();");

	/// Wait for data before proceeding to next frame.
	SWFMovie_nextFrame(mo);
	
	// Check result, restore builtin method.
	xcheck_equals(mo, "dataString",
			"'onData called with undefined argument undefined'");
	add_actions(mo, "l.onData = odatB;");

	/// onLoad
	add_actions(mo, "l.onLoad = ourLoad;"
			"loadString = '';"
			"e = l.load('fail');");
	check_equals(mo, "e", "true");
	add_actions(mo, "stop();");

	SWFMovie_nextFrame(mo);
		
	xcheck_equals(mo, "loadString",
			"'onLoad called with boolean argument false'");
	add_actions(mo, "l.onLoad = olB;");

	/// What happens when load succeeds?
	//
	/// Both methods are called separately.

	// onData
	add_actions(mo, "l = new LoadVars; l.onData = ourData;"
			"dataString = '';");
	add_actions(mo, loadvars);
	check_equals(mo, "e", "true");
	add_actions(mo, "stop();");

	SWFMovie_nextFrame(mo);
	xcheck_equals(mo, "dataString",
			"'onData called with undefined argument undefined'");
	add_actions(mo, "l.data = odatB;");

	// onLoad
	add_actions(mo, "l = new LoadVars; l.onLoad = ourLoad;"
			"loadString = '';");
	add_actions(mo, loadvars);
	check_equals(mo, "e", "true");
	add_actions(mo, "stop();");

	SWFMovie_nextFrame(mo);
	xcheck_equals(mo, "loadString",
			"'onLoad called with boolean argument false'");
	add_actions(mo, "l.load = olB;");

	/// What happens when we try loading into a LoadVars that has already
	/// loaded?
	//
	/// No call to onData or onLoad. It's hard to test this, as we
	/// don't know when to advance to the next frame, so don't wait on
	/// this test. That means you can't be sure the results aren't
	/// due to timing issues.

	// onLoad
	add_actions(mo, "l.onLoad = ourLoad;"
			"loadString = '';");
	add_actions(mo, loadvars);
	check_equals(mo, "e", "undefined");

	SWFMovie_nextFrame(mo);
	check_equals(mo, "loadString",
			"''");
	add_actions(mo, "l.load = olB;");

	/// onData
	add_actions(mo, "l.onData = ourData;"
			"dataString = '';");
	add_actions(mo, loadvars);
	check_equals(mo, "e", "undefined");

	SWFMovie_nextFrame(mo);
	check_equals(mo, "dataString",
			"''");
	add_actions(mo, "l.load = odatB;");

	check_equals(mo, "decodeCalled", "0");

	/// End of tests.

	add_actions(mo, "totals();");
	add_actions(mo, "stop();");

	/*****************************************************
	 *
	 * Output movie
	 *
	 *****************************************************/

	puts("Saving " OUTPUT_FILENAME );

	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}

