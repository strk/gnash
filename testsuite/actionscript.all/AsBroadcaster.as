// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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
// Test case for TextField ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: AsBroadcaster.as,v 1.1 2007/08/30 09:47:36 strk Exp $";

#include "check.as"

#if OUTPUT_VERSION < 6

note();
note("AsBroadcaster exists but doesn't provide any 'prototype' or 'initialize' for SWF < 6");

xcheck_equals(typeof(AsBroadcaster), 'function'); // ???
check_equals(typeof(AsBroadcaster.prototype), 'undefined'); 
check_equals(typeof(AsBroadcaster.initialize), 'undefined');

#else // OUTPUT_VERSION >= 6

xcheck_equals(typeof(AsBroadcaster), 'function'); // ???
xcheck_equals(typeof(AsBroadcaster.prototype), 'object'); 
xcheck_equals(typeof(AsBroadcaster.initialize), 'function');

bc = new AsBroadcaster;
xcheck_equals(typeof(bc), 'object');
xcheck(bc instanceof AsBroadcaster);
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

xcheck_equals(typeof(bcast._listeners), 'object');
xcheck(bcast._listeners instanceof Array);
xcheck_equals(bcast._listeners.length, 0);
xcheck_equals(typeof(bcast.addListener), 'function');
xcheck_equals(typeof(bcast.removeListener), 'function');
xcheck_equals(typeof(bcast.broadcastMessage), 'function');

//--------------------------------
// Some insane calls...
//--------------------------------

ret = bcast.addListener();
xcheck_equals(typeof(ret), 'boolean');
xcheck_equals(ret, true);
xcheck_equals(bcast._listeners.length, 1); // !!

ret = bcast.addListener();
xcheck_equals(bcast._listeners.length, 1); // undefined was already there as an element...

ret = bcast.addListener(2);
xcheck_equals(typeof(ret), 'boolean');
xcheck_equals(ret, true);
xcheck_equals(bcast._listeners.length, 2); // !!

ret = bcast.addListener(2);
xcheck_equals(bcast._listeners.length, 2); // 2 was already there as an element ...
ret = bcast.addListener(3);
xcheck_equals(bcast._listeners.length, 3); // 3 is a new element

ret = bcast.removeListener(); // will remove the undefined value !
xcheck_equals(typeof(ret), 'boolean');
xcheck_equals(ret, true);
xcheck_equals(bcast._listeners.length, 2); // element 'undefined' was removed

ret = bcast.removeListener(2); // will remove the element number:2 !
xcheck_equals(typeof(ret), 'boolean');
xcheck_equals(ret, true);
xcheck_equals(bcast._listeners.length, 1); // element '2' was removed

ret = bcast.removeListener(3); // will remove the element number:3 !
xcheck_equals(typeof(ret), 'boolean');
xcheck_equals(ret, true);
xcheck_equals(bcast._listeners.length, 0); // element '3' was removed

ret = bcast.removeListener(); // no such element ?
xcheck_equals(typeof(ret), 'boolean');
xcheck_equals(ret, false);

o = new Object; o.valueOf = function() { return 'yes I am'; };
bcast.addListener(o);
xcheck_equals(bcast._listeners.length, 1); 
ret = bcast.removeListener('yes I am'); // valueOf invoked
xcheck_equals(typeof(ret), 'boolean');
xcheck_equals(ret, true);
xcheck_equals(bcast._listeners.length, 0); // element '3' was removed

o.addListener = bcast.addListener;
check_equals(typeof(o._listeners), 'undefined');
check_equals(typeof(o.removeListenerCalled), 'undefined');
ret = o.addListener(); // automatically attempts to call o.removeListener()
xcheck_equals(typeof(ret), 'boolean');
xcheck_equals(ret, true);
check_equals(typeof(o._listeners), 'undefined');

o.removeListener = function() { this.removeListenerCalled = true; };
ret = o.addListener(); // automatically calls o.removeListener()
xcheck_equals(typeof(ret), 'boolean');
xcheck_equals(ret, true);
check_equals(typeof(o._listeners), 'undefined');
xcheck_equals(typeof(o.removeListenerCalled), 'boolean');
xcheck_equals(o.removeListenerCalled, true);

o.removeListener = bcast.removeListener;
o._listeners = new Object();
o._listeners.push = function() { this.pushCalled = true; this.length++; };
o._listeners.splice = function() { this.spliceCalled = true; };
o._listeners.length = 1;
o._listeners['0'] = 5;
ret = o.addListener(5); // automatically calls o._listeners.splice and o._listeners.push
xcheck_equals(o._listeners.pushCalled, true);
xcheck_equals(o._listeners.length, 2);
xcheck_equals(o._listeners.spliceCalled, true);


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
xcheck_equals(typeof(ret), 'boolean');
xcheck_equals(ret, true);
ret = bcast.addListener(b);
xcheck_equals(typeof(ret), 'boolean');
xcheck_equals(ret, true);
//note("Broadcasting");
ret = bcast.broadcastMessage('onTest');
xcheck_equals(typeof(ret), 'boolean');
xcheck_equals(ret, true);
xcheck_equals(a.order, 1);
xcheck_equals(b.order, 2);

ret = bcast.addListener(b); // b is not added again
xcheck_equals(typeof(ret), 'boolean');
xcheck_equals(ret, true);
//note("Broadcasting");
bcast.broadcastMessage('onTest');
xcheck_equals(a.order, 3);
xcheck_equals(b.order, 4);

ret = bcast.addListener(a); // listener a is moved from first to last position to _listeners
xcheck_equals(typeof(ret), 'boolean');
xcheck_equals(ret, true);
//note("Broadcasting");
bcast.broadcastMessage('onTest');
xcheck_equals(b.order, 5);
xcheck_equals(a.order, 6);

bcast._listeners.push(a); // force double a listener
//note("Broadcasting");
bcast.broadcastMessage('onTest');
xcheck_equals(b.order, 7);
xcheck_equals(a.order, 9); // a.order was set twice

bcast.addListener(a); // first a is moved from first to last position to _listeners
//note("Broadcasting");
ret = bcast.broadcastMessage('onTest');
xcheck_equals(typeof(ret), 'boolean');
xcheck_equals(ret, true);
xcheck_equals(b.order, 10);
xcheck_equals(a.order, 12); // a is still set twice

bcast._listeners.push(b); // force double b, order should now be: b,a,a,b
//note("Broadcasting");
bcast.broadcastMessage('onTest');
xcheck_equals(b.order, 16); 
xcheck_equals(a.order, 15); 

ret = bcast.addListener(b); // *first* b is removed, another one added, new order is a,a,b,b
xcheck_equals(typeof(ret), 'boolean');
xcheck_equals(ret, true);
//note("Broadcasting");
bcast.broadcastMessage('onTest');
xcheck_equals(a.order, 18); 
xcheck_equals(b.order, 20);

ret = bcast.removeListener(b); // only first is removed
xcheck_equals(typeof(ret), 'boolean');
xcheck_equals(ret, true);
xcheck_equals(bcast._listeners.length, 3); // expect: a,a,b
bcast.broadcastMessage('onTest');
xcheck_equals(a.order, 22); 
xcheck_equals(b.order, 23);

// TODO: test broadcastMessage with additional arguments
//       and effects of overriding Function.apply
//	 (should have no effects, being broadcastMessage
//        a native functioN)

#endif // OUTPUT_VERSION >= 6

