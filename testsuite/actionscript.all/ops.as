// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modchecky
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
// along with this program; check not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fcheckth Floor, Boston, MA  02110-1301  USA
//

/*
 *  Test binary predicates (equal, less_then, greater_then, logical and bitwise ops)
 */


rcsid="$Id: ops.as,v 1.38 2008/04/02 08:21:49 strk Exp $";
#include "check.as"

//--------------------------------------------
// Equality operator (ACTION_NEWEQUALS : 0x49)
//--------------------------------------------
check('xyz' == 'xyz');
check(!('xyz' == 'XYZ'));
check(!('xyz' == 'abc'));
check(1 == 1); // doh
check(1 == "1");
check("1" == 1);
check(0 == -0);
check(0 == "-0");
check("0" == -0);
check(null == null);
check(undefined == undefined);
check(null==undefined); 
check(undefined==null); 
check(! (NaN == 0) );
check(! (0 == NaN) );
check(! ('string' == 0) );
check(! (0 == 'string') );
check(! ('string' == NaN) );
check(! (NaN == 'string') );
check(! (Infinity == 'Infinity') );
check(! ('Infinity' == Infinity) );
check(! (-Infinity == '-Infinity') );
check(! ('-Infinity' == -Infinity) );
check(Infinity == Infinity);
check(-Infinity == -Infinity);
check(! (-Infinity == Infinity));
x = 1;
check_equals(x/0, Infinity);
check_equals(-x/0, -Infinity);
check(1==true);
check(true==1);
check(2!=true);
check(true!=2);
check_equals(1+true, 2);
check_equals(true+1, 2);
check_equals(1+false, 1);
check_equals(false+1, 1);
check_equals(true+true, 2);
check_equals(true+false, 1);
check_equals(false, 0);
check_equals(false+false, 0);

x = "abc";
y = 0;
z = x * y;

// in swf4, z is 0, tested in swf4opcode.sc
// in swf5~8, z is NaN.
check(! (z == 0));  
check(isNaN(z));
// both EMACS and Moock's book says NaN != NaN, now I believe they are correct.
// we have enough tests to confirm this.
xcheck(! (z == NaN)); 
// Deduction: Flash built-in NaN constant(string) has a special representation.
// Note: This is the ONLY case where NaN == NaN, IIRC
check(NaN == NaN);

// for Number
x = new Number(3);
y = 3;
check(x == y);

// for different nan values...
x = Number(new Number(NaN));
y = NaN;
z = 0/0;
check_equals(typeof(x), 'number')
check_equals(typeof(y), 'number');
check_equals(typeof(z), 'number');
check(isNaN(x));
check(isNaN(y));
check(isNaN(z));
xcheck(x != y);
xcheck(y != z);
xcheck(x != z);
check(x == x);
check(y == y);
check(z == z);
check((1/0) == (1/0));

// for Arrays
ary1 = [1,2,3];
ary2 = [1,2,3];
check( ! (ary1 == ary2) ); // two different objects
check( ! (ary1 == "1,2,3") ); // the array doesn't get converted to a string
check( ! ("1,2,3" == ary1) ); // the array doesn't get converted to a string

// for String
str1 = new String("hello");
str2 = new String("hello");
str3 = new String("3");
check( ! (str1 == str3) );  
check_equals(str1, "hello"); // str1 is automatically converted to a string
check_equals("hello", str1); // str1 is automatically converted to a string

check_equals( str3, 3.0 ); // str3 (object) is automatically converted to a number 
check_equals( str3.toString(), 3.0 ); // str3 (primitive string) is automatically converted to a number 

check( ! (str1 == 0) ); // str1 (object) is NOT converted to a number (due to NaN?)
check( ! (str1 == NaN) ); // str1 (object) is NOT converted to a number (due to NaN?)

#if OUTPUT_VERSION > 5
  check( ! (str1 == str2) ); // they are not the same object
#else // OUTPUT_VERSION <= 5
  check( str1 == str2 );  // SWF5 automatically converts to primitive before comparison !
#endif // OUTPUT_VERSION <= 5

// for MovieClip

check("_root" != _root);
check(_root != "_root");
o = new Object(); o.valueOf = function() { return _root; };
check_equals(_root, o);
check_equals(o, _root);

//---------------------------------------------
// Less then operator (ACTION_LESSTHAN : 0x0F)
//---------------------------------------------

x = 0.999;
y = 1.0;
check(x<y);

x=String("0.999");
y=String("1.0");
check(x<y);

x=String("A");
y=String("a");
check(x<y);

x=String("abc");
y=String("abcd");
check(x<y);

x=0.999;
y=String("1.000");
check(x<y);

x=String("0.999");
y=1.0;
check(x<y);

check(! (NaN < NaN) );
check(! (undefined < undefined) );

check(! (NaN < undefined) );
check(! (undefined < NaN) );

r = null < undefined;
#if OUTPUT_VERSION < 7
 check_equals(typeof(r), 'boolean');
 check_equals(r, false);
#else
 check_equals(typeof(r), 'undefined');
 check(r != true);
 check(r != false);
#endif

r = null > undefined;
#if OUTPUT_VERSION < 7
 check_equals(typeof(r), 'boolean');
 check_equals(r, false);
#else
 check_equals(typeof(r), 'undefined');
 check(r != true);
 check(r != false);
#endif

//------------------------------------------------
// Logical AND operator (ACTION_LOGICALAND : 0x10)
//------------------------------------------------

#ifndef MING_LOGICAL_ANDOR_BROKEN

x = true;
y = true;
z = x && y;
check_equals(z, y);

x = true;
y = false;
z = x && y;
check_equals(z, y);

x = true;
y = 0;
z=x && y;
check_equals(z, y);

x = true;
y = 1;
z=x && y;
check_equals(z, y);

x = true;
y = String("1.999");
z=x && y;
check_equals(z, "1.999"); 

x=String("1.999");
y=true;
z=x && y;
check_equals(z, y);

x=String("adcd");
y=true;
z=x && y;
#if OUTPUT_VERSION < 7
	check_equals(z, "adcd");
#else
	check_equals(z, true);
#endif 

x=true;
y=String("adcd");
z=x && y;
check_equals(z, "adcd");

x=0;
y=true;
z=x && y;
check_equals(z, x);

x=1;
y=false;
z=x && y;
check_equals(z, y);

x = new Number(9);
y=false;
z=x && y;
check_equals(z, y);

x = new Object();
y = false;
check_equals(z, y);

x=String("adcd");
y=false;
z=x && y;
#if OUTPUT_VERSION < 7
	check_equals(z, "adcd"); 
#else
	check_equals(z, false);
#endif

#endif // ndef MING_LOGICAL_ANDOR_BROKEN

//------------------------------------------------
// Logical OR operator (ACTION_LOGICALOR : 0x11)
//------------------------------------------------

#ifndef MING_LOGICAL_ANDOR_BROKEN

x = 0;
y = 0;
z = x || y;
check_equals(z, y); 

x = 0;
y = 1;
z = x || y;
check_equals(z, y); 

x = 0;
y = 0.0001;
z = x || y;
check_equals(z, y); 

x = 0;
y = "abcd";
z = x || y;
#if OUTPUT_VERSION == 4
	check_equals(z, 1); 
#elseif < 7
	check_equals(z, false); 
#esle
	check_equals(z, true);
#endif

x = 0;
y = String("abcd");
z = x || y;
#if OUTPUT_VERSION < 7
	check_equals(z, "abcd"); 
#else
	check_equals(z, "abcd");
#endif

x = 0;
y = new String("abcd");
z = x || y;
check_equals(z, "abcd");

x = 0;
y = Object();
z = x || y;
check_equals(z, y);

x = 0;
y = new Object();
z = x || y;
check_equals(z, y);

x = 0;
y = "0";
z = x || y;
check_equals(typeof(z), 'string');

x = 0;
y = String("0");
z = x || y;
check_equals(typeof(z), 'string');

x = 0;
y = new String("0");
z = x || y;
check_equals(z, y);

#endif // ndef MING_LOGICAL_ANDOR_BROKEN

//------------------------------------------------
// String comparison (ACTION_STRINGCOMPARE : 0x29)
//------------------------------------------------

asm {
	push "z", undefined, null
	stringlessthan setvariable
};
check_equals(typeof(z), 'boolean');
#if OUTPUT_VERSION < 7
 check(z);
#else
 check(!z);
#endif

asm {
	push "z", null, undefined
	stringlessthan setvariable
};
check_equals(typeof(z), 'boolean');
#if OUTPUT_VERSION < 7
 check(!z);
#else
 check(z);
#endif

asm {
	push "z", null, null
	stringlessthan setvariable
};
check_equals(typeof(z), 'boolean');
check(!z);

asm {
	push "z", undefined, undefined
	stringlessthan setvariable
};
check_equals(typeof(z), 'boolean');
check(!z);

// TODO: add saner tests for stringcompare here

//------------------------------------------------
// Bitwise AND operator (ACTION_BITWISEAND : 0x60)
//------------------------------------------------

x=1;
y=1;
check_equals(x&y, 1); 

x=1;
y=2;
check_equals(x&y, 0); 

x = 1.0;
y = 3.0;
check_equals(x&y, 1); 

x = 1.9999;
y = 3.0;
check_equals(x&y, 1); 

x = 1.9999;
y = 3.9999;
check_equals(x&y, 1); 

x = new String("1");
y = new String("3");
check_equals(x&y, 1); 

x = new String("1.0");
y = new String("3.0");
check_equals(x&y, 1); 

x = new String("1.999");
y = new String("3.999");
check_equals(x&y, 1); 

x = new String("3.999");
y = 7;
check_equals(x&y, 3); 

x = Number("7.999");
y = 3;
check_equals(x&y, 3);

check_equals( (undefined&1), 0 );
check_equals( (1&undefined), 0 );
check_equals( (1&null), 0 );
check_equals( (null&1), 0 );
check_equals( (null&null), 0 );
check_equals( (3&2), 2 );
// TODO ... 

check_equals ((-1 & 1), 1);
check_equals ((1 & -1), 1);

check_equals ((-2 & 1), 0);
check_equals ((1 & -2), 0);

//------------------------------------------------
// Bitwise OR operator (ACTION_BITWISEOR : 0x61)
//------------------------------------------------

x = 1;
y = 8;
check_equals(x|y, 9); 

x = 1.1;
y = 8.1;
check_equals(x|y, 9); 

x = 1.999;
y = 8.999;
check_equals(x|y, 9); 

x = new String("1.999");
y = 8.999;
check_equals(x|y, 9); 

x = String("1.999");
y = String("8.999");
check_equals(x|y, 9); 

x = 9;
y = String("1.5");
check_equals(x|y, 9); 

check_equals( (undefined|1), 1 );
check_equals( (1|undefined), 1 );
check_equals( (undefined|undefined), 0 );
check_equals( (null|1), 1 );
check_equals( (1|null), 1 );
check_equals( (null|null), 0 );
check_equals( (8|4), 12 );

// The check below will fail if Ming converts the long int to a double
// (only with Gnash, it works fine with the proprietary player)
check_equals((0xffffffff|0), -1); 

//------------------------------------------------
// Bitwise XOR operator (ACTION_BITWISEOR : 0x62)
//------------------------------------------------

check_equals( (undefined^1), 1 );
check_equals( (1^undefined), 1 );
check_equals( (undefined^undefined), 0 );
check_equals( (null^1), 1 );
check_equals( (1^null), 1 );
check_equals( (null^null), 0 );
check_equals( (8^12), 4 );

x = 1;
y = 2;
check_equals(x^y, 3);

x = 1.1;
y = 2.1;
check_equals(x^y, 3);

x = 1.999;
y = 2.999;
check_equals(x^y, 3);

x = new String("1.999");
y = new String("2.999");
check_equals(x^y, 3);

x = new String("1.999");
y = 2.999;
check_equals(x^y, 3);

x = 1.999;
y = new String("2.999");
check_equals(x^y, 3);

x = 1082401;
y = x^32800;
check_equals(y, 1049601);

//------------------------------------------------
// Shift left operator (ACTION_SHIFTLEFT : 0x63)
//------------------------------------------------

x = 1;
y = x << 2;
check_equals(y, 4);

x = 0xffffffff; // Ming up to 0.4.0.beta4 will convert this to -1 !
                // Newer Ming will store it as a double. Still, the
                // player itself converts it back to -1 as an integer
                // prior to applying the bitwise operator.
y = x << 16;
check_equals(y, -65536);

x = -1;
y = x << 16;
check_equals(y, -65536);

x = 4.294967295e+09;
y = x << 16;
check_equals(y, -65536);

x = 4.29497e+09;
y = x << 16;
check_equals(y, 177209344);

x = 1.9;
y = x << 2;
check_equals(y, 4);

x= undefined;
y = x << 1;
check_equals(typeof(y), 'number');
check(! isnan(y) );
check_equals(y, 0);

check_equals(0 << 1, 0);

x= NaN;
y = x << 1;
check_equals(y, 0);

x = "abcd";
y = x << 1;
check_equals(y, 0);

x = "3";
y = x << 1;
check_equals(y, 6);

x = String("3");
y = x << 1;
check_equals(y, 6);

x = new String("3");
y = x << 1;
check_equals(y, 6); 

y = 2147483647 << Infinity;
check_equals(y,  2147483647);

y = 2147483647 << 0;
check_equals(y, 2147483647);

//------------------------------------------------
// Shift right operator (ACTION_SHIFTRIGHT : 0x64)
//------------------------------------------------

x = 7;
y = x >> 2;
check_equals(y, 1);

x = 0xffffffff;
y = x >> 16;
check_equals(y, -1);

x = 7.9;
y = x >> 2;
check_equals(y, 1);

x= undefined;
y = x >> 1;
check_equals(y, 0);

x= NaN;
y = x >> 1;
check_equals(y, 0);

x = "abcd";
y = x >> 1;
check_equals(y, 0);

x = "7";
y = x >> 1;
check_equals(y, 3);

x = String("7");
y = x >> 1;
check_equals(y, 3);

x = new String("7");
y = x >> 1;
check_equals(y, 3);

x = 32800;
y = x >> 5;
check_equals(y, 1025);

x = -32;
y = x >> 5;
check_equals(y, -1);

x = -1023;
y = x >> 5;
check_equals(y, -32);

x = -32736;
y = x >> 5;
check_equals(y, -1023);

x = 32800;
y = x >> 1082400;
check_equals(y, 32800);

x = 32800;
y = x >> 1082401;
check_equals(y, 16400);

x = 32800;
y = x >> -2;
check_equals(y, 0);



//-------------------------------------------------
// Shift right2 operator (ACTION_SHIFTRIGHT2 : 0x65)
//-------------------------------------------------

y = 1 >>> 3;
check_equals(y, 0);

y = 32 >>> 4;
check_equals(y, 2);

y = -1 >>> Infinity;
check_equals(y, -1);

y = -1 >>> -Infinity;
check_equals(y, -1);

y = -1 >>> 0;
check_equals(y, -1);

y = -1 >>> NaN;
check_equals(y, -1);

y = -1 >>> 32;
check_equals(y, -1);

y = -1 >>> -32;
check_equals(y, -1);

y = -1 >>> 2147483648;
check_equals(y, -1);

y = -1 >>> 4294967296;
check_equals(y, -1);

y = -2 >>> Infinity;
check_equals(y, -2);

y = -2 >>> 32;
check_equals(y, -2);

//-------------------------------------------------
// Strict equality operator (ACTION_STRICTEQ : 0x66)
//-------------------------------------------------

// TODO ...  


//------------------------------------------------
// Decrement Operator (ACTION_DECREMENT: 0x51)
//------------------------------------------------

x = 1;
y = --x;
check_equals(y, 0);

x = 0;
y = --x;
check_equals(y, -1);

x = new String("1.9");
y = --x;
//xcheck_equals(y, 0.9);
check( (y-0.9) < 0.001 );

x = new String("0.0");
y = --x;
//xcheck_equals(y, -1.0);
check( (y+1.0) < 0.001 );

x = new String("a");
y = --x;
check_equals(typeof(y), 'number');
xcheck(y!=NaN); // uh ? is it a different NaN ?
check(! (NaN!=NaN));
check(isNaN(y));
check(isNaN(NaN));

//------------------------------------------------
// Increment Operator (ACTION_DECREMENT: 0x50)
//------------------------------------------------

x = 0;
y = ++x;
check_equals(y, 1);

x = -1;
y = ++x;
check_equals(y, 0);

x = new String("1.9");
y = ++x;
//xcheck_equals(y, 2.9);
check( (y-2.9) < 0.001 );

x = new String("0.0");
y = ++x;
check_equals(y, 1);

x = new String("a");
y = ++x;
xcheck(y!=NaN);
check(isNaN(y));

//------------------------------------------------------
// Less logical not operator (ACTION_LOGICALNOT : 0x12)
//-----------------------------------------------------

check(!"");
#if OUTPUT_VERSION < 7
 check(!"a");
 check(!"true"); 
 check(!"false"); 
 check(!"0000.000"); 
#else
 check("a"); 
 check("true"); 
 check("false"); 
 check("0000.000"); 
#endif

check("1");
check(!false); // doh !
check(true); // doh !
check(!0); 
check(4); 
check(_root); 
check(!null); 
check(!undefined); 

//------------------------------------------------------
// END OF TEST
//-----------------------------------------------------

#if OUTPUT_VERSION < 7
# ifndef MING_LOGICAL_ANDOR_BROKEN
 totals(239);
# else
 totals(216);
# endif
#else
# ifndef MING_LOGICAL_ANDOR_BROKEN
 totals(241);
# else
 totals(218);
# endif
#endif
