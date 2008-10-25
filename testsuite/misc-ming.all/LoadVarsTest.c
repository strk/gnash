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


	sprintf(loadvars, "e = l.load('%s/vars2.txt');", srcdir);
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
			"odatB = LoadVars.prototype.onData;"
			"olB = LoadVars.prototype.onLoad;"
			"odecB = LoadVars.prototype.decode;"
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
			"decodeString += 'decode called with ' + typeof(arg)"
			"+ ' argument ' + arg;"
			"decodeCalled++;"
			"};");

	/// The decode method is stays overwritten to see where it gets
	/// called from. We don't call it ourselves. Don't forget to 
	/// overwrite it again when the LoadVars object is construct again.
	add_actions(mo, "l.decode = ourDecode;");


	/// What happens when load fails?
	//
	/// The onLoad method is called from the default implementation of onData.
	
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

    /// Both onData and onLoad
	add_actions(mo, "l.onLoad = ourLoad;"
            "l.onData = ourData;"
			"loadString = '';"
            "dataString = '';"
			"e = l.load('fail');");
	check_equals(mo, "e", "true");
	add_actions(mo, "stop();");

	SWFMovie_nextFrame(mo);
		
	check_equals(mo, "loadString",
			"''");
	xcheck_equals(mo, "dataString",
			"'onData called with undefined argument undefined'");
	add_actions(mo, "l.onLoad = olB;");
    add_actions(mo, "l.onData = odatB;");

	/// What happens when load succeeds?
	//
	/// Both methods are called separately.

	// onData
	add_actions(mo, "l = new LoadVars; l.onData = ourData;"
            "l.decode = ourDecode;"
			"dataString = '';");
	add_actions(mo, loadvars);
	check_equals(mo, "e", "true");
	add_actions(mo, "stop();");

	SWFMovie_nextFrame(mo);
    // check_equals is too braindead to do this without escaping.
	xcheck_equals(mo, "escape(dataString)",
			"'onData%20called%20with%20string%20argument%20v2%5Fvar1%3D"
            "val1%26v2%5Fvar2%3Dval2%26%0A'");
	add_actions(mo, "l.onData = odatB;");

    check_equals(mo, "decodeCalled", "0");
    check_equals(mo, "decodeString", "''");

	// onLoad
	add_actions(mo, "l = new LoadVars; l.onLoad = ourLoad;"
            "l.decode = ourDecode;"
			"loadString = '';");
	add_actions(mo, loadvars);
	check_equals(mo, "e", "true");
	add_actions(mo, "stop();");

	SWFMovie_nextFrame(mo);
	check_equals(mo, "loadString",
			"'onLoad called with boolean argument true'");
	add_actions(mo, "l.onLoad = olB;");

    /// decode is called from onData (i.e. it's called when we overwrite
    /// onLoad, not onData).
    xcheck_equals(mo, "decodeCalled", "1");
    // check_equals is too braindead to do this without escaping.
	xcheck_equals(mo, "escape(decodeString)",
			"'decode%20called%20with%20string%20argument%20v2%5Fvar1%3D"
            "val1%26v2%5Fvar2%3Dval2%26%0A'");

	/// End of tests.

	add_actions(mo, "totals(15);");
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

