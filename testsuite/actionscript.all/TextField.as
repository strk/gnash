// 
//   Copyright (C) 2007, 2009, 2010 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
// Test case for TextField ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="TextField.as";
#include "check.as"

printBounds = function(b)
{
    var s = '';
    s += "xmin:"+b.xMin;
    s += " ymin:"+b.yMin;
    s += " xmax:"+b.xMax;
    s += " ymax:"+b.yMax;
    return s;
};

TextField.prototype.getBounds = MovieClip.prototype.getBounds;


// check that the dejagnu clip is *not* a TextField, which is why the
// following prototype properties are not yet initialized.
#if OUTPUT_VERSION > 6
 check_equals(_root.getInstanceAtDepth(-16383), _level0.__shared_assets);
 xcheck(!_level0.__shared_assets.instance1._xtrace_win instanceof TextField);
 xcheck_equals(typeof(_level0.__shared_assets.instance1._xtrace_win),
            "movieclip");
 note(_level0.__shared_assets.instance1._xtrace_win.toString());
#endif

#if OUTPUT_VERSION < 6

    check_equals(typeof(TextField), 'function');
    check_equals(TextField.prototype, undefined);
    createTextField("tf", 50, 10, 10, 10, 10);
    xcheck_equals(typeof(tf), "movieclip");
    xcheck_equals(tf.__proto__.toString(), "[object Object]");
    check(!tf instanceOf TextField);

    tf = new TextField();
    xcheck(tf instanceOf TextField);
    totals();

#else

check_equals(typeof(TextField), 'function');
check_equals(typeof(TextField.prototype), 'object');
check_equals(typeof(TextField.prototype.__proto__), 'object');
check_equals(TextField.prototype.__proto__, Object.prototype);
check_equals(typeof(TextField.prototype.setTextFormat), 'function');
check_equals(typeof(TextField.prototype.getTextFormat), 'function');
check_equals(typeof(TextField.prototype.setNewTextFormat), 'function');
check_equals(typeof(TextField.prototype.getNewTextFormat), 'function');
check_equals(typeof(TextField.prototype.getDepth), 'function');
check_equals(typeof(TextField.prototype.removeTextField), 'function');
check_equals(typeof(TextField.prototype.replaceSel), 'function');

 // TextField.prototype was implicitly initialized by ASBroadcaster.initialize !
 // See http://www.senocular.com/flash/tutorials/listenersasbroadcaster/?page=2
 check_equals(typeof(TextField.prototype.addListener), 'function');
 check_equals(typeof(TextField.prototype.removeListener), 'function');
 check_equals(typeof(TextField.prototype.broadcastMessage), 'function');
 check(TextField.prototype.hasOwnProperty("_listeners"));
 check_equals(typeof(TextField.prototype._listeners), 'object');
 check(TextField.prototype._listeners instanceof Array);
 check_equals(TextField.prototype._listeners.length, 0);

// NOTE: the following will be true after a call to createTextField ! Seek forward to see..
xcheck( !TextField.prototype.hasOwnProperty('background'));
xcheck( !TextField.prototype.hasOwnProperty('backgroundColor'));
xcheck( !TextField.prototype.hasOwnProperty('autoSize') );
xcheck( !TextField.prototype.hasOwnProperty('border') );
xcheck( !TextField.prototype.hasOwnProperty('borderColor') );
xcheck( !TextField.prototype.hasOwnProperty('bottomScroll') );
xcheck( !TextField.prototype.hasOwnProperty('embedFonts') );
xcheck( !TextField.prototype.hasOwnProperty('hscroll') );
xcheck( !TextField.prototype.hasOwnProperty('html') );
xcheck( !TextField.prototype.hasOwnProperty('htmlText') );
xcheck( !TextField.prototype.hasOwnProperty('length') );
xcheck( !TextField.prototype.hasOwnProperty('maxChars') );
xcheck( !TextField.prototype.hasOwnProperty('maxhscroll') );
xcheck( !TextField.prototype.hasOwnProperty('maxscroll') );
xcheck( !TextField.prototype.hasOwnProperty('multiline') );
xcheck( !TextField.prototype.hasOwnProperty('password') );
xcheck( !TextField.prototype.hasOwnProperty('restrict') );
xcheck( !TextField.prototype.hasOwnProperty('scroll') );
xcheck( !TextField.prototype.hasOwnProperty('selectable') );
xcheck( !TextField.prototype.hasOwnProperty('text') );
xcheck( !TextField.prototype.hasOwnProperty('textColor') );
xcheck( !TextField.prototype.hasOwnProperty('textHeight') ); // should be available on first instantiation
xcheck( !TextField.prototype.hasOwnProperty('textWidth') ); // should be available on first instantiation
xcheck( !TextField.prototype.hasOwnProperty('type') ); // should be available on first instantiation
xcheck( !TextField.prototype.hasOwnProperty('variable') );
xcheck( !TextField.prototype.hasOwnProperty('wordWrap') );

// this is a static method
check_equals(typeof(TextField.getFontList), 'function');
check_equals(typeof(TextField.prototype.getFontList), 'undefined');

tfGetFontList = TextField.getFontList;
tfGetFontListObj = new Object();
tfGetFontListObj.f = tfGetFontList;
check_equals(TextField.getFontList() instanceof Array, true);
check_equals(tfGetFontList() instanceof Array, true);
check_equals(tfGetFontListObj.f() instanceof Array, true);
check_equals(tfGetFontList.call(null) instanceof Array, true);
check_equals(tfGetFontList.call(undefined) instanceof Array, true);

check(TextField.prototype.hasOwnProperty('replaceText'));
#if OUTPUT_VERSION > 6
check_equals(typeof(TextField.prototype.replaceText), 'function');
#else
check_equals(typeof(TextField.prototype.replaceText), 'undefined');
// but exists!
#endif

tfObj = new TextField();
check_equals(typeof(tfObj), 'object');
check(tfObj instanceof TextField);

check_equals(typeof(tfObj.setTextFormat), 'function');
check_equals(typeof(tfObj.getTextFormat), 'function');
check_equals(typeof(tfObj.setNewTextFormat), 'function');
check_equals(typeof(tfObj.getNewTextFormat), 'function');
check_equals(typeof(tfObj.addListener), 'function');
check_equals(typeof(tfObj.removeListener), 'function');
check_equals(typeof(tfObj.getDepth), 'function');
check_equals(typeof(tfObj.removeTextField), 'function');
check_equals(typeof(tfObj.replaceSel), 'function');
// this is a static method, it's available as TextField.getFontList
check_equals(typeof(tfObj.getFontList), 'undefined');
check_equals(typeof(tfObj._parent), 'undefined'); // no parent

//--------------------------------------------------
// Check textfield creation trough createTextField
//--------------------------------------------------

ret = createTextField("tf", 99, 10, 10, 500, 500);
#if OUTPUT_VERSION < 8
check_equals(typeof(ret), 'undefined');
#else
check_equals(typeof(ret), 'object');
check_equals(ret, _root.tf);
#endif

check(tf instanceof TextField);
check_equals(typeof(tf), 'object');
check(tf.hasOwnProperty('_listeners'));
check_equals(tf._listeners.length, 1); // adds self to the listeners
check_equals(tf._listeners[0], tf); // adds self to the listeners set
check(!tf.hasOwnProperty('broadcastMessage'));
check(!tf.hasOwnProperty('addListener'));
check(!tf.hasOwnProperty('removeListener'));

// NOTE: the following were false before the call to createTextField ! Seek backward to see..
check( TextField.prototype.hasOwnProperty('background'));
check( TextField.prototype.hasOwnProperty('backgroundColor'));
check( TextField.prototype.hasOwnProperty('autoSize') );
check( TextField.prototype.hasOwnProperty('border') );
check( TextField.prototype.hasOwnProperty('borderColor') );
check( TextField.prototype.hasOwnProperty('bottomScroll') );
check( TextField.prototype.hasOwnProperty('embedFonts') );
check( TextField.prototype.hasOwnProperty('hscroll') );
check( TextField.prototype.hasOwnProperty('html') );
check( TextField.prototype.hasOwnProperty('htmlText') );
check( TextField.prototype.hasOwnProperty('length') );
check( TextField.prototype.hasOwnProperty('maxChars') );
check( TextField.prototype.hasOwnProperty('maxhscroll') );
check( TextField.prototype.hasOwnProperty('maxscroll') );
check( TextField.prototype.hasOwnProperty('multiline') );
check( TextField.prototype.hasOwnProperty('password') );
check( TextField.prototype.hasOwnProperty('restrict') );
check( TextField.prototype.hasOwnProperty('scroll') );
check( TextField.prototype.hasOwnProperty('selectable') );
check( TextField.prototype.hasOwnProperty('text') );
check( TextField.prototype.hasOwnProperty('textColor') );
check( TextField.prototype.hasOwnProperty('textHeight') );
check( TextField.prototype.hasOwnProperty('textWidth') );
check( TextField.prototype.hasOwnProperty('type') );
check( TextField.prototype.hasOwnProperty('variable') );
check( TextField.prototype.hasOwnProperty('wordWrap') );
check( TextField.prototype.hasOwnProperty('length') );

check( ! TextField.prototype.hasOwnProperty('valueOf') );
check( ! TextField.prototype.hasOwnProperty('toString') );
check( TextField.prototype.__proto__.hasOwnProperty('valueOf') );
check( TextField.prototype.__proto__.hasOwnProperty('toString') );

// Check TextField._alpha

check_equals(typeof(tf._alpha), 'number');
check( ! tf.hasOwnProperty('_alpha') ); 
check( ! tf.__proto__.hasOwnProperty('_alpha') ); 

// Check TextField.autoSize

check_equals(typeof(tf.autoSize), 'string');
check_equals(tf.autoSize, 'none'); // TODO: research which valid values we have
check(! tf.hasOwnProperty('autoSize'));
tf.autoSize = false;
check_equals(tf.autoSize, 'none'); // false is a synonim for 'none'
tf.autoSize = true;
check_equals(tf.autoSize, 'left'); // true is a synonim for 'left'
tf.autoSize = 'true';
check_equals(tf.autoSize, 'none'); // 'true' (as a string) is invalid, thus equivalent to 'none'
tf.autoSize = 'center';
check_equals(tf.autoSize, 'center'); // 'center' is a valid value
tf.autoSize = 'right';
check_equals(tf.autoSize, 'right'); // 'right' is a valid value
o = new Object(); o.toString = function() { return 'center'; };
tf.autoSize = o;
check_equals(tf.autoSize, 'center'); // toString is called for object args
tf.autoSize = 'lEft';
check_equals(tf.autoSize, 'left'); // arg is not case sensitive 
tf.autoSize = new Boolean(true);
check_equals(tf.autoSize, 'none'); // a Boolean is the same as any other object

tf.autoSize = 'none';

// Check TextField.background

check_equals(typeof(tf.background), 'boolean');
check(!tf.hasOwnProperty('background'));
check_equals(tf.background, false);
tf.background = true;
check_equals(tf.background, true);
tf.background = 0;
check_equals(tf.background, false);
check_equals(typeof(tf.background), 'boolean');
tf.background = 54.3;
check_equals(typeof(tf.background), 'boolean');
check_equals(tf.background, true);
o = new Object; o.valueOf = function() { return 0x0000FF; };
tf.background = o;
check_equals(tf.background, true);
o = new Object; o.valueOf = function() { return 'string'; };
tf.background = o;
check_equals(typeof(tf.background), 'boolean');
check_equals(tf.background, true); // 'string' evaluates to true
tf.background = new Boolean(false);
check_equals(typeof(tf.background), 'boolean');
check_equals(tf.background, true);  // dunno why, but Boolean evaluates to false 
tf.background = false;

// Check TextField.backgroundColor

check_equals(typeof(tf.backgroundColor), 'number');
check(!tf.hasOwnProperty('backgroundColor'));
tf.backgroundColor = 0x00FF00;
check_equals(tf.backgroundColor, 0x00FF00);
tf.backgroundColor = 'red';
check_equals(tf.backgroundColor, 0x000000); // string value evaluates to NaN thus 0
o = new Object; o.valueOf = function() { return 0x0000FF; };
tf.backgroundColor = o;
check_equals(tf.backgroundColor, 0x0000FF); // valueOf is invoked

// Check TextField.border

check_equals(typeof(tf.border), 'boolean');
check(!tf.hasOwnProperty('border'));

// Check TextField.borderColor

check_equals(typeof(tf.borderColor), 'number');
check(!tf.hasOwnProperty('borderColor'));

// Check TextField.bottomScroll

check_equals(typeof(tf.bottomScroll), 'number');
check(!tf.hasOwnProperty('bottomScroll'));
check_equals(tf.bottomScroll, 1);
tf.bottomScroll = 100; // bottomScroll is read-only
check_equals(tf.bottomScroll, 1);

// Check TextField.embedFonts

check_equals(typeof(tf.embedFonts), 'boolean');
check(!tf.hasOwnProperty('embedFonts'));
check_equals(tf.embedFonts, false);
tf.embedFonts = true;
check_equals(tf.embedFonts, true);
tf.embedFonts = new Number(0); // will be converted to bool (true)
check_equals(typeof(tf.embedFonts), 'boolean');
check_equals(tf.embedFonts, true);
tf.embedFonts = ""; // will be converted to bool (false);
check_equals(typeof(tf.embedFonts), 'boolean');
check_equals(tf.embedFonts, false);
// TODO: do this test with really embedded fonts, in misc-ming.all/DefineEditTextTest.c

// Check TextField._highquality

check_equals(typeof(tf._highquality), 'number');
check(!tf.hasOwnProperty('_highquality'));
check(!tf.__proto__.hasOwnProperty('_highquality'));
check_equals(tf._highquality, 1);
tf._highquality = 0;
check_equals(tf._highquality, 0);
tf._highquality = 1;

// Check TextField._height (how is this different from textHeight?)

check_equals(typeof(tf._height), 'number');
check(!tf.hasOwnProperty('_height'));
check(!tf.__proto__.hasOwnProperty('_height'));
check_equals(tf._height, 500); // as we created it, see createTextField call
tf._height = 99999;
check_equals(tf._height, 99999); 
tf._height = 500;

// Check TextField.hscroll

check_equals(typeof(tf.hscroll), 'number');
check(!tf.hasOwnProperty('hscroll'));
check_equals(tf.hscroll, 0);
tf.hscroll = 1;
xcheck_equals(tf.hscroll, 0);
tf.hscroll = 0;

// Check TextField.html

check_equals(typeof(tf.html), 'boolean');
check(!tf.hasOwnProperty('html'));
check_equals(tf.html, false);
tf.html = true;
check_equals(tf.html, true);
tf.html = false;

// Check TextField.htmlText (the displayed text in explicit HTML)

check_equals(typeof(tf.htmlText), 'string');
check(!tf.hasOwnProperty('htmlText'));
check_equals(tf.htmlText, '');
tf.htmlText = new Array;
check_equals(typeof(tf.htmlText), 'string'); // forced cast to string
check_equals(tf.htmlText, ''); 
check_equals(tf.html, false);
tf.htmlText = "Hello <b>html</b> world";
check_equals(tf.html, false); // assigning to htmlText doesn't change the 'html' flag
check_equals(tf.htmlText, 'Hello <b>html</b> world');
// Changing htmlText also changes text
check_equals(tf.text, 'Hello <b>html</b> world');
tf.text = "Hello world";
check_equals(tf.htmlText, 'Hello world');

//-------------------------------------------------------------------------
// Check TextField.length  (number of characters in text)
//-------------------------------------------------------------------------

check_equals(typeof(tf.length), 'number');
check(!tf.hasOwnProperty('length'));
tf.text = "";
check_equals(tf.length, 0);
tf.length = 10; // you don't change lenght like this, you assign to text instead
check_equals(tf.length, 0);
tf.text = "Hello world";
check_equals(tf.length, 11);
tf.htmlText = "Hello <b>world</b>";
check_equals(tf.length, 18); // the tags are also counted

// Check TextField.maxChars

check_equals(typeof(tf.maxChars), 'null');
check(!tf.hasOwnProperty('maxChars'));
tf.maxChars = 5;
check_equals(tf.maxChars, 5);
tf.text = "0123456789";
// no effect (maybe only limits user input)
check_equals(tf.text, "0123456789");
tf.maxChars = "string";
check_equals(typeof(tf.maxChars), "null");
tf.maxChars = -6;
check_equals(typeof(tf.maxChars), "number");
check_equals(tf.maxChars, -6);
tf.maxChars = 0;
check_equals(typeof(tf.maxChars), "null");

tf.maxChars = null;

// Check TextField.maxhscroll

check_equals(typeof(tf.maxhscroll), 'number');
check(!tf.hasOwnProperty('maxhscroll'));
check_equals(tf.maxhscroll, 0);
tf.maxhscroll = 10;
check_equals(tf.maxhscroll, 0); // read-only

// Check TextField.maxscroll

check_equals(typeof(tf.maxscroll), 'number');
check(!tf.hasOwnProperty('maxscroll'));
check_equals(tf.maxscroll, 1);
tf.maxscroll = 10;
check_equals(tf.maxscroll, 1); // read-only

// Check TextField.multiline

check_equals(typeof(tf.multiline), 'boolean');
check_equals(tf.multiline, false);
check(!tf.hasOwnProperty('multiline'));
check_equals(tf.multiline, false);
tf.multiline = true;
check_equals(tf.multiline, true);
tf.multiline = 54;
check_equals(typeof(tf.multiline), 'boolean'); // always converted
tf.multiline = false;

//-------------------------------------------------------------------------
// Check TextField._name
//-------------------------------------------------------------------------

check_equals(typeof(tf._name), 'string');
check(!tf.hasOwnProperty('_name'));
check(!tf.__proto__.hasOwnProperty('_name'));
check_equals(tf._name, 'tf');
tfref = tf;
tf._name = 'changed';
check_equals(typeof(tf), 'undefined');
check_equals(typeof(tfref), 'object');
check_equals(tfref._name, 'changed');
check_equals(tfref._target, '/changed');
tfref._name = 'tf';
check_equals(typeof(tf), 'object');
check_equals(typeof(tfref), 'object');
// TODO: see effects of unloading the textfield ?

//-------------------------------------------------------------------------
// Check TextField._root and _global
//-------------------------------------------------------------------------

check(tf._root);
check_equals(_root, _root.tf._root);

check(tf._global);
check_equals(_global, _root.tf._global);

check(tf._level0);

// They would exist if they had been loaded...
check(!tf._level1);
check(!tf._level2);

#if OUTPUT_VERSION > 6
check(tf._root != tf._ROOT);
check(tf._root != tf._GLOBAL);
check(tf._level0 != tf._LEVEL0);
#endif 

//-------------------------------------------------------------------------
// Check TextField._parent
//-------------------------------------------------------------------------

check_equals(typeof(tf._parent), 'movieclip');
check(!tf.hasOwnProperty('_parent'));
check(!tf.__proto__.hasOwnProperty('_parent'));
check_equals(tf._parent, _root);
bk = tf._parent;
tf._parent = 23;
xcheck_equals(tf._parent, 23); // can be overridden !
check_equals(tf._target, "/tf"); // but won't change _target
r = delete tf._parent;
xcheck(r);
r = delete tf._parent;
check(!r);
TextField.prototype._parent = "from proto";
check_equals(tf._parent, _root); // still unchanged
delete TextField.prototype._parent;
tf._parent = bk;

//-------------------------------------------------------------------------
// Check TextField.password
//-------------------------------------------------------------------------

// This verifies it really uses to_bool.

check_equals(typeof(tf.password), 'boolean');
check_equals(tf.password, false);
check(!tf.hasOwnProperty('password'));
check_equals(tf.password, false);
tf.password = true;
check_equals(tf.password, true);
tf.password = 7;
check_equals(tf.password, true);
tf.password = "string";
#if OUTPUT_VERSION > 6
check_equals(tf.password, true);
#else
check_equals(tf.password, false);
#endif
tf.password = 0;
check_equals(tf.password, false);
tf.password = "a string";
#if OUTPUT_VERSION > 6
check_equals(tf.password, true);
#else
check_equals(tf.password, false);
#endif
tf.password = undefined;
check_equals(tf.password, false);
// TODO: check effects of setting to 'password' (should hide characters)
tf.password = false;

// Check TextField.quality

// TODO: check this, might be a string
check_equals(typeof(tf._quality), 'string');
check(!tf.hasOwnProperty('_quality'));
check(!tf.__proto__.hasOwnProperty('_quality'));
check(!tf.__proto__.__proto__.hasOwnProperty('_quality'));
check(!tf.__proto__.__proto__.__proto__.hasOwnProperty('_quality'));
check_equals(tf._quality, "HIGH");
tf._quality = "FAKE VALUE";
check_equals(tf._quality, "HIGH");
tf._quality = "LOW";
check_equals(tf._quality, "LOW");
tf._quality = "HIGH";

// Check TextField.restrict (the set of characters a user can input)

check_equals(typeof(tf.restrict), 'null');
check(!tf.hasOwnProperty('restrict'));
check_equals(typeof(tf.restrict), 'null');
tf.text = "Hello World";
tf.restrict = "olH";
check_equals(typeof(tf.restrict), 'string');
check_equals(tf.text, "Hello World");
tf.text = "Hello World"; // override
// doesn't influence explicit setting, only GUI modification
// of the textfield (TODO: test with a MovieTester)
check_equals(tf.text, "Hello World");

tf.restrict = "";
check_equals(tf.restrict, "");
check_equals(typeof(tf.restrict), 'string');
tf.restrict = "ä";
check_equals(tf.restrict, "ä");
check_equals(typeof(tf.restrict), 'string');

tf.restrict = 9;
check_equals(tf.restrict, "9");
check_equals(typeof(tf.restrict), 'string');

o = {};
o.toString = function() { return "üöä"; };
o.valueOf = function() { return 8; };

tf.restrict = o;
check_equals(tf.restrict, "üöä");
check_equals(typeof(tf.restrict), 'string');

tf.restrict = null;
xcheck_equals(tf.restrict, null);
xcheck_equals(typeof(tf.restrict), "null");

// Check TextField._rotation

check_equals(typeof(tf._rotation), 'number');
check(!tf.hasOwnProperty('_rotation'));
check(!tf.__proto__.hasOwnProperty('_rotation'));
check_equals(tf._rotation, 0);
check_equals(tf._width, 500);
tf._rotation = 10;
check_equals(tf._rotation, 10);
check_equals(tf._width, 579.2);
check_equals(tf._xscale, 100.0);
tf._rotation = 0;
check_equals(tf._width, 500.0);
check_equals(tf._xscale, 100.0);

// Check TextField.scroll

// TODO: better test for this, might do nothing if there's no scrollin
check_equals(typeof(tf.scroll), 'number');
check( ! tf.hasOwnProperty('scroll') ); 
check_equals(tf.scroll, 1);
tf.scroll = 10;
xcheck_equals(tf.scroll, 1); // read-only // is it?

// Check TextField.selectable

check_equals(typeof(tf.selectable), 'boolean');
check( ! tf.hasOwnProperty('selectable') ); 
check_equals(tf.selectable, true);
tf.selectable = false;
check_equals(tf.selectable, false);
tf.selectable = "Hello";
check_equals(typeof(tf.selectable), 'boolean');
tf.selectable = true;

// Check TextField._soundbuftime

check_equals(typeof(tf._soundbuftime), 'number');
check( ! tf.hasOwnProperty('_soundbuftime') ); 
check( ! tf.__proto__.hasOwnProperty('_soundbuftime') ); 
xcheck_equals(tf._soundbuftime, 5); // the default is 5, it seems

// These seem to be only valid for MovieClips, but they are still read-only
check_equals(typeof(tf._currentframe), 'undefined');
tf._currentframe = "6";
check_equals(tf._currentframe, undefined);

check_equals(typeof(tf._totalframes), 'undefined');
tf._totalframes = 67;
check_equals(tf._totalframes, undefined);

check_equals(typeof(tf._framesloaded), 'undefined');
tf._framesloaded = "hi";
check_equals(tf._framesloaded, undefined);

// Check TextField._focusrect
check(tf._focusrect !== 'null');
check(tf._focusRect !== 'null');
check_equals(typeof(tf._focusrect), 'null');
check_equals(typeof(tf._focusRect), 'null');
check(! tf.hasOwnProperty('_focusrect') ); 
check(! tf.__proto__.hasOwnProperty('_focusrect') ); 


// Check TextField.tabEnabled

check_equals(typeof(tf.tabEnabled), 'undefined');
check( ! tf.hasOwnProperty('tabEnabled') ); 
check( ! tf.__proto__.hasOwnProperty('tabEnabled') ); 
tf.tabEnabled = false;
check_equals(tf.tabEnabled, false);
delete(tf.tabEnabled);

// Check TextField.tabIndex

check_equals(typeof(tf.tabIndex), 'undefined');
check( ! tf.hasOwnProperty('tabIndex') ); 
check( ! tf.__proto__hasOwnProperty('tabIndex') ); 
tf.tabIndex = 9;
check_equals(tf.tabIndex, 9);
delete(tf.tabIndex);

//-------------------------------------------------------------------------
// Check TextField._target
//-------------------------------------------------------------------------

check_equals(typeof(tf._target), 'string');
check( ! tf.hasOwnProperty('_target') ); 
check( ! tf.__proto__.hasOwnProperty('_target') ); 
check_equals(tf._target, '/tf');
// TODO: check the effect of changing _name on the _target value
tf._target = "fake_target"; // read-only
check_equals(tf._target, '/tf');

// Check TextField.text

check_equals(typeof(tf.text), 'string');
check( ! tf.hasOwnProperty('text') ); 
check_equals(tf.text, 'Hello World');
tf.text = "hello world";
check_equals(tf.text, 'hello world');
check_equals(tf.length, 11); // number of characters in "hello world"


// Check TextField.textColor

check_equals(typeof(tf.textColor), 'number');
check( ! tf.hasOwnProperty('textColor') ); 
check_equals(tf.textColor, 0);
tf.textColor = 0xFF0000;
check_equals(tf.textColor, 0xFF0000);
// TODO: check color (use misc-ming.all/DefineEditTextTest.swf and a test runner with check_pixel)

// Check TextField.textHeight (height of the bounding box)

check_equals(typeof(tf.textHeight), 'number');
check( ! tf.hasOwnProperty('textHeight') ); 
currentHeight = tf.textHeight; // WARNING: this might depend on the default font height
tf.textHeight = 1000;
check_equals(tf.textHeight, currentHeight); // was read-only (I think)

// Check TextField.textWidth (width of the bounding box)

check_equals(typeof(tf.textWidth), 'number');
check( ! tf.hasOwnProperty('textWidth') ); 
currentWidth = tf.textWidth; // WARNING: this might depend on the default font height
tf.textWidth = 1000;
check_equals(tf.textWidth, currentWidth); // was read-only (I think)

// Check TextField.type (input or dynamic)

check_equals(typeof(tf.type), 'string');
check( ! tf.hasOwnProperty('type') ); 
check_equals(tf.type, 'dynamic'); 
tf.type = "input";
check_equals(tf.type, 'input'); 
tf.type = new Array();
check_equals(typeof(tf.type), 'string');  // invalid assignment
check_equals(tf.type, 'input');  // keeps previous value
tf.type = "dynamic";
check_equals(tf.type, 'dynamic');  
tf.type = new Array();
check_equals(tf.type, 'dynamic'); // keeps previous value 
o = {}; o.toString = function() { return 'Input'; };
tf.type = o;
check_equals(tf.type, 'input');

// Check TextField._url (url of the SWF that created the textfield)

check_equals(typeof(tf._url), 'string');
check( ! tf.hasOwnProperty('_url') ); 
check( ! tf.__proto__.hasOwnProperty('_url') ); 
check_equals(tf._url, _root._url);
tf._url = "fake url";
check_equals(tf._url, _root._url); // read-only

//-------------------------------------------------------------------------
// Check TextField.variable (variable name associated with the textfield)
//-------------------------------------------------------------------------
//
// SUMMARY: write summary here
//
//-------------------------------------------------------------------------

check_equals(typeof(tf.variable), 'null');
check( ! tf.hasOwnProperty('variable') ); 
tf.variable = _level0.inputVar;
check_equals(typeof(tf.variable), 'null'); // _level0.inputVar doesn't exist !
tf.variable = 2;
check_equals(typeof(tf.variable), 'string'); 
check_equals(tf.variable, '2'); 
tf.variable = undefined;
check_equals(typeof(tf.variable), 'null'); 
tf.variable = 2;
tf.variable = null;
check_equals(typeof(tf.variable), 'null'); 
tf.variable = "_level0.inputVar";
check_equals(tf.variable, '_level0.inputVar'); 
xcheck_equals(typeof(_level0.inputVar), 'undefined');
check_equals(tf.text, "hello world");  // as _level0.inputVar is unexistent
xcheck(!_level0.hasOwnProperty('inputVar'));
_level0.inputVar = "dynamic variable";
check_equals(tf.text, "dynamic variable");
tf.text = "back-propagated";
check_equals(_level0.inputVar, "back-propagated");
o = new Object();
tf.variable = "_level0.o.t"; // non-existent member (yet)
check_equals(tf.text, "back-propagated");  // _level0.o.t doesn't exist yet
o.t = "from object"; // here we create _level0.o.t
check_equals(tf.text, "back-propagated"); // but creating _level0.o.t doesn't trigger textfield text update
tf.text = "back-to-object"; // instead, assigning to TextField.text updates the object
check_equals(o.t, "back-to-object");  
o.t = "from object again"; // but updates to the object still don't update the TextField
check_equals(tf.text, "back-to-object");  // assigning to the object doesn't trigger update of text ?
tf.variable = "_level0.o.t"; // We re-assign TextField.variable, now the variable exists
xcheck_equals(tf.text, "from object again");  // this time the TextField.text is updated
check_equals(o.t, "from object again");
o.t = "and forever";
xcheck_equals(tf.text, "from object again"); // but updating o.t still doesn't trigger update of the text ?
tf.text = "and forever back";
check_equals(o.t, "and forever back"); // while updating textfield's text updates o.t

// Test value of variable if the property is present but undefined.

mc45 = _root.createEmptyMovieClip("mc45", getNextHighestDepth());
mc45.createTextField("tf45", 0, 0, 100, 100, getNextHighestDepth());
mc45.tf45.variable = "_root.mc45.vari";
_root.mc45.vari = undefined;
#if OUTPUT_VERSION < 7
check_equals(mc45.tf45.text, "");
#else
check_equals(mc45.tf45.text, "undefined");
#endif

//-------------------------------------------------------------------------
// TODO: check TextField.getDepth() 
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// Check TextField.removeTextField and soft references
//-------------------------------------------------------------------------
//
// TextField as_value references are soft references like MovieClip ones.
// Till the ref is not danglign, typeof(ref) will return 'object', once
// the ref is dangling it will return 'movieclip' and will successfully 
// rebind to a real MovieClip ! When rebound, typeof(ref) will return 'object'
// or 'movieclip' depending on the bound thing.
//
//-------------------------------------------------------------------------

createTextField("hardref", 23, 10, 10, 160, 200);
hardref.prop = 5;
softref = hardref;
check_equals(typeof(hardref), 'object');
check_equals(typeof(softref), 'object');
check_equals(softref.prop, 5);
check_equals(softref.getDepth(), 23);
hardref.removeTextField();
check_equals(typeof(hardref), 'undefined');
check_equals(typeof(softref), 'movieclip'); // a dangling character ref is always reported to be a 'movieclip' (historical reasons probably)
check_equals(typeof(softref.prop), 'undefined');
createEmptyMovieClip("hardref", 24);
check_equals(typeof(hardref), 'movieclip');
hardref.prop = 7;
check_equals(typeof(softref), 'movieclip');
check_equals(softref.prop, 7); // and it's actually also rebound to one if available
hardref.removeMovieClip();
createTextField("hardref", 25, 10, 10, 160, 200);
hardref.prop = 9;
check_equals(typeof(softref), 'object'); // changes type on rebind
check_equals(softref.prop, 9);


//-------------------------------------------------------------------------
// Check TextField._visible 
//-------------------------------------------------------------------------

check_equals(typeof(tf._visible), 'boolean');
check( ! tf.hasOwnProperty('_visible') ); 
check( ! tf.__proto__.hasOwnProperty('_visible') ); 
check_equals(tf._visible, true);
tf._visible = false;
check_equals(tf._visible, false);
tf._visible = true;

//-------------------------------------------------------------------------
// Check TextField._width (how is this different from textWidth ?)
//-------------------------------------------------------------------------

check_equals(typeof(tf._width), 'number');
check( ! tf.hasOwnProperty('_width') ); 
check( ! tf.__proto__.hasOwnProperty('_width') ); 
check_equals(tf._width, 500); // as it was set by createTextField, see above
tf._width = 99999;
check_equals(tf._width, 99999); 
b = tf.getBounds(_root); bs = printBounds(b);
check_equals(bs, 'xmin:10 ymin:10 xmax:100009 ymax:510');
tf.autoSize = false;
tf.text = 'small'; // doesn't reset bounds (being autoSize false);
check_equals(tf._width, 99999); 
b = tf.getBounds(_root); bs = printBounds(b);
check_equals(bs, 'xmin:10 ymin:10 xmax:100009 ymax:510');

tf.autoSize = true; // changes width !!
check(tf._width < 99999); 
ow = tf._width;

tf.autoSize = false;  // doesn't reset to last manually set one
check_equals(tf._width, ow);

tf._width = 500;

//-------------------------------------------------------------------------
// Check TextField.wordWrap (should text wrap when bbox limit is hit?)
//-------------------------------------------------------------------------

check_equals(typeof(tf.wordWrap), 'boolean');
check( ! tf.hasOwnProperty('wordWrap') ); 
check_equals(tf.wordWrap, false);
// TODO: check what can be assigned to wordWrap and what not...

//-------------------------------------------------------------------------
// Check TextField._x 
//-------------------------------------------------------------------------

check_equals(typeof(tf._x), 'number');
check_equals(typeof(tf._X), 'number');
check( ! tf.hasOwnProperty('_x') );
check( ! tf.__proto__.hasOwnProperty('_x') );
check_equals(tf._x, 10); // as set by createTextField
tf._x = 20;
check_equals(tf._x, 20);

//-------------------------------------------------------------------------
// Check TextField._xmouse
//-------------------------------------------------------------------------

check_equals(typeof(tf._xmouse), 'number');
check( ! tf.hasOwnProperty('_xmouse') );
check( ! tf.__proto__.hasOwnProperty('_xmouse') );
currXmouse = tf._xmouse; // unsafe, if user moves the mouse while running the test
tf._xmouse = "a string";
check_equals(typeof(tf._xmouse), 'number');
check_equals(tf._xmouse, currXmouse); // possibly unsafe, if user moves the mouse while running the test

//-------------------------------------------------------------------------
// Check TextField._xscale
//-------------------------------------------------------------------------

check_equals(typeof(tf._xscale), 'number');
check( ! tf.hasOwnProperty('_xscale') );
check( ! tf.__proto__.hasOwnProperty('_xscale') );
check_equals(tf._xscale, 100); 
// check how .textWidth and ._width change when changing _xscale
currTextWidth = tf.textWidth;
currWidth = tf._width;
tf._xscale = 200;
note("textWidth: _xscale=100: "+currTextWidth+"; _xscale=200: "+tf.textWidth);
// check_equals(tf.textWidth, currTextWidth*2); // not clear what does textWidth depend on
check_equals(tf._width, currWidth*2);
tf._xscale = 100;

//-------------------------------------------------------------------------
// Check TextField._y 
//-------------------------------------------------------------------------

check_equals(typeof(tf._y), 'number');
check( ! tf.hasOwnProperty('_y') );
check( ! tf.__proto__.hasOwnProperty('_y') );
check_equals(tf._y, 10); // as set by createTextField
tf._y = 5;
check_equals(tf._y, 5);

//-------------------------------------------------------------------------
// Check TextField._ymouse
//-------------------------------------------------------------------------

check_equals(typeof(tf._ymouse), 'number');
check( ! tf.hasOwnProperty('_ymouse') );
check( ! tf.__proto__.hasOwnProperty('_ymouse') );
currYmouse = tf._ymouse;
tf._ymouse = "a string";
check_equals(typeof(tf._ymouse), 'number');
check_equals(tf._ymouse, currYmouse); // possibly unsafe, if user moves the mouse while running the test

//-------------------------------------------------------------------------
// Check TextField._yscale
//-------------------------------------------------------------------------

check_equals(typeof(tf._yscale), 'number');
check( ! tf.hasOwnProperty('_yscale') );
check( ! tf.__proto__.hasOwnProperty('_yscale') );
check_equals(tf._yscale, 100); 
// check how .textHeight and ._height change based on _yscale
currTextHeight = tf.textHeight;
currHeight = tf._height;
tf._yscale = 200;
note("textHeight: _yscale=100: "+currTextHeight+"; _yscale=200: "+tf.textHeight);
// check_equals(tf.textHeight, currTextHeight*2); // not clear what does textHeight depend on
check_equals(tf._height, currHeight*2);
tf._yscale = 100;

//-------------------------------------------------------------------------
// Check interaction between autoSize and _width and wordWrap
//-------------------------------------------------------------------------

tf._width = 10; // "hello world" text should overflow this
tf.text = "Hello world";
tf.autoSize = 'none';
tf.wordWrap = false;
check_equals(tf._width, 10);
origTextWidth = tf.textWidth;
tf.autoSize = 'center';
check(tf._width > 10);
check_equals(tf.textWidth, origTextWidth); // textWidth isn't influenced by autoSize 
tf.autoSize = 'none';
tf.wordWrap = true;
note("After setting wordWrap flat: textWidth: "+tf.textWidth+" origTextWidth:"+origTextWidth);
check_equals(tf.textWidth, origTextWidth);  
tf._width = 10;
note("After reducing _width: textWidth: "+tf.textWidth+" origTextWidth:"+origTextWidth);
check_equals(tf._width, 10);

#if OUTPUT_VERSION < 8
 check_equals(origTextWidth, tf.textWidth); 
#else
 xcheck(origTextWidth > tf.textWidth); 
#endif

// test that adding a newline doesn't change the bounds width
// see bug #22216
tf.autoSize = 'center';

// Word wrap is still true, so nothing should happen!
check_equals(tf._width, 10);
tf.text = "single line";

// Changing text should also not change width.
check_equals(tf._width, 10);

linewidth = tf._width;
tf.text = "single line\n";

check_equals(tf._width, 10);
check_equals(tf._width, linewidth); 

// Test that setting autoSize = none
// doesn't reset the bounding box.
// See bug #24266
//
oldwidth = tf._width;
note("width: "+oldwidth);
check(oldwidth>0); // or the test is invalid
tf.autoSize = 'none'; // tf was created with bounds 0,0
check_equals(tf._width, oldwidth); // but _width didn't change to that

// Test that when autoSize != none,
// and wordWrap is true, text box isn't
// reset. See #24348
//
tf.wordWrap = false;
check_equals(tf._width, oldwidth); // text takes more width with wordWrap false
note("width on wordWrap="+tf.wordWrap+" autoSize="+tf.autoSize+": "+tf._width);
tf.autoSize = 'center'; 
note("width on wordWrap="+tf.wordWrap+" autoSize="+tf.autoSize+": "+tf._width);
check(tf._width > oldwidth); 
oldwidth = tf._width;
tf.wordWrap = true;
note("width on wordWrap="+tf.wordWrap+" autoSize="+tf.autoSize+": "+tf._width);
check_equals(tf._width, oldwidth);  // wordWrap change didn't reset the bbox


//------------------------------------------------------------
// Test insane calls
//------------------------------------------------------------

ret = createTextField("tf2", 99, 5, 6, -1, -2);
check_equals(typeof(tf2), 'object');
check_equals(tf2._width, 1);
check_equals(tf2._height, 2);
check_equals(tf2._x, 5);
check_equals(tf2._y, 6);

createTextField("tf3", 99, 10.87, 10.12, NAN, 50.74);
check_equals(tf3._x, 10);
check_equals(tf3._y, 10);
check_equals(tf3._width, 0);
check_equals(tf3._height, 50);

createTextField("tf4", 99, 10, 50, NAN, "20");
check_equals(tf4._width, 0);
check_equals(tf4._height, 20);

createTextField(3, "101", "10", '100', '32', '15');
check_equals(_root[3].getDepth(), 101);
check_equals(_root[3]._x, 10);
check_equals(_root[3]._y, 100);
check_equals(_root[3]._width, 32);
check_equals(_root[3]._height, 15);

// One argument more
createTextField("tf5", 102, 10, 130, 3, 2, 12);
check_equals(tf5._name, "tf5");
check_equals(tf5._target, "/tf5");
check_equals(tf5.getDepth(), 102);
check_equals(tf5._x, 10);
check_equals(tf5._y, 130);
check_equals(tf5._width, 3);
check_equals(tf5._height, 2);

// One argument missing
createTextField("tf6", 103, 10, 10, 160);
check_equals(typeof(tf6), 'undefined');

/// Test TextField.replaceSel

createTextField('repl1', 99, 10, 10, 10, 10);
Selection.setFocus(repl1);
check_equals(Selection.getFocus(), '_level0.repl1');
repl1.text = "Text in a string";

ret = repl1.replaceSel("More ");
/// Check that the selection start and end indices are adjusted.
check_equals(Selection.getBeginIndex(), 5);
check_equals(Selection.getEndIndex(), 5);
check_equals(repl1.text, "More Text in a string");

ret = repl1.replaceSel("");
check_equals(Selection.getEndIndex(), 5);
check_equals(repl1.text, "More Text in a string");


Selection.setSelection(0, 1);
ret = repl1.replaceSel("HU");
check_equals(ret, undefined);
check_equals(repl1.text, "HUore Text in a string");

check_equals(Selection.getBeginIndex(), 2);
check_equals(Selection.getEndIndex(), 2);

Selection.setSelection(2, 5);
ret = repl1.replaceSel("HUU");
check_equals(ret, undefined);
check_equals(repl1.text, "HUHUU Text in a string");

check_equals(Selection.getBeginIndex(), 5);
check_equals(Selection.getEndIndex(), 5);

Selection.setSelection(10, 13);
repl1.replaceSel(7);
check_equals(repl1.text, "HUHUU Text7 a string");
check_equals(Selection.getBeginIndex(), 11);
check_equals(Selection.getEndIndex(), 11);

Selection.setSelection(10, 13);
repl1.replaceSel(new Object());
check_equals(repl1.text, "HUHUU Text[object Object] string");
check_equals(Selection.getBeginIndex(), 25);
check_equals(Selection.getEndIndex(), 25);

Selection.setSelection(1, 20);
repl1.replaceSel("");
#if OUTPUT_VERSION < 8
check_equals(repl1.text, "HUHUU Text[object Object] string");
check_equals(Selection.getBeginIndex(), 1);
check_equals(Selection.getEndIndex(), 20);
#else
check_equals(repl1.text, "Hject] string");
check_equals(Selection.getBeginIndex(), 1);
check_equals(Selection.getEndIndex(), 1);
#endif

repl1.text = "New text";

Selection.setSelection(2, 5);
repl1.replaceSel();
check_equals(repl1.text, "New text");
check_equals(Selection.getBeginIndex(), 2);
check_equals(Selection.getEndIndex(), 5);

//------------------------------------------------------------
// Test TextField.replaceText
//------------------------------------------------------------

#if OUTPUT_VERSION > 6
createTextField ("t", 0, 0, 0, 200, 150);
check_equals(t.text, '');
r = t.replaceText();
check_equals(typeof(r), 'undefined');
check_equals(t.text, '');
r = t.replaceText(0, 0);
check_equals(typeof(r), 'undefined');
check_equals(t.text, '');
r = t.replaceText(0, 0, 'a');
check_equals(typeof(r), 'undefined');
check_equals(t.text, 'a');
r = t.replaceText(0, 0, 'b');
check_equals(t.text, 'ba');
t.replaceText(-1, 0, 'c');
check_equals(t.text, 'ba');
t.replaceText(0, 5, 'd');
check_equals(t.text, 'd');
t.replaceText(-1, 5, 'e');
check_equals(t.text, 'd');
t.replaceText(1, 5, 'f');
check_equals(t.text, 'df');
t.replaceText(0, 5, 'ϦeϦ');
check_equals(t.text, 'ϦeϦ');
t.replaceText(1, 1, 'h');
check_equals(t.text, 'ϦheϦ');
t.replaceText(4, 4, 'h');
check_equals(t.text, 'ϦheϦh');
t.replaceText(4, 4, undef);
check_equals(t.text, 'ϦheϦundefinedh');
t.replaceText(3, 10, 't');
check_equals(t.text, 'Ϧhetnedh');
t.replaceText(3, -1, 'y');
check_equals(t.text, 'Ϧhetnedh');
// TODO: check registered variables
#endif

//------------------------------------------------------------
// Test properties
//------------------------------------------------------------

_root._visible = true; // just to be sure
_root._xscale = _root._yscale = 100;
_root.createTextField('htf',0,0,0,0,0);
check_equals(typeof(htf), 'object');
tf = htf;
with(tf) {
    _x=10;
    _y=11;
    _visible=false;
    _xscale=200;
    _yscale=201;
    _parent='fake_parent';
    _name='fake_name';
    _target='fake';
}

check_equals(_root._x, 0);
check_equals(_root._y, 0);
check_equals(_root._visible, true);
check_equals(_root._xscale, 100);
check_equals(_root._yscale, 100);
check_equals(_root._target, '/');
xcheck_equals(_root._parent, 'fake_parent');
check_equals(_root._name, '');

check_equals(tf._x, 10);
check_equals(tf._y, 11);
check_equals(tf._visible, false);
check_equals(tf._xscale, 200);
check_equals(tf._yscale, 201);
check_equals(tf._target, '/fake_name');
check_equals(tf._parent, _level0); 
check_equals(tf._name, 'fake_name');

_root._visible = true;
_root._x = _root._y = 0;
_root._xscale = _root._yscale = 100;


// Check that "new _global.TextField()" is called in createTextField

backup = _global.TextField;
count = 0;
args = 0;

storedthis = undefined;

_global.TextField = function() {
    storedthis = this;
    this.bo = "stringo";
    args = arguments.length;
    count++;
};

TextField.prototype = {};
TextField.prototype.toString = function() { return "Hoppla!"; };

// The fact that createTextField works even when _global.TextField is
// replaced shows that the native functions (making into a real TextField)
// is done in createTextField.
r = _root.createTextField("tfmo", 2, 2, 10, 10, 6);
check_equals(count, 1);
check_equals(args, 0);
check_equals(_root.tfmo._x, 2);

/// The returned object is still the this pointer that our fake constructor
/// worked on.
check_equals(_root.tfmo, storedthis);
xcheck(_root.tfmo === storedthis);

check_equals(_root.tfmo.bo, "stringo");
check(_root.tfmo.hasOwnProperty("bo"));

// Not sure why this isn't the case for version 6 or 7.
#if OUTPUT_VERSION >= 8
check_equals(r.toString(), "Hoppla!");
#else
check_equals(r.toString(), undefined);
#endif

_global.TextField = backup;

// So if createTextField calls the TextField ctor, what does that constructor
// do?

// This only confirms that the TextField constructor a) removes the
// array typing, b) removes the relay, and c) doesn't produce a
// DisplayObject. We still don't have a way to check what happens
// inside createTextField.

CTF = function () {

  // We are called with 'new'.
  fun = ASnative(2, 0);
  xcheck_equals(fun(), true);

  backup = this;
  this.__proto__.__constructor__ = Array;
  super ();
  check_equals(this.length, 0);

  // It's not a proper TextField.
  this.__proto__.__constructor__ = TextField;
  super ();
  check_equals(backup, this);
  check_equals(this.length, 0);
  check_equals(this._x, undefined);
  check_equals(this._visible, undefined);
  check_equals(this._width, undefined);

  // It is no longer an array.
  this[2] = 3;
  check_equals(this.length, 0);
  check_equals(this[2], 3);

  this.__proto__.__constructor__ = Date;
  this.getTime = Date.prototype.getTime;
  super();
  check_equals(typeof(this.getTime()), "number");

  this.__proto__.__constructor__ = TextField;
  super();
  check_equals(typeof(this.getTime()), "undefined");

};

o = new CTF();

//------------------------------------------------------------
// END OF TESTS
//------------------------------------------------------------

#if OUTPUT_VERSION == 6
     check_totals(531);
#elif OUTPUT_VERSION == 7
 check_totals(555);
#elif OUTPUT_VERSION == 8
 check_totals(556);
#endif

#endif
