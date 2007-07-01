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
 * Test DefineEditText tag with VariableName
 *
 * Uses "embedded" font in two textfields with initial
 * texts  "Hello world" and "Hi there".
 *
 * Then, every second it toggles the text between "Hello",
 * "World" and "" (the empty string) in the first textfield
 * and between "Hi", "There" and "" (the empty string) in
 * the second textfield.
 *
 * The text is set assigning a value to the associated
 * VariableName and verified by getting the TextField's 
 * 'text' member.
 * 
 * Verifications use the Dejagnu intterface, so you'll see
 * visual traces of failures and a final visual report
 * or successes and failures.
 *
 * The first TextField characters is stored inside a MovieClip,
 * and its variables is set on the root. 
 *
 * The second TextField characters is stored inside a MovieClip,
 * and its variables is set on a third MovieClip which is placed
 * *after* the definition of the TextField. Things should still work.
 *
 * Note that the ActionScript code also tries to *move* the character trough
 * the VariableName (incdement varname._x).
 * The correct behaviour is for the character to NOT move
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
void set_text(SWFMovie mo, const char* txt, const char* varname);
void shift_horizontally(SWFMovie mo, const char* what, int howmuch);
void set_x(SWFMovie mo, const char* what, int x);

void
shift_horizontally(SWFMovie mo, const char* what, int howmuch)
{ 
	static const size_t BUFLEN = 256;

	char buf[BUFLEN];
	SWFAction ac;

	snprintf(buf, BUFLEN, "%s._x += %d;", what, howmuch);
	buf[BUFLEN-1] = '\0';

	ac = compileSWFActionCode(buf);
	SWFMovie_add(mo, (SWFBlock)ac);
}

void
set_x(SWFMovie mo, const char* what, int x)
{ 
	static const size_t BUFLEN = 256;

	char buf[BUFLEN];
	SWFAction ac;

	snprintf(buf, BUFLEN, "%s._x = %d;", what, x);
	buf[BUFLEN-1] = '\0';

	ac = compileSWFActionCode(buf);
	SWFMovie_add(mo, (SWFBlock)ac);
}


void
set_text(SWFMovie mo, const char* txt, const char* varname)
{
	static const size_t BUFLEN = 512;

	char buf[BUFLEN];
	SWFAction ac;
	snprintf(buf, BUFLEN, "%s = \"%s\"; ",
		varname, txt);
	buf[BUFLEN-1] = '\0';
	ac = compileSWFActionCode(buf);
	SWFMovie_add(mo, (SWFBlock)ac);
}

// @param txt: make of textfield
// @param varname: variable name to set
void
setVariableName(SWFMovie mo, const char* txt, const char* varname)
{
	static const size_t BUFLEN = 512;

	char buf[BUFLEN];
	SWFAction ac;
	snprintf(buf, BUFLEN, "%s.variable = \"%s\"; ", txt, varname);
	buf[BUFLEN-1] = '\0';
	ac = compileSWFActionCode(buf);
	SWFMovie_add(mo, (SWFBlock)ac);
}

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

	SWFMovieClip_nextFrame(mo);

}

static void
testVariableNameGetSet(SWFMovie mo, const char *varName1, const char *varName2)
{
	char tmp[1024];

	set_text(mo, "Hello", varName1);
	shift_horizontally(mo, varName1, 10);
	check_equals(mo, "mc1.textfield.text", "'Hello'");
	check_equals(mo, varName1, "'Hello'");
	check_equals(mo, "mc1.textfield._x", "0");
	check_equals(mo, "mc1._height", "16");
	check_equals(mo, "mc1._width", "136");
	check_equals(mo, "mc2._height", "16");
	check_equals(mo, "mc2._width", "100");

	set_text(mo, "Hi", varName2);
	shift_horizontally(mo, varName2, 10);
	check_equals(mo, "mc2.textfield.text", "'Hi'"); 
	check_equals(mo, varName2, "'Hi'");
	check_equals(mo, "mc2.textfield._x", "150");
	snprintf(tmp, 1024, "'%s'", varName2);
	check_equals(mo, "mc2.textfield.variable", tmp);

	SWFMovie_nextFrame(mo); /* showFrame */

	set_text(mo, "World", varName1);
	shift_horizontally(mo, varName1, 10);
	check_equals(mo, "mc1.textfield.text", "'World'");
	check_equals(mo, varName1, "'World'");
	check_equals(mo, "mc1.textfield._x", "0");

	set_text(mo, "There", varName2);
	shift_horizontally(mo, varName2, 10);
	check_equals(mo, "mc2.textfield.text", "'There'");
	check_equals(mo, varName2, "'There'");
	check_equals(mo, "mc2.textfield._x", "150");

	SWFMovie_nextFrame(mo); /* showFrame */

	set_text(mo, "", varName1);
	shift_horizontally(mo, varName1, 10);
	check_equals(mo, "mc1.textfield.text", "''");
	check_equals(mo, varName1, "''");
	check_equals(mo, "mc1.textfield._x", "0");

	set_text(mo, "", varName2);
	shift_horizontally(mo, varName2, 10);
	check_equals(mo, "mc2.textfield.text", "''");
	check_equals(mo, varName2, "''");
	check_equals(mo, "mc2.textfield._x", "150");

	SWFMovie_nextFrame(mo); /* showFrame */
}

int
main(int argc, char** argv)
{
	SWFMovie mo;
	SWFMovieClip mc1, mc2, mc3;
	const char *srcdir=".";
	/* The variable name for textfield */
	char* varName1 = "_root.testName";
	char* varName2 = "_root.mc3.testName";
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
	SWFMovie_setDimension(mo, 628, 451);

	bfont = get_default_font(srcdir);

	/***************************************************
	 *
	 * Add MovieClips mc1 and mc2 with their textfield
	 *
	 ***************************************************/

	{
		SWFDisplayItem it;
		mc1 = newSWFMovieClip();
		add_text_field(mc1, (SWFBlock)bfont, varName1, "Hello World");
		it = SWFMovie_add(mo, (SWFBlock)mc1);
		SWFDisplayItem_setName(it, "mc1");
		set_x(mo, "mc1.textfield", 0);
	}

	{
		SWFDisplayItem it;
		mc2 = newSWFMovieClip();
		add_text_field(mc2, (SWFBlock)bfont, varName2, "Hi There");
		it = SWFMovie_add(mo, (SWFBlock)mc2);
		SWFDisplayItem_setName(it, "mc2");
		set_x(mo, "mc2.textfield", 150);
	}

	/*********************************************
	 *
	 * Now add mc3, which is referenced by
	 * mc2.textfield variableName (after it's placement)
	 *
	 *********************************************/

	{
		SWFDisplayItem it;
		mc3 = newSWFMovieClip();
		it = SWFMovie_add(mo, (SWFBlock)mc3);
		SWFDisplayItem_setName(it, "mc3");
		/*SWFMovie_nextFrame(mo);*/
	}



	/*********************************************
	 *
	 * Add xtrace code
	 *
	 *********************************************/

	/*add_xtrace_function(mo, 3000, 0, 50, 400, 800);*/
	add_dejagnu_functions(mo, (SWFBlock)bfont, 3000, 0, 50, 400, 800);

	/*********************************************
	 *
	 * Set and get value of the textfields using
	 * their variableName
	 *
	 *********************************************/

	testVariableNameGetSet(mo, varName1, varName2);

	/*********************************************
	 *
	 * Now change the textfields 'variable' property
	 * and test again
	 *
	 *********************************************/

	{
		const char* varName1bis = "_root.varName1bis";
		const char* varName2bis = "_root.varName2bis";
		setVariableName(mo, "mc1.textfield", varName1bis);
		setVariableName(mo, "mc2.textfield", varName2bis);
		testVariableNameGetSet(mo, varName1bis, varName2bis);
	}

	/*********************************************
	 *
	 * Print test results
	 *
	 *********************************************/

	SWFMovie_nextFrame(mo); /* showFrame */

	print_tests_summary(mo);

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
