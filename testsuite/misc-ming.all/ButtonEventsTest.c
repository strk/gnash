/***********************************************************************
 *
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 ***********************************************************************
 *
 * Test case for mouse events.
 *
 * In a movie of 120x120 pixels, it places a movieclip containing a squared
 * button in the middle of the stage, and a text area on top.
 *
 * The movie has 4 frames, with the second adding a shape at a lower depth,
 * the third one at an higher depth, and fourth disabling the button.
 *
 * The following events print the event name in the text area
 * (called _root.textfield) and change the color of the button:
 *
 * MouseOut  : red button (initial state)
 * MouseOver : yellow button
 * MouseDown : green button
 * MouseUp   : yellow button (same as MouseOver, but the label on top changes)
 *
 * Tests are triggered by events, in particular:
 * - Test for _target and _name referring to button's parent.
 * - Test for bounds of buttons being the union of all active state
 *   characters' bounds.
 *
 * Note that you need to play with your mouse on the button for the tests
 * to be run, and that there's currently no END OF TEST condition.
 * For gnash test automation, we use the ButtonEventsTest-Runner script
 * that supposedly triggers all tests (still worth making the test
 * more explicitly guided, also to provide an end-of-test flags for
 * consistency checking).
 *
 * TODO:
 *  - Turn the test into a guided interaction, like the DragDropTest.swf one..
 *  - Add tests for invalidated bounds
 *  - Add matrix transformation to some child to also test that.
 *
 ***********************************************************************/

#include "ming_utils.h"

#include <ming.h>
#include <stdio.h>
#include <stdlib.h>

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "ButtonEventsTest.swf"

SWFFont font;

void add_event(SWFMovie mo, const char* name, const char* event, const char* action);
void add_code(SWFMovie mo, const char* code);
void add_text_field(SWFMovie mo, const char* name, const char* varname, const char* initial_label, int depth, int x, int y);
void set_text(SWFMovie mo, const char* text);
SWFDisplayItem add_square(SWFMovie mo, byte r, byte g, byte b, int depth);

void
add_event(SWFMovie mo, const char* name, const char* event, const char* action)
{
	SWFAction ac;
	char buf[1024];

	sprintf(buf,
	"event=undefined;"
	"%s.on%s=function() { %s; };"
	, name, event, action
	);
	ac = compileSWFActionCode(buf);

	SWFMovie_add(mo, (SWFBlock)ac);
}

void
add_code(SWFMovie mo, const char* code)
{
	SWFAction ac;

	ac = compileSWFActionCode(code);

	SWFMovie_add(mo, (SWFBlock)ac);
}

SWFDisplayItem add_button(SWFMovie mo);
SWFDisplayItem
add_button(SWFMovie mo)
{
	SWFDisplayItem it;
	SWFMovieClip mc;
	SWFButtonRecord br;
	SWFShape sh1, sh2, sh3, sh4, sh1a, sh2a, sh3a, sh4a;
	SWFButton bu = newSWFButton();
	mc = newSWFMovieClip();

	sh1 = make_fill_square(0, 0, 40, 40, 0, 0, 0, 0, 0, 0);
	sh1a = make_fill_square(30, 30, 5, 5, 128, 128, 128, 128, 128, 128);
	sh2 = make_fill_square(0, 0, 40, 40, 255, 0, 0, 255, 0, 0);
	sh2a = make_fill_square(30, 30, 5, 5, 128, 0, 0, 128, 0, 0);
	sh3 = make_fill_square(0, 0, 40, 40, 0, 255, 0, 0, 255, 0);
	sh3a = make_fill_square(30, 30, 5, 5, 0, 128, 0, 0, 128, 0);
	sh4 = make_fill_square(0, 0, 40, 40, 255, 255, 0, 255, 255, 0);
	sh4a = make_fill_square(30, 30, 5, 5, 128, 128, 0, 128, 128, 0);

	/* Higher depth character is intentionally added before lower depth one */
	br = SWFButton_addCharacter(bu, (SWFCharacter)sh1a, SWFBUTTON_HIT);
	SWFButtonRecord_setDepth(br, 2);
	br = SWFButton_addCharacter(bu, (SWFCharacter)sh1, SWFBUTTON_HIT);
	SWFButtonRecord_setDepth(br, 1);

	/* Higher depth character is intentionally added before lower depth one */
	br = SWFButton_addCharacter(bu, (SWFCharacter)sh2a, SWFBUTTON_UP );
	SWFButtonRecord_setDepth(br, 2);
	br = SWFButton_addCharacter(bu, (SWFCharacter)sh2, SWFBUTTON_UP );
	SWFButtonRecord_setDepth(br, 1);

	/* Higher depth character is intentionally added before lower depth one */
	br = SWFButton_addCharacter(bu, (SWFCharacter)sh3a, SWFBUTTON_DOWN );
	SWFButtonRecord_setDepth(br, 2);
	br = SWFButton_addCharacter(bu, (SWFCharacter)sh3, SWFBUTTON_DOWN );
	SWFButtonRecord_setDepth(br, 1);

	/* Higher depth character is intentionally added before lower depth one */
	br = SWFButton_addCharacter(bu, (SWFCharacter)sh4a, SWFBUTTON_OVER );
	SWFButtonRecord_setDepth(br, 2);
	br = SWFButton_addCharacter(bu, (SWFCharacter)sh4, SWFBUTTON_OVER );
	SWFButtonRecord_setDepth(br, 1);

	SWFButton_addAction(bu, compileSWFActionCode(
		"_root.msg='MouseOut';"
		"_root.note('SWFBUTTON_MOUSEOUT');"
		"_root.check_equals(_root.printBounds(getBounds()), '-0.05,-0.05 40.05,40.05');"
		// Target of button action is the button's parent sprite
		"_root.check_equals(_target, '/square1');"
		"setTarget('/');"
		"_root.check_equals(_target, '/');"
		), SWFBUTTON_MOUSEOUT);

	SWFButton_addAction(bu, compileSWFActionCode(
		"_root.msg='MouseOver';"
		"_root.note('SWFBUTTON_MOUSEOVER');"
		"_root.check_equals(_root.printBounds(getBounds()), '-0.05,-0.05 40.05,40.05');"
		// Target of button action is the button's parent sprite
		"_root.check_equals(_target, '/square1');"
		"setTarget('/');"
		"_root.check_equals(_target, '/');"
		), SWFBUTTON_MOUSEOVER);

	SWFButton_addAction(bu, compileSWFActionCode(
		"_root.msg='MouseDown';"
		"_root.note('SWFBUTTON_MOUSEDOWN');"
		"_root.check_equals(_root.printBounds(getBounds()), '-0.05,-0.05 40.05,40.05');"
		/* Target (and name) of button action is the button's parent sprite */
		"_root.check_equals(_target, '/square1');"
		"_root.check_equals(_name, 'square1');"
		"setTarget('/');"
		"_root.check_equals(_target, '/');"
		"_root.check_equals(typeof(_name), 'string');"
		"_root.check_equals(_name, '');"
		), SWFBUTTON_MOUSEDOWN);

	SWFButton_addAction(bu, compileSWFActionCode(
		"_root.msg='MouseUp';"
		"_root.note('SWFBUTTON_MOUSEUP');"
		"_root.check_equals(_root.printBounds(getBounds()), '-0.05,-0.05 40.05,40.05');"
		/* Target of button action is the button's parent sprite */
		"_root.check_equals(_target, '/square1');"
		"setTarget('/');"
		"_root.check_equals(_target, '/');"
		), SWFBUTTON_MOUSEUP);

	/* SWFBUTTON_MOUSEUPOUTSIDE *should* be invoked !! */
	SWFButton_addAction(bu, compileSWFActionCode(
		"_root.msg='MouseUpOutside';"
		"_root.note('SWFBUTTON_MOUSEUPOUTSIDE');"
		"_root.check_equals(_root.printBounds(getBounds()), '-0.05,-0.05 40.05,40.05');"
		/* Target of button action is the button's parent sprite */
		"_root.check_equals(_target, '/square1');"
		"_root.check_equals(_name, 'square1');"
		"setTarget('/');"
		"_root.check_equals(_target, '/');"
		), SWFBUTTON_MOUSEUPOUTSIDE);

	it = SWFMovieClip_add(mc, (SWFBlock)bu);
	SWFDisplayItem_setName(it, "button");
	SWFMovieClip_nextFrame(mc); /* showFrame */

	it = SWFMovie_add(mo, (SWFBlock)mc);
	return it;
}

void
add_text_field(SWFMovie mo, const char* name, const char* varname, const char* initial_label, int depth, int x, int y)
{
	SWFDisplayItem it;
	SWFTextField tf = newSWFTextField();
	SWFTextField_setFont(tf, (void*)font);
	SWFTextField_addChars(tf, " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ012345689:.,/\\#@?!");
	SWFTextField_setVariableName(tf, varname);
	SWFTextField_addString(tf, "Idle");
	SWFTextField_setBounds(tf, 120, 12);
	SWFTextField_setFlags(tf, SWFTEXTFIELD_DRAWBOX|SWFTEXTFIELD_NOEDIT);

	it = SWFMovie_add(mo, (SWFBlock)tf);
	SWFDisplayItem_moveTo(it, x, y+2);
	SWFDisplayItem_setDepth(it, depth);
	SWFDisplayItem_setName(it, name); // "textfield");

	// Label 
	tf = newSWFTextField();
	SWFTextField_setFont(tf, (void*)font);
	SWFTextField_addString(tf, initial_label);
	SWFTextField_setFlags(tf, SWFTEXTFIELD_DRAWBOX|SWFTEXTFIELD_NOEDIT);
	it = SWFMovie_add(mo, (SWFBlock)tf);
	SWFDisplayItem_scale(it, 0.3, 0.3);
	SWFDisplayItem_setDepth(it, depth*10);
	SWFDisplayItem_moveTo(it, x, y);
}

void
set_text(SWFMovie mo, const char* text)
{
	char buf[1024];
	sprintf(buf, "_root.msg=\"%s\";", text);
	add_code(mo, buf);
}

int
main(int argc, char **argv)
{
	SWFMovie mo;
	SWFDisplayItem it;
	const char *srcdir=".";
	SWFMovieClip dejagnuclip;

	/*********************************************
	 *
	 * Initialization
	 *
	 *********************************************/

	puts("Setting things up");

	Ming_init();
        Ming_useSWFVersion (OUTPUT_VERSION);
	Ming_setScale(20.0); 
 
	mo = newSWFMovie();
	SWFMovie_setDimension(mo, 800, 600);
	SWFMovie_setRate(mo, 0.2);

	if ( argc>1 ) srcdir=argv[1];
	else
	{
		fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
		return 1;
	}

	font = get_default_font(srcdir); 

	/* Dejagnu equipment */
	dejagnuclip = get_dejagnu_clip((SWFBlock)font, 10, 0, 0, 800, 600);
	it = SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  	SWFDisplayItem_setDepth(it, 200); 
  	SWFDisplayItem_move(it, 200, 0); 

	add_text_field(mo, "textfield", "_root.msg", "Button events", 10, 0, 5);
	add_text_field(mo, "textfield2", "_root.msg2", "Mouse events", 11, 0, 100);
	add_text_field(mo, "textfield3", "_root.msg3", "Key events", 12, 0, 80);

	/*****************************************************
	 *
	 * Add button
	 *
	 *****************************************************/

	it = add_button(mo);
	SWFDisplayItem_moveTo(it, 40, 30);
	SWFDisplayItem_setName(it, "square1");
	SWFDisplayItem_setDepth(it, 2);

	add_actions(mo,
		"function printBounds(b) {"
		"   return ''+Math.round(b.xMin*100)/100+','+Math.round(b.yMin*100)/100+' '+Math.round(b.xMax*100)/100+','+Math.round(b.yMax*100)/100;"
		"}"
	);

	//
	// Mouse pointer events
	//

	add_actions(mo,
		"square1.button.onRollOver = function() { "
		"	_root.msg2 = 'RollOver'; "
		// Target is the one this function was defined in
		"	check_equals(_target, '/');"
		"};"
		);

	add_actions(mo,
		"square1.button.onRollOut = function() {"
		"	_root.msg2 = 'RollOut'; "
		// Target is the one this function was defined in
		"	check_equals(_target, '/');"
		"};"
		);

	//
	// Mouse buttons events
	//

	add_actions(mo,
		"square1.button.onPress = function() {"
		"	_root.msg2 = 'Press'; "
		// Target is the one this function was defined in
		"	check_equals(_target, '/');"
		"};"
		);

	add_actions(mo,
		"square1.button.onRelease = function() {"
		"	_root.msg2 = 'Release'; "
		// Target is the one this function was defined in
		"	check_equals(_target, '/');"
		"};"
		);

	add_actions(mo,
		"square1.button.onReleaseOutside = function() {"
		"	_root.msg2 = 'ReleaseOutside'; "
		// Target is the one this function was defined in
		"	check_equals(_target, '/');"
		"};"
		);

	//
	// Focus events
	//

	add_actions(mo,
		"square1.button.onSetFocus = function() {"
		"	_root.msg3 = 'SetFocus';"
		// Target is the one this function was defined in
		"	check_equals(_target, '/');"
		"};"
		);

	//
	// Key events - button needs focus for these to work
	//

	add_actions(mo,
		"square1.button.onKeyDown = function() {"
		"	_root.msg3 = 'KeyDown';"
		// Target is the one this function was defined in
		"	check_equals(_target, '/');"
		"};"
		);

	add_actions(mo, 
		"square1.button.onKeyUp = function() {"
		"	_root.msg3 = 'KeyUp';"
		// Target is the one this function was defined in
		"	check_equals(_target, '/');"
		"};"
		);


	SWFMovie_nextFrame(mo); /* showFrame */

	/*****************************************************
	 *
	 * On second frame, add a shape at lower depth,
	 * and check bounds of square1
	 *
	 *
	 *****************************************************/

	{
		SWFShape sh = make_fill_square(0, 0, 120, 120, 0, 0, 0, 0, 255, 0);
		SWFDisplayItem itsh = SWFMovie_add(mo, (SWFBlock)sh);
		SWFDisplayItem_setDepth(itsh, 1);

		check_equals(mo, "printBounds(square1.getBounds())", "'-0.05,-0.05 40.05,40.05'");

		SWFMovie_nextFrame(mo); /* showFrame */
	}

	/*****************************************************
	 *
	 * On third frame, add a shape at higher depth 
	 *
	 *****************************************************/

	{
		SWFShape sh = make_fill_square(0, 0, 120, 120, 0, 0, 0, 0, 255, 0);
		SWFDisplayItem itsh = SWFMovie_add(mo, (SWFBlock)sh);
		SWFDisplayItem_setDepth(itsh, 3);
		SWFDisplayItem_setColorAdd(itsh, 0, 0, 0, -128);

		SWFMovie_nextFrame(mo); /* showFrame */
	}

	/*****************************************************
	 *
	 * On fourth frame, disable the button
	 *
	 *****************************************************/

	{

		add_actions(mo, "square1.button.enabled = false;");
		SWFMovie_nextFrame(mo); /* showFrame */
	}

	/*****************************************************
	 *
	 * Save it...
	 *
	 *****************************************************/

	puts("Saving " OUTPUT_FILENAME );

	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
