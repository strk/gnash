// 
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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

note();
note("NOTE: for this test to work you should have copied");
note("      testsuite/misc-ming.all/SharedObjectTest.sol/*.sol");
note("      to this flash player's appropriate SharedObject dir");
note();

so1 = SharedObject.getLocal("sol1");

check_equals(so1.data.num, 2);
check_equals(so1.data.str, 'a string');
check_equals(typeof(so1.data.tbool), 'boolean');
check_equals(so1.data.tbool, true);
check_equals(typeof(so1.data.fbool), 'boolean');
check_equals(so1.data.fbool, false);

// Test reading STRICT_ARRAY, NULL and UNDEFINED
xcheck_equals(typeof(so1.data.ary), 'object');
xcheck_equals(so1.data.ary.toString(), '1,true,string,null,');
xcheck_equals(typeof(so1.data.ary[0]), 'number');
xcheck_equals(typeof(so1.data.ary[1]), 'boolean');
xcheck_equals(typeof(so1.data.ary[2]), 'string');
xcheck_equals(typeof(so1.data.ary[3]), 'null');
check_equals(typeof(so1.data.ary[4]), 'undefined');
xcheck_equals(so1.data.ary.length, 5);

// Test reading ECMA_ARRAY
xcheck_equals(typeof(so1.data.aryns), 'object', 'aryns was not read from .sol');
xcheck_equals(so1.data.aryns.toString(), '4,5,6,,,,,');
xcheck_equals(so1.data.aryns.length, 8);
xcheck_equals(so1.data.aryns.custom, 7);

// Test reading OBJECT
xcheck_equals(typeof(so1.data.obj), 'object');
xcheck_equals(typeof(so1.data.obj.a), 'number');

// Test reading NUMBER
xcheck_equals(so1.data.obj.a, 10);

// Test reading STRING
xcheck_equals(typeof(so1.data.obj.b), 'string');
xcheck_equals(so1.data.obj.b, '20');

// Test reading BOOLEAN
xcheck_equals(typeof(so1.data.obj.c), 'boolean');
xcheck_equals(so1.data.obj.c, true);

// Test reading REFERENCE
xcheck_equals(typeof(so1.data.ref), 'object');
check_equals(so1.data.ref, so1.data.obj); 

// force writing the sol or the adobe player won't save it
// again. This will also serve as a kind of reference for
// how the sol file was produced in the first place
so1.data.num = 2; 
so1.data.str = 'a string'; 
so1.data.tbool = true;
so1.data.fbool = false;
so1.data.ary = [1,true,'string',null, undefined]; // strict array (STRICT_ARRAY)
so1.data.aryns = [4,5,6]; so1.data.aryns.custom = 7; so1.data.aryns.length = 8; // non-strict array (ECMA_ARRAY)
so1.data.obj = {a:10,b:'20',c:true};
so1.data.ref = so1.data.obj;

so1.flush();

quit = function()
{
	loadMovie('fscommand:quit', '');
};

note(" -  Will quit in 5 seconds, feel free to explicitly quit otherwise -");
note();

setInterval(quit, 5000);
stop();

totals();
