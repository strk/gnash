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

// Test case for Object ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="$Id: Object.as,v 1.66 2008/04/28 16:10:00 bwy Exp $";
#include "check.as"

// Test things in Class Object (swf5~swf8)
check_equals(typeof(Object), 'function');
check_equals(typeof(Object.prototype), 'object');
check_equals(typeof(Object.constructor), 'function');
#if OUTPUT_VERSION == 5
	check_equals(typeof(Object.__proto__), 'undefined');
	// make Object.__proto__  visible in swf5
	ASSetPropFlags(Object, null, 8, 128 + 1);
#endif
check_equals(typeof(Object.__proto__), 'object');

// registerClass is a public static function of Object
check_equals(typeof(Object.registerClass), 'function');
check_equals(Object.prototype.registerClass, undefined);

// Test things in Object.prototype (swf5~swf8)
check_equals(typeof(Object.prototype.toString), 'function');
check_equals(typeof(Object.prototype.toLocaleString), 'function');
check_equals(typeof(Object.prototype.valueOf), 'function');
check_equals(typeof(Object.prototype.constructor), 'function'); 
#if OUTPUT_VERSION == 5
	check_equals(typeof(Object.prototype.addProperty), 'undefined');
	check_equals(typeof(Object.prototype.watch), 'undefined');
	check_equals(typeof(Object.prototype.unwatch), 'undefined');
	check_equals(typeof(Object.prototype.isPropertyEnumerable), 'undefined'); 
	check_equals(typeof(Object.prototype.isPrototypeOs), 'undefined');
	check_equals(typeof(Object.prototype.hasOwnProperty), 'undefined'); 
	// make functions above visible
	ASSetPropFlags(Object.prototype, null, 8, 128 + 1);
#endif
// These functions are visible now even in swf5 (swf5 ~ swf8)
check_equals(typeof(Object.prototype.addProperty), 'function');
check_equals(typeof(Object.prototype.watch), 'function');
check_equals(typeof(Object.prototype.unwatch), 'function');
check_equals(typeof(Object.prototype.isPropertyEnumerable), 'function'); 
check_equals(typeof(Object.prototype.isPrototypeOf), 'function');
check_equals(typeof(Object.prototype.hasOwnProperty), 'function'); 

check_equals(Object.prototype.constructor, Object); 
check_equals(typeof(Object.prototype.toString.constructor), 'function');

#if OUTPUT_VERSION > 5
 // Function Object only exists in swf6 and above
 check_equals(Object.prototype.toString.constructor, Function);
#else
 check_equals(typeof(Function), 'undefined');
#endif
check_equals(Object.prototype.prototype, undefined);

// The bug below (no end in the inheritance chain) triggers endless
// recursions during run of the trace_properties() function defined
// in trace_properties.as from swfdec testsuite.
// WE WANT THIS FIXED !!
check_equals(Object.prototype.__proto__, undefined);
check(!Object.prototype.hasOwnProperty("__proto__"));


#if OUTPUT_VERSION > 5

// Through test of existance of methods!

check(Object.hasOwnProperty('__proto__'));

O = Object;
check_equals(O, Object);

// found 4 methods in Object
check(O.hasOwnProperty('__proto__'));
check(O.hasOwnProperty('registerClass'));
check(O.hasOwnProperty('constructor'));
check(O.hasOwnProperty('prototype'));

// fount 4 methods in Object.__proto__
check(O.__proto__.hasOwnProperty('__proto__'));
check(O.__proto__.hasOwnProperty('apply'));
check(O.__proto__.hasOwnProperty('call'));
check(O.__proto__.hasOwnProperty('constructor'));

check(O.__proto__ != Object.prototype);
check(O.__proto__.__proto__ == Object.prototype);

// found 3 methods in Object.constructor
check(O.constructor.hasOwnProperty('__proto__'));
check(O.constructor.hasOwnProperty('constructor'));
check(O.constructor.hasOwnProperty('prototype'));

// found 9 methods in Object.prototype
check(O.prototype.hasOwnProperty('addProperty'));
check(O.prototype.hasOwnProperty('constructor'));
check(O.prototype.hasOwnProperty('hasOwnProperty'));
check(O.prototype.hasOwnProperty('isPropertyEnumerable'));
check(O.prototype.hasOwnProperty('isPrototypeOf'));
check(O.prototype.hasOwnProperty('toString'));
check(O.prototype.hasOwnProperty('valueOf'));
check(O.prototype.hasOwnProperty('unwatch'));
check(O.prototype.hasOwnProperty('watch'));

// found 3 methods in Object.prototype.constructor
check(O.prototype.constructor.hasOwnProperty('__proto__'));
check(O.prototype.constructor.hasOwnProperty('constructor'));
check(O.prototype.constructor.hasOwnProperty('prototype'));

check_equals(O.prototype.constructor, Object);
check_equals(O.constructor, Function);

check_equals(typeof(Object.prototype.addProperty), 'function');
check_equals(typeof(Object.prototype.hasOwnProperty), 'function');
check_equals(typeof(Object.prototype.isPropertyEnumerable), 'function');
check_equals(typeof(Object.prototype.isPrototypeOf), 'function');
check_equals(typeof(Object.prototype.watch), 'function');
check_equals(typeof(Object.prototype.unwatch), 'function');
#endif

// Test Object creation using 'new'
var obj = new Object; // uses SWFACTION_NEWOBJECT
check (obj != undefined);
check_equals (typeof(obj), "object");

check_equals(obj.__proto__, Object.prototype);
check_equals(typeof(obj.prototype), 'undefined');
check_equals(typeof(obj.__proto__), 'object');

#if OUTPUT_VERSION == 5
	check(obj.__constructor__ == undefined);
	// make obj.__constructor__ visible
	ASSetPropFlags(obj, null, 8, 128 + 1);
	check(obj.__constructor__ == Object);
#else
	check(obj.__constructor__ == Object);
#endif

check(obj.constructor == Object);
check(Object.prototype.constructor == Object); 
check (obj.__proto__.constructor == Object);

// Test instantiated Object methods
check_equals(typeof(obj.toString), 'function');
check_equals(typeof(obj.valueOf), 'function');
check_equals(typeof(obj.addProperty), 'function');

check(Object.hasOwnProperty('constructor'));
#if OUTPUT_VERSION == 6
	check(obj.hasOwnProperty('constructor'));  
#elif  OUTPUT_VERSION > 6
	// seems swf7 and above are different.
	check(!obj.hasOwnProperty('constructor'));  
#endif
// swf5~8
check(obj.hasOwnProperty('__constructor__')); 

// Test instantiated Object members
obj.member = 1;
check (obj.member == 1)
check_equals(typeof(obj.toString()), 'string');
check_equals(obj.toString(), '[object Object]');
check_equals(typeof(obj.valueOf()), 'object');
check_equals(obj.valueOf(), obj);

// Test Object creation using literal initialization
var obj2 = { member:1 }; // uses SWFACTION_INITOBJECT
check_equals(typeof(obj2), "object");
check_equals(typeof(obj2.__proto__), 'object');
check_equals(obj2.__proto__, Object.prototype);
check_equals(obj2.__proto__.constructor, Object);
check_equals(typeof(obj2.prototype), 'undefined');


// Test initialized object members
check ( obj2.member == 1 )

// Test Object creation using initializing constructor
var obj3 = new Object({ member:1 });
check (obj3 != undefined);
check (typeof(obj3) == "object");
check (obj3.__proto__.constructor == Object);

// Test initialized object members
check ( obj3.member != undefined );
check ( obj3.member == 1 );

// Test after-initialization members set/get
obj3.member2 = 3;
check ( obj3.member2 != undefined );
check ( obj3.member2 == 3 );

//----------------------
// Test copy ctors
//----------------------

var copy = new Object(obj3);
check_equals( copy.member2, 3 );
copy.test = 4;
check_equals( copy.test, 4 );
check_equals( obj3.test, 4 );
check (copy.__proto__.constructor == Object);


//---------------------------------------------
// Test addProperty / hasOwnProperty (SWF6 up)
//---------------------------------------------

// Object.addProperty wasn't in SWF5
#if OUTPUT_VERSION > 5

// the 'getter' function
function getLen() {
  return this._len;
}

// the 'setter' function
function setLen(l) {
  this._len = l;
}

// add the "len" property
var ret = obj3.addProperty("len", getLen, setLen);
check_equals(ret, true);

// toString is not obj3 "own" property, but it's inherited !
check_equals(typeof(obj3.toString), 'function');
ownPropertyCheck = obj3.hasOwnProperty("toString");
check_equals(typeof(ownPropertyCheck), 'boolean');
check(!ownPropertyCheck);

// 'member' is an obj3 "own" member (not inherited) 
ownPropertyCheck = obj3.hasOwnProperty("member");
check_equals(typeof(ownPropertyCheck), 'boolean');
check(ownPropertyCheck);

// 'len' is an obj3 "own" property (not inherited) 
ownPropertyCheck = obj3.hasOwnProperty("len");
check_equals(typeof(ownPropertyCheck), 'boolean');
check(ownPropertyCheck);

check_equals (obj3.len, undefined);
obj3._len = 3;
check_equals (obj3.len, 3);
obj3.len = 5;
check_equals (obj3._len, 5);
check_equals (obj3.len, 5);

// TODO: try omitting the "setter" argument
var ret = obj3.addProperty("len2", getLen);
check_equals(ret, false);
check_equals (obj3.len2, undefined);
obj3.len2 = 'test';
check_equals (obj3.len2, 'test');

obj3.__proto__ = undefined;
check_equals(typeof(obj3), "object");
check_equals(typeof(obj3.__proto__), 'undefined');
xcheck_equals(obj3, undefined);

// Use name of an existing property

o = {};
o.test = 5;
function test_get() { _root.test_get_calls++; return this.test; }
function test_set(v) { this.test=v; _root.test_set_calls++; }
test_set_calls=test_get_calls=0;
r = o.addProperty("test", test_get, test_set);
check(r);
check_equals(test_set_calls, 0);
check_equals(test_get_calls, 0);
test_set_calls=test_get_calls=0;
v = o.test;
check_equals(test_set_calls, 0);
#if OUTPUT_VERSION < 7
 check_equals(test_get_calls, 1);
#else
 xcheck_equals(test_get_calls, 65); // urgh ! :)
#endif
check_equals(v, 5); // underlying value was initializied to existing prop
test_set_calls=test_get_calls=0;
o.test = 16; // should change underlying as well I guess
check_equals(test_get_calls, 0);
#if OUTPUT_VERSION < 7
 check_equals(test_set_calls, 1);
#else
 xcheck_equals(test_set_calls, 65); // urgh ! :)
#endif
test_set_calls=test_get_calls=0;
r = o.addProperty("test", test_get, test_set);
check(r);
check_equals(test_get_calls, 0); // didn't invoke the former getter..
check_equals(test_set_calls, 0); // .. to fetch underlying var

test_set_calls=test_get_calls=0;
v = o.test;
// got underlying value from previous getter-setter
check_equals(v, 16);
#if OUTPUT_VERSION < 7
 check_equals(test_get_calls, 1);
#else
 xcheck_equals(test_get_calls, 65); // urgh ! :)
#endif
check_equals(test_set_calls, 0);

// Existing property higher in inheritance chain

delete o.test;
o2 = {};
o2.test = 19;
o.__proto__ = o2;
check_equals(o.test, 19); 
r = o.addProperty("test", test_get, test_set);
check(r);
v = o.test;
check_equals(v, undefined); // but not existing prop from inheritance chain

// Existing native property 

o = createEmptyMovieClip("hello", 10);
check_equals(o._target, "/hello");
function target_get() { _root.target_get_calls++; return this._target; }
function target_set(v) { this._target=v; _root.target_set_calls++; }
target_get_calls=target_set_calls=0;
o.addProperty("_target", target_get, target_set);
check_equals(_root.target_get_calls, 0);
check_equals(_root.target_set_calls, 0);
check_equals(typeof(o._target), "undefined"); // native getter-setter don't get initialized with underlying value

// Try property inheritance

var proto = new Object();
check(proto.addProperty("len", getLen, setLen));
var inh1 = new Object();
inh1.__proto__ = proto;
var inh2 = new Object();
inh2.__proto__ = proto;
check_equals (inh1._len, undefined);
check_equals (inh2._len, undefined);
inh1.len = 4; 
inh2.len = 9;
check_equals (inh1._len, 4);
check_equals (inh2._len, 9);
check_equals (proto._len, undefined);
inh1._len = 5;
inh2._len = 7;
check_equals (inh1.len, 5);
check_equals (inh2.len, 7);
check_equals (proto.len, undefined);

inh2.ogs = 5; // overridden getter-setter
var inh2d = new Object;
check(proto.addProperty("ogs", getLen, setLen));
inh2d.__proto__ = inh2;
inh2d._len = 8;
proto._len = 14;

//
// Inheritance is: inh2d : inh2 : proto
//	inh2d.ogs doesn't exist at this time
//	inh2.ogs is a normal member (value: 5)
//	proto.ogs is a getter-setter (changes: this._len)
//
check( ! inh2d.hasOwnProperty("ogs") );
check( inh2d.__proto__.hasOwnProperty("ogs") );
check( inh2d.__proto__.__proto__.hasOwnProperty("ogs") );
check_equals(inh2d.ogs, 5); // find inh2.ogs
check_equals(inh2.ogs, 5); // find inh2.ogs
check_equals(inh2.__proto__.ogs, 14); // find proto.ogs
inh2d.ogs = 54; // sets what ?
check_equals(inh2d._len, 54); // this._len ! So the getter-setter takes precedence
check_equals(inh2d.ogs, 5); // does NOT override inh2.ogs normal member
check_equals(inh2.ogs, 5); // find inh2.ogs
check_equals(inh2.__proto__.ogs, 14); // find proto.ogs

// Override addProperty member
var o = new Object();
o.addProperty = function(a, b) { return a+b; };
var c = o.addProperty(2,5);
check_equals(c, 7);
check(o.addProperty != Object.prototype.addProperty );

// recursive setter
mem_setter = function(x)
{
	this.mem2 = x;
	this.mem = x;
};
mem_getter = function()
{
	return this.mem;
};
o = {};
o.addProperty('mem', mem_getter, mem_setter);
check_equals(typeof(o.mem), 'undefined');
o.mem = 3; // watch out for recursion !
check_equals(o.mem, 3);
check_equals(o.mem2, 3);

// Test double-recursion of setter:
//  setter1 triggers setter2

mem1_getter = function() { return this.mem1; };
mem1_setter = function(x)
{
	if ( this.setterCalls > 2 )
	{
		return;
	}
	++this.setterCalls;
	o2.mem2 = x;
	this.mem1 = x;
};

mem2_getter = function() { return this.mem2; };
mem2_setter = function(x)
{
	if ( this.setterCalls > 2 )
	{
		return;
	}
	++this.setterCalls;
	o1.mem1 = x;
	this.mem2 = x;
};

o1 = {}; o1.addProperty('mem1', mem1_getter, mem1_setter);
o2 = {}; o2.addProperty('mem2', mem2_getter, mem2_setter);

o1.setterCalls = o2.setterCalls = 0; // reset counters
o1.mem1 = 3;
#if OUTPUT_VERSION == 6
 check_equals(o1.setterCalls, 1);
 check_equals(o2.setterCalls, 1);
#else
 // SWF7+ doesn't protect recursion..
 xcheck_equals(o1.setterCalls, 3);
 xcheck_equals(o2.setterCalls, 3);
#endif
check_equals(o1.mem1, 3);
check_equals(o1.mem1, 3);

o1.setterCalls = o2.setterCalls = 0; // reset counters
o2.mem2 = 6;
#if OUTPUT_VERSION == 6
 check_equals(o1.setterCalls, 1);
 check_equals(o2.setterCalls, 1);
#else
 // SWF7+ doesn't protect recursion..
 xcheck_equals(o1.setterCalls, 3);
 xcheck_equals(o2.setterCalls, 3);
#endif
check_equals(o1.mem1, 6);
check_equals(o2.mem2, 6);

// Test having a getter but not a setter
getter = function() { _root.getcalls++; return this[_root.retwhat]; };
o = {};
r = o.addProperty('lmissing', getter);
check(!r);
r = o.addProperty('lundef', getter, undefined);
check(!r);
r = o.addProperty('lnull', getter, o); // self as setter..
check(!r);
r = o.addProperty('lnull', getter, null);
check(r);
getcalls=0;
t=o.lnull;
check_equals(getcalls, 1);
o.lnull = 5;
_root.retwhat='lnull';
check_equals(o.lnull, 5);

// Test having a setter but not a getter (invalid)
setter = function() { _root.setcalls++; };
o = {};
r = o.addProperty('lmissing', undefined, setter);
check(!r);
r = o.addProperty('lundef', null, setter);
check(!r);

// not-setting setter
noset_setter = function(v) { noset_setter_calls++; }; // doesn't set cache
simple_test_getter = function() { return this.test; };
o = {};
o.addProperty("test", simple_test_getter, noset_setter);
noset_setter_calls=0;
o.test = 2;
check_equals(noset_setter_calls, 1);
v = o.test;
check_equals(v, 2); // did still set the cache
o.test = 5;
check_equals(noset_setter_calls, 2);
v = o.test;
check_equals(v, 5);

// test setter visibility of value (multiplies * 2)
timetwo_test_setter = function(v) {
	// note("timetwo_test_setter sees this.test as "+this.test);
	this.test *= 2;
};
o = {};
o.test = 1;
o.addProperty("test", simple_test_getter, timetwo_test_setter);
o.test = 2;
v = o.test;
check_equals(v, 2);
o.test = 5;
v = o.test;
check_equals(v, 5);

///
/// more sane getter setter test
///
obj8 = {};
obj8_getter_cnt = 0;
obj8_setter_cnt = 0;
function obj8_getter () { 
	obj8_getter_cnt++;
	return obj8_prop; 
}
function obj8_setter (v) { 
	obj8_setter_cnt++;
	obj8_prop = v; 
}
obj8.addProperty("obj8_prop", obj8_getter, obj8_setter);
obj8.obj8_prop = 1;
v = obj8.obj8_prop;
check_equals(obj8_getter_cnt, 1);
check_equals(obj8_setter_cnt, 1);

obj9 = {};
obj9_getter_cnt = 0;
obj9_setter_cnt = 0;
function obj9_getter () { 
	obj9_getter_cnt++;
	return this.obj9_prop_cache; 
}
function obj9_setter (v) { 
	obj9_setter_cnt++;
	this.obj9_prop_cache = v; 
}
obj9.addProperty("obj9_prop", obj9_getter, obj9_setter);
obj9.obj9_prop = 10;
check_equals(obj9_getter_cnt, 0);
check_equals(obj9_setter_cnt, 1);
check_equals(obj9.obj9_prop, 10);
check_equals(obj9_getter_cnt, 1);
check_equals(obj9_setter_cnt, 1);

// Object.addProperty wasn't in SWF5
#endif // OUTPUT_VERSION > 5

//----------------------------------------------------
// Test Object.toString 
//----------------------------------------------------

check_equals(Object.prototype.toString(), '[object Object]');
backup = Object.prototype.toString;
Object.prototype.toString = function() { return new Object; };
check_equals(typeof(Object.prototype.toString()), 'object');
Object.prototype.toString = function() { return new Number; };
check_equals(Object.prototype.toString(), 0);
Object.prototype.toString = backup;

/// This will ruin later tests while it fails hasOwnProperty.
//xcheck_equals(Object.prototype.toLocaleString(), '[object Object]');
//backup = Object.prototype.toLocaleString;
//Object.prototype.toLocaleString = function() { return new Object; };
//xcheck_equals(typeof(Object.prototype.toLocaleString()), 'object');
//Object.prototype.toLocaleString = function() { return new Number; };
//xcheck_equals(Object.prototype.toLocaleString(), 0);
//Object.prototype.toLocaleString = NULL;

// Check toLocaleString calls toString
backup = Object.prototype.toString;
Object.prototype.toString = function() { return "toString"+arguments.length; };
check_equals(Object.prototype.toLocaleString(1), "toString0");
Object.prototype.toString = backup;

//----------------------
// Test enumeration
//----------------------

function enumerate(obj, enum)
{
  var enumlen = 0;
  for (var i in obj) {
    enum[i] = obj[i];
    ++enumlen;
  }
  return enumlen;
}

var l0 = new Object({a:1, b:2});
var l1 = new Object({c:3, d:4});
l1.__proto__ = l0;
var l2 = new Object({e:5, f:6});
l2.__proto__ = l1;

// check properties
var enum = new Object;
var enumlen = enumerate(l2, enum);
#if OUTPUT_VERSION  > 5
	check_equals( enumlen, 6);
#endif
check_equals( enum["a"], 1);
check_equals( enum["b"], 2);
check_equals( enum["c"], 3);
check_equals( enum["d"], 4);
check_equals( enum["e"], 5);
check_equals( enum["f"], 6);

// Hide a property of a base object
var ret = ASSetPropFlags(l0, "a", 1);

var enum = new Object;
var enumlen = enumerate(l2, enum);
#if OUTPUT_VERSION  > 5
	check_equals( enumlen, 5);
#endif

check_equals( enum["a"], undefined);

var obj5 = new Object();
obj5['a'] = 1;
check_equals(obj5['a'], 1);
#if OUTPUT_VERSION < 7
check_equals(obj5['A'], 1);
#else
check_equals(obj5['A'], undefined);
#endif

//----------------------------------------------------
// Test Object.isPropertyEnumerable (SWF6 up)
//----------------------------------------------------

#if OUTPUT_VERSION > 5

// quick built-ins check
check( Object.prototype.hasOwnProperty("isPropertyEnumerable") );
check( ! Object.prototype.isPropertyEnumerable("isPropertyEnumerable") );
check( Object.prototype.hasOwnProperty("addProperty") );
check( ! Object.prototype.isPropertyEnumerable("addProperty") );
check( Object.prototype.hasOwnProperty("hasOwnProperty") );
check( ! Object.prototype.isPropertyEnumerable("hasOwnProperty") );

obj6 = new Object();
obj6.member = "a member";

ret = obj6.isPropertyEnumerable('unexistent');
check_equals(typeof(ret), 'boolean');
check( ! ret ); // non-existant

ret = obj6.isPropertyEnumerable('member');
check_equals(typeof(ret), 'boolean');
check( ret );

function enumerableThings()
{
  this.member1 = "a string";
  this.member2 = 3;
  ASSetPropFlags(this, "member2", 1); // hide member2
}
enumerableThings.prototype.member3 = new Object;

ret = enumerableThing.isPropertyEnumerable('member1');
check_equals( typeof(ret), 'undefined' );

obj7 = new enumerableThings();
check( obj7.isPropertyEnumerable('member1') );
check( ! obj7.isPropertyEnumerable('member2') );
check_equals( typeof(obj7.member3), 'object' );
check( ! obj7.isPropertyEnumerable('member3') );

#endif // OUTPUT_VERSION > 5

//----------------------------------------------------
// Test Object.isPrototypeOf (SWF6 up)
//----------------------------------------------------

#if OUTPUT_VERSION > 5

obj8 = function() {};
obj9 = function() {};
obj9.__proto__ = new obj8;
obj10 = new obj9;

check_equals( typeof(obj8.isPrototypeOf(obj8)), 'boolean' );
check_equals( typeof(obj8.isPrototypeOf(undefined)), 'boolean' );
check( obj9.prototype.isPrototypeOf(obj10) );
check( ! obj8.prototype.isPrototypeOf(obj10) );
check( obj8.prototype.isPrototypeOf(obj9) );
// TODO: add tests here !

#endif // OUTPUT_VERSION > 5

//----------------------------------------------------
// Test Object.watch (SWF6 up)
//----------------------------------------------------

#if OUTPUT_VERSION > 5

o = {};
simplewatch = function(nam, ov, nv, d) {
	_root.info = { nam:nam, ov:ov, nv:nv, d:d, tv:this };
	return _root.ret;
};
r = o.watch('l', simplewatch, 'cust');
check(r); // can watch unexisting prop
_root.ret = 2;
o.l = 5;
check_equals(o.l, 2); // returned by watcher
check_equals(_root.info.nam, 'l');
check_equals(typeof(_root.info.ov), 'undefined');
check_equals(_root.info.nv, 5);
check_equals(_root.info.d, 'cust');
check_equals(_root.info.tv, o);
delete _root.info;
check(delete o.l);
o.p = 4;
check(!o.unwatch('p')); // can not unwatch not-watched props
check(!o.unwatch('r')); // can not unwatch non-watched props
check(o.unwatch('l')); // can unwatch non-existing but watched vars

// watch a getter-setter 

get_l = function() { _root.get_l_calls++; return this.l; };
set_l = function(v) { _root.set_l_calls++; this.l=v; };
r = o.watch('l', simplewatch, 'cust2');
check(r);
check_equals(typeof(_root.info), 'undefined'); // just checking...
_root.ret = 'return from watch';
_root.get_l_calls=_root.set_l_calls=0;
r = o.addProperty("l", get_l, set_l);
check(r);
check_equals(_root.info.nam, 'l'); // gnash fails calling the watch trigger at all here
check_equals(typeof(_root.info.ov), 'undefined');
check_equals(typeof(_root.info.nv), 'undefined'); // underlying value of getter-setter was undefined
check_equals(_root.info.d, 'cust2');
check_equals(_root.info.tv, o); 
check_equals(_root.get_l_calls, 0);
check_equals(_root.set_l_calls, 0);

// if a property did exist already when adding a getter-setter, it's watcher
// isn't called
delete _root.info;
_root.get_l_calls=_root.set_l_calls=0;
r = o.addProperty("l", get_l, set_l);
check(r);
check_equals(typeof(_root.info), 'undefined');
check_equals(_root.get_l_calls, 0);
check_equals(_root.set_l_calls, 0);
r = o.l;
check_equals(r, 'return from watch');

// Getter/setter is not invoked, but watcher was used to set it's 
// underlying value, check this:
v = o.l;
check_equals(v, 'return from watch'); 

delete _root.info;
_root.get_l_calls=_root.set_l_calls=0;

o.l = 'ciao'; // watched, and invokes setter
#if OUTPUT_VERSION < 7
  check_equals(_root.info.ov, 'return from watch'); // old value
  xcheck_equals(_root.info.nv, 'ciao'); // we requested this
  check_equals(_root.info.d, 'cust2'); 
  check_equals(_root.info.tv, o); 
  check_equals(_root.get_l_calls, 0); // should get underlying value, not invoke getter
  check_equals(_root.set_l_calls, 1);
#else
  check_equals(_root.info.ov, 'return from watch'); // old value
  check_equals(_root.info.nv, 'return from watch'); // mmm ?
  check_equals(_root.info.d, 'cust2'); 
  check_equals(_root.info.tv, o); 
  check_equals(_root.get_l_calls, 0); // should get underlying value, not invoke getter
  xcheck_equals(_root.set_l_calls, 65);
#endif

r = o.unwatch("l");
check(!r); // can't unwatch while the property is a getter-setter
check(delete o.l);
r = o.unwatch("l");
check(r); // now we can unwatch.. (gnash fails as it removed the watch before)

// TODO: watch a getter-setter in the inheritance chain

//o2 = {}; o2.__proto__ = o;
//o2.l = 

#endif // OUTPUT_VERSION > 5

//----------------------------------------------------
// Test Object.unwatch (SWF6 up)
//----------------------------------------------------

#if OUTPUT_VERSION > 5

// TODO: add tests here !

#endif // OUTPUT_VERSION > 5


nothing = new Object ();
nothing.toString = function() { return "toString"; };

check_equals ("string + " + nothing, "string + toString");
nothing.__proto__ = undefined;
#if OUTPUT_VERSION < 7
check_equals ("string + " + nothing, "string + ");
#else
check_equals ("string + " + nothing, "string + undefined");
#endif

nothing2 = new Object();
nothing2.__proto__ = undefined;
#if OUTPUT_VERSION < 7
check_equals ("string + " + nothing2, "string + ");
#else
check_equals ("string + " + nothing, "string + undefined");
#endif

nothing2.valueOf = function() { return "valueOf"; };
check_equals ("string + " + nothing2, "string + valueOf");

#if OUTPUT_VERSION <= 5
totals(83);
#endif

#if OUTPUT_VERSION >= 6
totals(262);
#endif

