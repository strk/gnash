// 
//   Copyright (C) 2008, 2009, 2010 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
//
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
// Test case for TextFormat ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="TextFormat.as";

#include "check.as"

Object.prototype.hasOwnProperty = ASnative(101, 5);

check_equals(typeof(TextFormat), 'function');
check_equals(typeof(TextFormat.prototype), 'object');
tfObj = new TextFormat();
check_equals(typeof(tfObj), 'object');
check(tfObj instanceof TextFormat);

// The members below would not exist before
// the construction of first TextFormat object
check(TextFormat.prototype.hasOwnProperty('display'));
check(TextFormat.prototype.hasOwnProperty('bullet'));
check(TextFormat.prototype.hasOwnProperty('tabStops'));
check(TextFormat.prototype.hasOwnProperty('blockIndent'));
check(TextFormat.prototype.hasOwnProperty('leading'));
check(TextFormat.prototype.hasOwnProperty('indent'));
check(TextFormat.prototype.hasOwnProperty('rightMargin'));
check(TextFormat.prototype.hasOwnProperty('leftMargin'));
check(TextFormat.prototype.hasOwnProperty('align'));
check(TextFormat.prototype.hasOwnProperty('underline'));
check(TextFormat.prototype.hasOwnProperty('italic'));
check(TextFormat.prototype.hasOwnProperty('bold'));
check(TextFormat.prototype.hasOwnProperty('target'));
check(TextFormat.prototype.hasOwnProperty('url'));
check(TextFormat.prototype.hasOwnProperty('color'));
check(TextFormat.prototype.hasOwnProperty('size'));
check(TextFormat.prototype.hasOwnProperty('font'));
check(!TextFormat.prototype.hasOwnProperty('getTextExtent'));
check(tfObj.hasOwnProperty('getTextExtent'));


// When you construct a TextFormat w/out args all members
// are of the 'null' type. In general, uninitialized members
// are all of the 'null' type.
check_equals(typeof(tfObj.display), 'string');
check_equals(tfObj.display, 'block');
check_equals(typeof(tfObj.bullet), 'null');
check_equals(typeof(tfObj.tabStops), 'null');
check_equals(typeof(tfObj.blockIndent), 'null');
check_equals(typeof(tfObj.leading), 'null');
check_equals(typeof(tfObj.indent), 'null');
check_equals(typeof(tfObj.rightMargin), 'null');
check_equals(typeof(tfObj.leftMargin), 'null');
check_equals(typeof(tfObj.align), 'null');
check_equals(typeof(tfObj.underline), 'null');
check_equals(typeof(tfObj.italic), 'null');
check_equals(typeof(tfObj.bold), 'null');
check_equals(typeof(tfObj.target), 'null');
check_equals(typeof(tfObj.url), 'null');
check_equals(typeof(tfObj.color), 'null');
check_equals(typeof(tfObj.size), 'null');
check_equals(typeof(tfObj.font), 'null');
check_equals(typeof(tfObj.getTextExtent), 'function');

// new TextFormat([font, [size, [color, [bold, [italic, [underline, [url, [target, [align,[leftMargin, [rightMargin, [indent, [leading]]]]]]]]]]]]])
tfObj = new TextFormat("fname", 2, 30, true, false, true, 'http', 'tgt', 'cEnter', '23', '32', 12, 4);
check_equals(typeof(tfObj.display), 'string');
check_equals(tfObj.display, 'block');
check_equals(typeof(tfObj.bullet), 'null');
check_equals(typeof(tfObj.tabStops), 'null');
check_equals(typeof(tfObj.blockIndent), 'null');
check_equals(tfObj.leading, 4);
check_equals(tfObj.indent, 12);
check_equals(typeof(tfObj.rightMargin), 'number'); // even if we passed a string to it
check_equals(tfObj.rightMargin, 32);
check_equals(typeof(tfObj.leftMargin), 'number'); // even if we passed a string to it
check_equals(tfObj.leftMargin, 23);
check_equals(tfObj.align, 'center');
check_equals(tfObj.target, 'tgt');
check_equals(tfObj.url, 'http');
check_equals(tfObj.underline, true);
check_equals(typeof(tfObj.italic), 'boolean');
check_equals(tfObj.italic, false);
check_equals(tfObj.bold, true);
check_equals(tfObj.color, 30);
check_equals(tfObj.size, 2);
check_equals(tfObj.font, 'fname');


/// Bold

// The boolean conversion of a string is version dependent.
stringbool = "string" ? true : false;

tf = new TextFormat();
check_equals(tf.bold, null);
tf.bold = true;
check_equals(tf.bold, true);
tf.bold = false;
check_equals(tf.bold, false);
tf.bold = "string";
check_equals(tf.bold, stringbool);
tf.bold = null;
check_equals(tf.bold, null);
tf.bold = "string";
check_equals(tf.bold, stringbool);
tf.bold = undefined;
check_equals(tf.bold, null);


// rightMargin
tf = new TextFormat();
check_equals(tf.rightMargin, null);
tf.rightMargin = 10;
check_equals(tf.rightMargin, 10);
tf.rightMargin = -10;
check_equals(tf.rightMargin, 0);
tf.rightMargin = "string";
check_equals(tf.rightMargin, 0);
tf.rightMargin = null;
check_equals(tf.rightMargin, null);
tf.rightMargin = "string";
check_equals(tf.rightMargin, 0);
tf.rightMargin = undefined;
check_equals(tf.rightMargin, null);

// leftMargin
tf = new TextFormat();
check_equals(tf.leftMargin, null);
tf.leftMargin = 10;
check_equals(tf.leftMargin, 10);
tf.leftMargin = -10;
check_equals(tf.leftMargin, 0);
tf.leftMargin = "string";
check_equals(tf.leftMargin, 0);
tf.leftMargin = null;
check_equals(tf.leftMargin, null);
tf.leftMargin = "string";
check_equals(tf.leftMargin, 0);
tf.leftMargin = undefined;
check_equals(tf.leftMargin, null);

// blockIndent
tf = new TextFormat();
check_equals(tf.blockIndent, null);
tf.blockIndent = 10;
check_equals(tf.blockIndent, 10);
tf.blockIndent = -10;

#if OUTPUT_VERSION < 8
check_equals(tf.blockIndent, 0);
#else
xcheck_equals(tf.blockIndent, -10);
#endif

tf.blockIndent = "string";
#if OUTPUT_VERSION < 8
check_equals(tf.blockIndent, 0);
#else
xcheck_equals(tf.blockIndent, -2147483648);
#endif

tf.blockIndent = null;
check_equals(tf.blockIndent, null);

tf.blockIndent = "string";
#if OUTPUT_VERSION < 8
check_equals(tf.blockIndent, 0);
#else
xcheck_equals(tf.blockIndent, -2147483648);
#endif
tf.blockIndent = undefined;
check_equals(tf.blockIndent, null);

// leading
tf = new TextFormat();
check_equals(tf.leading, null);
tf.leading = 10;
check_equals(tf.leading, 10);
tf.leading = -10;

#if OUTPUT_VERSION < 8
check_equals(tf.leading, 0);
#else
xcheck_equals(tf.leading, -10);
#endif

tf.leading = "string";
#if OUTPUT_VERSION < 8
check_equals(tf.leading, 0);
#else
xcheck_equals(tf.leading, -2147483648);
#endif

tf.leading = null;
check_equals(tf.leading, null);

tf.leading = "string";
#if OUTPUT_VERSION < 8
check_equals(tf.leading, 0);
#else
xcheck_equals(tf.leading, -2147483648);
#endif
tf.leading = undefined;
check_equals(tf.leading, null);

// indent
tf = new TextFormat();
check_equals(tf.indent, null);
tf.indent = 10;
check_equals(tf.indent, 10);
tf.indent = -10;

#if OUTPUT_VERSION < 8
check_equals(tf.indent, 0);
#else
xcheck_equals(tf.indent, -10);
#endif

tf.indent = "string";
#if OUTPUT_VERSION < 8
check_equals(tf.indent, 0);
#else
xcheck_equals(tf.indent, -2147483648);
#endif

tf.indent = null;
check_equals(tf.indent, null);

tf.indent = "string";
#if OUTPUT_VERSION < 8
check_equals(tf.indent, 0);
#else
xcheck_equals(tf.indent, -2147483648);
#endif
tf.indent = undefined;
check_equals(tf.indent, null);

// size
tf = new TextFormat();
check_equals(tf.size, null);
tf.size = 10;
check_equals(tf.size, 10);

tf.size = -10;
xcheck_equals(tf.size, -10);

tf.size = "string";
#if OUTPUT_VERSION < 8
check_equals(tf.size, 0);
#else
xcheck_equals(tf.size, -2147483648);
#endif

tf.size = null;
check_equals(tf.size, null);

tf.size = "string";
#if OUTPUT_VERSION < 8
check_equals(tf.size, 0);
#else
xcheck_equals(tf.size, -2147483648);
#endif
tf.size = undefined;
check_equals(tf.size, null);

// align
tf.align = "hi";
check_equals(tf.align, null);
tf.align = "Left";
check_equals(tf.align, "left");
tf.align = "o";
check_equals(tf.align, "left");
tf.align = "righto";
check_equals(tf.align, "left");
tf.align = "center";
check_equals(tf.align, "center");
tf.align = "right";
check_equals(tf.align, "right");
tf.align = undefined;
check_equals(tf.align, "right");
tf.align = null;
check_equals(tf.align, "right");

// Check tabStops property.
// The passed array is processed before assignment, not simply stored.
tf = new TextFormat();

o = {};
o.valueOf = function() { return 6; };
o.toString = function() { return "string"; };

a = [ o ];

tf.tabStops = a;
check_equals(a.toString(), "string");
xcheck_equals(tf.tabStops.toString(), "6");

tf2 = new TextFormat("dejafont", 12);

// getTextExtent has different behaviour for SWF6.
#if OUTPUT_VERSION > 6

// I don't know how to test this properly, as we can only test device fonts
// here, and the pp uses a different font from Gnash.

te = tf2.getTextExtent("Hello");

// The object is a bare object
te.hasOwnProperty = Object.prototype.hasOwnProperty;

check(te.hasOwnProperty("ascent"));
check(te.hasOwnProperty("descent"));
check(te.hasOwnProperty("textFieldWidth"));
check(te.hasOwnProperty("textFieldHeight"));
check(te.hasOwnProperty("width"));
check(te.hasOwnProperty("height"));

xcheck_equals(Math.round(te.textFieldWidth), 37);
check_equals(Math.round(te.ascent), 11);

#if OUTPUT_VERSION > 7
 xcheck_equals(Math.round(te.textFieldHeight), 18); 
 check_equals(Math.round(te.descent), 3); 
#else
 xcheck_equals(Math.round(te.textFieldHeight), 17); 
 xcheck_equals(Math.round(te.descent), 2); 
#endif

te = tf2.getTextExtent("Hello", 10);
#if OUTPUT_VERSION > 7
 xcheck_equals(Math.round(te.textFieldHeight), 74);
#else
 xcheck_equals(Math.round(te.textFieldHeight), 17);
#endif

check_equals(te.textFieldWidth, 10);

#if OUTPUT_VERSION > 7
check_equals(Math.round(te.width), 9);
#else
xcheck_equals(Math.round(te.width), 33);
#endif


te = tf2.getTextExtent("Hello", 5);
#if OUTPUT_VERSION < 8
 xcheck_equals(Math.round(te.textFieldHeight), 17);
 xcheck_equals(Math.round(te.descent), 2); 
#else
 xcheck_equals(Math.round(te.textFieldHeight), 74);
 check_equals(Math.round(te.descent), 3);
#endif
check_equals(te.textFieldWidth, 5);
check_equals(Math.round(te.ascent), 11);

#if OUTPUT_VERSION > 7
// Width of largest character in version 8?
check_equals(Math.round(te.width), 9);
#else
xcheck_equals(Math.round(te.width), 33);
#endif


te = tf2.getTextExtent("Longer sentence with more words.", 30);
check_equals(te.textFieldWidth, 30);
xcheck_equals(Math.round(te.width), 25);
#if OUTPUT_VERSION > 7
xcheck_equals(te.height, 152.9);
#else
xcheck_equals(te.height, 152);
#endif

te = tf2.getTextExtent("o");
xcheck_equals(Math.round(te.textFieldWidth), 12);
check_equals(Math.round(te.ascent), 11);
#if OUTPUT_VERSION < 8
 xcheck_equals(Math.round(te.textFieldHeight), 17); 
 xcheck_equals(Math.round(te.descent), 2); 
#else
 xcheck_equals(Math.round(te.textFieldHeight), 18); 
 check_equals(Math.round(te.descent), 3); 
#endif

te = tf2.getTextExtent("oo");
xcheck_equals(Math.round(te.textFieldWidth), 20);
check_equals(Math.round(te.ascent), 11);
#if OUTPUT_VERSION < 8
 xcheck_equals(Math.round(te.textFieldHeight), 17); 
 xcheck_equals(Math.round(te.descent), 2); 
#else
 xcheck_equals(Math.round(te.textFieldHeight), 18); 
 check_equals(Math.round(te.descent), 3); 
#endif

te = tf2.getTextExtent("ool");
xcheck_equals(Math.round(te.textFieldWidth), 24);
check_equals(Math.round(te.ascent), 11);
#if OUTPUT_VERSION < 8
 xcheck_equals(Math.round(te.textFieldHeight), 17); 
 xcheck_equals(Math.round(te.descent), 2); 
#else
 xcheck_equals(Math.round(te.textFieldHeight), 18); 
 check_equals(Math.round(te.descent), 3); 
#endif

tf2.size = 20;
te = tf2.getTextExtent("ool");
xcheck_equals(Math.round(te.textFieldHeight), 27);
xcheck_equals(Math.round(te.textFieldWidth), 36);
#if OUTPUT_VERSION < 8
 xcheck_equals(Math.round(te.ascent), 18); 
 xcheck_equals(Math.round(te.descent), 4); 
#else
 check_equals(Math.round(te.ascent), 19); 
 check_equals(Math.round(te.descent), 5);
#endif

#endif

#if OUTPUT_VERSION < 7
    check_totals(122);
#elif OUTPUT_VERSION == 7
    check_totals(159);
#else 
    check_totals(159);
#endif
