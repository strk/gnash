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

rcsid="$Id: ops.as,v 1.2 2007/08/02 03:48:32 zoulunkai Exp $";

#include "check.as"

//--------------------------------------------
// Equality operator (ACTION_NEWEQUALS : 0x49)
//--------------------------------------------

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

// for Arrays
ary1 = [1,2,3];
ary2 = [1,2,3];
check( ! (ary1 == ary2) ); // two different objects
check( ! (ary1 == "1,2,3") ); // the array doesn't get converted to a string
xcheck( ! ("1,2,3" == ary1) ); // the array doesn't get converted to a string

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
xcheck( ! (str1 == NaN) ); // str1 (object) is NOT converted to a number (due to NaN?)

#if OUTPUT_VERSION > 5
  check( ! (str1 == str2) ); // they are not the same object
#else // OUTPUT_VERSION <= 5
  xcheck( str1 == str2 );  // SWF5 automatically converts to a string for comparison !
#endif // OUTPUT_VERSION <= 5


//---------------------------------------------
// Less then operator (ACTION_LESSTHAN : 0x0F)
//---------------------------------------------

x = 0.999;
y = 1.0;
check(x<y);

x=String("0.999");
y=String("1.0");
xcheck(x<y);

x=String("A");
y=String("a");
xcheck(x<y);

x=String("abc");
y=String("abcd");
xcheck(x<y);

x=0.999;
y=String("1.000");
xcheck(x<y);

x=String("0.999");
y=1.0;
xcheck(x<y);

//------------------------------------------------
// Logical AND operator (ACTION_LOGICALAND : 0x10)
//------------------------------------------------
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
check_equals(z, x); 

x=String("1.999");
y=true;
z=x && y;
check_equals(z, y);

x=String("adcd");
y=true;
z=x && y;
#if OUTPUT_VERSION < 7
	xcheck_equals(z, false);
#else
	check_equals(z, true);
#endif 

x=true;
y=String("adcd");
z=x && y;
#if OUTPUT_VERSION < 7
	xcheck_equals(z, false);
#else
	check_equals(z, true);
#endif

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
check_equals(z, y); 

//------------------------------------------------
// Logical OR operator (ACTION_LOGICALOR : 0x11)
//------------------------------------------------

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
xcheck(z!=y); 
check_equals(z, true); 

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
	xcheck_equals(z, false); 
#else
	check_equals(z, true);
#endif

x = 0;
y = new String("abcd");
z = x || y;
check_equals(z, true);

x = 0;
y = Object();
z = x || y;
check_equals(z, true);

x = 0;
y = new Object();
z = x || y;
check_equals(z, true);

x = 0;
y = "0";
z = x || y;
#if OUTPUT_VERSION < 7
	check_equals(z, false);
#else
	check_equals(z, true);
#endif

x = 0;
y = String("0");
z = x || y;
#if OUTPUT_VERSION < 7
	xcheck_equals(z, false);
#else
	check_equals(z, true);
#endif

x = 0;
y = new String("0");
z = x || y;
xcheck(z!=y);
check_equals(z, true);


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
// TODO ... 

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
// TODO ... 

//------------------------------------------------
// Shift left operator (ACTION_SHIFTLEFT : 0x63)
//------------------------------------------------

// TODO ... 

//------------------------------------------------
// Shift right operator (ACTION_SHIFTRIGHT : 0x64)
//------------------------------------------------

// TODO ... 

//-------------------------------------------------
// Shift right2 operator (ACTION_SHIFTRIGHT2 : 0x65)
//-------------------------------------------------

// TODO ... 

//-------------------------------------------------
// Strict equality operator (ACTION_STRICTEQ : 0x66)
//-------------------------------------------------

// TODO ...  
