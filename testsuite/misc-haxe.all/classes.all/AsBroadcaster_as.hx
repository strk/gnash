// AsBroadcaster_as.hx
//
//   Copyright (C) 2007, 2009, 2010 Free Software Foundation, Inc.
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
import flash.TextField;
#end
import flash.Lib;
import flash.MovieClip;
import Type;
import Reflect;
import haxe.PosInfos;

// import our testing API
import DejaGnu;

class AsBroadcaster_as {
	static function main() {

	#if flash9
    //In actionsscript 3 the AsBroadcaster class does not exist
    //Many of the methods have been moved to flash.events.{method}
    //Look at the Actionscript 2 migration document from adobe to find the new methods

	#else
	// NOTE: Haxe does not provide support for flash version proir to flash 6
	//     However, AsBroadcaster works slightly differently in flash1-5.
	//     In those versions AsBroadcaster does not provide prototype or
	//     initialize
	
	// The following tests should be valid for flash6, 7, 8
	if (Type.typeof(untyped AsBroadcaster) == ValueType.TFunction) {
	    DejaGnu.pass("typeof(AsBroadcaster) == 'function' [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
	    DejaGnu.fail("typeof(AsBroadcaster) != 'function' [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}

	//Testing for prototype
	if (Type.typeof(untyped AsBroadcaster.prototype) == ValueType.TObject) {
		DejaGnu.pass("typeof(AsBroadcaster.prototype) == 'object'  [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("typeof(AsBroadcaster) != 'function' [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (Type.typeof(untyped AsBroadcaster.__proto__) == ValueType.TObject) {
		DejaGnu.pass("AsBroadcaster.__proto__ == Function.prototype [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("AsBroadcaster.__proto__ != Function.prototype [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	var obj = { f : function() { trace("Hi There!");} };
	var target = { x1 : "testing" };
	var event:String = "f";
		
	//Testing for initialize()
	if (untyped AsBroadcaster.hasOwnProperty('initialize')) {
		DejaGnu.pass("typeof(AsBroadcaster.initialize) == 'function' [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("typeof(AsBroadcaster.initialize) != 'function' [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (Type.typeof(untyped AsBroadcaster.initialize) == ValueType.TFunction) {
		DejaGnu.pass("AsBroadcaster.initialize is a function [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("AsBroadcaster.initialize is not a function [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (! untyped AsBroadcaster.prototype.hasOwnProperty('initialize')) {
		DejaGnu.pass("AsBroadcaster.hasOwnProperty('initialize') [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("!AsBroadcaster.hasOwnProperty('initialize') [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (Type.typeof(untyped AsBroadcaster.initialize(target)) == ValueType.TNull) {
		DejaGnu.pass("!AsBroadcaster.prototype.hasOwnProperty('initialize') [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("AsBroadcaster.prototype.hasOwnProperty('initialize') [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	//Testing for addListener()
	if (untyped AsBroadcaster.hasOwnProperty('addListener')) {
		DejaGnu.pass("AsBroadcaster.hasOwnProperty('addListener') [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("!AsBroadcaster.hasOwnProperty('addListener') [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (Type.typeof(untyped AsBroadcaster.addListener) == ValueType.TFunction) {
		DejaGnu.pass("typeof(AsBroadcaster.addListener) == 'function' [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("typeof(AsBroadcaster.addListener) != 'function' [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (Type.typeof(untyped AsBroadcaster.addListener(obj)) == ValueType.TBool) {
		DejaGnu.pass("typeof(AsBroadcaster.addListener(obj)) == 'boolean' [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("AsBroadcaster.addListener() does not return Boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	//Testing for broadcastMessage()
	if (untyped AsBroadcaster.hasOwnProperty('broadcastMessage')) {
		DejaGnu.pass("AsBroadcaster.broadcastMessage property exists [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("AsBroadcaster.broadcastMessage property does not exist [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (Type.typeof(untyped AsBroadcaster.broadcastMessage) == ValueType.TFunction) {
		DejaGnu.pass("AsBroadcaster.broadcastMessage is a function [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("AsBroadcaster.broadcastMessage is not a function [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (Type.typeof(untyped AsBroadcaster.broadcastMessage(event)) == ValueType.TNull) {
		DejaGnu.pass("AsBroadcaster.broadcastMessage is a void function [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("AsBroadcaster.broadcastMessage is not a void function [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	//Testing for removeListener
	if (untyped AsBroadcaster.hasOwnProperty('removeListener')) {
		DejaGnu.pass("AsBroadcaster.broadcastMessage property exists [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("AsBroadcaster.broadcastMessage property does not exist [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (Type.typeof(untyped AsBroadcaster.removeListener) == ValueType.TFunction) {
		DejaGnu.pass("AsBroadcaster.removeListener is a function [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("AsBroadcaster.removeListener is not a function [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (Type.typeof(untyped AsBroadcaster.removeListener(obj)) == ValueType.TBool) {
		DejaGnu.pass("AsBroadcaster.removeListener() returns Boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("AsBroadcaster.removeListener() does not return Boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]" 	);
	}
	
	var bc = untyped __new__(AsBroadcaster);
	
	if (untyped __instanceof__(bc, AsBroadcaster)) {
	    DejaGnu.pass("bc is an instance of AsBroadcaster ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bc is not an instance of AsBroadcaster ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}	
	if (untyped __instanceof__(bc, Object)) {
		DejaGnu.pass("bc is an object ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bc is not an object ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (untyped bc.addListener==null) {
		DejaGnu.pass("bc.addListener is undefined ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bc.addListener is defined ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (untyped bc.removeListener==null) {
		DejaGnu.pass("bc.removeListener is undefined ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bc.removeListener is defined ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (untyped bc.broadcastMessage==null) {
		DejaGnu.pass("bc.broadcaster is undefined ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bc.broadcaster is defined ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}	
	if (untyped bc.intialize==null) {
		DejaGnu.pass("bc.initialize is undefined ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bc.initialize is defined ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	var bcast = untyped __new__(Object);
	
	if (untyped bcast._listeners==null) {
		DejaGnu.pass("bcast._listeners is undefined ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast._listeners is defined ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (untyped bcast.addListener==null) {
		DejaGnu.pass("bcast.addListener is undefined ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener is defined ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (untyped bcast.removeListener==null) {
		DejaGnu.pass("bcast.removeListener is undefined ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.removeListener is defined ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (untyped bcast.broadcastMessage==null) {
		DejaGnu.pass("bcast.broadcastMessage is undefined ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.broadcastMessage is defined ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	untyped AsBroadcaster.initialize(bcast);
	
	var x2 = untyped __new__(Object);
	var x3 = untyped __new__(Object);
	var x4 = untyped __new__(Object);
	var x5 = untyped __new__(Object);

	if (Reflect.isObject(bcast._listeners)) {
		DejaGnu.pass("bcast._listeners returns an object./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast._listeners does not return an object./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (untyped __instanceof__(bcast._listeners, Array)) {
		DejaGnu.pass("bcast._listeners is an array ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast._listeners is not an array ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (bcast._listeners.length==0) {
		DejaGnu.pass("bcast._listeners.length = 0 ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast._listener.length != 0 ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (Type.typeof(bcast.addListener)==ValueType.TFunction) {
		DejaGnu.pass("bcast.addListener is a function ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener is not a function ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (Type.typeof(bcast.removeListener)==ValueType.TFunction) {
		DejaGnu.pass("bcast.removeListener is a function ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.removeListener is not a function ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (Type.typeof(bcast.broadcastMessage)==ValueType.TFunction) {
		DejaGnu.pass("bcast.broadcastMessage is a function ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.broadcastMessage is not a function ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	var bob = { _listeners:5, addListener:"string"};
	
	if (bob._listeners==5) {
		DejaGnu.pass("bob._listeners = 5 ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bob._listeners != 5 ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (bob.addListener=="string") {
		DejaGnu.pass("bob.addListener = 'string' ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bob.addListener != 'string' ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	untyped AsBroadcaster.initialize(bob);
	
	if (Reflect.isObject(bob._listeners)) {
		DejaGnu.pass("bob._listeners returns a object ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bob._listeners does not return a object ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (Type.typeof(bob.addListener)==ValueType.TFunction) {
		DejaGnu.pass("bob.addListener returns a function ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bob.addListener does not return a function ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	// addListener() always returns true according to documentation,
	// so take these tests with a grain of salt...
	var ret = bcast.addListener(x2);
	
	if (Type.typeof(ret)==ValueType.TBool) {
		DejaGnu.pass("bcast.addListener() returns a boolean ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener() does not return a boolean ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (ret==true) {
		DejaGnu.pass("bcast.addListener() returns true ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener() returns false ./AsBroadcaster_as.hx: " + here.lineNumber + "]");	
	}
	if (bcast._listeners.length==1) {
		DejaGnu.pass("bcast._listeners.length = 1 ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast._listeners.length != 1 ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	ret = bcast.addListener(x2);
	
	if (bcast._listeners.length==1) {
		DejaGnu.pass("bcast._listeners.length = 1 ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast._listeners.length != 1 ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
		
	ret = bcast.addListener(x3);
	
	if (bcast._listeners.length==2) {
		DejaGnu.pass("bcast._listeners.length = 2 ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast._listeners.length = " + bcast._listeners.length + "./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	ret = bcast.addListener(x4);
	
	if (bcast._listeners.length==3) {
		DejaGnu.pass("bcast._listeners.length = 3 [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast._listeners.length = " + bcast._listeners.length + "[./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	ret = bcast.removeListener(x4);
	
	if (Type.typeof(ret)==ValueType.TBool) {
		DejaGnu.pass("bcast.addListener() returns a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener() does not return a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (ret==true) {
		DejaGnu.pass("bcast.addListener() returns true [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener() returns false [./AsBroadcaster_as.hx: " + here.lineNumber + "]");	
	}
	if (bcast._listeners.length==2) {
		DejaGnu.pass("bcast._listeners.length = 2 [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast._listeners.length = " + bcast._listeners.length + "[./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	ret = bcast.removeListener(x3);
	
	if (Type.typeof(ret)==ValueType.TBool) {
		DejaGnu.pass("bcast.addListener() returns a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener() does not return a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (ret==true) {
		DejaGnu.pass("bcast.addListener() returns true [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener() returns false [./AsBroadcaster_as.hx: " + here.lineNumber + "]");	
	}
	if (bcast._listeners.length==1) {
		DejaGnu.pass("bcast._listeners.length = 1 [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast._listeners.length = " + bcast._listeners.length + "[./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	ret = bcast.removeListener(x2);
	
	if (Type.typeof(ret)==ValueType.TBool) {
		DejaGnu.pass("bcast.addListener() returns a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener() does not return a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (ret==true) {
		DejaGnu.pass("bcast.addListener() returns true [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener() returns false [./AsBroadcaster_as.hx: " + here.lineNumber + "]");	
	}
	if (bcast._listeners.length==0) {
		DejaGnu.pass("bcast._listeners.length = 0 --> bcast.removeListener() worked [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast._listeners.length = " + bcast._listeners.length + "[./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	// No such element -- previously deleted
	var ret = bcast.removeListener(x4);
	
	if (Type.typeof(ret)==ValueType.TBool) {
		DejaGnu.pass("bcast.addListener() returns a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener() does not return a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (ret==false) {
		DejaGnu.pass("bcast.addListener() returns false [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener() returns true [./AsBroadcaster_as.hx: " + here.lineNumber + "]");	
	}
	
	var o : Dynamic = {}; 
	
	o.valueOf = function() { 
		return "yes I am"; 
	}
		
	bcast.addListener(o);
	
	if (bcast._listeners.length==1) {
		DejaGnu.pass("bcast._listeners.length = 1 [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast._listeners.length = " + bcast._listeners.length + "[./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	ret = bcast.removeListener("yes I am");
		
	if (Type.typeof(ret)==ValueType.TBool) {
		DejaGnu.pass("bcast.addListener() returns a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener() does not return a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (ret==true) {
		DejaGnu.pass("bcast.addListener() returns true [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener() returns false [./AsBroadcaster_as.hx: " + here.lineNumber + "]");	
	}
	if (bcast._listeners.length==0) {
		DejaGnu.pass("bcast._listeners.length = 0 [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast._listeners.length = " + bcast._listeners.length + "[./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	o.addListener = bcast.addListener;
	
	if (untyped o._listeners==null) {
		DejaGnu.pass("o._listeners property is undefined [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("o._listeners property is defined [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (untyped o.removeListenerCalled==null) {
		DejaGnu.pass("o.removeListenerCalled property is undefined[./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else { 
		DejaGnu.fail("o.removeListenerCalled property is defined [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	ret = o.addListener();
	
	if (Type.typeof(ret)==ValueType.TBool) {
		DejaGnu.pass("bcast.addListener() returns a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener() does not return a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (ret==true) {
		DejaGnu.pass("bcast.addListener() returns true [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener() returns false [./AsBroadcaster_as.hx: " + here.lineNumber + "]");	
	}
	if (untyped o._listeners==null) {
		DejaGnu.pass("o._listeners property is undefined [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("o._listeners property is defined [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	o.removeListener = function () {
		o.removeListenerCalled = true; 
	}
	
	ret = o.addListener();
	
	// Will this test only pass if the fcn is called?
	o.removeListener();
	
	if (Type.typeof(ret)==ValueType.TBool) {
		DejaGnu.pass("o.addListener() returns a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("o.addListener() does not return a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (ret==true) {
		DejaGnu.pass("o.addListener() returns true [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("o.addListener() returns false [./AsBroadcaster_as.hx: " + here.lineNumber + "]");	
	}
	if (untyped o._listeners==null) {
		DejaGnu.pass("o._listeners property is undefined [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("o._listeners property is defined [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	if (Type.typeof(o.removeListenerCalled)==ValueType.TBool) {
		DejaGnu.pass("o.removeListenerCalled returns a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("o.removeListenerCalled does not return a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (o.removeListenerCalled==true) {
		DejaGnu.pass("o.removeListenerCalled returns true [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("o.removeListenerCalled returns false [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}

	o.removeListener = bcast.removeListener;
	
	//var o._listeners : Dynamic = {};
	o._listeners = untyped __new__(Object);
	
	o._listeners.push = function() {
		o._listeners.pushCalled = true;
		o._listeners.length++ ;
	};
	
	o._listeners.splice = function() {
		o._listeners.spliceCalled = true;
	};
	
	o._listeners.length = 1;
	o._listeners[0] = 5;
	
	ret = o.addListeners(x5);
	
	// Will these tests only pass if the fcns are actually called?
	o._listeners.push();
	o._listeners.splice();
	
	if (o._listeners.pushCalled==true) {
		DejaGnu.pass("o._listeners.pushCalled returns true [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("o._listener.pushCalled returns false [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (o._listeners.length==2) {
		DejaGnu.pass("o._listeners.length = 2 [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("o._listeners.length != 2 " + o._listeners.length +"[./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (o._listeners.spliceCalled==true) {
		DejaGnu.pass("o._listeners.spliceCalled is true [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("o._listeners.spliceCalled is false [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}	
	
	var dang:MovieClip = untyped MovieClip.createEmptyMovieClip("dangling", 1);

	if (dang.addListener==null) {
		DejaGnu.pass("dang.addListener is undefined [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("dang.addListener is defined [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	dang.removeMovieClip();
	untyped AsBroadcaster.initialize(dang);
	
	if (dang.addListener==null) {
		DejaGnu.pass("dang.addListener is undefined [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("dang.addListener is defined [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	// FAILS -- try to come back and change, but cannot figure this one out...
	/*var dang:MovieClip = untyped MovieClip.createEmptyMovieClip("dangling", 2);
	untyped AsBroadcaster.initialize(dang);

	if (Type.typeof(dang.addListener)==ValueType.TFunction) {
		DejaGnu.pass("dang.addListener function exists [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("dang.addListener function does not exist [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}*/
	
	var counter = 0;

	var onTest = function() {
        untyped this.order = ++counter;
    };
			
	var a : Dynamic = {}; 
	a.name = "a";
	a.onTest = onTest;
	var b : Dynamic = {}; 
	b.name = "b"; 
	b.onTest = onTest;
	
	Reflect.setField(bcast._listeners, "length", 0);

	ret = bcast.addListener(a);

	if (Type.typeof(ret)==ValueType.TBool) {
		DejaGnu.pass("bcast.addListener returns a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener does not return a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (ret==true) {
		DejaGnu.pass("bcast.addListener returned a true [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener returned a false [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	ret = bcast.addListener(b);
	
	if (Type.typeof(ret)==ValueType.TBool) {
		DejaGnu.pass("bcast.addListener returns a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener does not return a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (ret==true) {
		DejaGnu.pass("bcast.addListener returned a true [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener returned a false [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}

	ret = bcast.broadcastMessage("onTest");
	
	if (Type.typeof(ret)==ValueType.TBool) {
		DejaGnu.pass("bcast.broadcastMessage returns a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.broadcastMessage does not return a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (ret==true) {
		DejaGnu.pass("bcast.broadcastMessage returned a true [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.broadcastMessage returned a false [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	if (a.order==1) {
		DejaGnu.pass("a.order = 1 [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("a.order = " + a.order + "[./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (b.order==2) {
		DejaGnu.pass("b.order = 2 [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("b.order = " + b.order + "[./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	ret = bcast.addListener(b);
		
	if (Type.typeof(ret)==ValueType.TBool) {
		DejaGnu.pass("bcast.addListener returns a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener does not return a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (ret==true) {
		DejaGnu.pass("bcast.addListener returned a true [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener returned a false [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	bcast.broadcastMessage("onTest");
		
	if (a.order==3) {
		DejaGnu.pass("a.order = 3 [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("a.order = " + a.order + "[./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (b.order==4) {
		DejaGnu.pass("b.order = 4 [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("b.order = " + b.order + "[./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
		
	ret = bcast.addListener(a);
	
	if (Type.typeof(ret)==ValueType.TBool) {
		DejaGnu.pass("bcast.addListener returns a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener does not return a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (ret==true) {
		DejaGnu.pass("bcast.addListener returned a true [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener returned a false [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	bcast.broadcastMessage("onTest");

	if (a.order==6) {
		DejaGnu.pass("a.order = 6 [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("a.order = " + a.order + "[./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (b.order==5) {
		DejaGnu.pass("b.order = 5 [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("b.order = " + b.order + "[./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	
	bcast._listeners.push(a);
	
	bcast.broadcastMessage("onTest");
	
	if (a.order==9) {
		DejaGnu.pass("a.order = 9 [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("a.order = " + a.order + "[./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (b.order==7) {
		DejaGnu.pass("b.order = 7 [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("b.order = " + b.order + "[./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
		
	bcast.addListener(a);
	
	ret = bcast.broadcastMessage("onTest");
	
	if (Type.typeof(ret)==ValueType.TBool) {
		DejaGnu.pass("bcast.broadcastMessage returns a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.broadcastMessage does not return a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (ret==true) {
		DejaGnu.pass("bcast.broadcastMessage returns true [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.broadcastMessage returns false [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
		
	if (a.order==12) {
		DejaGnu.pass("a.order = 12 [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("a.order = " + a.order + "[./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (b.order==10) {
		DejaGnu.pass("b.order = 10 [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("b.order = " + b.order + "[./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	bcast._listeners.push(b);
	
	bcast.broadcastMessage("onTest");
	
	if (a.order==15) {
		DejaGnu.pass("a.order = 15 [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("a.order = " + a.order + "[./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (b.order==16) {
		DejaGnu.pass("b.order = 16 [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("b.order = " + b.order + "[./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
		
	ret = bcast.addListener(b); // *first* b is removed, another one added, new onTest is a,a,b,b
		
	if (Type.typeof(ret)==ValueType.TBool) {
		DejaGnu.pass("bcast.addListener returns a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener does not return a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (ret==true) {
		DejaGnu.pass("bcast.addListener returns true [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener returns false [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}

	bcast.broadcastMessage("onTest");
	
	if (a.order==18) {
		DejaGnu.pass("a.order = 18 [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("a.order = " + a.order + "[./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (b.order==20) {
		DejaGnu.pass("b.order = 20 [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("b.order = " + b.order + "[./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	ret = bcast.removeListener(b); // only first is removed

	if (Type.typeof(ret)==ValueType.TBool) {
		DejaGnu.pass("bcast.addListener returns a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener does not return a boolean [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (ret==true) {
		DejaGnu.pass("bcast.addListener returns true [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.addListener returns false [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	if (bcast._listeners.length==3) {
		DejaGnu.pass("bcast._listeners.length = 3 [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast._listeners.length = " + bcast._listeners.length + "[./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	bcast.broadcastMessage("onTest");
	
	if (a.order==22) {
		DejaGnu.pass("a.order = 22 [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("a.order = " + a.order + "[./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	if (b.order==23) {
		DejaGnu.pass("b.order = 23 [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("b.order = " + b.order + "[./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}

	ret = bcast.broadcastMessage("onUnexistent");
	
	if (ret==true) {
		DejaGnu.pass("bcast.broadcastMessage returns true [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.broadcastMessage returns false [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}

	Reflect.setField(bcast._listeners, "length", 0);
	
	ret = bcast.broadcastMessage("onUnexistent");
	
	if (ret==null) {
		DejaGnu.pass("bcast.broadcastMessage('onUnexistent') is undefined [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("bcast.broadcastMessage('onUnexistent') is defined (!) [./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}

	var total = untyped __new__(Int);
		
	var o : Dynamic = {};

	o.addThis = function(what) {
		//DejaGnu.note("Arg0 is: " + what);
		untyped total += what;
	};
	
	o.setSum = function() {
		// Reset untyped total as it should not be an accumulator
		untyped total = 0;
		var i:Int = 0;
		while (i < untyped __arguments__.length) {
			untyped total += untyped __arguments__[i];
			i++;
		}
	}
	
	// Reset untyped total only once for addThis, because we want it to 
	// act as an accumulator		
	untyped total = 0;
	
	bcast.addListener(o);
	Reflect.callMethod(bcast, Reflect.field(bcast, "broadcastMessage"), ["addThis", 3]);
	
	if (untyped total==3) {
		DejaGnu.pass("_root.total==3 ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("_root.total!=3" + " " + untyped total + " ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	Reflect.callMethod(bcast, Reflect.field(bcast, "broadcastMessage"), ["addThis", 2]);
	
	if (untyped total==5) {
		DejaGnu.pass("_root.total==5 ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("_root.total= " + untyped total + " ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	Reflect.callMethod(bcast, Reflect.field(bcast, "broadcastMessage"), ["setSum", 1, 2, 3, 4]);
	
	if (untyped total==10) {
		DejaGnu.pass("_root.total==10 ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("_root.total= " + untyped total + " ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	Reflect.callMethod(bcast, Reflect.field(bcast, "broadcastMessage"), ["setSum", 1, 2, 3, 4, 5, 6, 7, 8]);
	
	if (untyped total==36) {
		DejaGnu.pass("_root.total==36 ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("_root.total= " + untyped total + " ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
		
	Reflect.callMethod(bcast, Reflect.field(bcast, "broadcastMessage"), ["setSum", "one", "two", "three"]);
	
	if (untyped total=="0onetwothree") {
		DejaGnu.pass("_root.total=='0onetwothree' ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("_root.total= " + untyped total +" ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}

	var A1 : Dynamic = {};
	var B1 : Dynamic = {};
	var o : Dynamic = {msg : ""};
	var bobj : Dynamic = {};
	
	A1 = function() {};
	B1 = function() {};
	
	A1.prototype.add = function(o) { 
		o.msg += 'A'; 
	};
	
	var aobj = untyped __new__(A1);
	aobj.add(o);
		
	B1.prototype = untyped __new__(A1);
	
	B1.prototype.add = function(o) { 
		o.msg += 'B'; 
	};
		
	bobj = untyped __new__(B1);
	
	bobj.add(o);
	
	if (o.msg=="AB") {
		DejaGnu.pass("o.msg=='AB' ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("o.msg= " + o.msg + " ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
	Reflect.setField(bcast._listeners, "length", 0);

	Reflect.callMethod(bcast, Reflect.field(bcast, "addListener"), ["bobj"]);
	Reflect.callMethod(bcast, Reflect.field(bcast, "broadcastMessage"), ["add", o]);
	
	if (o.msg=="AB") {
		DejaGnu.pass("o.msg=='AB' ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	} else {
		DejaGnu.fail("o.msg= " + o.msg + " ./AsBroadcaster_as.hx: " + here.lineNumber + "]");
	}
	
#end // flash version < 6

		//call after finishing all
				DejaGnu.done();
	}//End main
}//End class ASBroadcaster

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
