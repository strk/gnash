// 
//   Copyright (C) 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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
//
// NOTE: to test this with the proprietary player:
//
//     $ make SharedObjectTestRunner
//     $ ./SharedObjectTestRunner flashplayer ~/.macromedia/Flash_Player/#SharedObjects/<key>
//     
// take a look at the #SharedObject dir to figure out what <key> should be
// 
//
//

#define pass_check(x) _root.pass(x)
#define xpass_check(x) _root.xpass(x)
#define fail_check(x) _root.fail(x)
#define xfail_check(x) _root.xfail(x)

#define USE_DEJAGNU_MODULE
#include "../actionscript.all/check.as"

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

// MovieClip value mc was NOT discarded, but written (or read?) as undefined
check(so1.data.hasOwnProperty('mc'));
check_equals(typeof(so1.data.mc), 'undefined');

// Function value was discarded
check(! so1.data.hasOwnProperty('fun') );


// Test reading mixed types in ECMA_ARRAY 
check_equals(typeof(so1.data.ary), 'object');
check_equals(so1.data.ary.toString(), '1,true,string,null,');
check_equals(typeof(so1.data.ary[0]), 'number');
check_equals(typeof(so1.data.ary[1]), 'boolean');
check_equals(typeof(so1.data.ary[2]), 'string');
check_equals(typeof(so1.data.ary[3]), 'null');
check_equals(typeof(so1.data.ary[4]), 'undefined');
check_equals(so1.data.ary.length, 5);
// test composition
tmp = so1.data.ary; // work-around to an old Ming bug [ chokes on 'for (in in a.b.c)' ]
a=[]; for (i in tmp) a.push(i);
a.sort();
check_equals(a.toString(), '0,1,2,3,4'); // note: no 'length'

// Test reading ECMA_ARRAY
check_equals(typeof(so1.data.aryns), 'object');
check_equals(so1.data.aryns.toString(), '4,5,6,,,,,');
check_equals(so1.data.aryns.length, 8);
check_equals(so1.data.aryns.custom, 7);
// test composition
tmp = so1.data.aryns; // work-around to an old Ming bug [ chokes on 'for (in in a.b.c)' ]
a=[]; for (i in tmp) a.push(i);
a.sort();
check_equals(a.toString(), '0,1,2,custom'); // note: no 'length'

// Test reading STRICT_ARRAY
check_equals(typeof(so1.data.strictary), 'object');
check_equals(so1.data.strictary.toString(), 'a,b,c');
check_equals(so1.data.strictary.length, 3);

// Test reading OBJECT
check(so1.data.obj instanceOf Object);
check_equals(typeof(so1.data.obj), 'object');
check_equals(typeof(so1.data.obj.a), 'number');
check(so1.data.obj.hasOwnProperty('a'));
check(!so1.data.obj.hasOwnProperty('hidden'));
check(so1.data.obj.hasOwnProperty('mc'));
check_equals(typeof(so1.data.obj.mc), 'undefined');
// Function value was discarded
check(! so1.data.obj.hasOwnProperty('fun') );

// Test reading NUMBER
check_equals(so1.data.obj.a, 10);

// Test reading STRING
check_equals(typeof(so1.data.obj.b), 'string');
check_equals(so1.data.obj.b, '20');

// Test reading BOOLEAN
check_equals(typeof(so1.data.obj.c), 'boolean');
check_equals(so1.data.obj.c, true);

// Test reading REFERENCE
check_equals(typeof(so1.data.ref), 'object');
check_equals(so1.data.ref, so1.data.obj); 

// Test reading DATE
check_equals(typeof(so1.data.dat), 'object');
check(so1.data.dat instanceof Date);
check_equals(so1.data.dat.getYear(), 70);
check_equals(so1.data.dat.getFullYear(), 1970);
check_equals(so1.data.dat.getMonth(), 0);
check_equals(so1.data.dat.getDate(), 1);
check_equals(so1.data.dat.getDay(), 4);	// It was a Thursday
check_equals(so1.data.dat.getHours(), 0);
check_equals(so1.data.dat.getMinutes(), 0);
check_equals(so1.data.dat.getSeconds(), 0);
check_equals(so1.data.dat.getMilliseconds(), 0);

// Test reading LONG STRING
lstr = 'a';
for (var i=0; i<16; ++i) lstr = lstr+lstr; // 65536 long
check_equals(so1.data.lstr, lstr);


// force writing the sol or the adobe player won't save it
// again. This will also serve as a kind of reference for
// how the sol file was produced in the first place
so1.data.num = 2; 
so1.data.str = 'a string'; 
so1.data.tbool = true;
so1.data.fbool = false;

so1.data.ary = [1,true,'string',null, undefined];  // ECMA_ARRAY
so1.data.ary.hidden = 6;
AsSetPropFlags(so1.data.ary, 'hidden', 1); // hide from enumeration, should not end into the sol file


so1.data.aryns = [4,5,6];
so1.data.aryns.fun = function() {}; // functions in arrays are simply skipped
so1.data.aryns.custom = 7;
so1.data.aryns.length = 8; // non-strict array (ECMA_ARRAY)

// This member is an attempt to trigger encoding
// of a STRICT array. The attempt seems to be
// unsuccessful tough, but no way to test with 
// ActionScript. Time to look at the binary differences
// between the pp and gnash versions of the saved sol.
so1.data.strictary = ['a','b','c'];

so1.data.obj = {a:10,b:'20',c:true};
so1.data.obj.fun = function() {}; // functions in objects are simply skipped
so1.data.obj.mc = createEmptyMovieClip("mc1", 1); // movieclip values are skipped

//AsSetPropFlags(so1.data.obj, '__proto__', 0, 1); // if we unhide __proto__ we'll find it in the SharedObject

AsSetPropFlags(so1.data.obj, '__constructor__', 0, 1); // unhide __constructor__ (it's a function so will be skipped anyway)
AsSetPropFlags(so1.data.obj, 'constructor', 0, 1); // unhide constructor (it's a function so will be skipped anyway)
// so1.data.obj.constructor = 4; // if we override constructor we'll find it

so1.data.obj.hidden = 7;
AsSetPropFlags(so1.data.obj, 'hidden', 1); // hide from enumeration, should not end into the sol file

so1.data.ref = so1.data.obj;

so1.data.fun = function() {}; // functions in data 
so1.data.mc = createEmptyMovieClip("mc2", 2); // movieclip values are skipped

//AsSetPropFlags(so1.data, '__proto__', 0, 1); // if we unhide __proto__ we'll find it in the SharedObject
AsSetPropFlags(so1.data, '__constructor__', 0, 1); // unhide __constructor__ (it's a function so will be skipped anyway)
AsSetPropFlags(so1.data, 'constructor', 0, 1); // unhide constructor (it's a function so will be skipped anyway)

so1.data.dat = new Date(70,0); // 1 Jan 1970 00:00:00 localtime

so1.data.lstr = lstr;

f = new Date(0);
f.valueOf = function() { return "Overridden date"; };
so1.data.fakedate = f;

g = new Date(0);
g.valueOf = function() { return 35; };
so1.data.fakedate2 = g;

so1.flush();

quit = function()
{
	loadMovie('fscommand:quit', '');
};

note(" -  Will quit in 5 seconds, feel free to explicitly quit otherwise -");
note();

setInterval(quit, 5000);
stop();

check_totals(53);
