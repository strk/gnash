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

// Test case for SharedObject ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="SharedObject.as";
#include "check.as"

var sharedobjectObj = new SharedObject;

// test the SharedObject constuctor
check_equals (typeof(sharedobjectObj), 'object');


// test the SharedObject.getlocal method
check_equals (typeof(sharedobjectObj.getLocal), 'undefined');
check_equals (typeof(SharedObject.getLocal), 'function');

#if OUTPUT_VERSION > 5

check(SharedObject.prototype.hasOwnProperty("connect"));
check(SharedObject.prototype.hasOwnProperty("send"));
check(SharedObject.prototype.hasOwnProperty("flush"));
check(SharedObject.prototype.hasOwnProperty("getSize"));
check(SharedObject.prototype.hasOwnProperty("setFps"));
check(SharedObject.prototype.hasOwnProperty("clear"));
check(SharedObject.prototype.hasOwnProperty("close"));
check(!SharedObject.prototype.hasOwnProperty("data"));

check(!SharedObject.prototype.hasOwnProperty("getLocal"));
check(!SharedObject.prototype.hasOwnProperty("getRemote"));
check(!SharedObject.prototype.hasOwnProperty("getDiskUsage"));
check(!SharedObject.prototype.hasOwnProperty("deleteAll"));
check(SharedObject.hasOwnProperty("getLocal"));
check(SharedObject.hasOwnProperty("getRemote"));
check(SharedObject.hasOwnProperty("getDiskUsage"));
check(SharedObject.hasOwnProperty("deleteAll"));

check_equals (typeof(sharedobjectObj.connect), 'function');
check_equals (typeof(sharedobjectObj.send), 'function');
check_equals (typeof(sharedobjectObj.flush), 'function');
check_equals (typeof(sharedobjectObj.close), 'function');
check_equals (typeof(sharedobjectObj.getSize), 'function');
check_equals (typeof(sharedobjectObj.setFps), 'function');
check_equals (typeof(sharedobjectObj.clear), 'function');
check_equals (typeof(sharedobjectObj.data), 'undefined');
#else
check_equals (typeof(sharedobjectObj.connect), 'undefined');
check_equals (typeof(sharedobjectObj.send), 'undefined');
check_equals (typeof(sharedobjectObj.flush), 'undefined');
check_equals (typeof(sharedobjectObj.close), 'undefined');
check_equals (typeof(sharedobjectObj.getSize), 'undefined');
check_equals (typeof(sharedobjectObj.setFps), 'undefined');
check_equals (typeof(sharedobjectObj.clear), 'undefined');
#endif

// FIXME: Test code that will soon be a formal test case.
so = SharedObject.getLocal("level1/level2/settings", "/");
#if OUTPUT_VERSION > 5
check_equals (so.getSize(), 297);
#endif
check(so instanceof SharedObject);

// Private data
so.name = "Joe";
so.age = 20;
so.pet = "Dog";
#if OUTPUT_VERSION > 5
check_equals (so.getSize(), 297);
#endif
// public data that gets written
#if OUTPUT_VERSION > 5
check(so.hasOwnProperty("data"));
check(!so.data.hasOwnProperty("toString"));
check(!so.data.hasOwnProperty("valueOf"));
check_equals(typeof(so.data.valueOf), "function");
#endif

so.data.gain = 50.0;
so.data.echosuppression = false;
so.data.defaultmicrophone = "/dev/input/mic";
so.data.defaultcamera = "";
so.data.defaultklimit = 100.0;
so.data.defaultalways = false;
so.data.crossdomainAllow = true;
so.data.crossdomainAlways = true;
so.data.allowThirdPartyLSOAccess = true;
so.data.localSecPath = "";
so.data.localSecPathTime = 1.19751160683e+12;


// Verify that a new getLocal call using
// the same "id" returns the same in-memory object.
so.data.tmp = "custom value";
so2 = SharedObject.getLocal("level1/level2/settings", "/");
check_equals(so2.data.tmp, "custom value");
check_equals(so2.data.toString(), "[object Object]");
check_equals(so, so2);

#if OUTPUT_VERSION > 5
check_equals (so2.getSize(), 318);
check_equals (so.getSize(), 318);
#endif

// Check SOL names validity.
so2bis = SharedObject.getLocal("level1//level2/settings", "/");
check_equals(typeof(so2bis), 'null'); // invalid path
so2bis = SharedObject.getLocal("a");
check_equals(typeof(so2bis), 'object'); 
so2bis = SharedObject.getLocal("a~");
check_equals(typeof(so2bis), 'null'); 
so2bis = SharedObject.getLocal("a ");
check_equals(typeof(so2bis), 'null'); 
so2bis = SharedObject.getLocal("a'");
check_equals(typeof(so2bis), 'null'); 
so2bis = SharedObject.getLocal("%");
check_equals(typeof(so2bis), 'object'); 
so2bis = SharedObject.getLocal("a%");
check_equals(typeof(so2bis), 'object'); 
so2bis = SharedObject.getLocal("a&");
check_equals(typeof(so2bis), 'null'); 
so2bis = SharedObject.getLocal("a\\");
check_equals(typeof(so2bis), 'null'); 
so2bis = SharedObject.getLocal("a;");
check_equals(typeof(so2bis), 'null'); 
so2bis = SharedObject.getLocal("a:");
check_equals(typeof(so2bis), 'null'); 
so2bis = SharedObject.getLocal("a\"");
check_equals(typeof(so2bis), 'null'); 
so2bis = SharedObject.getLocal("a,");
check_equals(typeof(so2bis), 'null'); 
so2bis = SharedObject.getLocal("a>");
check_equals(typeof(so2bis), 'null'); 
so2bis = SharedObject.getLocal("a<");
check_equals(typeof(so2bis), 'null'); 
so2bis = SharedObject.getLocal("a#");
check_equals(typeof(so2bis), 'null'); 
so2bis = SharedObject.getLocal("a?");
check_equals(typeof(so2bis), 'null'); 
so2bis = SharedObject.getLocal("a(");
check_equals(typeof(so2bis), 'object'); 
so2bis = SharedObject.getLocal("a)");
check_equals(typeof(so2bis), 'object'); 
so2bis = SharedObject.getLocal("a{");
check_equals(typeof(so2bis), 'object'); 
so2bis = SharedObject.getLocal("a}");
check_equals(typeof(so2bis), 'object'); 
so2bis = SharedObject.getLocal("a$");
check_equals(typeof(so2bis), 'object'); 
so2bis = SharedObject.getLocal("a!");
check_equals(typeof(so2bis), 'object'); 
so2bis = SharedObject.getLocal("ü");
check_equals(typeof(so2bis), 'object');
so2bis = SharedObject.getLocal("a*");
check_equals(typeof(so2bis), 'object'); 

so2bis = SharedObject.getLocal("level1/./level2/settings", "/");
check_equals(typeof(so2bis), 'object'); // valid path
check(so2bis != so2); // but not recognized as the same as level1/./level2/settings

delete so.data.tmp;

#if OUTPUT_VERSION > 5
check_equals(so.getSize(), 297);
#endif 

// But a getLocal call using a *different* "id" returns
// a different SharedObject...
so3 = SharedObject.getLocal("level1/level2/settings3", "/");
check(so3 != so);

#if OUTPUT_VERSION > 5
check_equals (so3.getSize(), 0);
#endif

// Doesn't make much sense to test the rest for SWF5
#if OUTPUT_VERSION > 5

// trace(so.getSize());
ret = so.flush();
check_equals(typeof(ret), 'boolean');
check_equals(ret, true);

newso = SharedObject.getLocal("level1/level2/settings", "/");
check_equals (typeof(newso), 'object');
trace(newso.getSize());
#if OUTPUT_VERSION > 5
check_equals (newso.getSize(), 297);
#endif

if (typeof(newso.data) != 'undefined') {
    trace("New Shared Object, checking data...");
    trace(typeof(newso.data.gain));
    check_equals (typeof(newso.data.gain), 'number');
    check_equals (newso.data.gain, so.data.gain);
    check_equals (typeof(newso.data.echosuppression), 'boolean');
    check_equals (newso.data.echosuppression, so.data.echosuppression);
    check_equals (typeof(newso.data.defaultmicrophone), 'string');
    check_equals (newso.data.defaultmicrophone, so.data.defaultmicrophone);
    check_equals (typeof(newso.data.defaultcamera), 'string');
    check_equals (newso.data.defaultcamera,  '');
    check_equals (typeof(newso.data.defaultklimit), 'number');
    check_equals (newso.data.defaultklimit, so.data.defaultklimit);
    check_equals (typeof(newso.data.defaultalways), 'boolean');
    check_equals (newso.data.defaultalways, so.data.defaultalways);

    check_equals (typeof(newso.data.crossdomainAllow), 'boolean');
    check_equals (newso.data.crossdomainAllow, true);
    check_equals (typeof(newso.data.crossdomainAlways), 'boolean');
    check_equals (newso.data.crossdomainAlways, true);
    check_equals (typeof(newso.data.allowThirdPartyLSOAccess), 'boolean');
    check_equals (newso.data.allowThirdPartyLSOAccess, true);
    check_equals (typeof(newso.data.localSecPath), 'string');
    check_equals (newso.data.localSecPath, '');
    check_equals (typeof(newso.data.localSecPathTime), 'number');
    check_equals (newso.data.localSecPathTime, 1.19751160683e+12);
} else {
    trace("New Shared Object doesn't exist!");
}

/// Check localPath argument.

// It seems this is quite restrictive, using a non-normalized text match
// for the path, and removing the first directory for filesystem loads. Case
// is irrelevant, however.

note("Some of the following tests (all preceded by a 'checking' message) will fail when run over the network. Only the tests where an object is expected should fail");

so4 = SharedObject.getLocal("Another-one", "/subdir");
check_equals (so4.getSize(), undefined);
check_equals(typeof(so4), "null");
check(so4 != so3);
check_equals(typeof(so4.data), 'undefined');
ret = so4.flush();
check_equals(typeof(ret), 'undefined');

so4 = SharedObject.getLocal("name", "subdir");
check_equals(typeof(so4), "null");

so4 = SharedObject.getLocal("name", "http://ahost/subdir");
check_equals(typeof(so4), "null");

ourPath = SWFDIR + "/";
note ("checking getLocal with path: " + ourPath);
so4 = SharedObject.getLocal("name", ourPath);
check_equals(typeof(so4), "null");

ourPath = ourPath.substr(0, ourPath.lastIndexOf("/"));
note ("checking getLocal with path: " + ourPath);
so4 = SharedObject.getLocal("name", ourPath);
check_equals(typeof(so4), "null");

// Knock the first directory off. This is also confirmed to work with 
// a SWF in /tmp/, but it can't really be tested here.
ourPath = ourPath.substr(ourPath.indexOf("/", 1));
note ("checking getLocal with path: " + ourPath);
so4 = SharedObject.getLocal("name", ourPath);
check_equals(typeof(so4), "object");

// Upper case
note ("checking getLocal with path: " + ourPath.toUpperCase());
so4 = SharedObject.getLocal("name", ourPath.toUpperCase());
check_equals(typeof(so4), "object");

// Knock the last directory off
ourPath = ourPath.substr(0, ourPath.lastIndexOf("/"));
note ("checking getLocal with path: " + ourPath);
so4 = SharedObject.getLocal("name", ourPath);
check_equals(typeof(so4), "object");

// Put one trailing slash on 
ourPath += "/";
note ("checking getLocal with path: " + ourPath);
so4 = SharedObject.getLocal("name", ourPath);
check_equals(typeof(so4), "object");

// Put another trailing slash on 
ourPath += "/";
note ("checking getLocal with path: " + ourPath);
so4 = SharedObject.getLocal("name", ourPath);
xcheck_equals(typeof(so4), "null");

// Take the last slash off and add a bit of rubbish
ourPath = ourPath.substr(0, ourPath.lastIndexOf("/"));
ourPath += "aswfd.df";
note ("checking getLocal with path: " + ourPath);
so4 = SharedObject.getLocal("name", ourPath);
check_equals(typeof(so4), "null");

//------------------------------------------
// Test that if 'data' is a getter-setter,
// it isn't called on .flush()
//------------------------------------------

so5 = SharedObject.getLocal("getset");
check(so5 instanceof SharedObject);
dataGet = function() { getCalls++; return new Object(); };
getCalls=0;
so5.addProperty('data', dataGet, dataGet);
junk=so5.data;
check_equals(getCalls, 1); // the getter works
getCalls=0;
ret=so5.flush();
check_equals(ret, true);
check_equals(getCalls, 0); // flush didn't cal the getter

//------------------------------------------
// Test that 'data' is enumerable, read-only
// and protected from deletion
//------------------------------------------

so6 = SharedObject.getLocal("so6");
check(so6.hasOwnProperty("data"));
ret = so6.flush();
check_equals(ret, true);

a = new Array;
for (var i in so6) a.push(i);
check_equals(a.toString(), 'data');
delete so6;
check_equals(typeof(so.data), 'object');
so6.data = 5;
check_equals(typeof(so.data), 'object');

//------------------------------------------
// Test calling getLocal with no args
//------------------------------------------

so7 = SharedObject.getLocal();
#if OUTPUT_VERSION > 6
 // produces 'undefined.sol'
 check(so7 instanceof SharedObject);
#else
 // returns undefined
 check_equals(typeof(so7), 'null');
#endif
so7.data.a = 1;
ret = so7.flush();
#if OUTPUT_VERSION < 7
check_equals(ret, undefined);
#else
check_equals(ret, true);
#endif 

so8 = SharedObject.getLocal('');
check_equals(typeof(so8), 'null');

so9 = SharedObject.getLocal('', 'something');
check_equals(typeof(so9), 'null');

//------------------------------------------
// END OF TESTS
//------------------------------------------

check_totals(123);

#else

// SWF5 totals
check_totals(42);

#endif

