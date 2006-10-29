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

// Test case for TextSnapshot ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: TextSnapshot.as,v 1.5 2006/10/29 18:34:18 rsavoye Exp $";

#include "check.as"

var textsnapshotObj = new TextSnapshot;

// test the TextSnapshot constuctor
check (textsnapshotObj != undefined);

// test the TextSnapshot::findtext method
check (textsnapshotObj.findtext != undefined);
// test the TextSnapshot::getcount method
check (textsnapshotObj.getcount != undefined);
// test the TextSnapshot::getselected method
check (textsnapshotObj.getselected != undefined);
// test the TextSnapshot::getselectedtext method
check (textsnapshotObj.getselectedtext != undefined);
// test the TextSnapshot::gettext method
check (textsnapshotObj.gettext != undefined);
// test the TextSnapshot::hittesttextnearpos method
check (textsnapshotObj.hittesttextnearpos != undefined);
// test the TextSnapshot::setselectcolor method
check (textsnapshotObj.setselectcolor != undefined);
// test the TextSnapshot::setselected method
check (textsnapshotObj.setselected != undefined);
