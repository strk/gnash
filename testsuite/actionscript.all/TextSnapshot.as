// 
//   Copyright (C) 2005, 2006, 2007, 2009, 2010 Free Software
//   Foundation, Inc
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

// Test case for TextSnapshot ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

// TextSnapshot works only on static text, so this test file only tests
// AS compatibility (return, argument handling).


rcsid="TextSnapshot.as";
#include "check.as"

#if OUTPUT_VERSION < 6

 check_equals(typeof(TextSnapshot), "function");
 totals(1);

#else

 check(TextSnapshot.prototype.hasOwnProperty('findText'));
 check(TextSnapshot.prototype.hasOwnProperty('getCount'));
 check(TextSnapshot.prototype.hasOwnProperty('getSelected'));
 check(TextSnapshot.prototype.hasOwnProperty('getSelectedText'));
 check(TextSnapshot.prototype.hasOwnProperty('getText'));
 check(TextSnapshot.prototype.hasOwnProperty('setSelectColor'));
 check(TextSnapshot.prototype.hasOwnProperty('hitTestTextNearPos'));
 check(TextSnapshot.prototype.hasOwnProperty('setSelected'));

 check_equals( typeof(TextSnapshot), 'function' );
 
 var textsnapshotObj = new TextSnapshot;

 check(textsnapshotObj instanceof TextSnapshot);
 check(!textsnapshotObj.hasOwnProperty('findText'));
 check(!textsnapshotObj.hasOwnProperty('getCount'));
 check(!textsnapshotObj.hasOwnProperty('getSelected'));
 check(!textsnapshotObj.hasOwnProperty('getSelectedText'));
 check(!textsnapshotObj.hasOwnProperty('getText'));
 check(!textsnapshotObj.hasOwnProperty('setSelectColor'));
 check(!textsnapshotObj.hasOwnProperty('hitTestTextNearPos'));
 check(!textsnapshotObj.hasOwnProperty('setSelected'));

 // test the TextSnapshot constuctor
 check_equals( typeof(textsnapshotObj), 'object' );
 
 check_equals(typeof(textsnapshotObj.findText), 'function');
 check_equals(typeof(textsnapshotObj.getCount), 'function');
 check_equals(typeof(textsnapshotObj.getSelected), 'function');
 check_equals(typeof(textsnapshotObj.getSelectedText), 'function');
 check_equals (typeof(textsnapshotObj.getText), 'function');
 check_equals (typeof(textsnapshotObj.hitTestTextNearPos), 'function');
 check_equals (typeof(textsnapshotObj.setSelectColor), 'function');
 check_equals (typeof(textsnapshotObj.setSelected), 'function');

 gh = new TextSnapshot("hello");
 check_equals(gh.toString(), "[object Object]");
 check_equals(gh.getCount.toString(), "[type Function]");
 check_equals(gh.getCount(), undefined);

 o = {};
 gh = new TextSnapshot(o);
 check_equals(gh.toString(), "[object Object]");
 check_equals(gh.getCount.toString(), "[type Function]");
 check_equals(gh.getCount(), undefined);

 gh = new TextSnapshot(this);
 check_equals(gh.toString(), "[object Object]");
 check_equals(gh.getCount.toString(), "[type Function]");
 check_equals(gh.getCount(), 0);

 gh = new TextSnapshot(this, true);
 check_equals(gh.toString(), "[object Object]");
 check_equals(gh.getCount.toString(), "[type Function]");
 check_equals(gh.getCount(), undefined);

 ts = _root.getTextSnapshot();
 check(ts instanceof TextSnapshot);
 check(!ts.hasOwnProperty('findText'));
 check(!ts.hasOwnProperty('getCount'));
 check(!ts.hasOwnProperty('getSelected'));
 check(!ts.hasOwnProperty('getSelectedText'));
 check(!ts.hasOwnProperty('getText'));
 check(!ts.hasOwnProperty('setSelectColor'));
 check(!ts.hasOwnProperty('hitTestTextNearPos'));
 check(!ts.hasOwnProperty('setSelected'));

 // getText() and getCount()

 check_equals(typeof(ts.getCount()), "number");
 check_equals(typeof(ts.getCount(0)), "undefined");
 check_equals(typeof(ts.getCount("a")), "undefined");
 check_equals(typeof(ts.getCount(true)), "undefined");
 check_equals(typeof(ts.getCount(0, 1)), "undefined");
 check_equals(ts.getCount(), 0);

 check_equals(typeof(ts.findText()), "undefined");
 check_equals(typeof(ts.findText("a")), "undefined");
 check_equals(typeof(ts.findText(1)), "undefined");
 check_equals(typeof(ts.findText(1, "a")), "undefined");

 // Test with no text.
 check_equals(typeof(ts.findText(1, "a", true)), "number");
 check_equals(typeof(ts.findText(1, "a", 1)), "number");
 check_equals(typeof(ts.findText(1, "a", new Date())), "number");
 check_equals(typeof(ts.findText("6", "a", new Date())), "number");
 check_equals(typeof(ts.findText("b", "a", new Date())), "number");
 check_equals(typeof(ts.findText(-1, "a", new Date())), "number");
 check_equals(typeof(ts.findText(Infinity, "a", new Date())), "number");
 check_equals(typeof(ts.findText(-1, "a", new Date(), "e")), "undefined");
 check_equals(typeof(ts.findText(Infinity, "a", new Date(), 3)), "undefined");

 check_equals(ts.findText(1, "a", true), -1);
 check_equals(ts.findText(1, "a", 1), -1);
 check_equals(ts.findText(1, "a", new Date()), -1);
 check_equals(ts.findText("6", "a", false), -1);
 check_equals(ts.findText("b", "a", true), -1);
 check_equals(ts.findText(-1, "a", new Date()), -1);
 check_equals(ts.findText(Infinity, "a", new Date()), -1);

 // Shouldn't work with dynamic text.
 _root.createTextField("tf", 10, 30, 30, 100, 100);
 _root.tf.text = "ghjkab";
 ts = _root.getTextSnapshot();
 check_equals(ts.getCount(), 0);
 check_equals(ts.findText(1, "a", true), -1);
 check_equals(ts.findText(1, "a", false), -1);

 // getSelected

 check_equals(typeof(ts.getSelected()), "undefined");
 check_equals(typeof(ts.getSelected(0)), "undefined");
 check_equals(typeof(ts.getSelected("a")), "undefined");
 check_equals(typeof(ts.getSelected(new Date())), "undefined");
 check_equals(typeof(ts.getSelected([0, 1])), "undefined");
 check_equals(typeof(ts.getSelected([0, 1], 2)), "boolean");
 check_equals(typeof(ts.getSelected(0, 1)), "boolean");
 check_equals(typeof(ts.getSelected(1, 0)), "boolean");
 check_equals(typeof(ts.getSelected(-1, 3)), "boolean");
 check_equals(typeof(ts.getSelected(1, 0)), "boolean");
 check_equals(typeof(ts.getSelected(1, 0)), "boolean");
 check_equals(typeof(ts.getSelected(0, "a")), "boolean");
 check_equals(typeof(ts.getSelected("b", 0)), "boolean");
 check_equals(typeof(ts.getSelected(true, false)), "boolean");
 check_equals(typeof(ts.getSelected(0, 10, 10)), "undefined");
 check_equals(typeof(ts.getSelected(0, 10, true)), "undefined");
 check_equals(typeof(ts.getSelected(0, 10, "a")), "undefined");

 check_equals(typeof(ts.getSelectedText()), "string");
 check_equals(typeof(ts.getSelectedText(0)), "string");
 check_equals(typeof(ts.getSelectedText("a")), "string");
 check_equals(typeof(ts.getSelectedText(new Date())), "string");
 check_equals(typeof(ts.getSelectedText([0, 2])), "string");
 check_equals(typeof(ts.getSelectedText(0, 1)), "undefined");
 check_equals(typeof(ts.getSelectedText(1, 0)), "undefined");
 check_equals(typeof(ts.getSelectedText(-1, 3)), "undefined");
 check_equals(typeof(ts.getSelectedText(1, 0)), "undefined");
 check_equals(typeof(ts.getSelectedText(1, 0)), "undefined");
 check_equals(typeof(ts.getSelectedText(0, "a")), "undefined");
 check_equals(typeof(ts.getSelectedText("b", 0)), "undefined");
 check_equals(typeof(ts.getSelectedText(true, false)), "undefined");
 check_equals(typeof(ts.getSelectedText(0, 10, 10)), "undefined");
 check_equals(typeof(ts.getSelectedText(0, 10, true)), "undefined");
 check_equals(typeof(ts.getSelectedText(0, 10, "a")), "undefined");

 check_equals(typeof(ts.getText()), "undefined");
 check_equals(typeof(ts.getText(0)), "undefined");
 check_equals(typeof(ts.getText("a")), "undefined");
 check_equals(typeof(ts.getText(new Date())), "undefined");
 check_equals(typeof(ts.getText(0, 1)), "string");
 check_equals(typeof(ts.getText(1, 0)), "string");
 check_equals(typeof(ts.getText(-1, 3)), "string");
 check_equals(typeof(ts.getText(1, 0)), "string");
 check_equals(typeof(ts.getText(1, 0)), "string");
 check_equals(typeof(ts.getText(0, "a")), "string");
 check_equals(typeof(ts.getText("b", 0)), "string");
 check_equals(typeof(ts.getText(true, false)), "string");
 check_equals(typeof(ts.getText(0, 10, 10)), "string");
 check_equals(typeof(ts.getText(0, 10, true)), "string");
 check_equals(typeof(ts.getText(0, 10, "a", 11)), "undefined");
 check_equals(typeof(ts.getText(0, 10, 10, "hello")), "undefined");
 check_equals(typeof(ts.getText(0, 10, true, [3, 4])), "undefined");

 // setSelectColor(). Returns void
 check_equals(typeof(ts.setSelectColor()), "undefined");
 check_equals(typeof(ts.setSelectColor(0)), "undefined");
 check_equals(typeof(ts.setSelectColor(0, 4)), "undefined");
 check_equals(typeof(ts.setSelectColor(0, 5, 6)), "undefined");
 check_equals(typeof(ts.setSelectColor(0, 5, true)), "undefined");
 check_equals(typeof(ts.setSelectColor(0, 5, 8, 5)), "undefined");

 // hitTestTextNearPos()
 check_equals(typeof(ts.hitTestTextNearPos()), "undefined");
 check_equals(typeof(ts.hitTestTextNearPos(0)), "undefined");
 check_equals(typeof(ts.hitTestTextNearPos("a")), "undefined");
 check_equals(typeof(ts.hitTestTextNearPos(new Date())), "undefined");
 xcheck_equals(typeof(ts.hitTestTextNearPos(0, 1)), "number");
 xcheck_equals(typeof(ts.hitTestTextNearPos(1, 0)), "number");
 xcheck_equals(typeof(ts.hitTestTextNearPos(-1, 3)), "number");
 xcheck_equals(typeof(ts.hitTestTextNearPos(1, 0)), "number");
 xcheck_equals(typeof(ts.hitTestTextNearPos(1, 0)), "number");
 xcheck_equals(typeof(ts.hitTestTextNearPos(0, "a")), "number");
 xcheck_equals(typeof(ts.hitTestTextNearPos("b", 0)), "number");
 xcheck_equals(typeof(ts.hitTestTextNearPos(true, false)), "number");
 xcheck_equals(typeof(ts.hitTestTextNearPos(0, 10, 10)), "number");
 xcheck_equals(typeof(ts.hitTestTextNearPos(0, 10, true)), "number");
 check_equals(typeof(ts.hitTestTextNearPos(0, 10, "a", 11)), "undefined");
 check_equals(typeof(ts.hitTestTextNearPos(0, 10, 10, "hello")), "undefined");
 check_equals(typeof(ts.hitTestTextNearPos(0, 10, true, [3, 4])), "undefined");
 
 // setSelected() // Three arguments, returns void.
 check_equals(typeof(ts.setSelected()), "undefined");
 check_equals(typeof(ts.setSelected(0)), "undefined");
 check_equals(typeof(ts.setSelected("a")), "undefined");
 check_equals(typeof(ts.setSelected(new Date())), "undefined");
 check_equals(typeof(ts.setSelected(0, 1)), "undefined");
 check_equals(typeof(ts.setSelected(1, 0)), "undefined");
 check_equals(typeof(ts.setSelected(-1, 3)), "undefined");
 check_equals(typeof(ts.setSelected(1, 0)), "undefined");
 check_equals(typeof(ts.setSelected(1, 0)), "undefined");
 check_equals(typeof(ts.setSelected(0, "a")), "undefined");
 check_equals(typeof(ts.setSelected("b", 0)), "undefined");
 check_equals(typeof(ts.setSelected(true, false)), "undefined");
 check_equals(typeof(ts.setSelected(0, 10, 10)), "undefined");
 check_equals(typeof(ts.setSelected(0, 10, true)), "undefined");
 check_equals(typeof(ts.setSelected(0, 10, "a", 11)), "undefined");
 check_equals(typeof(ts.setSelected(0, 10, 10, "hello")), "undefined");
 check_equals(typeof(ts.setSelected(0, 10, true, [3, 4])), "undefined");

 totals(167);

#endif // OUTPUT_VERSION > 5
