// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License

// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

//

// Test case for Selection ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: Selection.as,v 1.6 2006/10/29 18:34:18 rsavoye Exp $";

#include "check.as"

var selectionObj = new Selection;

// test the Selection constuctor
check (selectionObj != undefined);

// test the Selection::addlistener method
check (selectionObj.addlistener != undefined);
// test the Selection::getbeginindex method
check (selectionObj.getbeginindex != undefined);
// test the Selection::getcaretindex method
check (selectionObj.getcaretindex != undefined);
// test the Selection::getendindex method
check (selectionObj.getendindex != undefined);
// test the Selection::getfocus method
check (selectionObj.getfocus != undefined);
// test the Selection::removelistener method
check (selectionObj.removelistener != undefined);
// test the Selection::setfocus method
check (selectionObj.setfocus != undefined);
// test the Selection::set method
xcheck (selectionObj.set != undefined);
