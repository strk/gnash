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
 *   DisplayObjects' bounds.
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
	static SWFMovieClip ermc; /* Events-reporting mc */
	mc = newSWFMovieClip();

	if ( ! ermc )
	{
		ermc = newSWFMovieClip();
		SWFMovieClip_add(ermc, (SWFBlock)newSWFAction(
			"_global.dumpObj = function(o,indent) {"
			"	var s = '';"
			"	if ( typeof(o) == 'object' ) {"
			"		s += '{';"
			"		var first=1;"
			"		for (var i in o) {"
			"			if (!first) s+=',';"
			"			s+= i+':'+dumpObj(o[i]);"
			"			first=0;"
			"		}"
			"		s += '}';"
			"	} else {"
			"		s += o;"
			"	}"
			"	return s;"
			"};"
			"if ( _root.buttonChild == undefined ) _root.buttonChild = [];"
			"var myDepth = getDepth()+16383;"
			"var myName = ''+this;"
			"if ( _root.buttonChild[myDepth] == undefined ) _root.buttonChild[myDepth] = {nam:myName,exe:1,uld:0};"
			"else _root.buttonChild[myDepth]['exe']++;"
			 //"_root.note('Actions in frame0 of '+this+' at depth '+myDepth+' executed.');" 
			"this.onUnload = function() {"
			"	var myDepth = -(getDepth()+32769-16383);"
			//"	_root.note(''+this+' at depth '+myDepth+' unloaded.');"
			"	_root.buttonChild[myDepth]['uld']++;"
			"};"
            "for (i in _level0.square1.button) { trace (i); };"
			//"_root.note('buttonChilds:'+dumpObj(_root.buttonChild));"
		));
		SWFMovieClip_nextFrame(ermc);
	}

	sh1 = make_fill_square(0, 0, 40, 40, 0, 0, 0, 0, 0, 0);
	sh1a = make_fill_square(30, 30, 5, 5, 128, 128, 128, 128, 128, 128);
	sh2 = make_fill_square(0, 0, 40, 40, 255, 0, 0, 255, 0, 0);
	sh2a = make_fill_square(30, 30, 5, 5, 128, 0, 0, 128, 0, 0);
	sh3 = make_fill_square(0, 0, 40, 40, 0, 255, 0, 0, 255, 0);
	sh3a = make_fill_square(30, 30, 5, 5, 0, 128, 0, 0, 128, 0);
	sh4 = make_fill_square(0, 0, 40, 40, 255, 255, 0, 255, 255, 0);
	sh4a = make_fill_square(30, 30, 5, 5, 128, 128, 0, 128, 128, 0);

	/* Higher depth DisplayObject is intentionally added before lower depth one */
	br = SWFButton_addCharacter(bu, (SWFCharacter)sh1a, SWFBUTTON_HIT);
	SWFButtonRecord_setDepth(br, 2);
	br = SWFButton_addCharacter(bu, (SWFCharacter)sh1, SWFBUTTON_HIT);
	SWFButtonRecord_setDepth(br, 1);

	/* Higher depth DisplayObject is intentionally added before lower depth one */
	br = SWFButton_addCharacter(bu, (SWFCharacter)sh2a, SWFBUTTON_UP );
	SWFButtonRecord_setDepth(br, 2);
	br = SWFButton_addCharacter(bu, (SWFCharacter)sh2, SWFBUTTON_UP );
	SWFButtonRecord_setDepth(br, 1);

	/* Higher depth DisplayObject is intentionally added before lower depth one */
	br = SWFButton_addCharacter(bu, (SWFCharacter)sh3a, SWFBUTTON_DOWN );
	SWFButtonRecord_setDepth(br, 2);
	br = SWFButton_addCharacter(bu, (SWFCharacter)sh3, SWFBUTTON_DOWN );
	SWFButtonRecord_setDepth(br, 1);

	/* Higher depth DisplayObject is intentionally added before lower depth one */
	br = SWFButton_addCharacter(bu, (SWFCharacter)sh4a, SWFBUTTON_OVER );
	SWFButtonRecord_setDepth(br, 2);
	br = SWFButton_addCharacter(bu, (SWFCharacter)sh4, SWFBUTTON_OVER );
	SWFButtonRecord_setDepth(br, 1);

	/* Add events reported DisplayObject in all states at depth 10 */
	br = SWFButton_addCharacter(bu, (SWFCharacter)ermc, SWFBUTTON_HIT|SWFBUTTON_DOWN|SWFBUTTON_OVER|SWFBUTTON_UP);
	SWFButtonRecord_setDepth(br, 10);

	/* Add events reported DisplayObject just HIT state at depth 11 */
	br = SWFButton_addCharacter(bu, (SWFCharacter)ermc, SWFBUTTON_HIT);
	SWFButtonRecord_setDepth(br, 11);

	/* Add events reported DisplayObject just UP state at depth 12 */
	br = SWFButton_addCharacter(bu, (SWFCharacter)ermc, SWFBUTTON_UP);
	SWFButtonRecord_setDepth(br, 12);

	/* Add events reported DisplayObject just OVER state at depth 13 */
	br = SWFButton_addCharacter(bu, (SWFCharacter)ermc, SWFBUTTON_OVER);
	SWFButtonRecord_setDepth(br, 13);

	/* Add events reported DisplayObject just DOWN state at depth 14 */
	br = SWFButton_addCharacter(bu, (SWFCharacter)ermc, SWFBUTTON_DOWN);
	SWFButtonRecord_setDepth(br, 14);


	SWFButton_addAction(bu, compileSWFActionCode(
		"_root.msg='MouseOut';"
		"if ( _root.testno == 4 || _root.testno == 9 || _root.testno == 14 ) {"
		"	_root.check_equals(_root.printBounds(getBounds()), '-0.05,-0.05 40.05,40.05');"
		/* Target of button action is the button's parent sprite */
		"	_root.check_equals(_target, '/square1');"
		"	setTarget('/');"
		"	_root.check_equals(_target, '/');"
		"	_root.testno++;"
		"	_root.note(_root.testno+'. Press mouse button inside the square, and release it outside.');"
		"} else {"
		//"	_root.note('SWFBUTTON_MOUSEOUT');"
		"	_root.xfail('Unexpectedly got SWFBUTTON_MOUSEOUT event (testno:'+_root.testno+')');"
		"}"
		), SWFBUTTON_MOUSEOUT);

	SWFButton_addAction(bu, compileSWFActionCode(
		"_root.msg='MouseOver';"

		"if ( _root.testno == 1 ) {" /* ONLY CHECK buttonChild on first frame */

		/* "_root.note('buttonChild is '+dumpObj(_root.buttonChild));" */

		/* added OVER state char */
		"	_root.check_equals(_root.buttonChild.realLength(), 3);"

		/* OVER state char loaded */
		"	_root.check_equals(typeof(_root.buttonChild[13]), 'object');"
		"	_root.check_equals(_root.buttonChild[13].nam, '_level0.square1.button.instance7');"
		"	_root.check_equals(_root.buttonChild[13].exe, 1);" /* OVER state char */
		"	_root.check_equals(_root.buttonChild[13].uld, 0);" /* OVER state char */

		/* UP state char unloaded */
		"	_root.check_equals(_root.buttonChild[12].exe, 1);"
		"	_root.check_equals(_root.buttonChild[12].uld, 1);"
		"	_root.check_equals(typeof(_level0.square1.button.instance6), 'movieclip');"
		"	_root.check_equals(_level0.square1.button.instance6._name, 'instance6');"
		"	_root.check_equals(_level0.square1.button.instance6.getDepth(), -16398);"

		/* ALL state char still there, not reloaded, not unloaded */
		"	_root.check_equals(_root.buttonChild[10].exe, 1);"
		"	_root.check_equals(_root.buttonChild[10].uld, 0);"

		"}"

		"if ( _root.testno == 1 || _root.testno == 6 || _root.testno == 11 ) {"

		//"	_root.note('buttonChild is '+dumpObj(_root.buttonChild));"
		"	_root.check_equals(_root.printBounds(getBounds()), '-0.05,-0.05 40.05,40.05');"
		/* Target of button action is the button's parent sprite */
		"	_root.check_equals(_target, '/square1');"
		"	setTarget('/');"
		"	_root.check_equals(_target, '/');"
		"	_root.testno++;"
		"	_root.note(_root.testno+'. Press (and keep pressed) the mouse button inside the square.');"
		"} else {"
		//"	_root.note('SWFBUTTON_MOUSEOVER');"
		// need MOUSEOVER for MOUSEUPOUTSIDE
		"	if ( _root.testno != 5 && _root.testno != 10 && _root.testno != 15 ) {"
		"		_root.fail('Unexpectedly got SWFBUTTON_MOUSEOVER event (testno:'+_root.testno+')');"
		"	}"
		"}"
		), SWFBUTTON_MOUSEOVER);

	SWFButton_addAction(bu, compileSWFActionCode(
		"_root.msg='MouseDown';"

		"if ( _root.testno == 2 ) {" /* ONLY CHECK buttonChild on first frame */

		/* Added DOWN state char */
		"	_root.check_equals(_root.buttonChild.realLength(), 4);"

		/* DOWN state char loaded */
		"	_root.check_equals(typeof(_root.buttonChild[14]), 'object');"
		"	_root.check_equals(_root.buttonChild[14].nam, '_level0.square1.button.instance8');"
		"	_root.check_equals(_root.buttonChild[14].exe, 1);" 
		"	_root.check_equals(_root.buttonChild[14].uld, 0);" 

		/* OVER state char unloaded */
		"	_root.check_equals(_root.buttonChild[13].exe, 1);" 
		"	_root.check_equals(_root.buttonChild[13].uld, 1);"

		/* ALL state char still there, not reloaded, not unloaded */
		"	_root.check_equals(_root.buttonChild[10].exe, 1);"
		"	_root.check_equals(_root.buttonChild[10].uld, 0);"

		"}"

		"if ( _root.testno == 2 || _root.testno == 7 || _root.testno == 12 ) {"
		"	_root.check_equals(_root.printBounds(getBounds()), '-0.05,-0.05 40.05,40.05');"
		/* Target (and name) of button action is the button's parent sprite */
		"	_root.check_equals(_target, '/square1');"
		"	_root.check_equals(_name, 'square1');"
		"	setTarget('/');"
		"	_root.check_equals(_target, '/');"
		"	_root.check_equals(typeof(_name), 'string');"
		"	_root.check_equals(_name, '');"
		"	_root.testno++;"
		"	_root.note(_root.testno+'. Depress the mouse button inside the square.');"
		"} else {"
		//"	_root.note('SWFBUTTON_MOUSEDOWN');"
		// need MOUSEDOWN for MOUSEUPOUTSIDE
		"	if ( _root.testno != 5 && _root.testno != 10 && _root.testno != 15 ) {"
		"		_root.fail('Unexpectedly got SWFBUTTON_MOUSEDOWN event (testno:'+_root.testno+')');"
		"	}"
		"}"
		), SWFBUTTON_MOUSEDOWN);

	SWFButton_addAction(bu, compileSWFActionCode(
		"_root.msg='MouseUp';"
		"if ( _root.testno == 3 || _root.testno == 8 || _root.testno == 13 ) {"
		"	_root.check_equals(_root.printBounds(getBounds()), '-0.05,-0.05 40.05,40.05');"
		/* Target of button action is the button's parent sprite */
		"	_root.check_equals(_target, '/square1');"
		"	setTarget('/');"
		"	_root.check_equals(_target, '/');"
		"	_root.testno++;"
		"	_root.note(_root.testno+'. Move the mouse pointer off the square.');"
		"} else {"
		//"	_root.note('SWFBUTTON_MOUSEUP');"
		"	_root.fail('Unexpectedly got SWFBUTTON_MOUSEUP event (testno:'+_root.testno+')');"
		"}"
		), SWFBUTTON_MOUSEUP);

	/* SWFBUTTON_MOUSEUPOUTSIDE *should* be invoked !! */
	SWFButton_addAction(bu, compileSWFActionCode(
		"_root.msg='MouseUpOutside';"
		"if ( _root.testno == 5 || _root.testno == 10 || _root.testno == 15 ) {"
		"	_root.check_equals(_root.printBounds(getBounds()), '-0.05,-0.05 40.05,40.05');"
		/* Target of button action is the button's parent sprite */
		"	_root.check_equals(_target, '/square1');"
		"	_root.check_equals(_name, 'square1');"
		"	setTarget('/');"
		"	_root.check_equals(_target, '/');"
		"	_root.nextFrame();"
		"} else {"
		//"	_root.note('SWFBUTTON_MOUSEUPOUTSIDE');"
		"	_root.fail('Unexpectedly got SWFBUTTON_MOUSEUPOUTSIDE event (testno:'+_root.testno+')');"
		"}"
		), SWFBUTTON_MOUSEUPOUTSIDE);

	/* Keypress */
	SWFButton_addAction(bu, compileSWFActionCode(
		"_root.note('KeyPress: a');"
		//"_root.check(Key.isDown('a'));"
	), SWFBUTTON_KEYPRESS('a'));
	SWFButton_addAction(bu, compileSWFActionCode(
		"_root.note('KeyPress: b');"
		//"_root.check(Key.isDown('b'));"
	), SWFBUTTON_KEYPRESS('b'));

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
	SWFMovie_setRate(mo, 12);

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

	SWFMovie_nextFrame(mo); /* showFrame */

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

	check_equals(mo, "typeof(square1.button)", "'object'");
	check(mo, "square1.button instanceOf Button");
	check_equals(mo, "typeof(square1.button.useHandCursor)", "'boolean'");
	check_equals(mo, "square1.button.useHandCursor", "true");

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
	 * On second frame, check construction of the button
	 * DisplayObject states and give instructions to proceed
	 *
	 *
	 *****************************************************/

	add_actions(mo, "Array.prototype.realLength = function() {"
		" var l=0; for (var i in this) { "
		"	if (Number(i) == i) l++;" /* count only numbers */
		" };"
		" return l;"
		"};");

	/* buttonChild was initialized with 2 elements */
	check_equals(mo, "typeof(_root.buttonChild)", "'object'");
	check(mo, "_root.buttonChild instanceof Array");
	check_equals(mo, "_root.buttonChild.realLength()", "2"); /* UP and ALL states */

	/* sprite for ALL states */
	check_equals(mo, "typeof(_root.buttonChild[10])", "'object'");
	check_equals(mo, "(_root.buttonChild[10].nam)", "'_level0.square1.button.instance5'"); 
	check_equals(mo, "(_root.buttonChild[10].exe)", "1");
	check_equals(mo, "(_root.buttonChild[10].uld)", "0");

	/* sprite for UP state */
	check_equals(mo, "typeof(_root.buttonChild[12])", "'object'");
	check_equals(mo, "(_root.buttonChild[12].nam)", "'_level0.square1.button.instance6'"); 
	check_equals(mo, "(_root.buttonChild[12].exe)", "1");
	check_equals(mo, "(_root.buttonChild[12].uld)", "0");
	check_equals(mo, "_level0.square1.button.instance6._name", "'instance6'");
	check_equals(mo, "_level0.square1.button.instance6.getDepth()", "-16371");

	/* sprite for HIT state not constructed */
	check_equals(mo, "typeof(_root.buttonChild[11])", "'undefined'");

	/* sprite for DOWN state not constructed */
	check_equals(mo, "typeof(_root.buttonChild[13])", "'undefined'"); 

	add_actions(mo,
		"stop();"
		/*"_root.note('buttonChild is '+dumpObj(_root.buttonChild));"*/
		"_root.testno=0;"
		"_root.square1.onRollOut = function() { _root.testno++; delete _root.square1.onRollOut; nextFrame(); };"
		"_root.note('"
		"0. Roll over and out the red square, not touching the small dark-red square in it.\n   The cursor should turn to an hand while on the square."
		"');");

	/* hitTest should work on every child, not just first added */
	check(mo, "_level0.square1.hitTest(60,60,true)");

	SWFMovie_nextFrame(mo); /* showFrame */

	/*****************************************************
	 *
	 * On third frame, start the button event test
	 *
	 *****************************************************/

	add_actions(mo,
		"stop();"
		/*"_root.note('buttonChild is '+dumpObj(_root.buttonChild));"*/
		"_root.testno=1;"
		"_root.note('"
		"1. Roll over the red square."
		"');");

	SWFMovie_nextFrame(mo); /* showFrame */

	/*****************************************************
	 *
	 * On fourth frame, add a shape at lower depth,
	 * and check bounds of square1
	 *
	 *
	 *****************************************************/

	{
		SWFShape sh = make_fill_square(0, 0, 120, 120, 0, 0, 0, 0, 255, 0);
		SWFDisplayItem itsh = SWFMovie_add(mo, (SWFBlock)sh);
		SWFDisplayItem_setDepth(itsh, 1);

		check_equals(mo, "printBounds(square1.getBounds())", "'-0.05,-0.05 40.05,40.05'");

		/* buttonChild should now have a total of 4 elements (UP,DOWN, OVER and ALL states) */
		check_equals(mo, "typeof(_root.buttonChild)", "'object'");
		check(mo, "_root.buttonChild instanceof Array");
		check_equals(mo, "_root.buttonChild.realLength()", "4"); 

		/* sprite for ALL states */
		check_equals(mo, "typeof(_root.buttonChild[10])", "'object'");
		check_equals(mo, "(_root.buttonChild[10].nam)", "'_level0.square1.button.instance5'"); 
		check_equals(mo, "(_root.buttonChild[10].exe)", "1");
		check_equals(mo, "(_root.buttonChild[10].uld)", "0");

		/* sprite for UP state */
		check_equals(mo, "typeof(_root.buttonChild[12])", "'object'");
		check_equals(mo, "(_root.buttonChild[12].nam)", "'_level0.square1.button.instance6'"); 
		check_equals(mo, "(_root.buttonChild[12].exe)", "3"); 
		check_equals(mo, "(_root.buttonChild[12].uld)", "2");

		/* sprite for OVER state */
		check_equals(mo, "typeof(_root.buttonChild[13])", "'object'");
		check_equals(mo, "(_root.buttonChild[13].nam)", "'_level0.square1.button.instance7'"); 
		check_equals(mo, "(_root.buttonChild[13].exe)", "4");
		check_equals(mo, "(_root.buttonChild[13].uld)", "4");

		/* sprite for DOWN state */
		check_equals(mo, "typeof(_root.buttonChild[14])", "'object'");
		check_equals(mo, "(_root.buttonChild[14].nam)", "'_level0.square1.button.instance8'"); 
		check_equals(mo, "(_root.buttonChild[14].exe)", "2");
		check_equals(mo, "(_root.buttonChild[14].uld)", "2");

		/* sprite for HIT state never constructed */
		check_equals(mo, "typeof(_root.buttonChild[11])", "'undefined'"); 



		add_actions(mo,
			"stop();"
			"_root.note('-- Added shape at lower depth --');"
			"_root.testno++;"
			"_root.note(_root.testno+'. Roll over the square.');"
		);

		SWFMovie_nextFrame(mo); /* showFrame */
	}

	/*****************************************************
	 *
	 * On fifth frame, add a shape at higher depth 
	 *
	 *****************************************************/

	{
		SWFShape sh = make_fill_square(0, 0, 120, 120, 0, 0, 0, 0, 255, 0);
		SWFDisplayItem itsh = SWFMovie_add(mo, (SWFBlock)sh);
		SWFDisplayItem_setDepth(itsh, 3);
		SWFDisplayItem_setColorAdd(itsh, 0, 0, 0, -128);

		/* buttonChild should now have a total of 4 elements (UP,DOWN, OVER and ALL states) */
		check_equals(mo, "typeof(_root.buttonChild)", "'object'");
		check(mo, "_root.buttonChild instanceof Array");
		check_equals(mo, "_root.buttonChild.realLength()", "4"); 

		/* sprite for ALL states */
		check_equals(mo, "typeof(_root.buttonChild[10])", "'object'");
		check_equals(mo, "(_root.buttonChild[10].nam)", "'_level0.square1.button.instance5'"); 
		check_equals(mo, "(_root.buttonChild[10].exe)", "1");
		check_equals(mo, "(_root.buttonChild[10].uld)", "0");

		/* sprite for UP state */
		check_equals(mo, "typeof(_root.buttonChild[12])", "'object'");
		check_equals(mo, "(_root.buttonChild[12].nam)", "'_level0.square1.button.instance6'"); 
		check_equals(mo, "(_root.buttonChild[12].exe)", "5"); 
		check_equals(mo, "(_root.buttonChild[12].uld)", "4");

		/* sprite for OVER state */
		check_equals(mo, "typeof(_root.buttonChild[13])", "'object'");
		check_equals(mo, "(_root.buttonChild[13].nam)", "'_level0.square1.button.instance7'"); 
		check_equals(mo, "(_root.buttonChild[13].exe)", "8");
		check_equals(mo, "(_root.buttonChild[13].uld)", "8");

		/* sprite for DOWN state */
		check_equals(mo, "typeof(_root.buttonChild[14])", "'object'");
		check_equals(mo, "(_root.buttonChild[14].nam)", "'_level0.square1.button.instance8'"); 
		check_equals(mo, "(_root.buttonChild[14].exe)", "4");
		check_equals(mo, "(_root.buttonChild[14].uld)", "4");

		/* sprite for HIT state never constructed */
		check_equals(mo, "typeof(_root.buttonChild[11])", "'undefined'"); 

		add_actions(mo,
			"stop();"
			"_root.note('-- Added shape at higher depth --');"
			"_root.testno++;"
			"_root.note(_root.testno+'. Roll over the square.');"
		);

		SWFMovie_nextFrame(mo); /* showFrame */
	}

	/*****************************************************
	 *
	 * On sixth frame, disable the button
	 * and check total tests so far
	 *
	 *****************************************************/

	{

		add_actions(mo,
            "check_equals(square1.button.enabled, true);"
			"square1.button.enabled = 6;"
            "check_equals(square1.button.enabled, 6);"
			"square1.button.enabled = 'string';"
            "check_equals(square1.button.enabled, 'string');"
            "square1.button._visible = false;"
            "check_equals(square1.button.enabled, 'string');"
            "square1.button._visible = true;"
			"square1.button.enabled = false;"
			"stop();"
			"_root.totals(164);"
			"_root.note('-- Button disabled, try playing with it, nothing should happen --');"
		);
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
