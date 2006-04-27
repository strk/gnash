// Mike Carlson's test program for actionscript arrays
// (initialization, 
// Jan. 17th, 2006

// Updated with sort functions, and to use check() macro
// by Mike Carlson Feb. 14th, 2006

#include "check.as"

var a;
a=[551,"asdf",12];
b=[];
b.push(551,"asdf",12);

check ( a != undefined );
check_equals ( typeof(a), "object" );
// reference at sephiroth.it/reference.php says (under "==")
// that two arrays are always considered NOT equal - need to verify
check ( a != b ); 

check_equals ( a.length, 3 );
check_equals ( a[2], 12 );
check ( a.pop() == 12 );
check ( a[2] == undefined );
check ( a[1] == "asdf" );
a[1] = a[0];
check ( a[1] == 551 );
a[0] = 200;
check ( a[0] == 200 );
check ( a.tostring() == "200,551");
a.push(7,8,9);
check_equals ( a.length, 5);
check ( a[100] == undefined );
check ( a[5] == undefined );
check ( a[4] == 9 );
check ( a.join() == "200,551,7,8,9" );
a.reverse();
check ( a.join() == "9,8,7,551,200" );
check ( a.join("test") == "9test8test7test551test200" );

// Test one of our sorting type members
check ( Array.CASEINSENSITIVE == 1 );
check ( Array.DESCENDING == 2 );
check ( Array.UNIQUESORT == 4 );
check ( Array.RETURNINDEXEDARRAY == 8 );
check ( Array.NUMERIC == 16 );

// Check sort functions
a.sort();
check_equals ( a.tostring(), "200,551,7,8,9" );
a.push(200,7,200,7,200,8,8,551,7,7);
a.sort( Array.NUMERIC );
check ( a.tostring() == "7,7,7,7,7,8,8,8,9,200,200,200,200,551,551" );
a.sort( Array.UNIQUESORT | Array.DESCENDING | Array.NUMERIC);
check_equals ( a.tostring() , "551,200,9,8,7" );

// Test multi-parameter constructor, and keep testing sort cases
var trysortarray = new Array("But", "alphabet", "Different", "capitalization");
trysortarray.sort( Array.CASEINSENSITIVE );
check ( trysortarray.tostring() == "alphabet,But,capitalization,Different");
trysortarray.sort();
check ( trysortarray.tostring() == "But,Different,alphabet,capitalization" );
// TODO - test sort(Array.RETURNINDEXEDARRAY)

check ( b.pop() == 12 );
check ( b.pop() == "asdf" );
check ( b.pop() == 551 );
// make sure pops on an empty array don't cause problems
check ( b.pop() == undefined );
b.pop(); b.pop();
check_equals ( b.length, 0 );
b.unshift(8,2);
b.push(4,3);
b.pop();
b.shift();
check ( b.tostring() == "2,4" );
b.shift();
b.pop();
check ( b.tostring() == "" );

// check reverse for empty case
b.reverse();
check ( b.tostring() == "" );

// check concat, slice
var bclone = b.concat();
check_equals ( bclone.length, 0 );
check_equals ( b.length, 0 );
var basic = b.concat(0,1,2);
var concatted = basic.concat(3,4,5,6);
check ( concatted.join() == "0,1,2,3,4,5,6" );
check ( concatted[4] == 4 );
check ( basic.tostring() == "0,1,2" );
var portion = concatted.slice( 2,-2 );
check ( portion.tostring() == "2,3,4" );
portion = portion.slice(1);
check ( portion.tostring() == "3,4" );
portion = portion.slice(1, 2);
check ( portion.tostring() == "4" );
check_equals ( portion.length, 1);

// Test single parameter constructor, and implicitly expanding array
var c = new Array(10);
check_equals ( typeof(c), "object" );
check_equals ( c.length, 10 );
check ( c[5] == undefined );
c[1000] = 283;
check ( c[1000] == 283 );
check ( c[1001] == undefined );
check ( c[999] == undefined );
check_equals ( c.length, 1001 );

// $Log: array.as,v $
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
