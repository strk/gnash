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
 *
 */ 

/*
 * Test DefineEditText tag with VariableName
 *
 * Uses "embedded" font with chars: "Hello world"
 *
 * Then, every second it toggles the text between "Hello"
 * and "World" by setting the VariableName.
 * 
 * After every variable set it also traces the value of the
 * VariableName, of the textfield and of it's 'text' member.
 * Note that the traces are both "visual" and normal ("visual"
 * traces use the drawing API).
 *
 * The EditText character is stored inside a MovieClip, and
 * it's variable is set on the root. Note that the ActionScript
 * code also tries to *move* the character trough the variable
 * (incdement varname._x).
 * The correct behaviour is for the character to NOT move
 *
 *
 * run as ./DefineEditTextVariableNameTest
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "DefineEditTextVariableNameTest.swf"

void add_text_field(SWFMovieClip mo, SWFBlock font, const char* varname, const char* text);


void
add_text_field(SWFMovieClip mo, SWFBlock font, const char* varname,
		const char* text)
{
	SWFTextField tf;
	SWFDisplayItem it;

	tf = newSWFTextField();

	SWFTextField_setFlags(tf, SWFTEXTFIELD_DRAWBOX);


	SWFTextField_setFont(tf, font);
	SWFTextField_addChars(tf, text);
	SWFTextField_addString(tf, text);

	/* Give the textField a variablename*/
	SWFTextField_setVariableName(tf, varname);

	it = SWFMovieClip_add(mo, (SWFBlock)tf);
	SWFDisplayItem_setName(it, "textfield");
}

int
main(int argc, char** argv)
{
	SWFMovie mo;
	SWFMovieClip mc;
	SWFDisplayItem it;
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
	SWFMovie_setRate(mo, 1);
	SWFMovie_setDimension(mo, 12560, 9020);

	/*********************************************
	 *
	 * Add a new MovieClip
	 *
	 *********************************************/

	mc = newSWFMovieClip();

	/*********************************************
	 *
	 * Add the textfield
	 *
	 *********************************************/

	/* 
	 * The variable name
	 */
	char* varName = "_root.testName";
	char buf[256];

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
		add_text_field(mc, (SWFBlock)bfont, varName, "Hello World");
		SWFMovieClip_nextFrame(mc);
	}

	it = SWFMovie_add(mo, (SWFBlock)mc);
	SWFDisplayItem_setName(it, "mc1");

	/*********************************************
	 *
	 * Add xtrace code
	 *
	 *********************************************/

	add_xtrace_function(mo, 3000, 0, 50, 400, 800);

	/*********************************************
	 *
	 * Access the value of the variablename
	 *
	 *********************************************/

	{
		SWFAction ac;
		sprintf(buf, "%s = \"Hello\"; "
			"xtrace(\"%s: \"+%s+\"\n"
			"mc1._width: \"+mc1._width+\"\n"
			"mc1._height: \"+mc1._height+\"\n"
			"_root._width: \"+_root._width+\"\n"
			"_root._height: \"+_root._height+\"\n"
			"mc1.textfield: \"+mc1.textfield+\"\n"
			"mc1.textfield.text: \"+mc1.textfield.text+\"\n"
			"mc1.textfield._width: \"+mc1.textfield._width+\"\n"
			"mc1.textfield._height: \"+mc1.textfield._height); ",
			varName, varName, varName);
		ac = compileSWFActionCode(buf);
		SWFMovie_add(mo, (SWFBlock)ac);
		SWFMovie_nextFrame(mo); 
	}

	{
		SWFAction ac;
		sprintf(buf, "%s = \"World\"; "
			"xtrace(\"%s: \"+%s+\"\n"
			"mc1._width: \"+mc1._width+\"\n"
			"mc1._height: \"+mc1._height+\"\n"
			"_root._width: \"+_root._width+\"\n"
			"_root._height: \"+_root._height+\"\n"
			"mc1.textfield: \"+mc1.textfield+\"\n"
			"mc1.textfield.text: \"+mc1.textfield.text+\"\n"
			"mc1.textfield._width: \"+mc1.textfield._width+\"\n"
			"mc1.textfield._height: \"+mc1.textfield._height); ",
			varName, varName, varName);
		ac = compileSWFActionCode(buf);
		SWFMovie_add(mo, (SWFBlock)ac);
		SWFMovie_nextFrame(mo); /* showFrame */
	}

	{ // set text to the empty string
		SWFAction ac;
		sprintf(buf, "%s = \"\"; "
			"xtrace(\"%s: \"+%s+\"\n"
			"mc1._width: \"+mc1._width+\"\n"
			"mc1._height: \"+mc1._height+\"\n"
			"_root._width: \"+_root._width+\"\n"
			"_root._height: \"+_root._height+\"\n"
			"mc1.textfield: \"+mc1.textfield+\"\n"
			"mc1.textfield.text: \"+mc1.textfield.text+\"\n"
			"mc1.textfield._width: \"+mc1.textfield._width+\"\n"
			"mc1.textfield._height: \"+mc1.textfield._height); ",
			varName, varName, varName);
		ac = compileSWFActionCode(buf);
		SWFMovie_add(mo, (SWFBlock)ac);
		SWFMovie_nextFrame(mo); /* showFrame */
	}

	{
		// testName (the variable) doesn't access the character,
		// only its text.
		SWFAction ac;
		sprintf(buf, "%s._x += 10;", varName);
		ac = compileSWFActionCode(buf);
		SWFMovie_add(mo, (SWFBlock)ac);
		SWFMovie_nextFrame(mo); /* showFrame */
	}

	/*****************************************************
	 *
	 * Output movie
	 *
	 *****************************************************/

	puts("Saving " OUTPUT_FILENAME );

	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
