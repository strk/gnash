// AsBroadcaster_as.hx
//
//   Copyright (C) 2007, 2009 Free Software Foundation, Inc.
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
// Test cases for the ASBroadcaster class
// This class exists in versions previous to Flash 9


// NOTE:
// Currently this test case is only partially migrated and does not compile.
// It is not clear where the AsBroadcaster class is located although it should
// exist in flash 8 or prior versions. It may be that haxe is not capable of 
// testing this class and we will need to retain the ming test.

#if flash9
import flash.display.MovieClip;
#else
import flash.MovieClip;
#end
import flash.Lib;
import Type;
import Reflect;

// import our testing API
import DejaGnu;

class AsBroadcaster_as {
	static function main() {

#if flash9
    //In actionsscript 3 the AsBroadcaster class does not exist
    //Many of the methods have been moved to flash.events.{method}
    //Look at the Actionscript 2 migration document from adobe to find the new methods

#else
	//NOTE
	// Haxe does not provide support for flash version proir to flash 6
	// However, AsBroadcaster works slightly differently in flash1-5.
	// In those versions AsBroadcaster does not provide prototype or initialize
	
	// The following tests should be valid for flash6, 7, 8

	if (Type.typeof(AsBroadcaster) == ValueType.TFunction) {
	    DejaGnu.pass("AsBroadcaster class exists");
	} else {
	    DejaGnu.fail("AsBroadcaster class does not exist");
	}

	DejaGnu.note("typeof (prototype) "   + Type.typeof(AsBroadcaster.prototype));
	DejaGnu.note("typeof (initialize) "   + Type.typeof(AsBroadcaster.initialize));

	if (Type.typeof(AsBroadcaster.prototype) == ValueType.TObject) {
		DejaGnu.pass("AsBroadcaster::prototype exists");
	} else {
		DejaGnu.fail("AsBroadcaster::prototype is does not exist");
	}

	if (Type.typeof(AsBroadcaster.initialize) == ValueType.TFunction) {
		DejaGnu.pass("AsBroadcaster::initialize exists");
	} else {
		DejaGnu.fail("AsBroadcaster::initialize does not exist");
	}

/*
	if (Type.typeof(AsBroadcaster) == ValueType.TFunction) {
		DejaGnu.pass("AsBroadcaster class exists");
	} else {
		DejaGnu.fail("AsBroadcaster class does not exist");
	}
check_equals(typeof(AsBroadcaster.prototype), 'object'); 
check_equals(AsBroadcaster.__proto__, Function.prototype); 
check_equals(typeof(AsBroadcaster.initialize), 'function');
check(AsBroadcaster.hasOwnProperty('initialize'));
check(!AsBroadcaster.prototype.hasOwnProperty('initialize'));

// These functions are available as AsBroadcaster "statics"
// and a lookup should be issued by 'initalize' so that overridden
// functions are attached to the initialized object rather then
// the original one (from swfdec/test/trace/asbroadcaster-override.as)
check_equals(typeof(AsBroadcaster.addListener), 'function');
check(AsBroadcaster.hasOwnProperty('addListener'));
check_equals(typeof(AsBroadcaster.removeListener), 'function');
check(AsBroadcaster.hasOwnProperty('removeListener'));
check_equals(typeof(AsBroadcaster.broadcastMessage), 'function');
check(AsBroadcaster.hasOwnProperty('broadcastMessage'));

bc = new AsBroadcaster;
check_equals(typeof(bc), 'object');
check(bc instanceof AsBroadcaster);
check(bc instanceof Object);
check_equals(typeof(bc.addListener), 'undefined');
check_equals(typeof(bc.removeListener), 'undefined');
check_equals(typeof(bc.broadcastMessage), 'undefined');
check_equals(typeof(bc.initialize), 'undefined');

bcast = new Object;

check_equals(typeof(bcast._listeners), 'undefined');
check_equals(typeof(bcast.addListener), 'undefined');
check_equals(typeof(bcast.removeListener), 'undefined');
check_equals(typeof(bcast.broadcastMessage), 'undefined');

AsBroadcaster.initialize(bcast);

check_equals(typeof(bcast._listeners), 'object');
check(bcast._listeners instanceof Array);
check_equals(bcast._listeners.length, 0);
check_equals(typeof(bcast.addListener), 'function');
check_equals(typeof(bcast.removeListener), 'function');
check_equals(typeof(bcast.broadcastMessage), 'function');

bob = { _listeners:5, addListener:"string" };
check_equals(bob._listeners, 5);
check_equals(bob.addListener, "string");
AsBroadcaster.initialize(bob);
check_equals(typeof(bob._listeners), "object");
check_equals(typeof(bob.addListener), "function");

//--------------------------------
// Some insane calls...
//--------------------------------

ret = bcast.addListener();
check_equals(typeof(ret), 'boolean');
check_equals(ret, true);
check_equals(bcast._listeners.length, 1); // !!

ret = bcast.addListener();
check_equals(bcast._listeners.length, 1); // undefined was already there as an element...

ret = bcast.addListener(2);
check_equals(typeof(ret), 'boolean');
check_equals(ret, true);
check_equals(bcast._listeners.length, 2); // !!

ret = bcast.addListener(2);
check_equals(bcast._listeners.length, 2); // 2 was already there as an element ...
ret = bcast.addListener(3);
check_equals(bcast._listeners.length, 3); // 3 is a new element

ret = bcast.removeListener(); // will remove the undefined value !
check_equals(typeof(ret), 'boolean');
check_equals(ret, true);
check_equals(bcast._listeners.length, 2); // element 'undefined' was removed

ret = bcast.removeListener(2); // will remove the element number:2 !
check_equals(typeof(ret), 'boolean');
check_equals(ret, true);
check_equals(bcast._listeners.length, 1); // element '2' was removed

ret = bcast.removeListener(3); // will remove the element number:3 !
check_equals(typeof(ret), 'boolean');
check_equals(ret, true);
check_equals(bcast._listeners.length, 0); // element '3' was removed

ret = bcast.removeListener(); // no such element ?
check_equals(typeof(ret), 'boolean');
check_equals(ret, false);

o = new Object; o.valueOf = function() { return 'yes I am'; };
bcast.addListener(o);
check_equals(bcast._listeners.length, 1); 
ret = bcast.removeListener('yes I am'); // valueOf invoked
check_equals(typeof(ret), 'boolean');
check_equals(ret, true);
check_equals(bcast._listeners.length, 0); // element '3' was removed

o.addListener = bcast.addListener;
check_equals(typeof(o._listeners), 'undefined');
check_equals(typeof(o.removeListenerCalled), 'undefined');
ret = o.addListener(); // automatically attempts to call o.removeListener()
check_equals(typeof(ret), 'boolean');
check_equals(ret, true);
check_equals(typeof(o._listeners), 'undefined');

o.removeListener = function() { this.removeListenerCalled = true; };
ret = o.addListener(); // automatically calls o.removeListener()
check_equals(typeof(ret), 'boolean');
check_equals(ret, true);
check_equals(typeof(o._listeners), 'undefined');
check_equals(typeof(o.removeListenerCalled), 'boolean');
check_equals(o./usr/bin/haxe -swf-version 8  -cp . -cp ./accessibility -cp ./display -cp ./errors -cp ./external -cp ./geom -cp ./media -cp ./net -cp ./printing -cp ./system -cp ./ui -cp ./xml -swf ${newname} -main BitmapData_as.hxremoveListenerCalled, true);

o.removeListener = bcast.removeListener;
o._listeners = new Object();
o._listeners.push = function() { this.pushCalled = true; this.length++; };
o._listeners.splice = function() { this.spliceCalled = true; };
o._listeners.length = 1;
o._listeners['0'] = 5;
ret = o.addListener(5); // automatically calls o._listeners.splice and o._listeners.push
// Gnash fails as it gives up if _listeners isn't an array
check_equals(o._listeners.pushCalled, true);
check_equals(o._listeners.length, 2);
check_equals(o._listeners.spliceCalled, true);

dang = createEmptyMovieClip('dangling', 1);
check_equals(typeof(dang.addListener), 'undefined');
dang.removeMovieClip();
AsBroadcaster.initialize(dang); // can't initialize a dangling thing
check_equals(typeof(dang.addListener), 'undefined');
createEmptyMovieClip('dangling', 2);
AsBroadcaster.initialize(dang); // but can initialize a rebound thing
check_equals(typeof(dang.addListener), 'function');

//--------------------------------
// A bit more sane calls...
//--------------------------------

counter = 0;

onTest = function()
{
	//note(" Called "+this.name+".onTest (order "+this.order+"->"+(counter+1)+")");
	this.order = ++counter;
};

a = new Object; a.name = 'a'; a.onTest = onTest;
b = new Object; b.name = 'b'; b.onTest = onTest;

ret = bcast.addListener(a);
check_equals(typeof(ret), 'boolean');
check_equals(ret, true);
ret = bcast.addListener(b);
check_equals(typeof(ret), 'boolean');
check_equals(ret, true);
//note("Broadcasting");
ret = bcast.broadcastMessage('onTest');
check_equals(typeof(ret), 'boolean');
check_equals(ret, true);
check_equals(a.order, 1);
check_equals(b.order, 2);

ret = bcast.addListener(b); // b is not added again
check_equals(typeof(ret), 'boolean');
check_equals(ret, true);
//note("Broadcasting");
bcast.broadcastMessage('onTest');
check_equals(a.order, 3);
check_equals(b.order, 4);

ret = bcast.addListener(a); // listener a is moved from first to last position to _listeners
check_equals(typeof(ret), 'boolean');
check_equals(ret, true);
//note("Broadcasting");
bcast.broadcastMessage('onTest');
check_equals(b.order, 5);
check_equals(a.order, 6);

bcast._listeners.push(a); // force double a listener
//note("Broadcasting");
bcast.broadcastMessage('onTest');
check_equals(b.order, 7);
check_equals(a.order, 9); // a.order was set twice

bcast.addListener(a); // first a is moved from first to last position to _listeners
//note("Broadcasting");
ret = bcast.broadcastMessage('onTest');
check_equals(typeof(ret), 'boolean');
check_equals(ret, true);
check_equals(b.order, 10);
check_equals(a.order, 12); // a is still set twice

bcast._listeners.push(b); // force double b, order should now be: b,a,a,b
//note("Broadcasting");
bcast.broadcastMessage('onTest');
check_equals(b.order, 16); 
check_equals(a.order, 15); 

ret = bcast.addListener(b); // *first* b is removed, another one added, new order is a,a,b,b
check_equals(typeof(ret), 'boolean');
check_equals(ret, true);
//note("Broadcasting");
bcast.broadcastMessage('onTest');
check_equals(a.order, 18); 
check_equals(b.order, 20);

ret = bcast.removeListener(b); // only first is removed
check_equals(typeof(ret), 'boolean');
check_equals(ret, true);
check_equals(bcast._listeners.length, 3); // expect: a,a,b
bcast.broadcastMessage('onTest');
check_equals(a.order, 22); 
check_equals(b.order, 23);

ret = bcast.broadcastMessage('onUnexistent');
check_equals(ret, true);
bcast._listeners.length=0;
ret = bcast.broadcastMessage('onUnexistent');
check_equals(typeof(ret), 'undefined');

//--------------------------------
// broadcastMessage with args
//--------------------------------

_root.total = 0;
o = {};
o.addThis = function(what)
{
	//note("Arg0 is "+what);
	_root.total += what;
};
o.setSum = function()
{
	_root.total = 0;
	for (var i=0; i< arguments.length; ++i)
	{
		//note("Arg "+i+" is "+arguments[i]);
		_root.total += arguments[i];
	}
};
bcast.addListener(o);
bcast.broadcastMessage('addThis', 3);
check_equals(_root.total, 3);
bcast.broadcastMessage('addThis', 2);
check_equals(_root.total, 5);
bcast.broadcastMessage('setSum', 1, 2, 3, 4);
check_equals(_root.total, 10);
bcast.broadcastMessage('setSum', 1, 2, 3, 4, 5, 6, 7, 8);
check_equals(_root.total, 36);
bcast.broadcastMessage('setSum', 'one', 'two', 'three');
check_equals(_root.total, '0onetwothree');

//--------------------------------
// event handlers calling super
//--------------------------------

function A1() {}
A1.prototype.add = function(o) { o.msg += 'A'; };
function B1() {}
B1.prototype = new A1;
B1.prototype.add = function(o) { super.add(o); o.msg += 'B'; };

bobj = new B1;
o = { msg:'' };
bobj.add(o);
check_equals(o.msg, "AB");
o.msg = '';

bcast._listeners.length=0;
bcast.addListener(bobj);
bcast.broadcastMessage('add', o);
check_equals(o.msg, "AB");

//-----------------------------------------------------------------------------------
// TODO: test override of AsBroadcaster.{addListener,removeListener,broadcastMessage}
// swfdec contains tests for this, which should now be pretty succeeding except for
// not-directly related checks which trigger failure due to all-or-nothing nature of
// the swfdec testsuite.
// See swfdec/test/trace/asbroadcaster-override.as for more info
//-----------------------------------------------------------------------------------

check_totals(115);
*/
#end // flash version < 6

		//call after finishing all tests
		DejaGnu.done();
	}//End main
}//End class ASBroadcaster

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
