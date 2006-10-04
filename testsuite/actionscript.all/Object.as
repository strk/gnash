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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
//

// Test case for Object ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: Object.as,v 1.8 2006/10/04 10:38:43 strk Exp $";

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


//----------------------
// Test addProperty
//----------------------

// the 'getter' function
function getLen() {
		return this._len;
}

// the 'setter' function
function setLen(l) {
		this._len = l;
}

// add the "len" property
var ret = obj3.addProperty("len", getLen, setLen);
check_equals(ret, true);

check_equals (obj3.len, undefined);
obj3._len = 3;
check_equals (obj3.len, 3);
obj3.len = 5;
check_equals (obj3._len, 5);
check_equals (obj3.len, 5);


//----------------------
// Test enumeration
//----------------------

function enumerate(obj, enum)
{
	var enumlen = 0;
	for (var i in obj) {
		enum[i] = obj[i];
		++enumlen;
	}
	return enumlen;
}

var l0 = new Object({a:1, b:2});
var l1 = new Object({c:3, d:4});
l1.__proto__ = l0;
var l2 = new Object({e:5, f:6});
l2.__proto__ = l1;

// check properties
var enum = new Object;
var enumlen = enumerate(l2, enum);
check_equals( enumlen, 6);
check_equals( enum["a"], 1);
check_equals( enum["b"], 2);
check_equals( enum["c"], 3);
check_equals( enum["d"], 4);
check_equals( enum["e"], 5);
check_equals( enum["f"], 6);

// Hide a property of a base object
var ret = ASSetPropFlags(l0, "a", 1);

var enum = new Object;
var enumlen = enumerate(l2, enum);
check_equals( enumlen, 5);
check_equals( enum["a"], undefined);

