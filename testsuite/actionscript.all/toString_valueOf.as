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
 *  Test toString and valueOf.
 */



rcsid="$Id: toString_valueOf.as,v 1.33 2008/04/02 01:52:14 zoulunkai Exp $";
#include "check.as"

//
//Testing toString and valueOf of Function
//
#if OUTPUT_VERSION > 5
  check_equals(typeof(Function), 'function');
  
  check(Function.prototype.hasOwnProperty('apply'));
  check(Function.prototype.hasOwnProperty('call'));
  check(Function.prototype.hasOwnProperty('__proto__'));
  check(Function.prototype.hasOwnProperty('constructor'));
  
  check(!Function.prototype.hasOwnProperty('toString'));
  check(!Function.prototype.hasOwnProperty('valueOf'));
  
  check(Function.prototype.__proto__.hasOwnProperty('toString'));
  check(Function.prototype.__proto__.hasOwnProperty('valueOf'));
  check_equals(Function.prototype.__proto__, Object.prototype);
  check_equals(Object.prototype.__proto__, undefined);
  
  check_equals(typeof(Function.prototype['toString']), 'function');
  check_equals(typeof(Function.prototype['valueOf']), 'function');
#else
  // No Function Class in swf5
  check_equals(typeof(Function), 'undefined');
   
  // test 'function' is supported in swf5:)
  x = function () {};
  check_equals(typeof(x), 'function');
#endif

//
//Testing toString and valueOf of Object
//
#if OUTPUT_VERSION > 5
  check(Object.prototype.hasOwnProperty('toString'));
  check(Object.prototype.hasOwnProperty('valueOf'));
#else
  // hasOwnProperty method is not visible in swf5 by default.
  check(typeof(hasOwnProperty) == 'undefined');
  // Object object still supports toString and valueOf in swf5
  check_equals(typeof(Object.prototype['toString']), 'function');
  check_equals(typeof(Object.prototype['valueOf']), 'function');
  // this is true because a function is considered like an undefined
  // value in equality context.
  check_equals(Object.prototype.toString, undefined);
  check_equals(Object.prototype.valueOf,  undefined);
#endif

obj = new Object();
x = obj.toString();
y = obj.valueOf();
check(typeof(x) == "string");
check(typeof(y) == "object");
check(x == "[object Object]"); 
check(y == obj); //true
//trace(obj); //invoke obj.toString(), output [object Object]
#if OUTPUT_VERSION > 5
  check(obj.toString == Object.prototype.toString);
  check(obj.valueOf == Object.prototype.valueOf);
#endif

obj.toString = function () {return "TO_STRING";};
obj.valueOf = function () {return "TO_VALUE";};
#if OUTPUT_VERSION > 5
  // no longer true in swf6, and a general case in swf6 and above
  check(obj.toString != Object.prototype.toString);
  check(obj.valueOf != Object.prototype.valueOf);
#else
  // this is true only in swf5 (any function is equal to any other in SWF5)
  check(obj.toString == undefined);
  check(obj.valueOf == undefined);
  check(obj.toString == obj.valueOf);
  check(Object.prototype.toString == undefined);
  check(Object.prototype.valueOf == undefined);
#endif
x = obj.toString();
y = obj.valueOf();
check(x=="TO_STRING");
check(y=="TO_VALUE");
check(typeof(obj)=="object"); 
//trace(obj); //invoke obj.toString(), output TO_STRING
check(typeof(y) == "string");
check(obj == y); 

//
//Testing toString and valueOf of Number
//
#if OUTPUT_VERSION > 5
  check(Number.prototype.hasOwnProperty('toString'));
  check(Number.prototype.hasOwnProperty('valueOf'));
  // no longer true in swf6, and a general case in swf6 and above
  check(Number.prototype.toString != Object.prototype.toString);
  check(Number.prototype.valueOf != Object.prototype.valueOf);
#else
  // toString and valueOf are not visible in swf5 by default.
  check(Number.prototype.toString == undefined);
  check(Number.prototype.valueOf == undefined);
#endif


num1 = new Number(1);
x = num1.toString();
y = num1.valueOf();
check(typeof(x)=="string"); 
check(typeof(y)=="number"); 
check(x==1); 
check(y==1); 

Number.prototype.toString = function () {return "TO_STRING"; };
Number.prototype.valueOf = function () {return "TO_VALUE"; };

num2 = new Number(2);
num3 = num1 + num2;
//valueOf called;
check_equals(num3, "TO_VALUETO_VALUE");

x = num1.toString();
y = num1.valueOf();
check(typeof(x)=="string"); 
check(typeof(y)=="string"); 
check(x=="TO_STRING"); 
check(y=="TO_VALUE");  
//trace(num1); // invoke num1.toString(), output TO_STRING

//
//Testing toString and valueOf of String
//
#if OUTPUT_VERSION > 5
  check(String.prototype.hasOwnProperty('toString'));
  check(String.prototype.hasOwnProperty('valueOf'));
  // no longer true in swf6, and a general case in swf6 and above
  check(String.prototype.toString != Object.prototype.toString);
  check(String.prototype.valueOf != Object.prototype.valueOf);
#else
  // toString and valueOf are not visible in swf5 by default.
  check(String.prototype.toString == undefined);
  check(String.prototype.valueOf == undefined);
#endif

str1 = new String("10");
x = str1.toString();
y = str1.valueOf();
check(typeof(x) == "string");
check(typeof(y) == "string");
check(x == 10);
check(y == 10);
check(x == "10");
check(y == "10");
//trace(x);  // output 10
//trace(y);  // output 10

str2 = new String("2");
str3 = str1+str2;
check(str3 == "102"); 
check(str3 == 102); 

String.prototype.toString = function () {return "TO_STRING";};
String.prototype.valueOf = function () {return "TO_VALUE";};

check_equals(parseInt(str1), 10); 
check_equals(parseInt(str2), 2);  
str3 =  str1 + str2;
check(typeof(str3) == "string");
//valueOf called
check_equals(str3, "TO_VALUETO_VALUE"); 
// trace a string Object won't invoke the toString method.
// I don't think it's a bug.
//trace(str1); //output 10 !

x = str1.toString();
y = str1.valueOf();
check(typeof(x) == "string"); 
check(typeof(y) == "string"); 
check(x == "TO_STRING");  
check(y == "TO_VALUE");   


//
//Testing toString and valueOf of movieclip
//
#if OUTPUT_VERSION > 5
  check(!MovieClip.prototype.hasOwnProperty('toString'));
  check(!MovieClip.prototype.hasOwnProperty('valueOf'));
  check_equals(typeof(MovieClip.prototype.toString), 'function');
  check_equals(typeof(MovieClip.prototype.valueOf), 'function');
#endif

// For movieclips, this true from swf5~swf8!
check_equals(MovieClip.prototype.toString, Object.prototype.toString);
check_equals(MovieClip.prototype.valueOf, Object.prototype.valueOf);

_root.createEmptyMovieClip("mc1", 1);
x = mc1.toString();
y = mc1.valueOf();
#if OUTPUT_VERSION > 5
  check_equals(typeof(x), 'string'); 
  check_equals(typeof(y), 'movieclip');  
  check_equals(x, '[object Object]'); 
  check_equals(y, _level0.mc1); 
#else
  // swf5 does not support createEmptyMovieClip by default
  check_equals(typeof(x), 'undefined'); 
  check_equals(typeof(y), 'undefined');  
  check_equals(x, undefined); 
  check_equals(y, undefined); 
#endif
check_equals(y, _level0.mc1);  

//trace a movieclip doesn't invoke the toString method, either.
//trace(mc1); //output _level0.mc

MovieClip.prototype.toString = function () {return "TO_STRING";};
MovieClip.prototype.valueOf = function () {return "TO_VALUE";};

check( mc1 != '_level0.mc1'); // won't invoke toString for comparison
check_equals( mc1, _level0.mc1 ); 

x = mc1.toString();
y = mc1.valueOf();
#if OUTPUT_VERSION > 5
  check(x=="TO_STRING");
  check(y=="TO_VALUE");
#else
  check(x==undefined);
  check(y==undefined);
#endif

_root.createEmptyMovieClip("mc2", 2);
#if OUTPUT_VERSION > 5
  check(typeof(mc1) == "movieclip"); 
  check(typeof(mc2) == "movieclip"); 
  mc3 = mc1 + mc2;
  check(typeof(mc3) == "number");    
  check(isNaN(mc3)); 
#else
  // swf5 does not support createEmptyMovieClip by default
  check(typeof(createEmptyMovieClip) == "undefined");
  check(typeof(mc1) == "undefined"); 
  check(typeof(mc2) == "undefined"); 
#endif

//
//Testing toString and valueOf of TextFields
//
#if OUTPUT_VERSION > 5
  check(!TextField.prototype.hasOwnProperty('toString'));
  check(!TextField.prototype.hasOwnProperty('valueOf'));
  check(typeof(TextField.prototype.toString) == 'function' );
  check(typeof(TextField.prototype.valueOf) == 'function' );
#else
  // TextField in swf5 does not have a prototype by default!
  check_equals(typeof(TextField.prototype), 'undefined'); 
#endif

// For TextFields, this true from swf5~swf8!
check(TextField.prototype.toString == Object.prototype.toString);
check(TextField.prototype.valueOf == Object.prototype.valueOf);

text1 = new TextField();
check(typeof(text1) == "object");
#if OUTPUT_VERSION > 5
  check_equals(typeof(text1.toString), "function");
  check_equals(typeof(text1.__proto__.toString), "function");
#else
  xcheck_equals(typeof(text1.toString), "undefined");
  xcheck_equals(typeof(text1.valueOf), "undefined");
  check_equals(typeof(text1.__proto__), 'object');
  xcheck_equals(typeof(text1.__proto__.toString), "undefined"); 
  xcheck_equals(typeof(text1.__proto__.valueOf), "undefined"); 
#endif

x = text1.toString();
y = text1.valueOf();
#if OUTPUT_VERSION > 5 
  check(typeof(x) == "string");   
  check(typeof(y) == "object");  
  check(x == "[object Object]"); 
  check(y.toString() == "[object Object]"); 
  check(typeof(y.valueOf()) == "object"); 
#else
  xcheck_equals(typeof(x), "undefined");   
  xcheck_equals(typeof(y), "undefined");  
  xcheck_equals(x, undefined); 
  xcheck_equals(y.toString(), undefined); 
  xcheck_equals(typeof(y.valueOf()),  "undefined"); 
#endif 
check_equals(y, text1);

text1.toString = function() { return "A STRING"; };
check(text1 != "A STRING");
a = "prefix_"+text1;
#if OUTPUT_VERSION > 5
 check_equals(a, "prefix_A STRING");
#else
 xcheck_equals(a, "prefix_");
#endif

//
//Testing toString and valueOf of Buttons
//

#if OUTPUT_VERSION > 5 
// hasOwnProperty method is not visible in swf5 by default.
check(!Button.prototype.hasOwnProperty('toString'));
check(!Button.prototype.hasOwnProperty('valueOf'));
#endif

check_equals(typeof(Button.prototype.toString), 'function' );
check_equals(typeof(Button.prototype.valueOf), 'function' );

// For Buttons, this true from swf5~swf8!
#if OUTPUT_VERSION < 6
 check(Button.prototype.toString == Object.prototype.toString);
 check(Button.prototype.valueOf == Object.prototype.valueOf);
 check_equals(Button.prototype.toString, undefined);
 check_equals(Button.prototype.valueOf, undefined);
#else
 check_equals(Button.prototype.toString, Object.prototype.toString);
 check_equals(Button.prototype.valueOf, Object.prototype.valueOf);
#endif

btn1 = new Button();
check_equals(typeof(btn1), "object");
x = btn1.toString();
y = btn1.valueOf();
check_equals(typeof(x), "string");  
check_equals(typeof(y), "object");   
check_equals(x, "[object Object]");  
check_equals(y.toString(), "[object Object]"); 
check_equals(typeof(y.valueOf()), "object");  
check_equals(typeof(btn1), "object");
check(y == btn1);
//trace(btn1); // invoke btn1.toString(), output [object Object]

//
//Testing toString and valueOf of Boolean
//

#if OUTPUT_VERSION > 5 
  check(Boolean.prototype.toString != Object.prototype.toString);
  check(Boolean.prototype.valueOf != Object.prototype.valueOf);
#else
  check(Boolean.prototype.toString == undefined);
  check(Boolean.prototype.valueOf == undefined);
#endif

b1 = new Boolean(false);
check(typeof(b1) == "object");
x = b1.toString();
y = b1.valueOf();
check(typeof(x) == "string");   
check(typeof(y) == "boolean");  
check(x == "false");
check(y == false);
//trace(b1); // invoke b1.toString(), output false 

b2 = new Boolean(true);
b3 = b1 + b2;
check(typeof(b3) == 'number');
check_equals(b3, 1);


//
//Testing toString and valueOf of Date
//

#if OUTPUT_VERSION > 5 
  check(Date.prototype.toString != Object.prototype.toString);
  check(Date.prototype.valueOf != Object.prototype.valueOf);
#else
  check(Date.prototype.toString == undefined);
  check(Date.prototype.valueOf == undefined);
#endif

d1 = new Date(0);
check_equals(typeof(d1), "object");
x = d1.toString();
y = d1.valueOf();
check_equals(typeof(x), "string");  
check_equals(typeof(y), "number");   
// NOTE: the value of toString() here depends on timezone !
//       It's the epoch, but depending on timezone the GMT+<x> and actual hour change
//       For this reason the test is disabled till a solution is found.
//check_equals(x, "Thu Jan 1 08:00:00 GMT+0800 1970"); 
check_equals(y, 0);

d2 = new Date(1);
d3 = d1 + d2; // in SWF5 this should result in a number, in SWF6 or higher, in a string
exp = d1.toString() + d2.toString();
#if OUTPUT_VERSION > 5 
  check_equals(typeof(d3), 'string');
  check_equals(d3, exp);
#else
  check_equals(typeof(d3), 'number');
  check_equals(d3, 1);
#endif

// Date(0) == Date(1) 
check_equals(d1.toString(), d2.toString());

//
//Testing toString and valueOf of Array
//

#if OUTPUT_VERSION > 5 
  check(Array.prototype.hasOwnProperty('toString'));
  check(!Array.prototype.hasOwnProperty('valueOf'));
  check(Array.prototype.toString != Object.prototype.toString);
#else
  check(Array.prototype.toString == undefined);
#endif

check(Array.prototype.valueOf == Object.prototype.valueOf);

a1 = new Array(1,2,3,4);
check(typeof(a1) == "object");
x = a1.toString();
y = a1.valueOf();
check(typeof(x) == "string");  
check(typeof(y) == "object");   
check(x == "1,2,3,4");  
check(y == a1);   
//trace(a1); // invoke a1.toString(), output 1,2,3,4     

a2 = new Array(2,3,4,5);
a3 = a1 + a2;
check(typeof(a3) == 'number');
check(isNaN(a3));


var v = function () {
        this.valueOfCalls++;
        return this.v;
};
var s = function () {
        this.toStringCalls++;
        return this.v;
};


var o = new Object ();
o.valueOfCalls = 0;
o.toStringCalls = 0;
o.valueOf = v;
o.toString = s;
o.v = new Object();
a = "" + o;
check_equals(o.valueOfCalls, 1);
check_equals(o.toStringCalls, 1);
check_equals(typeof(a), "string");
check_equals(a, "[type Object]");


#if OUTPUT_VERSION < 6
 check_totals(126);
#else
 check_totals(144);
#endif
