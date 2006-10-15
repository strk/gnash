// Mike Carlson's test program for actionscript arrays
// (initialization, 
// Jan. 17th, 2006

// Updated with sort functions, and to use check() macro
// by Mike Carlson Feb. 14th, 2006

rcsid="$Id: array.as,v 1.10 2006/10/15 02:30:55 rsavoye Exp $";

#include "check.as"

var a;
var popped;
a=[551,"asdf",12];

check (a instanceOf Array);

b=[];
b.push(551,"asdf",12);

check ( a != undefined );
check_equals ( typeof(a), "object" );
// reference at sephiroth.it/reference.php says (under "==")
// that two arrays are always considered NOT equal - need to verify
check ( a != b ); 

check_equals ( a.length, 3 );
check_equals ( a[2], 12 );
popped=a.pop();
check_equals ( popped , 12 );
check_equals ( a[2] , undefined );
check_equals ( a[1] , "asdf" );
a[1] = a[0];
check_equals ( a[1] , 551 );
a[0] = 200;
check_equals ( a[0] , 200 );
check_equals ( a.toString() , "200,551");
a.push(7,8,9);
check_equals ( a.length, 5);
check_equals ( a[100] , undefined );
check_equals ( a[5] , undefined );
check_equals ( a[4] , 9 );
check_equals ( a.join() , "200,551,7,8,9" );
a.reverse();
check_equals ( a.join() , "9,8,7,551,200" );
check_equals ( a.join("test") , "9test8test7test551test200" );

// Test one of our sorting type members
check_equals ( Array.CASEINSENSITIVE , 1 );
check_equals ( Array.DESCENDING , 2 );
check_equals ( Array.UNIQUESORT , 4 );
check_equals ( Array.RETURNINDEXEDARRAY , 8 );
check_equals ( Array.NUMERIC , 16 );

// Check sort functions
a.sort();
check_equals ( a.toString(), "200,551,7,8,9" );

// test flags
check_equals ( Array.CASEINSENSITIVE, 1 );
check_equals ( Array.DESCENDING, 2 );
check_equals ( Array.UNIQUESORT, 4 );
check_equals ( Array.RETURNINDEXEDARRAY, 8 );
check_equals ( Array.NUMERIC, 16 );

a.push(200,7,200,7,200,8,8,551,7,7);
a.sort( Array.NUMERIC );
check_equals ( a.toString() , "7,7,7,7,7,8,8,8,9,200,200,200,200,551,551" );

a.sort( Array.UNIQUESORT | Array.DESCENDING | Array.NUMERIC);
xcheck_equals (a.toString() , "7,7,7,7,7,8,8,8,9,200,200,200,200,551,551" );

// Test multi-parameter constructor, and keep testing sort cases
var trysortarray = new Array("But", "alphabet", "Different", "capitalization");
trysortarray.sort( Array.CASEINSENSITIVE );
check_equals ( trysortarray.toString() , "alphabet,But,capitalization,Different");
trysortarray.sort();
check_equals ( trysortarray.toString() , "But,Different,alphabet,capitalization" );
// TODO - test sort(Array.RETURNINDEXEDARRAY)

popped=b.pop();
check ( popped == 12 );
popped=b.pop();
check ( popped == "asdf" );
popped=b.pop();
check ( popped == 551 );
// make sure pops on an empty array don't cause problems
popped=b.pop();
check ( popped == undefined );
b.pop(); b.pop();
check_equals ( b.length, 0 );
b.unshift(8,2);
b.push(4,3);
b.pop();
b.shift();
check_equals ( b.toString() , "2,4" );
b.shift();
b.pop();
check_equals ( b.toString() , "" );

// check reverse for empty case
b.reverse();
check_equals ( b.toString() , "" );

// check concat, slice
var bclone = b.concat();
check_equals ( bclone.length, 0 );
check_equals ( b.length, 0 );
var basic = b.concat(0,1,2);
var concatted = basic.concat(3,4,5,6);
check_equals ( concatted.join() , "0,1,2,3,4,5,6" );
check_equals ( concatted[4] , 4 );
check_equals ( basic.toString() , "0,1,2" );
var portion = concatted.slice( 2,-2 );
check_equals ( portion.toString() , "2,3,4" );
portion = portion.slice(1);
check_equals ( portion.toString() , "3,4" );
portion = portion.slice(1, 2);
check_equals ( portion.toString() , "4" );
check_equals ( portion.length, 1);

// Test single parameter constructor, and implicitly expanding array
var c = new Array(10);
check (a instanceOf Array);
check_equals ( typeof(c), "object" );
check_equals ( c.length, 10 );
check_equals ( c[5] , undefined );
c[1000] = 283;
check_equals ( c[1000] , 283 );
check_equals ( c[1001] , undefined );
check_equals ( c[999] , undefined );
check_equals ( c.length, 1001 );

// $Log: array.as,v $
// Revision 1.10  2006/10/15 02:30:55  rsavoye
// 	* testsuite/actionscript.all/swf_exists.exp: Use local_exec()
// 	instead of spawn/expect. This works better with batch tests.
// 	* testsuite/actionscript.all/check.as: Add xcheck and
// 	xcheck_equals to handle expected failures.
// 	* testsuite/actionscript.all/dejagnu.as: Add xpass and xfail to
// 	handle expect failures.
// 	* testsuite/actionscript.all/Boolean.as, Date.as, Global.as,
// 	Inheritance.as, MovieClip.as, NetConnection.as, Number.as,
// 	Object.as, Selection.as, array.as, delete.as, inheritance.as: Use
// 	xcheck and xcheck_equals for tests expected to not work yet.
// 	* testsuite/actionscript.all/XML.as, XMLNode.as: Use xpass and
// 	xfail for tests expected to not work yet.
//
// Revision 1.9  2006/07/06 08:16:31  strk
// Added instanceOf test for both new Array() and [...] constructors.
//
// Revision 1.8  2006/07/06 07:55:24  strk
// "tostring" => "toString" (SWF 7 and up are case-sensitive in this); added tests for Array constants.
//
// Revision 1.7  2006/06/20 20:45:27  strk
//         * testsuite/actionscript.all/: added rcsid variable
//         to all testfiles, had check.as print testfile info at
//         the beginning rather then at each check.
//
// Revision 1.6  2006/04/27 16:31:56  strk
//         * server/: (array.cpp, array.h): big cleanup, provided
//         overrides for get_member() and set_member() to add support
//         for the special 'length' element, turned array_as_object into
//         a real class.
//         * server/: (Object.cpp, Object.h): moved get_member
//         and set_member to get_member_default and set_member_default
//         with protected access level, provided public virtuals
//         invoking the protected non-virtuals. This is to allow cleaner
//         hooking for ActionScript classes.
//
// Revision 1.5  2006/04/27 09:37:00  strk
// completed switch to check_equals() macro
//
// Revision 1.4  2006/04/27 07:27:25  strk
//         * testsuite/actionscript.all/array.as: turned length()
//         method calls to length data member accesses.
//
// Revision 1.3  2006/04/26 20:02:41  strk
// More uses of the check_equals macro
//
// Revision 1.2  2006/02/14 08:17:51  corfe
// Change all tests to use new check macro. Add tests for all implemented array functions, as well as several tests for the unimplemented sort function.
//
// Revision 1.1  2006/02/01 11:43:16  strk
// Added generic rule to build .swf from .as using makeswf (Ming).
// Changed array.as source to avoid ActionScript2 constructs (class).
// Added initial version of a movieclip AS class tester.
//
