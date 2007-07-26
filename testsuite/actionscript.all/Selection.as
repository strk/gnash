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

rcsid="$Id: Selection.as,v 1.10 2007/07/26 03:41:19 strk Exp $";

#include "check.as"

//-------------------------------
// Selection was added in SWF5
//-------------------------------

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

// Methods added in version 6
#if OUTPUT_VERSION >= 6

// test the Selection::addListener method
check_equals (typeof(Selection.addListener), 'function');

// test the Selection::removeListener method
check_equals (typeof(Selection.removeListener), 'function'); 

#endif // OUTPUT_VERSION >= 6

