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

// Initial test written by Mike Carlson


rcsid="$Id: array.as,v 1.63 2008/03/19 16:36:42 strk Exp $";
#include "check.as"

check_equals(typeof(Array), 'function');
check_equals ( Array.CASEINSENSITIVE , 1 );
check_equals ( Array.DESCENDING , 2 );
check_equals ( Array.UNIQUESORT , 4 );
check_equals ( Array.RETURNINDEXEDARRAY , 8 );
check_equals ( Array.NUMERIC , 16 );
check_equals(typeof(Array.prototype), 'object');
check_equals(typeof(Array.prototype.__proto__), 'object');
check_equals(Array.prototype.__proto__, Object.prototype);
check_equals(typeof(Array.prototype.concat), 'function');
check_equals(typeof(Array.prototype.join), 'function');
check_equals(typeof(Array.prototype.pop), 'function');
check_equals(typeof(Array.prototype.push), 'function');
check_equals(typeof(Array.prototype.reverse), 'function');
check_equals(typeof(Array.prototype.shift), 'function');
check_equals(typeof(Array.prototype.slice), 'function');
check_equals(typeof(Array.prototype.sort), 'function');
check_equals(typeof(Array.prototype.sortOn), 'function');
check_equals(typeof(Array.prototype.splice), 'function');
check_equals(typeof(Array.prototype.unshift), 'function');
check_equals(typeof(Array.prototype.toString), 'function');
check_equals(typeof(Array.prototype.length), 'undefined');
check_equals(typeof(Array.prototype.size), 'undefined');
check_equals ( typeof(Array.prototype.CASEINSENSITIVE), 'undefined' );
check_equals ( typeof(Array.prototype.DESCENDING), 'undefined' );
check_equals ( typeof(Array.prototype.UNIQUESORT), 'undefined' );
check_equals ( typeof(Array.prototype.RETURNINDEXEDARRAY), 'undefined' );
check_equals ( typeof(Array.prototype.NUMERIC), 'undefined' );
#if OUTPUT_VERSION >= 6
check ( Array.hasOwnProperty('CASEINSENSITIVE') );
check ( Array.hasOwnProperty('DESCENDING') );
check ( Array.hasOwnProperty('UNIQUESORT') );
check ( Array.hasOwnProperty('RETURNINDEXEDARRAY') );
check ( Array.hasOwnProperty('NUMERIC') );
check(Array.prototype.hasOwnProperty('concat'));
check(Array.prototype.hasOwnProperty('join'));
check(Array.prototype.hasOwnProperty('pop'));
check(Array.prototype.hasOwnProperty('push'));
check(Array.prototype.hasOwnProperty('reverse'));
check(Array.prototype.hasOwnProperty('shift'));
check(Array.prototype.hasOwnProperty('slice'));
check(Array.prototype.hasOwnProperty('sort'));
check(Array.prototype.hasOwnProperty('sortOn'));
check(Array.prototype.hasOwnProperty('splice'));
check(Array.prototype.hasOwnProperty('unshift'));
check(Array.prototype.hasOwnProperty('toString'));
check(!Array.prototype.hasOwnProperty('length'));
check(!Array.prototype.hasOwnProperty('valueOf'));
check(!Array.prototype.hasOwnProperty('size'));
#endif // OUTPUT_VERSION >= 6

check_equals(typeof(Array()), 'object');
check_equals(typeof(new Array()), 'object');
f = ASnative(252, 0);
check_equals(typeof(f), 'function');
a = f();
check_equals(typeof(a), 'object');
check_equals(typeof(a.pop), 'function');

neg = new Object();
neg.valueOf = function () { return -1; };
zero = new Object();
zero.valueOf = function () { return 0; };
pos = new Object();
pos.valueOf = function () { return 1; };
two = new Object();
two.valueOf = function () { return 2; };
numeric = new Object();
numeric.valueOf = function () { return Array.NUMERIC; };
numericRev = new Object();
numericRev.valueOf = function () { return (Array.NUMERIC | Array.DESCENDING); };

var a;
var popped;
a=[551,"asdf",12];
check_equals(typeof(a.size), 'undefined');

check (a instanceOf Array);
check_equals(a.length, 3);
#if OUTPUT_VERSION >= 6
check(a.hasOwnProperty('length'));
#endif

primitiveArrayValue = a.valueOf();
check_equals(typeof(primitiveArrayValue), 'object');
check_equals( primitiveArrayValue, a );
#if OUTPUT_VERSION > 5
check( primitiveArrayValue === a );
#endif

b=[];
b.push(551,"asdf",12);

check ( a != undefined );
check_equals ( typeof(a), "object" );
// reference at sephiroth.it/reference.php says (under "==")
// that two arrays are always considered NOT equal - need to verify
check ( a != b ); 


tmp = new Array(2);
#if OUTPUT_VERSION > 6
check_equals ( tmp.toString(), "undefined,undefined" );
#else
check_equals ( tmp.toString(), "," );
#endif

tmp = new Array(two);
check_equals ( tmp.length, 1 );

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
#if OUTPUT_VERSION > 5
check_equals ( Array.prototype.join.apply(a), "9,8,7,551,200" );
check_equals ( a.join.apply(a), "9,8,7,551,200" );
#else
// It seems that up to SWF5 we couldn't do this ...
check_equals ( Array.prototype.join.apply(a), undefined );
check_equals ( a.join.apply(a), undefined );
#endif
check_equals ( a.join("test") , "9test8test7test551test200" );

// Test one of our sorting type members
check_equals( typeof(Array.UNIQUE), 'undefined' );

// the following tests do not belong here, but
// better somewhere then nowhere (are here due to
// a typo in this testcase triggering this bug)
//
check_equals( (undefined|1), 1 );
check_equals( (1|undefined), 1 );
check_equals( (undefined&1), 0 );
check_equals( (1&undefined), 0 );
check_equals( (undefined^1), 1 );
check_equals( (1^undefined), 1 );


check_equals( Array.UNIQUE | Array.CASEINSENSITIVE | Array.RETURNINDEXEDARRAY, 9 );

// Check sort functions
a.sort();
check_equals ( a.toString(), "200,551,7,8,9" );

a.push(200,7,200,7,200,8,8,551,7,7);
a.sort( Array.NUMERIC );
check_equals ( a.toString() , "7,7,7,7,7,8,8,8,9,200,200,200,200,551,551" );

a.sort( Array.UNIQUESORT | Array.DESCENDING | Array.NUMERIC);
check_equals (a.toString() , "7,7,7,7,7,8,8,8,9,200,200,200,200,551,551" );

// Test multi-parameter constructor, and keep testing sort cases
var trysortarray = new Array("But", "alphabet", "Different", "capitalization");
trysortarray.sort( Array.CASEINSENSITIVE );
check_equals ( trysortarray.toString() , "alphabet,But,capitalization,Different");
trysortarray.sort();
check_equals ( trysortarray.toString() , "But,Different,alphabet,capitalization" );

gaparray = [];
gaparray[4] = '4';
gaparray[16] = '16';
check_equals(gaparray.length, 17);
check_equals(gaparray[4], '4');
check_equals(gaparray[16], '16');
#if OUTPUT_VERSION > 5
check(gaparray.hasOwnProperty('4'));
check(gaparray.hasOwnProperty('16'));
check(!gaparray.hasOwnProperty('0'));
check(!gaparray.hasOwnProperty('1'));
#endif
gaparray.sort();
check_equals(gaparray.length, 17);
#if OUTPUT_VERSION < 7
 xcheck_equals(gaparray[0], undefined); // this is 16 with gnash
 xcheck_equals(gaparray[1], undefined); // this is 4 with gnash
#else
 check_equals(gaparray[0], '16');
 check_equals(gaparray[1], '4');
#endif
check_equals(gaparray[2], undefined);
check_equals(gaparray[3], undefined);
check_equals(gaparray[4], undefined);
check_equals(gaparray[5], undefined);
check_equals(gaparray[6], undefined);
check_equals(gaparray[7], undefined);
check_equals(gaparray[8], undefined);
check_equals(gaparray[9], undefined);
check_equals(gaparray[10], undefined);
check_equals(gaparray[11], undefined);
check_equals(gaparray[12], undefined);
check_equals(gaparray[13], undefined);
check_equals(gaparray[14], undefined);
#if OUTPUT_VERSION < 7
  xcheck_equals(gaparray[15], '16'); // this is at [0] with gnash
  xcheck_equals(gaparray[16], '4'); // this is at [1] with gnash
#else
  check_equals(gaparray[15], undefined);
  check_equals(gaparray[16], undefined);
#endif

#if OUTPUT_VERSION > 5
#if OUTPUT_VERSION < 7
 xcheck(gaparray.hasOwnProperty('15'));
 xcheck(gaparray.hasOwnProperty('16'));
 xcheck(gaparray.hasOwnProperty('4')); // a-ha!
 xcheck(!gaparray.hasOwnProperty('0'));
#else
 xcheck(gaparray.hasOwnProperty('16'));
 xcheck(gaparray.hasOwnProperty('4')); 
 check(gaparray.hasOwnProperty('1'));
 check(gaparray.hasOwnProperty('0'));
 xcheck(gaparray.hasOwnProperty('2'));
#endif
#endif

tmp = []; for (v in gaparray) tmp.push(v);
tmp.sort();
#if OUTPUT_VERSION < 7
 xcheck_equals(tmp.length, '3'); // 4, 15 and 16
 xcheck_equals(tmp[0], '15');
 xcheck_equals(tmp[1], '16');
 xcheck_equals(tmp[2], '4');
#else
 xcheck_equals(tmp.length, '5'); // 0, 1, 2, 4, 16 
 check_equals(tmp[0], '0');
 check_equals(tmp[1], '1');
 xcheck_equals(tmp[2], '16');
 xcheck_equals(tmp[3], '2');
 xcheck_equals(tmp[4], '4');
#endif

// TODO - test sort(Array.RETURNINDEXEDARRAY)

//-----------------------------------------------------
// Test sorting using a custom comparison function
//-----------------------------------------------------

testCmpCalls=0;
testCmpThis="not set";
function testCmp (x,y)
{
	// Gnash fails here by *requiring* a not-null 'this_ptr' in fn_call
	// NOTE: we can't rely on the number of calls to this function,
	//       which is implementation-defined
	if ( testCmpCalls++ ) testCmpThis=this;

	if (x.length < y.length) { return -1; }
	if (x.length > y.length) { return 1; }
	return 0;
}

check_equals ( trysortarray.toString() , "But,Different,alphabet,capitalization" );
trysortarray.sort( testCmp );
check_equals ( trysortarray.toString() , "But,alphabet,Different,capitalization" );
xcheck_equals(typeof(testCmpThis), 'undefined');
xcheck_equals(testCmpCalls, 7); // I don't think this matters much..

function testCmpBogus1 (x,y) { return -1; }
trysortarray.sort( testCmpBogus1 );
check_equals ( trysortarray.toString() , "But,alphabet,Different,capitalization" );

function testCmpBogus2 (x,y) { return 1; }
trysortarray.sort( testCmpBogus2 );
xcheck_equals ( trysortarray.toString() , "alphabet,Different,capitalization,But" );

function testCmpBogus3 (x,y) { return 0; }
trysortarray.sort( testCmpBogus3 );
xcheck_equals ( trysortarray.toString() , "alphabet,Different,capitalization,But" );

function testCmpBogus4 (x,y) { return tmp++%2; }
trysortarray.sort( testCmpBogus4 );
xcheck_equals ( trysortarray.toString() , "alphabet,Different,capitalization,But" );

function testCmpBogus5 (x,y) { trysortarray.pop(); return -1; }
trysortarray.sort( testCmpBogus5 );
xcheck_equals ( trysortarray.length , 0 );

function testCmpBogus6 (x,y) { trysortarray.pop(); return 1; }
trysortarray = new Array(1,2,3,4);
check_equals ( trysortarray.toString(), "1,2,3,4" );
check_equals ( trysortarray.length, 4 );
trysortarray.sort( testCmpBogus6 );
check_equals ( trysortarray.length, 4 );
xcheck_equals ( trysortarray.toString(), "2,3,4,1" );


//-----------------------------------------------------
// Test non-integer and insane indices.
//-----------------------------------------------------

c = ["zero", "one", "two", "three"];
check_equals(typeof(c), "object");

c[1.1] = "one point one";
c[-3] = "minus three";

check_equals (c[0], "zero");
xcheck_equals (c[1], "one");
check_equals (c[1.1], "one point one");
xcheck_equals (c[1.9], undefined);
check_equals (c[-3], "minus three");
check_equals (c[-3.7], undefined);

c[-2147483648] = "lowest int";
check_equals (c[0], "zero");
xcheck_equals (c[1], "one");

// This appears to invalidate integer indices, but
// not non-integer ones.
c[-2147483649] = "too low";
xcheck_equals (c[0], undefined);
xcheck_equals (c[1], undefined);
xcheck_equals (c[2], undefined);
xcheck_equals (c[3], undefined);
check_equals (c[1.1], "one point one");
check_equals (c[-2147483649], "too low");
// doesn't set the int(-2147483649) element:
check_equals (c[int(-2147483649)], undefined); 

c[2147483649] = "too high";
check_equals (c[-2147483649], "too low");
check_equals (c[2147483649], "too high");
xcheck_equals (c[1], undefined);
xcheck_equals (c[2], undefined);
xcheck_equals (c[3], undefined);

xcheck_equals (c.length, -2147483646);

str = "";

for (i in c)
{
    str += i + ": " + c[i] + "; ";
}
xcheck_equals(str, "2147483649: too high; -2147483649: too low; -2147483648: lowest int; -3: minus three; 1.1: one point one; ");

c = ["zero", "one", "two", "three"];
c[1.1] = "one point one";
c[-3] = "minus three";

check_equals (c[0], "zero");
xcheck_equals (c[1], "one");

// No problem...
c[0xffffffff + 1] = "too high";
check_equals (c[0], "zero");
xcheck_equals (c[1], "one");
check_equals (c[0xffffffff], undefined);
check_equals (c[0xffffffff + 1], "too high");

c[0xfffffffffffffffff] = "much too high";
check_equals (c[0xfffffffffffffffff], "much too high");

// Also no problem. Looks like a fairly crappy bug to me.
c[-2147483650] = "still lower";
check_equals (c[0], "zero");
xcheck_equals (c[1], "one");

xcheck_equals (c.length, 2147483647);

str= "";

for (i in c)
{
    str += i + ": " + c[i] + "; ";
}

xcheck_equals(str, "-2147483650: still lower; 2.95147905179353e+20: much too high; 4294967296: too high; -3: minus three; 1.1: one point one; 3: three; 2: two; 1: one; 0: zero; ");

// Getting 'holes' crawls the inheritance chain !
Array.prototype[3] = 3;
sparse = new Array();
sparse[2] = 2;
check_equals(sparse[3], 3); // crawl inheritance chain !
sparse[4] = 4;
check_equals(sparse[3], 3); // crawl inheritance chain !
delete Array.prototype[3];

//-----------------------------------------------------
// Test Array.pop()
//-----------------------------------------------------

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

//------------------------------------------------------
// Test Array.reverse
//------------------------------------------------------

// check reverse for empty case
b.reverse();
check_equals ( b.toString() , "" );

// check reverse for sparse array
sparse = new Array();
sparse[5] = 5;
count=0; for (var i in sparse) count++;
check_equals(count, 1); // a single element exists
#if OUTPUT_VERSION > 5
 check(!sparse.hasOwnProperty(0));
 check(sparse.hasOwnProperty(5));
#endif
#if OUTPUT_VERSION < 7
 check_equals(sparse.toString(), ",,,,,5");
#else
 check_equals(sparse.toString(), "undefined,undefined,undefined,undefined,undefined,5");
#endif
sparse.reverse();
count=0; for (var i in sparse) count++;
check_equals(count, 6); // no more holes
#if OUTPUT_VERSION > 5
 check(sparse.hasOwnProperty(0));
 check(sparse.hasOwnProperty(5));
#endif
#if OUTPUT_VERSION < 7
 check_equals(sparse.toString(), "5,,,,,");
#else
 check_equals(sparse.toString(), "5,undefined,undefined,undefined,undefined,undefined");
#endif

//------------------------------------------------------
// Test Array.join
//------------------------------------------------------

// join a sparse array
j = new Array();
j[1] = 1;
j[3] = 3;
s = j.join("^");
#if OUTPUT_VERSION < 7
 check_equals(s, "^1^^3");
#else
 check_equals(s, "undefined^1^undefined^3");
#endif

//------------------------------------------------------
// Test Array.concat and Array.slice (TODO: split)
//------------------------------------------------------

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
portion = concatted.slice(-2, -1);
check_equals ( portion.toString(), "5");
portion = concatted.slice(-2);
check_equals ( portion.toString(), "5,6");
mixed = portion.concat([7,8,9]);
check_equals ( mixed.toString(), "5,6,7,8,9");
mixed = mixed.concat([10,11],12,[13]);
check_equals ( mixed.toString(), "5,6,7,8,9,10,11,12,13");

// invalid calls
portion = concatted.slice(0, -8);
check_equals ( portion.toString(), "");
portion = concatted.slice(-18);
check_equals ( portion.toString(), "0,1,2,3,4,5,6");
portion = concatted.slice(-18, 3);
check_equals ( portion.toString(), "0,1,2");
portion = concatted.slice(18);
check_equals ( portion.toString(), "");

// using objects that implement valueOf as index positions
portion = concatted.slice(zero, two);
check_equals ( portion.toString(), "0,1");

//------------------------------------------------------
// Test Array.concat 
//------------------------------------------------------

sparse1 = new Array();
sparse1[3] = 'a3';

sparse2 = new Array();
sparse2[2] = 'b2';

csp = sparse1.concat(sparse2);

count=0; for (var i in sparse1) count++;
check_equals(count, 1);

count=0; for (var i in sparse2) count++;
check_equals(count, 1);

count=0; for (var i in csp) count++;
check_equals(count, 7); // concat filled any holes

csp = sparse1.concat('onemore');
count=0; for (var i in csp) count++;
check_equals(count, 5); // concat filled any holes

//-------------------------------
// Test Array.splice
//-------------------------------

ary = [0,1,2,3,4,5];
check_equals ( ary.toString(), "0,1,2,3,4,5" );

// No args is invalid
spliced = ary.splice();
check_equals ( ary.toString(), "0,1,2,3,4,5" );
check_equals ( typeof(spliced), "undefined" );

// Zero and positive offset starts from the end (-1 is last)
spliced = ary.splice(0, 1);
check_equals ( ary.toString(), "1,2,3,4,5" );
check_equals ( spliced.toString(), "0" );
spliced = ary.splice(1, 1);
check_equals ( ary.toString(), "1,3,4,5" );
check_equals ( spliced.toString(), "2" );

// Negative offset starts from the end (-1 is last)
spliced = ary.splice(-1, 1);
check_equals ( ary.toString(), "1,3,4" );
check_equals ( spliced.toString(), "5" );
spliced = ary.splice(-2, 1);
check_equals ( ary.toString(), "1,4" );
check_equals ( spliced.toString(), "3" );

// Out-of bound zero or positive offset are taken as one-past the end
spliced = ary.splice(2, 1);
check_equals ( ary.toString(), "1,4" );
check_equals ( spliced.toString(), "" );
spliced = ary.splice(2, 10);
check_equals ( ary.toString(), "1,4" );
check_equals ( spliced.toString(), "" );

// Out-of bound negative offset are taken as zero
spliced = ary.splice(-20, 1);
check_equals ( ary.toString(), "4" );
check_equals ( spliced.toString(), "1" );

// rebuild the array
ary = [0,1,2,3,4,5,6,7,8];

// Zero length doesn't change anything, and return an empty array
spliced = ary.splice(2, 0);
check_equals ( ary.toString(), "0,1,2,3,4,5,6,7,8" );
check_equals ( spliced.toString(), "" );

// Out of bound positive length consumes up to the end
spliced = ary.splice(2, 100);
check_equals ( ary.toString(), "0,1" );
check_equals ( spliced.toString(), "2,3,4,5,6,7,8" );
ary=spliced; // reset array
spliced = ary.splice(-2, 100);
check_equals ( ary.toString(), "2,3,4,5,6" );
check_equals ( spliced.toString(), "7,8" );

// Negative length are invalid
spliced = ary.splice(0, -1);
check_equals ( typeof(spliced), 'undefined' );
check_equals ( ary.toString(), "2,3,4,5,6" );
spliced = ary.splice(3, -1);
check_equals ( typeof(spliced), 'undefined' );
check_equals ( ary.toString(), "2,3,4,5,6" );
spliced = ary.splice(-1, -1);
check_equals ( typeof(spliced), 'undefined' );
check_equals ( ary.toString(), "2,3,4,5,6" );
spliced = ary.splice(-1, -1, "a", "b", "c");
check_equals ( typeof(spliced), 'undefined' );
check_equals ( ary.toString(), "2,3,4,5,6" );

// Provide substitutions now
spliced = ary.splice(1, 1, "a", "b", "c");
check_equals ( ary.toString(), "2,a,b,c,4,5,6" );
check_equals ( spliced.toString(), '3' );
spliced = ary.splice(-4, 2, 8);
check_equals ( ary.toString(), "2,a,b,8,5,6" );
check_equals ( spliced.toString(), 'c,4' );

// Insert w/out deleting anything
spliced = ary.splice(3, 0, 10, 11, 12);
check_equals ( ary.toString(), "2,a,b,10,11,12,8,5,6" );
check_equals ( spliced.toString(), '' );

// Use arrays as replacement
spliced = ary.splice(0, 7, [1,2], [3,4]);
check_equals ( ary.toString(), "1,2,3,4,5,6" );
check_equals ( ary.length, 4 ); // don't be fooled by toString output !
check_equals ( spliced.toString(), '2,a,b,10,11,12,8' );

// Ensure the simplest usage cases are correct!
spliced = ary.splice(1);
check_equals ( spliced.toString(), "3,4,5,6");
spliced = ary.splice(0);
check_equals ( spliced.toString(), "1,2");

// Splice a sparse array
ary = new Array(); ary[2] = 2; ary[7] = 7;

check_equals(ary.length, 8);
count=0; for (var i in ary) count++;
check_equals(count, 2);

spliced = ary.splice(3, 0); // no op ?
check_equals(ary.length, 8); // no change in length
count=0; for (var i in ary) count++;
check_equals(count, 8); // but fills the gaps !

ary = new Array(); ary[2] = 2; ary[7] = 7;
spliced = ary.splice(3, 0, 3); // add 3 at index 3
check_equals(ary.length, 9); 
count=0; for (var i in ary) count++;
check_equals(count, 9); // fills the gaps !
check_equals(ary[3], 3);
check_equals(ary[2], 2);

ary = new Array(); ary[2] = 2; ary[7] = 7;
spliced = ary.splice(3, 1, 3); // replace index 3 (an hole) with a 3 value
count=0; for (var i in ary) count++;
check_equals(count, 8); // fills the gaps 
count=0; for (var i in spliced) count++;
check_equals(count, 1); // the returned array contains an actual value, not an hole

//-------------------------------
// Test single parameter constructor, and implicitly expanding array
//-------------------------------

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

// Test that the 'length' property is overridable
c[8] = 'eight';
c[0] = 'zero';
check_equals(c[8], 'eight');
c.length = 2;
check_equals(c.length, 2);
check_equals(c[8], undefined);
check_equals(c[0], 'zero');
c.length = -1;
// it seems Gnash needs to store the 'length' property as a normal property
xcheck_equals(c.length, -1);
check_equals(c[0], undefined);

//-------------------------------
// Test deleting an array element
//-------------------------------

var c = new Array(10,20,30);
check_equals ( c.length, 3 );
check_equals(c[0], 10);
check_equals(c[1], 20);
check_equals(c[2], 30);
#if OUTPUT_VERSION > 5
check(c.hasOwnProperty('0'));
check(c.hasOwnProperty('1'));
check(c.hasOwnProperty('2'));
#endif
check(delete c[1]);
check_equals ( c.length, 3 );
check_equals(c[0], 10);
check_equals(typeof(c[1]), 'undefined');
check_equals(c[2], 30);
#if OUTPUT_VERSION > 5
check(c.hasOwnProperty('0'));
check(!c.hasOwnProperty('1'));
check(c.hasOwnProperty('2'));
#endif

c[10] = 'ten';
check_equals(c.length, 11);
ASSetPropFlags(c, "2", 7, 0); // protect from deletion
xcheck( ! delete c[2] ); // gnash doesn't store prop flags here..
xcheck_equals(c[2], 30); // so won't respect delete-protection
c.length = 2;
xcheck_equals(c[2], 30); // was protected !
check_equals(typeof(c[10]), 'undefined'); // was not protected..
c.length = 11;
check_equals(typeof(c[10]), 'undefined'); // and won't come back

//-------------------------------
// Test sort
//-------------------------------

function cmp_fn(x,y)
{
	if (x.length < y.length) { return -1; }
	if (x.length > y.length) { return 1; }
	return 0;
}

function cmp_fn_obj(x,y)
{
	if (x.length < y.length) { return neg; }
	if (x.length > y.length) { return pos; }
	return zero;
}

function tolen(x)
{
	var i;
	str = "[";
	for (i = 0; i < x.length; i++) 
	{
		str += String(x[i].length);
		if (i != x.length - 1) str += ", ";
	}
	str += "]";
	return str;
}

id = new Object();
id.toString = function () { return "Name"; };
yr = new Object();
yr.toString = function () { return "Year"; };

a = ["ed", "emacs", "", "vi", "nano", "Jedit"];
b = [8, 1, -2, 5, -7, -9, 3, 0];
c = [7.2, 2.0, -0.5, 3/0, 0.0, 8.35, 0.001, -3.7];
d = [];
e = ["singleton"];
f = [id, yr, id];

//trace(" -- Basic Sort Tests -- ");

r = a.sort( Array.NUMERIC );
check_equals( r.toString(), ",Jedit,ed,emacs,nano,vi" );
check_equals( a.toString(), ",Jedit,ed,emacs,nano,vi" );
a.sort( Array.NUMERIC | Array.CASEINSENSITIVE );
check_equals( a.toString(), ",ed,emacs,Jedit,nano,vi" );
a.sort();
check_equals( a.toString(), ",Jedit,ed,emacs,nano,vi" );
a.sort( Array.CASEINSENSITIVE );
check_equals( a.toString(), ",ed,emacs,Jedit,nano,vi" );
a.sort( Array.UNIQUESORT );
check_equals( a.toString(), ",Jedit,ed,emacs,nano,vi" );
r = a.sort( Array.DESCENDING );
check_equals( r.toString(), "vi,nano,emacs,ed,Jedit," );
check_equals( a.toString(), "vi,nano,emacs,ed,Jedit," );

r = b.sort();
check_equals( r.toString(), "-2,-7,-9,0,1,3,5,8" );
check_equals( b.toString(), "-2,-7,-9,0,1,3,5,8" );
b.sort( Array.NUMERIC );
check_equals( b.toString(), "-9,-7,-2,0,1,3,5,8" );
b.sort( Array.UNIQUESORT );
check_equals( b.toString(), "-2,-7,-9,0,1,3,5,8" );
b.sort( Array.DESCENDING );
check_equals( b.toString(), "8,5,3,1,0,-9,-7,-2" );
r = b.sort( Array.DESCENDING | Array.NUMERIC );
check_equals( r.toString(), "8,5,3,1,0,-2,-7,-9" );
check_equals( b.toString(), "8,5,3,1,0,-2,-7,-9" );
r = b.sort( zero );
check_equals( r.toString(), "8,5,3,1,0,-2,-7,-9" );
check_equals( b.toString(), "8,5,3,1,0,-2,-7,-9" );
b.sort( numeric );
check_equals( b.toString(), "8,5,3,1,0,-2,-7,-9" );
b.sort( numericRev );
check_equals( b.toString(), "8,5,3,1,0,-2,-7,-9" );

r = c.sort();
check_equals( r.toString(), "-0.5,-3.7,0,0.001,2,7.2,8.35,Infinity" );
check_equals( c.toString(), "-0.5,-3.7,0,0.001,2,7.2,8.35,Infinity" );
c.sort( Array.CASEINSENSITIVE );
check_equals( c.toString(), "-0.5,-3.7,0,0.001,2,7.2,8.35,Infinity" );
c.sort( Array.NUMERIC );
check_equals( c.toString(), "-3.7,-0.5,0,0.001,2,7.2,8.35,Infinity" );
r = c.sort( Array.UNIQUESORT );
check_equals( c.toString(), "-0.5,-3.7,0,0.001,2,7.2,8.35,Infinity" );
r = c.sort( Array.DESCENDING | Array.NUMERIC );
check_equals( c.toString(), "Infinity,8.35,7.2,2,0.001,0,-0.5,-3.7" );

r = d.sort();
check_equals( r.toString(), "" );
check_equals( d.toString(), "" );
d.sort( Array.UNIQUESORT );
check_equals( d.toString(), "" );
d.sort( Array.DESCENDING | Array.NUMERIC );
check_equals( d.toString(), "" );

r = e.sort();
check_equals( r.toString(), "singleton" );
check_equals( e.toString(), "singleton" );
e.sort( Array.UNIQUESORT );
check_equals( e.toString(), "singleton" );
e.sort( Array.DESCENDING | Array.CASEINSENSITIVE );
check_equals( e.toString(), "singleton" );

r = f.sort();
check_equals( r.toString(), "Name,Name,Year" );
check_equals( f.toString(), "Name,Name,Year" );
r = f.sort( Array.UNIQUESORT );
check_equals( r.toString(), "0" );
f.sort( Array.DESCENDING | Array.CASEINSENSITIVE );
check_equals( f.toString(), "Year,Name,Name" );

//trace(" -- Return Indexed Array Tests -- ");

r = a.sort( Array.RETURNINDEXEDARRAY );
check_equals( r.toString(), "5,4,3,2,1,0" );
check_equals( a.toString(), "vi,nano,emacs,ed,Jedit," );
r = a.sort( Array.RETURNINDEXEDARRAY | Array.DESCENDING | Array.CASEINSENSITIVE );
check_equals( r.toString(), "0,1,4,2,3,5" );
check_equals( a.toString(), "vi,nano,emacs,ed,Jedit," );
r = b.sort( Array.RETURNINDEXEDARRAY );
check_equals( r.toString(), "5,6,7,4,3,2,1,0" );
r = b.sort( Array.RETURNINDEXEDARRAY | Array.NUMERIC );
check_equals( r.toString(), "7,6,5,4,3,2,1,0" );
r = b.sort( Array.RETURNINDEXEDARRAY | Array.DESCENDING | Array.CASEINSENSITIVE );
check_equals( r.toString(), "0,1,2,3,4,7,6,5" );
r = c.sort( Array.RETURNINDEXEDARRAY );
check_equals( r.toString(), "6,7,5,4,3,2,1,0" );
r = c.sort( Array.RETURNINDEXEDARRAY | Array.NUMERIC );
check_equals( r.toString(), "7,6,5,4,3,2,1,0" );
r = c.sort( Array.RETURNINDEXEDARRAY | Array.DESCENDING | Array.CASEINSENSITIVE );
check_equals( r.toString(), "0,1,2,3,4,5,7,6" );
r = d.sort( Array.RETURNINDEXEDARRAY | Array.DESCENDING );
check_equals( r.toString(), "" );
check_equals( d.toString(), "" );
r = d.sort( Array.NUMERIC | Array.RETURNINDEXEDARRAY );
check_equals( r.toString(), "" );
check_equals( d.toString(), "" );
r = e.sort( Array.CASEINSENSITIVE | Array.RETURNINDEXEDARRAY );
check_equals( r.toString(), "0" );
check_equals( e.toString(), "singleton" );
r = e.sort( Array.NUMERIC | Array.RETURNINDEXEDARRAY | Array.DESCENDING );
check_equals( r.toString(), "0" );

//trace(" -- Custom AS function tests -- ");
r = a.sort( cmp_fn, Array.UNIQUESORT );
check_equals( r.toString(), ",vi,ed,nano,emacs,Jedit" );
check_equals( a.toString(), ",vi,ed,nano,emacs,Jedit" );
r = a.sort( something_undefined );
check_equals(typeof(r), 'undefined');
r = a.sort( cmp_fn, Array.DESCENDING );
check_equals( tolen(r), "[5, 5, 4, 2, 2, 0]" );
check_equals( tolen(a), "[5, 5, 4, 2, 2, 0]" );
a.sort( cmp_fn, Array.CASEINSENSITIVE | Array.NUMERIC );
check_equals( tolen(a), "[0, 2, 2, 4, 5, 5]" );
r = a.sort( cmp_fn, Array.RETURNINDEXEDARRAY );
check_equals( r.toString(), "0,1,2,3,4,5" );
r = a.sort( cmp_fn, Array.RETURNINDEXEDARRAY | Array.DESCENDING );
check_equals( r.toString(), "5,4,3,2,1,0" );
r = d.sort( cmp_fn );
check_equals( r.toString(), "" );
check_equals( d.toString(), "" );
r = d.sort( cmp_fn, Array.UNIQUESORT | Array.CASEINSENSITIVE );
check_equals( r.toString(), "" );
check_equals( d.toString(), "" );
r = e.sort( cmp_fn, Array.UNIQUESORT | Array.CASEINSENSITIVE );
check_equals( r.toString(), "singleton" );
check_equals( e.toString(), "singleton" );

//trace(" -- Custom AS function tests using an AS comparator that returns objects -- ");
r = a.sort( cmp_fn_obj, Array.DESCENDING );
check_equals( tolen(r), "[5, 5, 4, 2, 2, 0]" );
check_equals( tolen(a), "[5, 5, 4, 2, 2, 0]" );
a.sort( cmp_fn_obj, Array.CASEINSENSITIVE | Array.NUMERIC );
check_equals( tolen(a), "[0, 2, 2, 4, 5, 5]" );
r = a.sort( cmp_fn_obj, Array.RETURNINDEXEDARRAY );
check_equals( r.toString(), "0,1,2,3,4,5" );
r = a.sort( cmp_fn_obj, Array.RETURNINDEXEDARRAY | Array.DESCENDING );
check_equals( r.toString(), "5,4,3,2,1,0" );
e.sort( cmp_fn_obj, Array.UNIQUESORT | Array.CASEINSENSITIVE );
check_equals( e.toString(), "singleton" );

a.push("ED");
b.push(3.0);
c.push(9/0);

//trace(" -- UNIQUESORT tests -- ");

r = a.sort( Array.UNIQUESORT );
check_equals( r.toString(), ",ED,Jedit,ed,emacs,nano,vi" );
check_equals( a.toString(), ",ED,Jedit,ed,emacs,nano,vi" );
r = a.sort( Array.UNIQUESORT | Array.CASEINSENSITIVE );
check_equals( r.toString(), "0" );
check_equals( a.toString(), ",ED,Jedit,ed,emacs,nano,vi" );
r = a.sort( Array.UNIQUESORT | Array.CASEINSENSITIVE | Array.DESCENDING );
check_equals( r.toString(), "0" );
check_equals( a.toString(), ",ED,Jedit,ed,emacs,nano,vi" );
r = a.sort( Array.UNIQUESORT | Array.RETURNINDEXEDARRAY );
check_equals( r.toString(), "0,1,2,3,4,5,6" );
r = a.sort( Array.UNIQUESORT | Array.CASEINSENSITIVE | Array.DESCENDING | Array.RETURNINDEXEDARRAY );
check_equals( r.toString(), "0" );

r = b.sort( Array.UNIQUESORT );
check_equals( r.toString(), "0" );
check_equals( b.toString(), "8,5,3,1,0,-2,-7,-9,3" );
r = b.sort( Array.UNIQUESORT | Array.NUMERIC );
check_equals( r.toString(), "0" );
r = b.sort( Array.UNIQUESORT | Array.NUMERIC | Array.DESCENDING );
check_equals( r.toString(), "0" );
r = b.sort( Array.UNIQUESORT | Array.RETURNINDEXEDARRAY );
check_equals( r.toString(), "0" );
r = b.sort( Array.UNIQUESORT | Array.NUMERIC | Array.DESCENDING | Array.RETURNINDEXEDARRAY );
check_equals( r.toString(), "0" );

r = c.sort( Array.UNIQUESORT );
check_equals( r.toString(), "0" );
check_equals( c.toString(), "Infinity,8.35,7.2,2,0.001,0,-0.5,-3.7,Infinity" );
r = c.sort( Array.UNIQUESORT | Array.NUMERIC );
check_equals( r.toString(), "0" );
r = c.sort( Array.UNIQUESORT | Array.NUMERIC | Array.DESCENDING );
check_equals( r.toString(), "0" );
r = c.sort( Array.UNIQUESORT | Array.RETURNINDEXEDARRAY );
check_equals( r.toString(), "0" );
r = c.sort( Array.UNIQUESORT | Array.NUMERIC | Array.DESCENDING | Array.RETURNINDEXEDARRAY );
check_equals( r.toString(), "0" );
r = a.sort( cmp_fn, Array.UNIQUESORT | Array.CASEINSENSITIVE );
check_equals( tolen(r), "[0, 2, 2, 2, 4, 5, 5]" );
check_equals( tolen(a), "[0, 2, 2, 2, 4, 5, 5]" );
r = a.sort( cmp_fn, Array.UNIQUESORT | Array.CASEINSENSITIVE | Array.RETURNINDEXEDARRAY );
check_equals( r.toString(), "0,1,2,3,4,5,6" ); 
r = a.sort( cmp_fn, Array.UNIQUESORT | Array.CASEINSENSITIVE | Array.RETURNINDEXEDARRAY | Array.DESCENDING );
check_equals( r.toString(), "6,5,4,3,2,1,0" );

//trace(" -- Array with null value  -- ");
c.push(null);

r = c.sort();
check_equals( r.toString(), "-0.5,-3.7,0,0.001,2,7.2,8.35,Infinity,Infinity,null" ); 
check_equals( c.toString(), "-0.5,-3.7,0,0.001,2,7.2,8.35,Infinity,Infinity,null" );
c.sort( Array.NUMERIC );
check_equals( c.toString(), "-3.7,-0.5,0,0.001,2,7.2,8.35,Infinity,Infinity,null" );
c.sort( Array.DESCENDING | Array.NUMERIC );
check_equals( c.toString(), "null,Infinity,Infinity,8.35,7.2,2,0.001,0,-0.5,-3.7" );
r = c.sort( Array.RETURNINDEXEDARRAY | Array.NUMERIC );
check_equals( r.toString(), "9,8,7,6,5,4,3,1,2,0" );
r = c.sort( Array.RETURNINDEXEDARRAY | Array.DESCENDING | Array.CASEINSENSITIVE );
check_equals( r.toString(), "0,1,2,3,4,5,6,7,9,8" );
r = c.sort( Array.UNIQUESORT );
check_equals( r.toString(), "0" );
r = c.sort( Array.UNIQUESORT | Array.NUMERIC );
check_equals( r.toString(), "0" );
r = c.sort( Array.UNIQUESORT | Array.RETURNINDEXEDARRAY );
check_equals( r.toString(), "0" );
r = c.sort( Array.UNIQUESORT | Array.NUMERIC | Array.DESCENDING | Array.RETURNINDEXEDARRAY );
check_equals( r.toString(), "0" );

//trace(" -- Array with 2 null values  -- ");
c = [7.2, 2.0, null, -0.5, 3/0, 0.0, null, 8.35, 0.001, -3.7];
c.sort( Array.NUMERIC );
check_equals( c.toString(), "-3.7,-0.5,0,0.001,2,7.2,8.35,Infinity,null,null" );
c.sort( Array.DESCENDING | Array.NUMERIC );
check_equals( c.toString(), "null,null,Infinity,8.35,7.2,2,0.001,0,-0.5,-3.7" );
r = c.sort( Array.RETURNINDEXEDARRAY | Array.NUMERIC );
check_equals( r.toString(), "9,8,7,6,5,4,3,2,0,1" );
check_equals( c.toString(), "null,null,Infinity,8.35,7.2,2,0.001,0,-0.5,-3.7" );
r = c.sort( Array.UNIQUESORT );
check_equals( r.toString(), "0" );
r = c.sort( Array.UNIQUESORT | Array.NUMERIC );
check_equals( r.toString(), "0" );

//trace(" -- Array with 2 undefined values  -- ");
c = [7.2, 2.0, undefined, -0.5, 3/0, 0.0, undefined, 8.35, 0.001, -3.7];
r = c.sort( Array.UNIQUESORT );
check_equals( r.toString(), "0" );
r = c.sort( Array.UNIQUESORT | Array.NUMERIC );
check_equals( r.toString(), "0" );

//trace(" -- Array with 2 NaN values  -- ");
c = [7.2, 2.0, NaN, -0.5, 3/0, 0.0, NaN, 8.35, 0.001, -3.7];
r = c.sort( Array.UNIQUESORT );
check_equals( r.toString(), "0" );
r = c.sort( Array.UNIQUESORT | Array.NUMERIC );
check_equals( r.toString(), "0" );

//-------------------------------
// Test sortOn
//-------------------------------

a = [];
a.push({Name: "Zuse Z3", Year: 1941, Electronic: false});
a.push({Name: "Colossus", Year: 1943, Electronic: true});
a.push({Name: "ENIAC", Year: 1944, Electronic: true});

b = [];
b.push({Name: id, Year: yr, Electronic: yr});
b.push({Name: yr, Year: id, Electronic: yr});

function tostr(x)
{
	var i;
	str = "";
	for(i = 0; i < x.length; i++)
	{
		y = x[i];
		str += (y.Name + "," + y.Year + "," + y.Electronic );
		if (i != x.length - 1) str += " | ";
	}
	return str;
}

//trace("sortOn a single property ");
r = a.sortOn( "Name" );
check_equals( tostr(r), "Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
check_equals( tostr(a), "Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );

r = a.sortOn( "Year" );
check_equals( tostr(r), "Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true" );
check_equals( tostr(a), "Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true" );

a.sortOn( "Electronic" );
check_equals( tostr(a), "Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true" );

a.sortOn("Year", Array.NUMERIC );
check_equals( tostr(a), "Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true" );

a.sortOn("Year", Array.NUMERIC | Array.DESCENDING );
check_equals ( tostr(a), "ENIAC,1944,true | Colossus,1943,true | Zuse Z3,1941,false" );

r = a.sortOn("Year", Array.UNIQUESORT | Array.NUMERIC );
check_equals ( tostr(r), "Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true" );
check_equals ( tostr(a), "Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true" );

r = a.sortOn("Year", Array.RETURNINDEXEDARRAY | Array.NUMERIC );
check_equals( r.toString(), "0,1,2" );
check_equals ( tostr(a), "Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true" );

r = a.sortOn("Name", Array.UNIQUESORT );
check_equals( tostr(r), "Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
check_equals( tostr(a), "Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );

r = a.sortOn("Name", Array.UNIQUESORT | Array.DESCENDING );
check_equals( tostr(r), "Zuse Z3,1941,false | ENIAC,1944,true | Colossus,1943,true" );

r = a.sortOn("Name", Array.UNIQUESORT | Array.RETURNINDEXEDARRAY );
check_equals( r.toString(), "2,1,0" );

r = a.sortOn("Electronic", Array.UNIQUESORT | Array.RETURNINDEXEDARRAY );
check_equals( r.toString(), "0" );
check_equals( tostr(a), "Zuse Z3,1941,false | ENIAC,1944,true | Colossus,1943,true");

//trace("sortOn multiple properties");
a.push({Name: "Atanasoff-Berry", Year: 1941, Electronic: true, Mass: 320});

r = a.sortOn( ["Name", "Year"] );
check_equals( tostr(r), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
check_equals( tostr(a), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );

a.sortOn( ["Electronic", "Year"] );
check_equals( tostr(a), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true" );

a.sortOn( ["Electronic", "Year"], [Array.DESCENDING, Array.NUMERIC] );
check_equals( tostr(a), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );

r = a.sortOn( ["Name", "Year"], [Array.UNIQUESORT, Array.NUMERIC] );
check_equals( tostr(r), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );

r = a.sortOn( ["Electronic", "Name"], [Array.UNIQUESORT, Array.NUMERIC] );
check_equals( tostr(r), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true" );


//trace("sortOn missing properties" );
r = a.sortOn(["Megaflops"] );
check_equals( tostr(r), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true" );
check_equals( tostr(a), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true" );

a.sortOn(["Binary", "Turing complete"] );
check_equals( tostr(a), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true" );

a.sortOn(["Inventor", "Cost"], [Array.DESCENDING, 0] );
check_equals( tostr(a), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true" );

r = a.sortOn(["Name", "Year", "Cost"], [Array.DESCENDING, Array.NUMERIC, 0] );
check_equals( tostr(a), "Zuse Z3,1941,false | ENIAC,1944,true | Colossus,1943,true | Atanasoff-Berry,1941,true" );

r = a.sortOn(["Name", "Cost", "Year"], [0, 0, Array.NUMERIC] );
check_equals( tostr(a), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );

r = a.sortOn(["Electronic", "Year", "Cost"], [Array.UNIQUESORT, Array.NUMERIC, Array.NUMERIC] );
check_equals( tostr(r), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true" );
check_equals( tostr(a), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true" );

r = a.sortOn(["Electronic", "Cost" ], [Array.UNIQUESORT, Array.NUMERIC] );
check_equals( r.toString(), "0" );

//trace("sortOn with mismatching array lengths");
r = a.sortOn( ["Name", "Year"], [0] );
check_equals( tostr(r), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
check_equals( tostr(a), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );

r = a.sortOn( ["Name", "Year"], [Array.DESCENDING] );
check_equals( tostr(r), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
check_equals( tostr(a), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );

a.sortOn(["Name", "Electronic"], [Array.DESCENDING] );
check_equals( tostr(a), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );

r = a.sortOn(["Name", "Year"], [Array.RETURNINDEXEDARRAY] );
check_equals( tostr(r), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );

//trace("sortOn, undocumented invocation");
r = a.sortOn( ["Name", "Year"], Array.DESCENDING );
check_equals( tostr(r), "Zuse Z3,1941,false | ENIAC,1944,true | Colossus,1943,true | Atanasoff-Berry,1941,true" );
check_equals( tostr(a), "Zuse Z3,1941,false | ENIAC,1944,true | Colossus,1943,true | Atanasoff-Berry,1941,true" );

a.sortOn( ["Year", "Name"], Array.NUMERIC );
check_equals( tostr(a), "Atanasoff-Berry,1941,true | Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true" );

a.sortOn( ["Electronic", "Year", "Name"], Array.NUMERIC | Array.DESCENDING );
check_equals( tostr(a), "ENIAC,1944,true | Colossus,1943,true | Atanasoff-Berry,1941,true | Zuse Z3,1941,false" );

r = a.sortOn(["Name", "Electronic"], [Array.DESCENDING] );
check_equals( tostr(r), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
check_equals( tostr(a), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );

r = a.sortOn(["Name", "Year"], [Array.RETURNINDEXEDARRAY]);
check_equals( tostr(r), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
check_equals( tostr(a), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );

//trace("sortOn using an object implementing/over-riding the toString() method as the property argument");

a.sortOn( id );
check_equals( tostr(a), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );

a.sortOn( id, Array.CASEINSENSITIVE | Array.DESCENDING );
check_equals( tostr(a), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );

a.sortOn( [id], 0 );
check_equals( tostr(a), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );

a.sortOn( [yr, id], [Array.NUMERIC, Array.DESCENDING] );
check_equals( tostr(a), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true" );

//trace("sortOn with properties that are objects implementing the toString() method");

r = b.sortOn( "Name" );
check_equals( tostr(r), "Name,Year,Year | Year,Name,Year");
check_equals( tostr(b), "Name,Year,Year | Year,Name,Year");
b.sortOn( "Year" );
check_equals( tostr(b), "Year,Name,Year | Name,Year,Year");
b.sortOn( ["Year", "Name"], [Array.NUMERIC | Array.DESCENDING, 0] );
check_equals( tostr(b), "Name,Year,Year | Year,Name,Year");

//trace("sortOn invalid calls");
r = a.sortOn();
check( r == undefined );
check_equals( tostr(a), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true" );

r = a.sortOn(undefined);
check_equals( typeof(r) , 'object' );
check( r instanceof Array );
check_equals( tostr(a), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true" );

//trace("sortOn with flag as an object overriding the valueOf method");
a.sortOn( ["Year", "Electronic", "Name"], numeric );
check_equals( tostr(a), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true" );

#if OUTPUT_VERSION < 7
//trace("sortOn property name case-mismatch");
a.sortOn( "name" );
check_equals( tostr(a), "Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );
a.sortOn( ["year", "name"], Array.NUMERIC );
check_equals( tostr(a), "Atanasoff-Berry,1941,true | Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true" );
#endif // OUTPUT_VERSION < 7

#if OUTPUT_VERSION > 6
//trace("sortOn with some properties undefined");
a.push({Name: "Harvard Mark I", Year: 1944, Mass: 4500});

a.sortOn(["Electronic", "Year"], Array.DESCENDING | Array.IGNORECASE );
check_equals( tostr(a), "Harvard Mark I,1944,undefined | ENIAC,1944,true | Colossus,1943,true | Atanasoff-Berry,1941,true | Zuse Z3,1941,false" );

a.sortOn( ["Electronic", "Name"], [Array.NUMERIC, Array.DESCENDING] );
check_equals( tostr(a), "Zuse Z3,1941,false | ENIAC,1944,true | Colossus,1943,true | Atanasoff-Berry,1941,true | Harvard Mark I,1944,undefined" );

r = a.sortOn( ["Electronic", "Name"], [Array.UNIQUESORT, Array.NUMERIC] );
check_equals( tostr(r), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Harvard Mark I,1944,undefined" );
check_equals( tostr(a), "Zuse Z3,1941,false | Atanasoff-Berry,1941,true | Colossus,1943,true | ENIAC,1944,true | Harvard Mark I,1944,undefined" );

a.sortOn( ["Mass", "Name"], [0, 0] );
check_equals( tostr(a), "Atanasoff-Berry,1941,true | Harvard Mark I,1944,undefined | Colossus,1943,true | ENIAC,1944,true | Zuse Z3,1941,false" );

a.sortOn( ["Mass", "Year", "Name"], [Array.NUMERIC | Array.DESCENDING, Array.NUMERIC | Array.DESCENDING | 0] );
check_equals( tostr(a), "Atanasoff-Berry,1941,true | Harvard Mark I,1944,undefined | Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true" );

a.sortOn( ["Mass", "Name"], [Array.UNIQUESORT, Array.DESCENDING] );
check_equals( tostr(a), "Atanasoff-Berry,1941,true | Harvard Mark I,1944,undefined | Zuse Z3,1941,false | ENIAC,1944,true | Colossus,1943,true" );

a.sortOn( ["Electronic", "Mass", "Name"], [0, Array.NUMERIC | Array.DESCENDING, 0] );
check_equals( tostr(a), "Zuse Z3,1941,false | Colossus,1943,true | ENIAC,1944,true | Atanasoff-Berry,1941,true | Harvard Mark I,1944,undefined" );

r = a.sortOn( ["Electronic", "Mass", "Year", "Name"], [Array.RETURNINDEXEDARRAY, Array.NUMERIC, Array.NUMERIC, Array.DESCENDING] );
check_equals( r.toString(), "0,3,1,2,4");
#endif // OUTPUT_VERSION > 6


//-------------------------------------------------------
// Test array enumeration
//------------------------------------------------------

b = ["a","b","c"];
out = {len:0}; for (var i in b) { out[i] = 1; out['len']++; }
check_equals(out['len'], 3);
check_equals(out[0], 1);
check_equals(out[1], 1);
check_equals(out[2], 1);

b = [];
out = {len:0}; for (var i in b) { out[i] = 1; out['len']++; }
check_equals(out['len'], 0);

// Changing length doesn't trigger enumeration of undefined values
b.length = 100;
out = {len:0}; for (var i in b) { out[i] = 1; out['len']++; }
check_equals(out['len'], 0);

b[1] = undefined;
out = {len:0}; for (var i in b) { out[i] = 1; out['len']++; }
check_equals(out['len'], 1);
check_equals(out[1], 1);

b[0] = undefined;
out = {len:0}; for (var i in b) { out[i] = 1; out['len']++; }
check_equals(out['len'], 2);
check_equals(out[1], 1);
check_equals(out[0], 1);

//-------------------------------
// Test length property
//-------------------------------

a = new Array();
check_equals(a.length, 0);
a[-1] = 'minusone';
check_equals(a.length, 0);
check_equals(a[-1], 'minusone');
a["Infinite"] = 'inf';
check_equals(a.length, 0);
check_equals(a["Infinite"], 'inf');

//----------------------------------------------
// Force an indexed property to a getter/setter
//---------------------------------------------

#if OUTPUT_VERSION > 5 // addProperty was added in SWF6

function get() { getCalls++; }
function set() { setCalls++; }
a = new Array();
a[2] = 2;
ret = a.addProperty('1', get, set);
check_equals(ret, true);
getCalls=0; setCalls=0;
junk = a[1];
check_equals(getCalls, 1);
check_equals(setCalls, 0);
getCalls=0; setCalls=0;
a[1] = 1;
check_equals(getCalls, 0);
xcheck_equals(setCalls, 1);

ret = a.addProperty('2', get, set);
check_equals(ret, true);
getCalls=0; setCalls=0;
junk = a[2];
xcheck_equals(getCalls, 1);
check_equals(setCalls, 0);
getCalls=0; setCalls=0;
a[2] = 2;
check_equals(getCalls, 0);
xcheck_equals(setCalls, 1);

check_equals(a.length, 3);
ret = a.addProperty('3', get, set);
xcheck_equals(a.length, 4);

a.length = 3;
getCalls=0; setCalls=0;
a.push(2);
check_equals(getCalls, 0);
check_equals(setCalls, 0);

#endif // OUTPUT_VERSION > 5

//--------------------------------------------------------
// pop an array with delete-protected elements
//--------------------------------------------------------

a = new Array();
a[0] = 'zero';
a[1] = 'one';
ASSetPropFlags(a, "0", 7, 0); // protect 0 from deletion
check_equals(a.length, 2);
f = a.shift();
check_equals(a.length, 1); 
check_equals(f, 'zero');
xcheck_equals(a[0], 'zero'); // could not delete for override
check_equals(typeof(a[1]), 'undefined');
#if OUTPUT_VERSION > 5
 check(!a.hasOwnProperty(1)); 
#endif

a = new Array();
a[0] = 'zero';
a[1] = 'one';
ASSetPropFlags(a, "1", 7, 0); // protect 1 from deletion
check_equals(a.length, 2);
f = a.shift();
check_equals(a.length, 1);
check_equals(f, 'zero');
check_equals(a[0], 'one'); // could replace
xcheck_equals(a[1], 'one'); // couldn't delete
#if OUTPUT_VERSION > 5
 check(a.hasOwnProperty(0)); 
 xcheck(a.hasOwnProperty(1)); 
#endif

//--------------------------------------------------------
// pop an array with read-only elements
//--------------------------------------------------------

a = new Array();
a[0] = 'zero';
a[1] = 'one';
ASSetPropFlags(a, "0", 4, 0); // protect 0 from override
check_equals(a.length, 2);
a[0] = 'overridden';
xcheck_equals(a[0], 'zero'); // was protected..
f = a.shift();
check_equals(a.length, 1); 
xcheck_equals(f, 'zero');
check_equals(a[0], 'one'); // 0 was replaced anyway, didn't care about protection
check_equals(typeof(a[1]), 'undefined');
a[0] = 'overridden';
check_equals(a[0], 'overridden'); // flag was lost
#if OUTPUT_VERSION > 5
 check(!a.hasOwnProperty(1)); 
#endif

a = new Array();
a[0] = 'zero';
a[1] = 'one';
a[2] = 'two';
ASSetPropFlags(a, "1", 4, 0); // protect 1 from override
a[1] = 'overridden';
xcheck_equals(a[1], 'one'); // was protected
check_equals(a.length, 3);
f = a.shift();
check_equals(a.length, 2);
check_equals(f, 'zero');
xcheck_equals(a[0], 'one'); // 0 was replaced anyway, didn't care about protection
check_equals(a[1], 'two');
check_equals(typeof(a[2]), 'undefined');
a[1] = 'overridden';
check_equals(a[1], 'overridden'); // flag was lost
#if OUTPUT_VERSION > 5
 check(a.hasOwnProperty(0)); 
 check(a.hasOwnProperty(1)); 
 check(!a.hasOwnProperty(2)); 
#endif


// TODO: test ASnative-returned functions:
//
// ASnative(252, 1) - [Array.prototype] push
// ASnative(252, 2) - [Array.prototype] pop
// ASnative(252, 3) - [Array.prototype] concat
// ASnative(252, 4) - [Array.prototype] shift
// ASnative(252, 5) - [Array.prototype] unshift
// ASnative(252, 6) - [Array.prototype] slice
// ASnative(252, 7) - [Array.prototype] join
// ASnative(252, 8) - [Array.prototype] splice
// ASnative(252, 9) - [Array.prototype] toString
// ASnative(252, 10) - [Array.prototype] sort
// ASnative(252, 11) - [Array.prototype] reverse
// ASnative(252, 12) - [Array.prototype] sortOn 
//


#if OUTPUT_VERSION < 6
 check_totals(484);
#else
# if OUTPUT_VERSION < 7
  check_totals(545);
# else
  check_totals(555);
# endif
#endif
