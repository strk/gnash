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


#include "check.as"

//
//Testing toString and valueOf of Object
//
#if OUTPUT_VERSION > 5
  check(Object.prototype.hasOwnProperty('toString'));
  check(Object.prototype.hasOwnProperty('valueOf'));
#else
  // swf5 does not support hasOwnProperty method
  check(typeof(hasOwnProperty) == 'undefined');
  check(!Object.prototype.hasOwnProperty('toString'));
  check(!Object.prototype.hasOwnProperty('valueOf'));
#endif

obj = new Object();
x = obj.toString();
y = obj.valueOf();
check(typeof(x) == "string");
check(typeof(y) == "object");
check(x == "[object Object]"); 
check(y == obj); //true
//trace(obj); //output [object Object]
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
  // this is true only in swf5
  xcheck(obj.toString == Object.prototype.toString);
  xcheck(obj.valueOf == Object.prototype.valueOf);
#endif
x = obj.toString();
y = obj.valueOf();
check(x=="TO_STRING");
check(y=="TO_VALUE");
check(typeof(obj)=="object"); 
//trace(obj); // output TO_STRING
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
  // this is true only in swf5
  xcheck(Number.prototype.toString == Object.prototype.toString);
  xcheck(Number.prototype.valueOf == Object.prototype.valueOf);
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
check(num3 == "TO_VALUETO_VALUE");

x = num1.toString();
y = num1.valueOf();
check(typeof(x)=="string"); 
check(typeof(y)=="string"); 
check(x=="TO_STRING"); 
check(y=="TO_VALUE");  

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
  // this is true only in swf5
  xcheck(String.prototype.toString == Object.prototype.toString);
  xcheck(String.prototype.valueOf == Object.prototype.valueOf);
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
xcheck(str3 == "102"); 
xcheck(str3 == 102); 

String.prototype.toString = function () {return "TO_STRING";};
String.prototype.valueOf = function () {return "TO_VALUE";};

xcheck(parseInt(str1) == 10); 
xcheck(parseInt(str2) == 2);  
str3 =  str1 + str2;
xcheck(typeof(str3) == "string");
//valueOf called
check(str3 == "TO_VALUETO_VALUE"); 
//trace a string won't invoke the toString method
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
  check(typeof(MovieClip.prototype.toString) == 'function');
  check(typeof(MovieClip.prototype.valueOf) == 'function');
#endif

// For movieclips, this true from swf5~swf8!
check(MovieClip.prototype.toString == Object.prototype.toString);
check(MovieClip.prototype.valueOf == Object.prototype.valueOf);

_root.createEmptyMovieClip("mc1", 1);
x = mc1.toString();
y = mc1.valueOf();
#if OUTPUT_VERSION > 5
  check(typeof(x) == 'string'); 
  check(typeof(y) == 'movieclip');  
  xcheck(x == '[object Object]'); 
  check(y == _level0.mc1); 
#else
  check(typeof(x) == 'undefined'); 
  check(typeof(y) == 'undefined');  
  check(x == undefined); 
  check(y == undefined); 
#endif
check(y == _level0.mc1);  
//trace a movieclip doesn't invoke the toString method
//trace(mc1); //_level0.mc

MovieClip.prototype.toString = function () {return "TO_STRING";};
MovieClip.prototype.valueOf = function () {return "TO_VALUE";};

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
  // swf5 does not support createEmptyMovieClip
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
  // TextField in swf5 does not have toString and valueOf methods(to be checked).
  xcheck(typeof(TextField.prototype.toString) == 'undefined' );
  xcheck(typeof(TextField.prototype.valueOf) == 'undefined' );
#endif

// For TextFields, this true from swf5~swf8!
check(TextField.prototype.toString == Object.prototype.toString);
check(TextField.prototype.valueOf == Object.prototype.valueOf);

text1 = new TextField();
check(typeof(text1) == "object");
x = text1.toString();
y = text1.valueOf();
#if OUTPUT_VERSION > 5 
  check(typeof(x) == "string");   
  check(typeof(y) == "object");  
  check(x == "[object Object]"); 
  check(y.toString() == "[object Object]"); 
  check(typeof(y.valueOf()) == "object"); 
#else
  xcheck(typeof(x) == "undefined");   
  xcheck(typeof(y) == "undefined");  
  xcheck(x == undefined); 
  xcheck(y.toString() == undefined); 
  xcheck(typeof(y.valueOf()) ==  "undefined"); 
#endif 
check(y == text1);

//
//Testing toString and valueOf of Buttons
//

#if OUTPUT_VERSION > 5 
// swf5 does not support hasOwnProperty method
check(!Button.prototype.hasOwnProperty('toString'));
check(!Button.prototype.hasOwnProperty('valueOf'));
#endif

xcheck(typeof(Button.prototype.toString) == 'function' );
xcheck(typeof(Button.prototype.valueOf) == 'function' );

// For Buttons, this true from swf5~swf8!
xcheck(Button.prototype.toString == Object.prototype.toString);
xcheck(Button.prototype.valueOf == Object.prototype.valueOf);

btn1 = new Button();
xcheck(typeof(btn1) == "object");
x = btn1.toString();
y = btn1.valueOf();
xcheck(typeof(x) == "string");  
xcheck(typeof(y) == "object");   
xcheck(x == "[object Object]");  
xcheck(y.toString() == "[object Object]"); 
xcheck(typeof(y.valueOf()) == "object");  
xcheck(typeof(btn1) == "object");
check(y == btn1);
//trace(btn1); //output [object Object]

//
//Testing toString and valueOf of Boolean
//

#if OUTPUT_VERSION > 5 
  check(Boolean.prototype.toString != Object.prototype.toString);
  check(Boolean.prototype.valueOf != Object.prototype.valueOf);
#else
  xcheck(Boolean.prototype.toString == Object.prototype.toString);
  xcheck(Boolean.prototype.valueOf == Object.prototype.valueOf);
#endif

b1 = new Boolean(false);
check(typeof(b1) == "object");
x = b1.toString();
y = b1.valueOf();
check(typeof(x) == "string");   
check(typeof(y) == "boolean");  
check(x == "false");
check(y == false);

b2 = new Boolean(true);
b3 = b1 + b2;
check(typeof(b3) == 'number');
xcheck(b3 == 1);


//
//Testing toString and valueOf of Date
//

#if OUTPUT_VERSION > 5 
  check(Date.prototype.toString != Object.prototype.toString);
  check(Date.prototype.valueOf != Object.prototype.valueOf);
#else
  xcheck(Date.prototype.toString == Object.prototype.toString);
  xcheck(Date.prototype.valueOf == Object.prototype.valueOf);
#endif

d1 = new Date(0);
check(typeof(d1) == "object");
x = d1.toString();
y = d1.valueOf();
check(typeof(x) == "string");  
check(typeof(y) == "number");   
check(x == "Thu Jan 1 08:00:00 GMT+0800 1970");
check(y == 0);

d2 = new Date(1);
d3 = d1 + d2;
#if OUTPUT_VERSION > 5 
  xcheck(typeof(d3) == 'string');
  xcheck(d3 == "Thu Jan 1 08:00:00 GMT+0800 1970Thu Jan 1 08:00:00 GMT+0800 1970");
#else
  check(typeof(d3) == 'number');
  check(d3 == 1);
#endif

//
//Testing toString and valueOf of Array
//

#if OUTPUT_VERSION > 5 
  check(Array.prototype.hasOwnProperty('toString'));
  check(!Array.prototype.hasOwnProperty('valueOf'));
  check(Array.prototype.toString != Object.prototype.toString);
#else
  xcheck(Array.prototype.toString == Object.prototype.toString);
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

a2 = new Array(2,3,4,5);
a3 = a1 + a2;
check(typeof(a3) == 'number');
check(isNaN(a3));

