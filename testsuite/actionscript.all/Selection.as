// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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


rcsid="$Id: Selection.as,v 1.15 2008/03/11 19:31:48 strk Exp $";
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
 xcheck_equals(typeof(Selection.broadcastMessage), 'function');
 xcheck(Selection.hasOwnProperty("_listeners"));
 xcheck_equals(typeof(Selection._listeners), 'object');
 xcheck(Selection._listeners instanceof Array);

 _root.createEmptyMovieClip("mc", getNextHighestDepth());
 check(mc instanceof MovieClip);
 ret = Selection.setFocus(mc);
 check_equals(ret, false);
 check_equals(Selection.getFocus(), null);

 mc.createTextField("tx", getNextHighestDepth(), 400, 400, 10, 10);

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


 Selection.setFocus(tx);
 check_equals(Selection.getFocus(), null);
 ret = Selection.setFocus("tx");
 check_equals(ret, false);
 check_equals(Selection.getFocus(), null);
 ret = Selection.setFocus(tx);
 check_equals(ret, true);
 check_equals(Selection.getFocus(), null);

#endif // OUTPUT_VERSION >= 6

totals();
