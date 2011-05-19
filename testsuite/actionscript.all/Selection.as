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

// Test case for Selection ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="Selection.as";
#include "check.as"

//-------------------------------
// Selection was added in SWF5
//-------------------------------

note("If you click on the window or change focus before or during this test, some checks may fail.");

check_equals (typeof(Selection), 'object');

// Selection is an obect, not a class !
var selectionObj = new Selection;
check_equals (typeof(selectionObj), 'undefined');

// test the Selection::getbeginindex method
check_equals (typeof(Selection.getBeginIndex), 'function');

// test the Selection::getcaretindex method
check_equals (typeof(Selection.getCaretIndex), 'function');

// test the Selection::getendindex method
check_equals (typeof(Selection.getEndIndex), 'function');

// test the Selection::getfocus method
check_equals (typeof(Selection.getFocus), 'function');

// test the Selection::setfocus method
check_equals (typeof(Selection.setFocus), 'function');

// test the Selection::setSelection method
check_equals (typeof(Selection.setSelection), 'function'); 

check_equals(typeof(Selection.getFocus()), "null");

ret = Selection.setFocus();
check_equals(ret, false);

ret = Selection.setFocus(4);
check_equals(ret, false);

ret = Selection.setFocus(_root);
check_equals(ret, false);

ret = Selection.setFocus(_root, 5);
check_equals(ret, false);

_root.focusEnabled = false;
ret = Selection.setFocus(_root, 5);
check_equals(ret, false);

// Methods added in version 6
#if OUTPUT_VERSION >= 6

 // Selection was implicitly initialized by ASBroadcaster.initialize !
 // See http://www.senocular.com/flash/tutorials/listenersasbroadcaster/?page=2
 check_equals (typeof(Selection.addListener), 'function');
 check_equals (typeof(Selection.removeListener), 'function'); 
 check_equals(typeof(Selection.broadcastMessage), 'function');
 check(Selection.hasOwnProperty("_listeners"));
 check_equals(typeof(Selection._listeners), 'object');
 check(Selection._listeners instanceof Array);

 _root.createEmptyMovieClip("mc", getNextHighestDepth());
 check(mc instanceof MovieClip);
 ret = Selection.setFocus(mc);
 check_equals(ret, false);
 check_equals(Selection.getFocus(), null);

 tx = undefined;

 check_equals(Selection.getFocus(), null);
 ret = Selection.setFocus(tx);
 check_equals(typeof(ret), "boolean");
 check_equals(ret, true);

 // An extra argument when the first argument is valid.
 ret = Selection.setFocus(tx, 5);
 check_equals(ret, false);
 check_equals(Selection.getFocus(), null);

 tx.focusEnabled = true;
 check_equals(Selection.getFocus(), null);
 ret = Selection.setFocus(tx);
 check_equals(ret, true);
 check_equals(Selection.getFocus(), null);
 ret = Selection.setFocus("tx");
 check_equals(ret, false);
 check_equals(Selection.getFocus(), null);

 mc.focusEnabled = true;
 check_equals(Selection.getFocus(), null);
 ret = Selection.setFocus(mc);
 check_equals(ret, false);
 check_equals(Selection.getFocus(), "_level0.mc");
 ret = Selection.setFocus("mc");
 check_equals(ret, false);
 check_equals(Selection.getFocus(), "_level0.mc");
 ret = Selection.setFocus(5);
 check_equals(ret, false);
 check_equals(Selection.getFocus(), "_level0.mc");

 // Setting _visible to false removes focus, otherwise visibility seems
 // to have no effect.
 mc._visible = false;
 check_equals(mc._visible, false);
 check_equals(Selection.getFocus(), null);
 ret = Selection.setFocus(mc);
 check_equals(ret, false);
 check_equals(Selection.getFocus(), "_level0.mc");
 mc._visible = false;
 check_equals(Selection.getFocus(), "_level0.mc");

 ret = Selection.setFocus(null);
 check_equals(ret, true);
 check_equals(Selection.getFocus(), null);

 ret = Selection.setFocus(mc);
 check_equals(ret, false);
 mc._visible = false;
 check_equals(Selection.getFocus(), "_level0.mc");
 mc._visible = true;
 check_equals(Selection.getFocus(), "_level0.mc");
 mc._visible = false;
 check_equals(Selection.getFocus(), null);

 // Check setting mouse events.
 Selection.setFocus(mc);
 check_equals(Selection.getFocus(), "_level0.mc");
 mc.onRelease = function() {};
 check_equals(Selection.getFocus(), "_level0.mc");
 Selection.setFocus(mc);
 check_equals(Selection.getFocus(), "_level0.mc");

 // Indices of MovieClip focus.
 ret = Selection.getBeginIndex();
 check_equals(ret, -1);
 ret = Selection.getEndIndex();
 check_equals(ret, -1);
 ret = Selection.getCaretIndex();
 check_equals(ret, -1);


 Selection.setFocus(tx);
 check_equals(Selection.getFocus(), null);
 ret = Selection.setFocus("tx");
 check_equals(ret, false);
 check_equals(Selection.getFocus(), null);
 ret = Selection.setFocus(tx);
 check_equals(ret, true);
 check_equals(Selection.getFocus(), null);

 // Indices of null focus.
 ret = Selection.getBeginIndex();
 check_equals(ret, -1);
 ret = Selection.getEndIndex();
 check_equals(ret, -1);
 ret = Selection.getCaretIndex();
 check_equals(ret, -1);

 createTextField('text1', 99, 10, 10, 10, 10);
 ret = Selection.setFocus(text1);
 check_equals(ret, false);
 check_equals(Selection.getFocus(), '_level0.text1');

 ret = Selection.getBeginIndex();
 check_equals(ret, 0);
 ret = Selection.getEndIndex();
 check_equals(ret, 0);
 ret = Selection.getCaretIndex();
 check_equals(ret, 0);

 // setSelection
 ret = Selection.setSelection(0, 1);
 check_equals(ret, undefined);
 ret = Selection.getBeginIndex();
 check_equals(ret, 0);
 ret = Selection.getEndIndex();
 check_equals(ret, 0);
 ret = Selection.getCaretIndex();
 check_equals(ret, 0);
 
 text1.text = "Some Text";
 ret = Selection.setSelection(0, 1);
 check_equals(ret, undefined);
 ret = Selection.getBeginIndex();
 check_equals(ret, 0);
 ret = Selection.getEndIndex();
 check_equals(ret, 1);
 ret = Selection.getCaretIndex();
 check_equals(ret, 1);

 ret = Selection.setSelection(4, -5);
 check_equals(ret, undefined);
 ret = Selection.getBeginIndex();
 check_equals(ret, 0);
 ret = Selection.getEndIndex();
 check_equals(ret, 4);
 ret = Selection.getCaretIndex();
 check_equals(ret, 0);

 ret = Selection.setSelection(6, 3);
 check_equals(ret, undefined);
 ret = Selection.getBeginIndex();
 check_equals(ret, 3);
 ret = Selection.getEndIndex();
 check_equals(ret, 6);
 ret = Selection.getCaretIndex();
 check_equals(ret, 3);

 ret = Selection.setSelection(1, 0);
 check_equals(ret, undefined);
 ret = Selection.getBeginIndex();
 check_equals(ret, 0);
 ret = Selection.getEndIndex();
 check_equals(ret, 1);
 ret = Selection.getCaretIndex();
 check_equals(ret, 0);

 ret = Selection.setSelection(2, 6);
 check_equals(ret, undefined);
 ret = Selection.getBeginIndex();
 check_equals(ret, 2);
 ret = Selection.getEndIndex();
 check_equals(ret, 6);
 ret = Selection.getCaretIndex();
 check_equals(ret, 6);

 ret = Selection.setSelection(3);
 check_equals(ret, undefined);
 ret = Selection.getBeginIndex();
 check_equals(ret, 2);
 ret = Selection.getEndIndex();
 check_equals(ret, 6);
 ret = Selection.getCaretIndex();
 check_equals(ret, 6);

 ret = Selection.setSelection(2, 25);
 check_equals(ret, undefined);
 ret = Selection.getBeginIndex();
 check_equals(ret, 2);
 ret = Selection.getEndIndex();
 check_equals(ret, 9);
 ret = Selection.getCaretIndex();
 check_equals(ret, 9);

 ret = Selection.setSelection(-1, 4);
 check_equals(ret, undefined);
 ret = Selection.getBeginIndex();
 check_equals(ret, 0);
 ret = Selection.getEndIndex();
 check_equals(ret, 4);
 ret = Selection.getCaretIndex();
 check_equals(ret, 4);

 ret = Selection.setSelection(1, 1);
 check_equals(ret, undefined);
 ret = Selection.getBeginIndex();
 check_equals(ret, 1);
 ret = Selection.getEndIndex();
 check_equals(ret, 1);
 ret = Selection.getCaretIndex();
 check_equals(ret, 1);

 ret = Selection.setSelection(1, 2, 3);
 check_equals(ret, undefined);
 ret = Selection.getBeginIndex();
 check_equals(ret, 1);
 ret = Selection.getEndIndex();
 check_equals(ret, 1);
 ret = Selection.getCaretIndex();
 check_equals(ret, 1);

 Selection.setFocus(text1);
 text1.text = "Some relatively long text";
 Selection.setSelection(10, 15);
 check_equals(Selection.getBeginIndex(), 10);
 text1.text = "Some slightly shorter te";
 check_equals(Selection.getBeginIndex(), 10);
 text1.text = "too short";
 check_equals(Selection.getBeginIndex(), 9);
 text1.text = "";
 check_equals(Selection.getBeginIndex(), 0);
 // This triggers a bug in Gnash 0.8.8 due to 
 // the selection being outside the text bounds.
 text1.replaceSel("hello");

#endif // OUTPUT_VERSION >= 6

totals();
