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

// Test case for Object ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

#include "check.as"

// Test Object creation using 'new'
var obj = new Object; // uses SWFACTION_NEWOBJECT
check (obj != undefined);
check (typeof(obj) == "object");

// Test instantiated Object members
obj.member = 1;
check (obj.member == 1)

// Test Object creation using literal initialization
var obj2 = { member:1 }; // uses SWFACTION_INITOBJECT
check (obj2 != undefined );
check (typeof(obj2) == "object");

// Test initialized object members
check ( obj2.member == 1 )

// Test Object creation using initializing constructor
var obj3 = new Object({ member:1 });
check (obj3 != undefined);
check (typeof(obj3) == "object");

// Test initialized object members
check ( obj3.member != undefined );
check ( obj3.member == 1 );

// Test after-initialization members set/get
obj3.member2 = 3;
check ( obj3.member2 != undefined );
check ( obj3.member2 == 3 );
