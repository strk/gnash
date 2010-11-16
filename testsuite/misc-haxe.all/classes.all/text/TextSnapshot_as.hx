// TextSnapshot_as.hx:  ActionScript 3 "TextSnapshot" class, for Gnash.
//
// Generated on: 20090602 by "bnaugle". Remove this
// after any hand editing loosing changes.
//
//   Copyright (C) 2009, 2010 Free Software Foundation, Inc.
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

// This test case must be processed by CPP before compiling to include the
//  DejaGnu.hx header file for the testing framework support.

#if flash9
import flash.text.TextSnapshot;
import flash.display.MovieClip;
import flash.display.DisplayObjectContainer;
import flash.display.Sprite;
#elseif !flash6
import flash.TextSnapshot;
import flash.MovieClip;
#end
import flash.Lib;
import Type;
import Std;
import Reflect;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class TextSnapshot_as {
    static var _root=flash.Lib.current;
    static function main() {
#if !flash6
#if flash9
		var d1:DisplayObjectContainer = new Sprite();
        var x1:TextSnapshot = d1.textSnapshot;
#else
		var m1:MovieClip = flash.Lib._root;
		var x1:TextSnapshot = m1.getTextSnapshot();
#end

        // Make sure we actually get a valid class        
        if (Std.is(x1, TextSnapshot)) {
            DejaGnu.pass("TextSnapshot class exists.");
        } else {
            DejaGnu.fail("TextSnapshot class doesn't exist!");
        }

// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.

#if flash9
	if (Type.typeof(x1.charCount) == ValueType.TInt) {
	    DejaGnu.pass("TextSnapshot.charCount property exists.");
	} else {
	    DejaGnu.fail("TextSnapshot.charCount property doesn't exist!");
	}
#end

// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
#if !flash9
	if (Type.typeof(x1.getCount) == ValueType.TFunction) {
	    DejaGnu.pass("TextSnapshot::getCount() method exists.");
	} else {
	    DejaGnu.fail("TextSnapshot::getCount() method doesn't exist!");
	}
#else
	if (Type.typeof(x1.getTextRunInfo) == ValueType.TFunction) {
	    DejaGnu.pass("TextSnapshot::getTextRunInfo() method exists.");
	} else {
	    DejaGnu.fail("TextSnapshot::getTextRunInfo() method doesn't exist!");
	}
#end
	if (Type.typeof(x1.findText) == ValueType.TFunction) {
	    DejaGnu.pass("TextSnapshot::findText() method exists.");
	} else {
	    DejaGnu.fail("TextSnapshot::findText() method doesn't exist!");
	}

	if (Type.typeof(x1.getSelected) == ValueType.TFunction) {
	    DejaGnu.pass("TextSnapshot::getSelected() method exists.");
	} else {
	    DejaGnu.fail("TextSnapshot::getSelected() method doesn't exist!");
	}

	if (Type.typeof(x1.getSelectedText) == ValueType.TFunction) {
	    DejaGnu.pass("TextSnapshot::getSelectedText() method exists.");
	} else {
	    DejaGnu.fail("TextSnapshot::getSelectedText() method doesn't exist!");
	}

	if (Type.typeof(x1.getText) == ValueType.TFunction) {
	    DejaGnu.pass("TextSnapshot::getText() method exists.");
	} else {
	    DejaGnu.fail("TextSnapshot::getText() method doesn't exist!");
	}

	if (Type.typeof(x1.hitTestTextNearPos) == ValueType.TFunction) {
	    DejaGnu.pass("TextSnapshot::hitTestTextNearPos() method exists.");
	} else {
	    DejaGnu.fail("TextSnapshot::hitTestTextNearPos() method doesn't exist!");
	}

	if (Type.typeof(x1.setSelectColor) == ValueType.TFunction) {
	    DejaGnu.pass("TextSnapshot::setSelectColor() method exists.");
	} else {
	    DejaGnu.fail("TextSnapshot::setSelectColor() method doesn't exist!");
	}

	if (Type.typeof(x1.setSelected) == ValueType.TFunction) {
	    DejaGnu.pass("TextSnapshot::setSelected() method exists.");
	} else {
	    DejaGnu.fail("TextSnapshot::setSelected() method doesn't exist!");
	}

        // Call this after finishing all tests. It prints out the totals.

//Si
//The following tests are from ming.

#if !flash9
	if (untyped TextSnapshot.prototype.hasOwnProperty("findText")) {
		DejaGnu.pass("TextSnapshot.prototype.'findText' property exists.");
	} else {
		DejaGnu.fail("TextSnapshot.prototype.'findText' property does not exist!");
	}

	if (untyped TextSnapshot.prototype.hasOwnProperty("getCount")) {
		DejaGnu.pass("TextSnapshot.prototype.'getCount' property exists.");
	} else {
		DejaGnu.fail("TextSnapshot.prototype.'getCount' property does not exist!");
	}

	if (untyped TextSnapshot.prototype.hasOwnProperty("getSelected")) {
		DejaGnu.pass("TextSnapshot.prototype.'getSelected' property exists.");
	} else {
		DejaGnu.fail("TextSnapshot.prototype.'getSelected' property does not exist!");
	}

	if (untyped TextSnapshot.prototype.hasOwnProperty("getSelectedText")) {
		DejaGnu.pass("TextSnapshot.prototype.'getSelectedText' property exists.");
	} else {
		DejaGnu.fail("TextSnapshot.prototype.'getSelectedText' property does not exist!");
	}

	if (untyped TextSnapshot.prototype.hasOwnProperty("getText")) {
		DejaGnu.pass("TextSnapshot.prototype.'getText' property exists.");
	} else {
		DejaGnu.fail("TextSnapshot.prototype.'getText' property does not exist!");
	}

	if (untyped TextSnapshot.prototype.hasOwnProperty("setSelectColor")) {
		DejaGnu.pass("TextSnapshot.prototype.'setSelectColor' property exists.");
	} else {
		DejaGnu.fail("TextSnapshot.prototype.'setSelectorColor' property does not exist!");
	}

	if (untyped TextSnapshot.prototype.hasOwnProperty("hitTestTextNearPos")) {
		DejaGnu.pass("TextSnapshot.prototype.'hitTestTextNearPos' property exists.");
	} else {
		DejaGnu.fail("TextSnapshot.prototype.'hitTestTextNearPos' property does not exist!");
	}

	if (untyped TextSnapshot.prototype.hasOwnProperty("setSelected")) {
		DejaGnu.pass("TextSnapshot.prototype.'setSelected' property exists.");
	} else {
		DejaGnu.fail("TextSnapshot.prototype.'setSelected' property does not exist!");
	}
#end


#if flash9
	if (Std.string(untyped __typeof__(TextSnapshot)) == 'object') {
		DejaGnu.pass("TextSnapshot is an 'object' in flash9.");
	} else {
	    DejaGnu.fail("TextSnapshot is not an 'object' in flash9.");
	}
#else
//	DejaGnu.note("TextSnapshot type:"+Std.string(untyped __typeof__(TextSnapshot)) );
	if (Std.string(untyped __typeof__(TextSnapshot)) == 'function') {
		DejaGnu.pass("TextSnapshot is a 'function'.");
	} else {
	    DejaGnu.fail("TextSnapshot is not a 'function'.");
	}
#end

	var textsnapshotobj:TextSnapshot = untyped __new__(TextSnapshot);

	#if  flash9 
	if (Std.is(textsnapshotobj, TextSnapshot) ){ 
		DejaGnu.pass("textsnapshotobj is an instance of 'TextSnapshot'.");
	} else {
	    DejaGnu.fail("textsnapshotobj is not an instance of 'TextSnapshot'.");
	} 
	#else 
	if (untyped __instanceof__(textsnapshotobj,TextSnapshot) ){ 
		DejaGnu.pass("textsnapshotobj is an instance of 'TextSnapshot'.");
	} else {
	    DejaGnu.fail("textsnapshotobj is not an instance of 'TextSnapshot'.");
	} 
	#end


#if !flash9
//	DejaGnu.note("TextSnapshot type:"+Std.string(untyped __typeof__(TextSnapshot)) );
	if (untyped !TextSnapshot.hasOwnProperty("findText")) {
		DejaGnu.pass("textsnapshotobj.'findText' property does not exists");
	} else {
		DejaGnu.fail("textsnapshotobj.'findText' property exists. Wrong!");
	}

	if (untyped !TextSnapshot.hasOwnProperty("getCount")) {
		DejaGnu.pass("textsnapshotobj.'getCount' property does not exists.");
	} else {
		DejaGnu.fail("textsnapshotobj.'getCount' property exists. Wrong!");
	}

	if (untyped !TextSnapshot.hasOwnProperty("getSelected")) {
		DejaGnu.pass("textsnapshotobj.'getSelected' property does not exists.");
	} else {
		DejaGnu.fail("textsnapshotobj.'getSelected' property exists. Wrong!");
	}

	if (untyped !TextSnapshot.hasOwnProperty("getSelectedText")) {
		DejaGnu.pass("textsnapshotobj.'getSelectedText property does not exists.");
	} else {
		DejaGnu.fail("textsnapshotobj.'getSelectedText property exists. Wrong!");
	}

	if (untyped !TextSnapshot.hasOwnProperty("getText")) {
		DejaGnu.pass("textsnapshotobj.'getText property does not exists.");
	} else {
		DejaGnu.fail("textsnapshotobj.'getText' property exists. Wrong!");
	}

	if (untyped !TextSnapshot.hasOwnProperty("setSelectColor")) {
			DejaGnu.pass("textsnapshotobj.'setSelectColor' property does not exists.");
	} else {
		DejaGnu.fail("textsnapshotobj.'setSelectColor' property exists. Wrong!");
	}

	if (untyped !TextSnapshot.hasOwnProperty("hitTestTextNearPos")) {
		DejaGnu.pass("textsnapshotobj.'hitTestTextNearPos' property does not exists.");
	} else {
		DejaGnu.fail("textsnapshotobj.'hitTestTextNearPos' property exists. Wrong!");
	}

	if (untyped !TextSnapshot.hasOwnProperty("setSelected")) {
		DejaGnu.pass("textsnapshotobj.'setSelected' property does not exists.");
	} else {
		DejaGnu.fail("textsnapshotobj.'setSelected' property exists. Wrong!");
	}

#end

//	DejaGnu.note("TextSnapshot type:"+Std.string(untyped __typeof__(textsnapshotobj)) );

	if (Std.string(untyped __typeof__(textsnapshotobj)) == 'object') {
		DejaGnu.pass("textsnapshotobj is an 'object'.");
	} else {
	    DejaGnu.fail("textsnapshotobj is not an 'object'.");
	}

#if !flash9
	if (Std.string(untyped __typeof__(textsnapshotobj.findText) )== 'function'){
		DejaGnu.pass("The type of textsnapshotobj.findText is a 'fucntion'.");
	} else {
	    DejaGnu.fail("The type of textsnapshotobj.findText is not a 'fucntion'!");
	}

	if (Std.string(untyped __typeof__(textsnapshotobj.getCount) )== 'function'){
		DejaGnu.pass("The type of textsnapshotobj.getCount is a 'fucntion'.");	} else {	    DejaGnu.fail("The type of textsnapshotobj.getCount is not a 'fucntion'!");	}
	
	if (Std.string(untyped __typeof__(textsnapshotobj.getSelected) )== 'function'){
		DejaGnu.pass("The type of textsnapshotobj.getSelected is a 'fucntion'.");
	} else {
	    DejaGnu.fail("The type of textsnapshotobj.getSelected is not a 'fucntion'!");
	}

	if (Std.string(untyped __typeof__(textsnapshotobj.getSelectedText) )== 'function'){
		DejaGnu.pass("The type of textsnapshotobj.getSelectedText is a 'fucntion'.");
	} else {
	    DejaGnu.fail("The type of textsnapshotobj.getSelectedText is not a 'fucntion'!");
	}

	if (Std.string(untyped __typeof__(textsnapshotobj.getText) )== 'function'){
		DejaGnu.pass("The type of textsnapshotobj.getText is a 'fucntion'.");
	} else {
	    DejaGnu.fail("The type of textsnapshotobj.getText is not a 'fucntion'!");
	}

	if (Std.string(untyped __typeof__(textsnapshotobj.hitTestTextNearPos) )== 'function'){
		DejaGnu.pass("The type of textsnapshotobj.hisTestTextNearPos is a 'fucntion'.");
	} else {
	    DejaGnu.fail("The type of textsnapshotobj.hisTestTextNearPos is not a 'fucntion'!");
	}

	if (Std.string(untyped __typeof__(textsnapshotobj.setSelectColor) )== 'function'){
		DejaGnu.pass("The type of textsnapshotobj.setSelectColor is a 'fucntion'.");
	} else {
	    DejaGnu.fail("The type of textsnapshotobj.setSelectColor is not a 'fucntion'!");
	}

	if (Std.string(untyped __typeof__(textsnapshotobj.setSelected) )== 'function'){
		DejaGnu.pass("The type of textsnapshotobj.setSelected is a 'fucntion'.");
	} else {
	    DejaGnu.fail("The type of textsnapshotobj.setSelected is not a 'fucntion'!");
	}
#end

#if !flash9
	var gh:TextSnapshot = untyped __new__(TextSnapshot,["hello"]);

//	DejaGnu.note("gh.toString():"+ untyped gh.toString()    );
//	DejaGnu.note("gh.getCount.toString():"+ untyped gh.getCount.toString()    );
//	DejaGnu.note("gh.getCount():"+ untyped gh.getCount()    );
	
	if (untyped gh.toString()=="[object Object]") {
		DejaGnu.pass("gh.toString() is ''[object Object]''.");
	} else {
		DejaGnu.fail("gh.toString() is not ''[object Object]''!");
	}

	if (untyped gh.getCount.toString()=="[type Function]") {
		DejaGnu.pass("gh.getCount.toString() is ''[type Function]''.");
	} else {
		DejaGnu.fail("gh.getCounttoString() is not ''[type Function]''!");
	}

	if (untyped gh.getCount()==undefined) {
		DejaGnu.pass("gh.getCount() is ''undefined''.");
	} else {
		DejaGnu.fail("gh.getCount() is not ''undefined''!");
	}

	var o = {};
	var gh:TextSnapshot = untyped __new__(TextSnapshot, [o]);
	
	DejaGnu.note("gh.toString():"+ untyped gh.toString()    );
	DejaGnu.note("gh.getCount.toString():"+ untyped gh.getCount.toString()    );
	DejaGnu.note("gh.getCount():"+ untyped gh.getCount()    );

	if (untyped gh.toString()=="[object Object]") {
		DejaGnu.pass("gh.toString() is ''[object Object]''.");
	} else {
		DejaGnu.fail("gh.toString() is not ''[object Object]''!");
	}

	if (untyped gh.getCount.toString()=="[type Function]") {
		DejaGnu.pass("gh.getCount.toString() is ''[type Function]''.");
	} else {
		DejaGnu.fail("gh.getCounttoString() is not ''[type Function]''!");
	}

	if (untyped gh.getCount()==undefined) {
		DejaGnu.pass("gh.getCount() is ''undefined''.");
	} else {
		DejaGnu.fail("gh.getCount() is not ''undefined''!");
	}

	var gh:TextSnapshot = untyped __new__(TextSnapshot,[flash.Lib.current]);

	if (untyped gh.toString()=="[object Object]") {
		DejaGnu.pass("gh.toString() is ''[object Object]''.");
	} else {
		DejaGnu.fail("gh.toString() is not ''[object Object]''!");
	}

	if (untyped gh.getCount.toString()=="[type Function]") {
		DejaGnu.pass("gh.getCount.toString() is ''[type Function]''.");
	} else {
		DejaGnu.fail("gh.getCounttoString() is not ''[type Function]''!");
	}

	DejaGnu.note("gh.getCount():"+ untyped gh.getCount()    );
	if (untyped gh.getCount()==0) {
		DejaGnu.pass("gh.getCount() is 0.");
	} else {
		DejaGnu.xfail("gh.getCount() is not 0!");
		DejaGnu.note("Warning! It is ''undefined'' here, not 0. Do not know why!");
	}

	var gh:TextSnapshot = untyped __new__(TextSnapshot,[untyped flash.Lib.current,true]);

	if (untyped gh.toString()=="[object Object]") {
		DejaGnu.pass("gh.toString() is ''[object Object]''.");
	} else {
		DejaGnu.fail("gh.toString() is not ''[object Object]''!");
	}

	if (untyped gh.getCount.toString()=="[type Function]") {
		DejaGnu.pass("gh.getCount.toString() is ''[type Function]''.");
	} else {
		DejaGnu.fail("gh.getCounttoString() is not ''[type Function]''!");
	}

	if (untyped gh.getCount()==undefined) {
		DejaGnu.pass("gh.getCount() is ''undefined''.");
	} else {
		DejaGnu.fail("gh.getCount() is not ''undefined''!");
	}
	
	
#end

/*
gh = new TextSnapshot("hello");
 check_equals(gh.toString(), "[object Object]");
 check_equals(gh.getCount.toString(), "[type Function]");
 check_equals(gh.getCount(), undefined);

 o = {};
 gh = new TextSnapshot(o);
 check_equals(gh.toString(), "[object Object]");
 check_equals(gh.getCount.toString(), "[type Function]");
 check_equals(gh.getCount(), undefined);

 gh = new TextSnapshot(this);
 check_equals(gh.toString(), "[object Object]");
 check_equals(gh.getCount.toString(), "[type Function]");
 check_equals(gh.getCount(), 0);

 gh = new TextSnapshot(this, true);
 check_equals(gh.toString(), "[object Object]");
 check_equals(gh.getCount.toString(), "[type Function]");
 check_equals(gh.getCount(), undefined);
*/

#if !flash9	
	 var ts:TextSnapshot = _root.getTextSnapshot();	
#else

	//In flash9
	//getTextSnapshot has been changed to flash.text.TextSnapshot.getSelectedText().
	//var ts:TextSnapshot = _root.getTextSnapshot();	
#end

#if !flash9
	if (untyped __instanceof__(textsnapshotobj,TextSnapshot) ){ 
		DejaGnu.pass("textsnapshotobj is an instance of 'TextSnapshot'.");
	} else {
	    DejaGnu.fail("textsnapshotobj is not an instance of 'TextSnapshot'.");
	} 

	if (untyped !ts.hasOwnProperty("findText")) {
		DejaGnu.pass("ts.'findText' property does not exists.");
	} else {
		DejaGnu.fail("ts.'findText' property exists. Wrong!");
	}

	if (untyped !ts.hasOwnProperty("getCount")) {
		DejaGnu.pass("ts.'getCount' property does not exists.");
	} else {
		DejaGnu.fail("ts.'getCount' property exists. Wrong!");
	}

	if (untyped !ts.hasOwnProperty("getSelected")) {
		DejaGnu.pass("ts.'getSelected' property does not exists.");
	} else {
		DejaGnu.fail("ts.'getSelected' property exists. Wrong!");
	}

	if (untyped !ts.hasOwnProperty("getSelectedText")) {
		DejaGnu.pass("ts.'getSelectedText property does not exists.");
	} else {
		DejaGnu.fail("ts.'getSelectedText property exists. Wrong!");
	}

	if (untyped !ts.hasOwnProperty("getText")) {
		DejaGnu.pass("ts.'getText property does not exists.");
	} else {
		DejaGnu.fail("ts.'getText' property exists. Wrong!");
	}

	if (untyped !ts.hasOwnProperty("setSelectColor")) {
			DejaGnu.pass("ts.'setSelectColor' property does not exists.");
	} else {
		DejaGnu.fail("ts.'setSelectColor' property exists. Wrong!");
	}

	if (untyped !Tts.hasOwnProperty("hitTestTextNearPos")) {
		DejaGnu.pass("ts.'hitTestTextNearPos' property does not exists.");
	} else {
		DejaGnu.fail("ts.'hitTestTextNearPos' property exists. Wrong!");
	}

	if (untyped !ts.hasOwnProperty("setSelected")) {
		DejaGnu.pass("ts.'setSelected' property does not exists.");
	} else {
		DejaGnu.fail("ts.'setSelected' property exists. Wrong!");
	}

//	DejaGnu.note("ts 1"+untyped __typeof__(ts.getCount() ) );
//	DejaGnu.note("ts 1"+untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getCount"), [0])   ) );
//	DejaGnu.note("ts 1"+Std.string(untyped __typeof__(ts.getCount('a')) ));
//	DejaGnu.note("ts 1"+Std.string(untyped __typeof__(ts.getCount(true)) ));
//	DejaGnu.note("ts 1"+Std.string(untyped __typeof__(ts.getCount(0,1)) ));
//	DejaGnu.note("ts 1"+typeof(ts.getCount()) );

	if (Std.string( untyped __typeof__(ts.getCount() ) )=="number"){
		DejaGnu.pass("ts.'getCount()' returns a number.");
	} else {
		DejaGnu.fail("ts.'getCount()' does not returns a number!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getCount"), [0])  ) )=="undefined"){
		DejaGnu.pass("ts.'getCount(0)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getCount(0)' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getCount"), ["a"])  ) )=="undefined"){
		DejaGnu.pass("ts.'getCount(''a'')' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getCount(''a'')' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getCount"), [true])  ) )=="undefined"){
		DejaGnu.pass("ts.'getCount(true)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getCount(true)' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getCount"), [0,1])  ) )=="undefined"){
		DejaGnu.pass("ts.'getCount(0,1)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getCount(0,1)' does not return 'undefined'!");
	}

	if ( ts.getCount() ==0){
		DejaGnu.pass("ts.'getCount()' returns 0.");
	} else {
		DejaGnu.fail("ts.'getCount()' does not returns 0!");
	}


	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"findText"), [])  ) )=="undefined"){
		DejaGnu.pass("ts.'findText()' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'findText()' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"findText"), ["a"])  ) )=="undefined"){
		DejaGnu.pass("ts.'findText(''a'')' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'findText(''a'')' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"findText"), [1])  ) )=="undefined"){
		DejaGnu.pass("ts.'findText(1)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'findText(1)' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"findText"), [1,"a"])  ) )=="undefined"){
		DejaGnu.pass("ts.'findText(1,''a'')' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'findText(1,''a'')' does not return 'undefined'!");
	}

	 // Test with no text.
	
	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"findText"), [1,"a",true])  ) )=="number"){
		DejaGnu.pass("ts.'findText(1,''a'',true)' returns a 'number'.");
	} else {
		DejaGnu.fail("ts.'findText(1,''a'',true)' does not return a 'number'!");
	}

		if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"findText"), [1,"a",1])  ) )=="number"){
		DejaGnu.pass("ts.'findText(1,''a'',1)' returns a 'number'.");
	} else {
		DejaGnu.fail("ts.'findText(1,''a'',1)' does not return a 'number'!");
	}

// Test with no test
// Si built dateObj as new Date();

	var dateObj:Date=untyped __new__(Date,[]);
	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"findText"), [1,"a", dateObj] )  ) )=="number"){
		DejaGnu.pass("ts.'findText(1,''a'',new Date())' returns a 'number'.");
	} else {
		DejaGnu.fail("ts.'findText(1,''a'',new Date())' does not return a 'number'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"findText"), [1,"a", dateObj] )  ) )=="number"){
		DejaGnu.pass("ts.'findText(1,''a'',new Date())' returns a 'number'.");
	} else {
		DejaGnu.fail("ts.'findText(1,''a'',new Date())' does not return a 'number'!");
	}
	
	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"findText"), ["6","a", dateObj] )  ) )=="number"){
		DejaGnu.pass("ts.'findText(''6'',''a'',new Date())' returns a 'number'.");
	} else {
		DejaGnu.fail("ts.'findText(''6'',''a'',new Date())' does not return a 'number'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"findText"), ["b","a", dateObj] )  ) )=="number"){
		DejaGnu.pass("ts.'findText(''b'',''a'',new Date())' returns a 'number'.");
	} else {
		DejaGnu.fail("ts.'findText(''b'',''a'',new Date())' does not return a 'number'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"findText"), [-1,"a", dateObj] )  ) )=="number"){
		DejaGnu.pass("ts.'findText(-1,''a'',new Date())' returns a 'number'.");
	} else {
		DejaGnu.fail("ts.'findText(-1,''a'',new Date())' does not return a 'number'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"findText"), [Infinity, "a", dateObj] )  ) )=="number"){
		DejaGnu.pass("ts.'findText(Infinity,''a'',new Date())' returns a 'number'.");
	} else {
		DejaGnu.fail("ts.'findText(Infinity,''a'',new Date())' does not return a 'number'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"findText"), [-1,"a", dateObj,"e"] )  ) )=="undefined"){
		DejaGnu.pass("ts.'findText(-1,''a'',new Date(),''e'')' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'findText(-1,''a'',new Date(),''e'')' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"findText"), [Infinity, "a", dateObj,3] )  ) )=="undefined"){
		DejaGnu.pass("ts.'findText(Infinity,''a'',new Date(),3)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'findText(Infinity,''a'',new Date(),3)' does not return 'undefined'!");
	}




	if ( Reflect.callMethod(ts, Reflect.field(ts,"findText"), [1,"a", true] )   ==-1){
		DejaGnu.pass("ts.findText(1, ''a'', true), returns -1.");
	} else {
		DejaGnu.fail("ts.findText(1, ''a'', true) does not return -1.");
	}

	if ( Reflect.callMethod(ts, Reflect.field(ts,"findText"), [1,"a", 1] )   ==-1){
		DejaGnu.pass("ts.findText(1, ''a'', 1), returns -1.");
	} else {
		DejaGnu.fail("ts.findText(1, ''a'', 1) does not return -1.");
	}

	if ( Reflect.callMethod(ts, Reflect.field(ts,"findText"), [1,"a", dateObj] )   ==-1){
		DejaGnu.pass("ts.findText(1, ''a'', new Date() ) returns -1.");
	} else {
		DejaGnu.fail("ts.findText(1, ''a'', new Date() ) does not return -1.");
	}

	if ( Reflect.callMethod(ts, Reflect.field(ts,"findText"), ["6","a", false] )   ==-1){
		DejaGnu.pass("ts.findText(''6'', ''a'', false), returns -1.");
	} else {
		DejaGnu.fail("ts.findText(''6'', ''a'', false) does not return -1.");
	}

	if ( Reflect.callMethod(ts, Reflect.field(ts,"findText"), ["6","a", true] )   ==-1){
		DejaGnu.pass("ts.findText(''6'', ''a'', true), returns -1.");
	} else {
		DejaGnu.fail("ts.findText(''6'', ''a'', true) does not return -1.");
	}

	if ( Reflect.callMethod(ts, Reflect.field(ts,"findText"), [-1,"a", dateObj] )   ==-1){
		DejaGnu.pass("ts.findText(-1, ''a'', new Date() ) returns -1.");
	} else {
		DejaGnu.fail("ts.findText(-1, ''a'', new Date() ) does not return -1.");
	}

	if ( Reflect.callMethod(ts, Reflect.field(ts,"findText"), [untyped Infinity,"a", dateObj] )  ==-1){
		DejaGnu.pass("ts.findText(Infinity, ''a'', new Date() ) returns -1.");
	} else {
		DejaGnu.fail("ts.findText(Infinity, ''a'', new Date() ) does not return -1.");
	}

	 // Shouldn't work with dynamic text.
 	_root.createTextField("tf", 10, 30, 30, 100, 100);
	 _root.tf.text = "ghjkab";
	 ts = _root.getTextSnapshot();

	if ( ts.getCount()  ==0){
		DejaGnu.pass("ts.getCount()  returns 0.");
	} else {
		DejaGnu.fail("ts.getCount() does not return 0.");
	}

	if ( ts.findText(1,"a",true)  ==-1){
		DejaGnu.pass("ts.findText(1,''a'',true)  returns -1.");
	} else {
		DejaGnu.fail("ts.findText(1,''a'',true) does not return -1.");
	}

	if ( ts.findText(1,"a",false)  ==-1){
		DejaGnu.pass("ts.findText(1,''a'',false)   returns 0.");
	} else {
		DejaGnu.fail("ts.findText(1,''a'',false)  does not return 0.");
	}


	//getSelected

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelected"), [] )  ) )=="undefined"){
		DejaGnu.pass("ts.'getSelected()' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getSelected()' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelected"), [0] )  ) )=="undefined"){
		DejaGnu.pass("ts.'getSelected(0)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getSelected(0)' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelected"), ["a"] )  ) )=="undefined"){
		DejaGnu.pass("ts.'getSelected(''a'')' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getSelected(''a'')' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelected"), [dateObj] )  ) )=="undefined"){
		DejaGnu.pass("ts.'getSelected(new Date())' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getSelected(new Date())' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelected"), [[0,1]] )  ) )=="undefined"){
		DejaGnu.pass("ts.'getSelected([0,1])' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getSelected([0,1])' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelected"), [[0,1],2] )  ) )=="boolean"){
		DejaGnu.pass("ts.'getSelected([0,1],2)' returns a 'boolean'.");
	} else {
		DejaGnu.fail("ts.'getSelected([0,1],2)' does not return a 'boolean'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelected"), [0,1] )  ) )=="boolean"){
		DejaGnu.pass("ts.'getSelected(0,1)' returns a 'boolean'.");
	} else {
		DejaGnu.fail("ts.'getSelected(0,1)' does not return a 'boolean'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelected"), [1,0] )  ) )=="boolean"){
		DejaGnu.pass("ts.'getSelected(1,0)' returns a 'boolean'.");
	} else {
		DejaGnu.fail("ts.'getSelected(1,0)' does not return a 'boolean'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelected"), [-1,3] )  ) )=="boolean"){
		DejaGnu.pass("ts.'getSelected(-1,3)' returns a 'boolean'.");
	} else {
		DejaGnu.fail("ts.'getSelected(-1,3)' does not return a 'boolean'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelected"), [1,0] )  ) )=="boolean"){
		DejaGnu.pass("ts.'getSelected(1,0)' returns a 'boolean'.");
	} else {
		DejaGnu.fail("ts.'getSelected(1,0)' does not return a 'boolean'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelected"), [1,0] )  ) )=="boolean"){
		DejaGnu.pass("ts.'getSelected(1,0)' returns a 'boolean'.");
	} else {
		DejaGnu.fail("ts.'getSelected(1,0)' does not return a 'boolean'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelected"), [0,"a"] )  ) )=="boolean"){
		DejaGnu.pass("ts.'getSelected(0,''a'')' returns a 'boolean'.");
	} else {
		DejaGnu.fail("ts.'getSelected(0,''a'')' does not return a 'boolean'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelected"), ["b",0] )  ) )=="boolean"){
		DejaGnu.pass("ts.'getSelected(''b'',0)' returns a 'boolean'.");
	} else {
		DejaGnu.fail("ts.'getSelected(''b'',0)' does not return a 'boolean'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelected"), [true,false] )  ) )=="boolean"){
		DejaGnu.pass("ts.'getSelected(true,false)' returns a 'boolean'.");
	} else {
		DejaGnu.fail("ts.'getSelected(true,false)' does not return a 'boolean'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelected"), [0,10,10] )  ) )=="undefined"){
		DejaGnu.pass("ts.'getSelected(0,10,10)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getSelected(0,10,10)' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelected"), [0,10,true] )  ) )=="undefined"){
		DejaGnu.pass("ts.'getSelected(0,10,true)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getSelected(0,10,true)' does not return 'undefined'!");
	}
	
	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelected"), [0,10,"a"] )  ) )=="undefined"){
		DejaGnu.pass("ts.'getSelected(0,10,''a'')' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getSelected(0,10,''a'')' does not return 'undefined'!");
	}

//getSelectedText

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelectedText"), [] )  ) )=="string"){
		DejaGnu.pass("ts.'getSelectedText()' returns a 'string'.");
	} else { 
		DejaGnu.fail("ts.'getSelectedText()' does not return a 'string'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelectedText"), [0] )  ) )=="string"){
		DejaGnu.pass("ts.'getSelectedText(0)' returns a 'string'.");
	} else {
		DejaGnu.fail("ts.'getSelectedText(0)' does not return a  'string'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelectedText"), ["a"] )  ) )=="string"){
		DejaGnu.pass("ts.'getSelectedText(''a'')' returns a  'string'.");
	} else {
		DejaGnu.fail("ts.'getSelectedText(''a'')' does not return a  'string'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelectedText"), [dateObj] )  ) )=="string"){
		DejaGnu.pass("ts.'getSelectedText(new Date())' returns a 'string'.");
	} else {
		DejaGnu.fail("ts.'getSelectedText(new Date())' does not return a 'string'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelectedText"), [[0,2]] )  ) )=="string"){
		DejaGnu.pass("ts.'getSelectedText([0,2])' returns a 'string'.");
	} else {
		DejaGnu.fail("ts.'getSelectedText([0,2])' does not return a  'string'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelectedText"), [0,1] )  ) )=="undefined"){
		DejaGnu.pass("ts.'getSelectedText(0,1)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getSelectedText(0,1)' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelectedText"), [1,0] )  ) )=="undefined"){
		DejaGnu.pass("ts.'getSelectedText(1,0)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getSelectedText(1,0)' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelectedText"), [-1,3] )  ) )=="undefined"){
		DejaGnu.pass("ts.'getSelectedText(-1,3)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getSelectedText(-1,3)' does not return 'undefined!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelectedText"), [1,0] )  ) )=="undefined"){
		DejaGnu.pass("ts.'getSelectedText(1,0)' returns 'undefined'");
	} else {
		DejaGnu.fail("ts.'getSelectedText(1,0)' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelectedText"), [1,0] )  ) )=="undefined"){
		DejaGnu.pass("ts.'getSelectedText(1,0)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getSelectedText(1,0)' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts,Reflect.field(ts,"getSelectedText"), [0,"a"] )  ) )=="undefined"){
		DejaGnu.pass("ts.'getSelectedText(0,''a'')' returns 'undefined'. ");
	} else {
		DejaGnu.fail("ts.'getSelectedText(0,''a'')' does not return 'undefined'.!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelectedText"), ["b",0] )  ) )=="undefined"){
		DejaGnu.pass("ts.'getSelectedText(''b'',0)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getSelectedText(''b'',0)' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelectedText"), [true,false] )  ) )=="undefined"){
		DejaGnu.pass("ts.'getSelectedText(true,false)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getSelectedText(true,false)' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelectedText"), [0,10,10] )  ) )=="undefined"){
		DejaGnu.pass("ts.'getSelectedText(0,10,10)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getSelectedText(0,10,10)' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelectedText"), [0,10,true] )  ) )=="undefined"){
		DejaGnu.pass("ts.'getSelectedText(0,10,true)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getSelectedText(0,10,true)' does not return 'undefined'!");
	}
	
	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getSelectedText"), [0,10,"a"] )  ) )=="undefined"){
		DejaGnu.pass("ts.'getSelectedText(0,10,''a'')' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getSelectedText(0,10,''a'')' does not return 'undefined'!");
	}

//getText

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getText"), [] )  ) )=="undefined"){
		DejaGnu.pass("ts.'getText()' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getText()' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getText"), [0] )  ) )=="undefined"){
		DejaGnu.pass("ts.'getText(0)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getText(0)' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getText"), ["a"] )  ) )=="undefined"){
		DejaGnu.pass("ts.'getText(''a'')' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getText(''a'')' does not return 'undefined'!");
	}	
	
	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getText"), [dateObj] )  ) )=="undefined"){
		DejaGnu.pass("ts.'getText(new Date())' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getText(new Date())' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getText"), [0,1] )  ) )=="string"){
		DejaGnu.pass("ts.'getText(0,1)' returns a 'string'.");
	} else {
		DejaGnu.fail("ts.'getText(0,1)' does not return 'string'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getText"), [1,0] )  ) )=="string"){
		DejaGnu.pass("ts.'getText(1,0)' returns a 'string'.");
	} else {
		DejaGnu.fail("ts.'getText(1,0)' does not return 'string'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getText"), [-1,3] )  ) )=="string"){
		DejaGnu.pass("ts.'getText(-1,3)' returns a 'string'.");
	} else {
		DejaGnu.fail("ts.'getText(-1,3)' does not return 'string'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getText"), [1,0] )  ) )=="string"){
		DejaGnu.pass("ts.'getText(1,0)' returns a 'string'.");
	} else {
		DejaGnu.fail("ts.'getText(1,0)' does not return 'string'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getText"), [1,0] )  ) )=="string"){
		DejaGnu.pass("ts.'getText(1,0)' returns a 'string'.");
	} else {
		DejaGnu.fail("ts.'getText(1,0)' does not return 'string'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getText"), [0,"a"] )  ) )=="string"){
		DejaGnu.pass("ts.'getText(0,''a'')' returns a 'string'.");
	} else {
		DejaGnu.fail("ts.'getText(0,''a'')' does not return 'string'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getText"), ["b",0] )  ) )=="string"){
		DejaGnu.pass("ts.'getText(''b'',0)' returns a 'string'.");
	} else {
		DejaGnu.fail("ts.'getText(''b'',0)' does not return 'string'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getText"), [true,false] )  ) )=="string"){
		DejaGnu.pass("ts.'getText(true,false)' returns a 'string'.");
	} else {
		DejaGnu.fail("ts.'getText(true,false)' does not return 'string'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getText"), [0,10,10] )  ) )=="string"){
		DejaGnu.pass("ts.'getText(0,10,10)' returns a 'string'.");
	} else {
		DejaGnu.fail("ts.'getText(0,10,10)' does not return 'string'!");
	}
	
		if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getText"), [0,10,true] )  ) )=="string"){
		DejaGnu.pass("ts.'getText(0,10,true)' returns a 'string'.");
	} else {
		DejaGnu.fail("ts.'getText(0,10,true)' does not return 'string'!");
	}

if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getText"), [0,10,"a",11] )  ) )=="undefined"){
		DejaGnu.pass("ts.'getText(0,10,''a'',11)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getText(0,10,''a'',11)' does not return 'undefined'!");
	}

if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getText"), [0,10,10,"hello"] )  ) )=="undefined"){
		DejaGnu.pass("ts.'getText(0,10,10,''hello'')' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getText(0,10,10,''hello'')' does not return 'undefined'!");
	}

if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getText"), [0,10,true,[3,4]] )  ) )=="undefined"){
		DejaGnu.pass("ts.'getText(0,10,true,[3,4])' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'getText(0,10,true,[3,4])' does not return 'undefined'!");
	}


	//setSelectColor()
 // setSelectColor(). Returns void
	
if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"setSelectColor"), [] )  ) )=="undefined"){
		DejaGnu.pass("ts.'setSelectColor()' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'setSelectColor()' does not return 'undefined'!");
	}

if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"setSelectColor"), [0] )  ) )=="undefined"){
		DejaGnu.pass("ts.'setSelectColor(0)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'setSelectColor(0)' does not return 'undefined'!");
	}
	
if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"setSelectColor"), [0,4] )  ) )=="undefined"){
		DejaGnu.pass("ts.'setSelectColor(0,4)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'setSelectColor(0,4)' does not return 'undefined'!");
	}

if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"setSelectColor"), [0,5,6] )  ) )=="undefined"){
		DejaGnu.pass("ts.'setSelectColor(0,5,6)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'setSelectColor(0,5,6)' does not return 'undefined'!");
	}

if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"setSelectColor"), [0,5,true] )  ) )=="undefined"){
		DejaGnu.pass("ts.'setSelectColor(0,5,true)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'setSelectColor(0,5,true)' does not return 'undefined'!");
	}

if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"setSelectColor"), [0,5,8,5] )  ) )=="undefined"){
		DejaGnu.pass("ts.'setSelectColor(0,5,8,5)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'setSelectColor(0,5,8,5)' does not return 'undefined'!");
	}

//hitTestTextNearPos()
//getText

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"hitTestTextNearPos"), [] )  ) )=="undefined"){
		DejaGnu.pass("ts.'hitTestTextNearPos()' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'hitTestTextNearPos()' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"hitTestTextNearPos"), [0] )  ) )=="undefined"){
		DejaGnu.pass("ts.'hitTestTextNearPos(0)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'hitTestTextNearPos(0)' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getText"), ["a"] )  ) )=="undefined"){
		DejaGnu.pass("ts.'hitTestTextNearPos(''a'')' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'hitTestTextNearPos(''a'')' does not return 'undefined'!");
	}	
	
	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"hitTestTextNearPos"), [dateObj] )  ) )=="undefined"){
		DejaGnu.pass("ts.'hitTestTextNearPos(new Date())' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'hitTestTextNearPos(new Date())' does not return 'undefined'!");
	}
	
//	DejaGnu.note(" *** " +Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"hitTestTextNearPos"), [0,1] )  ) ) );

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"hitTestTextNearPos"), [0,1] )  ) )=="number"){
		DejaGnu.xpass("ts.'hitTestTextNearPos(0,1)' returns a 'number'.");
	} else {
		DejaGnu.xfail("ts.'hitTestTextNearPos(0,1)' does not return 'number'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"hitTestTextNearPos"), [1,0] )  ) )=="number"){
		DejaGnu.xpass("ts.'hitTestTextNearPos(1,0)' returns a 'number'.");
	} else {
		DejaGnu.xfail("ts.'hitTestTextNearPos(1,0)' does not return 'number'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"hitTestTextNearPos"), [-1,3] )  ) )=="number"){
		DejaGnu.xpass("ts.'hitTestTextNearPos(-1,3)' returns a 'number'.");
	} else {
		DejaGnu.xfail("ts.'hitTestTextNearPos(-1,3)' does not return 'number'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"getText"), [1,0] )  ) )=="number"){
		DejaGnu.xpass("ts.'getText(1,0)' returns a 'number'.");
	} else {
		DejaGnu.xfail("ts.'getText(1,0)' does not return 'number'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"hitTestTextNearPos"), [1,0] )  ) )=="number"){
		DejaGnu.xpass("ts.'hitTestTextNearPos(1,0)' returns a 'number'.");
	} else {
		DejaGnu.xfail("ts.'hitTestTextNearPos(1,0)' does not return 'number'.!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"hitTestTextNearPos"), [0,"a"] )  ) )=="number"){
		DejaGnu.xpass("ts.'hitTestTextNearPos(0,''a'')' returns a 'number'.");
	} else {
		DejaGnu.xfail("ts.'hitTestTextNearPos(0,''a'')' does not return 'number'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"hitTestTextNearPos"), ["b",0] )  ) )=="number"){
		DejaGnu.xpass("ts.'hitTestTextNearPos(''b'',0)' returns a 'number'.");
	} else {
		DejaGnu.xfail("ts.'hitTestTextNearPos(''b'',0)' does not return 'number'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"hitTestTextNearPos"), [true,false] )  ) )=="number"){
		DejaGnu.xpass("ts.'hitTestTextNearPos(true,false)' returns a 'number'.");
	} else {
		DejaGnu.xfail("ts.'hitTestTextNearPos(true,false)' does not return 'number'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"hitTestTextNearPos"), [0,10,10] )  ) )=="number"){
		DejaGnu.xpass("ts.'hitTestTextNearPos(0,10,10)' returns a 'number'.");
	} else {
		DejaGnu.xfail("ts.'hitTestTextNearPos(0,10,10)' does not return 'number'!");
	}
	
		if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"hitTestTextNearPos"), [0,10,true] )  ) )=="number"){
		DejaGnu.xpass("ts.'hitTestTextNearPos(0,10,true)' returns a 'number'.");
	} else {
		DejaGnu.xfail("ts.'hitTestTextNearPos(0,10,true)' does not return 'number'!");
	}

if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"hitTestTextNearPos"), [0,10,"a",11] )  ) )=="undefined"){
		DejaGnu.pass("ts.'hitTestTextNearPos(0,10,''a'',11)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'hitTestTextNearPos(0,10,''a'',11)' does not return 'undefined'!");
	}

if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"hitTestTextNearPos"), [0,10,10,"hello"] )  ) )=="undefined"){
		DejaGnu.pass("ts.'hitTestTextNearPos(0,10,10,''hello'';)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'hitTestTextNearPos(0,10,10,''hello'';)' does not return 'undefined'!");
	}

if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"hitTestTextNearPos"), [0,10,true,[3,4]] )  ) )=="undefined"){
		DejaGnu.pass("ts.'hitTestTextNearPos(0,10,true,[3,4])' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'hitTestTextNearPos(0,10,true,[3,4])' does not return 'undefined'!");
	}

//setSelected()

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"setSelected"), [] )  ) )=="undefined"){
		DejaGnu.pass("ts.'setSelected()' returns 'undefined'.");
	} else { 
		DejaGnu.fail("ts.'setSelected()' does not return a 'string'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"setSelected"), [0] )  ) )=="undefined"){
		DejaGnu.pass("ts.'setSelected(0)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'setSelected(0)' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"setSelected"), ["a"] )  ) )=="undefined"){
		DejaGnu.pass("ts.'setSelected(''a'')' returns  'undefined'.");
	} else {
		DejaGnu.fail("ts.'setSelected(''a'')' does not return  'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"setSelected"), [dateObj] )  ) )=="undefined"){
		DejaGnu.pass("ts.'setSelected(new Date())' returns a 'undefined'.");
	} else {
		DejaGnu.fail("ts.'setSelected(new Date())' does not return a 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"setSelected"), [0,1] )  ) )=="undefined"){
		DejaGnu.pass("ts.'setSelected(0,1)' returns a 'undefined'.");
	} else {
		DejaGnu.fail("ts.'setSelected(0,1)' does not return a  'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"setSelected"), [1,0] )  ) )=="undefined"){
		DejaGnu.pass("ts.'setSelected(1,0)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'setSelected(1,0)' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"setSelected"), [-1,3] )  ) )=="undefined"){
		DejaGnu.pass("ts.'setSelected(-1,3)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'setSelected(-1,3)' does not return 'undefined!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"setSelected"), [1,0] )  ) )=="undefined"){
		DejaGnu.pass("ts.'setSelected(1,0)' returns 'undefined'");
	} else {
		DejaGnu.fail("ts.'setSelected(1,0)' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"setSelected"), [1,0] )  ) )=="undefined"){
		DejaGnu.pass("ts.'setSelected(1,0)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'setSelected(1,0)' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts,Reflect.field(ts,"setSelected"), [0,"a"] )  ) )=="undefined"){
		DejaGnu.pass("ts.'setSelected(0,''a'')' returns 'undefined'. ");
	} else {
		DejaGnu.fail("ts.'setSelected(0,''a'')' does not return 'undefined'.!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"setSelected"), ["b",0] )  ) )=="undefined"){
		DejaGnu.pass("ts.'setSelected(''b'',0)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'setSelected(''b'',0)' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"setSelected"), [true,false] )  ) )=="undefined"){
		DejaGnu.pass("ts.'setSelected(true,false)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'setSelected(true,false)' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"setSelected"), [0,10,10] )  ) )=="undefined"){
		DejaGnu.pass("ts.'setSelected(0,10,10)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'setSelected(0,10,10)' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"setSelected"), [0,10,true] )  ) )=="undefined"){
		DejaGnu.pass("ts.'setSelected(0,10,true)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'setSelected(0,10,true)' does not return 'undefined'!");
	}
	
	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"setSelected"), [0,10,"a",11] )  ) )=="undefined"){
		DejaGnu.pass("ts.'setSelected(0,10,''a'',11)' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'setSelected(0,10,''a''.11)' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"setSelected"), [0,10,10,"hello"] )  ) )=="undefined"){
		DejaGnu.pass("ts.'setSelected(0,10,10,''hello'')' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'setSelected(0,10,10,''hello'')' does not return 'undefined'!");
	}

	if (Std.string( untyped __typeof__(Reflect.callMethod(ts, Reflect.field(ts,"setSelected"), [0,10,true,[3,4]] )  ) )=="undefined"){
		DejaGnu.pass("ts.'setSelected(0,10,true,[3,4])' returns 'undefined'.");
	} else {
		DejaGnu.fail("ts.'setSelected(0,10,true,[3,4])' does not return 'undefined'!");
	}


#else
	//Si
	//Not implemented in flash9.
#end

        DejaGnu.done();
#else
	DejaGnu.note("This class (TextSnapshot) is only available in flash7, flash8, and flash9");
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

