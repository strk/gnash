// TextFormat_as.hx:  ActionScript 3 "TextFormat" class, for Gnash.
//
// Generated on: 20090602 by "bnaugle". Remove this
// after any hand editing loosing changes.
//
//   Copyright (C) 2009 Free Software Foundation, Inc.
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
import flash.text.TextFormat;
import flash.text.TextFormatAlign;
import flash.text.TextFormatDisplay;
#else
import flash.TextFormat;
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class TextFormat_as {
    static function main() {
#if flash9
		var x1:TextFormat = untyped __new__(TextFormat, ["fname", 2, 30, true, false, true,"http","tgt","center",23,32, 12, 4]);
#else
		var x1:TextFormat = new TextFormat("font",12.0,8,false,false,false,"url","target","align",1.0,1.0,0.0,0.0);
#end

        // Make sure we actually get a valid class        
        if (Std.is(x1, TextFormat)) {
            DejaGnu.pass("TextFormat class exists");
        } else {
            DejaGnu.fail("TextFormat class doesn't exist");
        }
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
#if flash9
	if (Std.is(x1.blockIndent, Dynamic)) {
	    DejaGnu.pass("TextFormat.blockIndent property exists");
	} else {
	    DejaGnu.fail("TextFormat.blockIndent property doesn't exist");
	}

	//~ if (Std.is(x1.align, String)) {
	    //~ DejaGnu.pass("TextFormat.align property exists");
	//~ } else {
	    //~ DejaGnu.fail("TextFormat.align property doesn't exist" + x1.align);
	//~ }

	if (Std.is(x1.bold, Dynamic)) {
	    DejaGnu.pass("TextFormat.bold property exists");
	} else {
	    DejaGnu.fail("TextFormat.bold property doesn't exist");
	}

	if (Std.is(x1.bullet, Dynamic)) {
	    DejaGnu.pass("TextFormat.bullet property exists");
	} else {
	    DejaGnu.xfail("TextFormat.bullet property doesn't exist");
	}

	if (Std.is(x1.color, Dynamic)) {
	    DejaGnu.pass("TextFormat.color property exists");
	} else {
	    DejaGnu.fail("TextFormat.color property doesn't exist");
	}
//FIXME: This only exists in haXe, not in the Adobe specs
//	if (Std.is(x1.display, TextFormatDisplay)) {
//	    DejaGnu.pass("TextFormat.display property exists");
//	} else {
//	    DejaGnu.fail("TextFormat.display property doesn't exist");
//	}
	if (Std.is(x1.indent, Dynamic)) {
	    DejaGnu.pass("TextFormat.indent property exists");
	} else {
	    DejaGnu.fail("TextFormat.indent property doesn't exist");
	}

	if (Std.is(x1.italic, Dynamic)) {
	    DejaGnu.pass("TextFormat.italic property exists");
	} else {
	    DejaGnu.fail("TextFormat.italic property doesn't exist");
	}

	if (Std.is(x1.kerning, Dynamic)) {
	    DejaGnu.pass("TextFormat.kerning property exists");
	} else {
	    DejaGnu.fail("TextFormat.kerning property doesn't exist");
	}

	if (Std.is(x1.leading, Dynamic)) {
	    DejaGnu.pass("TextFormat.leading property exists");
	} else {
	    DejaGnu.fail("TextFormat.leading property doesn't exist");
	}

	if (Std.is(x1.leftMargin, Dynamic)) {
	    DejaGnu.pass("TextFormat.leftMargin property exists");
	} else {
	    DejaGnu.fail("TextFormat.leftMargin property doesn't exist");
	}

	if (Std.is(x1.letterSpacing, Dynamic)) {
	    DejaGnu.pass("TextFormat.letterSpacing property exists");
	} else {
	    DejaGnu.fail("TextFormat.letterSpacing property doesn't exist");
	}

	if (Std.is(x1.rightMargin, Dynamic)) {
	    DejaGnu.pass("TextFormat.rightMargin property exists");
	} else {
	    DejaGnu.fail("TextFormat.rightMargin property doesn't exist");
	}

	if (Std.is(x1.size, Dynamic)) {
	    DejaGnu.pass("TextFormat.size property exists");
	} else {
	    DejaGnu.fail("TextFormat.size property doesn't exist");
	}

	if (Std.is(x1.underline, Dynamic)) {
	    DejaGnu.pass("TextFormat.underline property exists");
	} else {
	    DejaGnu.fail("TextFormat.underline property doesn't exist");
	}
#else
	x1.blockIndent = 0.0;

	//FIX ME: gnash uses incorrect data type Int
	//FIXED 
	// SI
/*	DejaGnu.note("Test 1 "+untyped __typeof__(x1.blockIndent) );
	DejaGnu.note("Test 2 "+untyped Type.typeof(x1.blockIndent) );
	DejaGnu.note("Test 3 "+untyped __typeof__(TextFormat.blockIndent) );
	DejaGnu.note("Test 4 "+untyped Type.typeof(TextFormat.blockIndent) );	
*/
//	if (Type.typeof(x1.blockIndent) == ValueType.TFloat) {
	if (Std.string(untyped __typeof__(x1.blockIndent) )== 'number'){
	    DejaGnu.pass("TextFormat.blockIndent property exists");
	} else {
	    DejaGnu.fail("TextFormat.blockIndent property should be float, returns type "+Type.typeof(x1.blockIndent));
	}

	if (Std.is(x1.align, String)) {
	    DejaGnu.pass("TextFormat.align property exists");
	} else {
	    DejaGnu.fail("TextFormat.align property doesn't exist");
	}

	if (Type.typeof(x1.bold) == ValueType.TBool) {
	    DejaGnu.pass("TextFormat.bold property exists");
	} else {
	    DejaGnu.fail("TextFormat.bold property doesn't exist");
	}

	if (Type.typeof(x1.bullet) == ValueType.TBool) {
	    DejaGnu.pass("TextFormat.bullet property exists");
	} else {
	    DejaGnu.xfail("TextFormat.bullet property doesn't exist");
	}

	if (Type.typeof(x1.color) == ValueType.TInt) {
	    DejaGnu.pass("TextFormat.color property exists");
	} else {
	    DejaGnu.fail("TextFormat.color property doesn't exist");
	}
	//FIX ME: gnash uses incorrect data type Int
	//FIXED 
	// SI
//	if (Type.typeof(x1.indent) == ValueType.TFloat) {
	if (Std.string(untyped __typeof__(x1.indent  ) )== 'number'){
	    DejaGnu.pass("TextFormat.indent property exists");
	} else {
	    DejaGnu.fail("TextFormat.indent property should be float, returns type "+Type.typeof(x1.indent));
	}

	if (Type.typeof(x1.italic) == ValueType.TBool) {
	    DejaGnu.pass("TextFormat.italic property exists");
	} else {
	    DejaGnu.fail("TextFormat.italic property doesn't exist");
	}
	//FIX ME: gnash uses incorrect data type Int
	//FIXED 
	// SI
//	if (Type.typeof(x1.leading) == ValueType.TFloat) {
	if (Std.string(untyped __typeof__(x1.leading  ) )== 'number'){
	    DejaGnu.pass("TextFormat.leading property exists");
	} else {
	    DejaGnu.fail("TextFormat.leading property should be float, returns type "+Type.typeof(x1.leading));
	}
	//FIX ME: gnash uses incorrect data type Int
	//FIXED 
	// SI
//	if (Type.typeof(x1.leftMargin) == ValueType.TFloat) {
	if (Std.string(untyped __typeof__(x1.leftMargin  ) )== 'number'){
	    DejaGnu.pass("TextFormat.leftMargin property exists");
	} else {
	    DejaGnu.fail("TextFormat.leftMargin property should be float, returns type "+Type.typeof(x1.leftMargin));
	}
	//FIX ME: gnash uses incorrect data type Int
	//FIXED 
	// SI
//	if (Type.typeof(x1.rightMargin) == ValueType.TFloat) {
	if (Std.string(untyped __typeof__(x1.rightMargin  ) )== 'number'){	
	    DejaGnu.pass("TextFormat.rightMargin property exists");
	} else {
	    DejaGnu.fail("TextFormat.rightMargin property should be float, returns type "+Type.typeof(x1.rightMargin));
	}
	//FIX ME: gnash uses incorrect data type Int
	//FIXED 
	// SI
//	if (Type.typeof(x1.size) == ValueType.TFloat) {
	if (Std.string(untyped __typeof__(x1.size  ) )== 'number'){
	    DejaGnu.pass("TextFormat.size property exists");
	} else {
	    DejaGnu.fail("TextFormat.size property should be float, returns type "+Type.typeof(x1.size));
	}

	if (Type.typeof(x1.underline) == ValueType.TBool) {
	    DejaGnu.pass("TextFormat.underline property exists");
	} else {
	    DejaGnu.fail("TextFormat.underline property doesn't exist");
	}
#if flash8
	x1.kerning = false;
	if (Type.typeof(x1.kerning) == ValueType.TBool) {
	    DejaGnu.pass("TextFormat.kerning property exists");
	} else {
	    DejaGnu.fail("TextFormat.kerning property doesn't exist");
	}
	x1.letterSpacing = 0.0;
	//FIX ME: gnash uses incorrect data type Int
	//FIXED 
	// SI
//	if (Type.typeof(x1.letterSpacing) == ValueType.TFloat) {
	if (Std.string(untyped __typeof__(x1.letterSpacing  ) )== 'number'){
		    DejaGnu.pass("TextFormat.letterSpacing property exists");
	} else {
	    DejaGnu.fail("TextFormat.letterSpacing property should be float, returns type "+Type.typeof(x1.letterSpacing));
	}

	if (Std.is(x1.display, String)) {
	    DejaGnu.xpass("TextFormat.display property exists");
	} else {
	    DejaGnu.xfail("TextFormat.display property doesn't exist");
	}
#end
#end
	if (Std.is(x1.font, String)) {
	    DejaGnu.pass("TextFormat.font property exists");
	} else {
	    DejaGnu.fail("TextFormat.font property doesn't exist");
	}

	x1.tabStops = [0,1,2,3];
	if (Std.is(x1.tabStops, Array)) {
	    DejaGnu.pass("TextFormat.tabStops property exists");
	} else {
	    DejaGnu.xfail("TextFormat.tabStops property doesn't exist");
	}

	if (Std.is(x1.target, String)) {
	    DejaGnu.pass("TextFormat.target property exists");
	} else {
	    DejaGnu.xfail("TextFormat.target property doesn't exist");
		
	}
	
	DejaGnu.note("typeof(url): " + Type.typeof(x1.url));

	if (Std.is(x1.url, String)) {
	    DejaGnu.pass("TextFormat.url property exists");
	} else {
	    DejaGnu.xfail("TextFormat.url property doesn't exist");
	}
// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
#if !flash9
	if (Type.typeof(x1.getTextExtent) == ValueType.TFunction) {
	    DejaGnu.pass("TextFormat.getTextExtent method exists");
	} else {
	    DejaGnu.fail("TextFormat.getTextExtent method doesn't exist");
	}
#end
        // Call this after finishing all tests. It prints out the totals.

//Si
//The followings are ming tests:

//      Not implemeted in the haXe here.
#if flash9
#else
        untyped Object.prototype.hasOwnProperty = ASnative(101, 5);
#end
//	DejaGnu.note("type of TextFormat" + untyped __typeof__(TextFormat));
//	DejaGnu.note("type of TextFormat.phototype" + untyped __typeof__(TextFormat.prototype));

#if flash9
	if (Std.string(untyped __typeof__(TextFormat) )== 'object'){
		DejaGnu.pass("The type of TextFormat is 'object' in flash 9");
	} else {
	    DejaGnu.fail("The type of TextFormat is not 'object' in flash 9");
	}
#else
	if (Std.string(untyped __typeof__(TextFormat) )== 'function'){
		DejaGnu.pass("The type of TextFormat is 'fucntion'");
	} else {
	    DejaGnu.fail("The type of TextFormat is not 'fucntion'");
	}
#end

	if (Std.string(untyped __typeof__(TextFormat.prototype))== 'object'){
		DejaGnu.pass("The type of TextFormat.prototype is 'object'");
	} else {
	    DejaGnu.fail("The type of TextFormat.prototype is not 'object'");
	}
	
//Si
//build an instance

	var tfObj:TextFormat = new TextFormat();

//	DejaGnu.note("type of TextFormat" + Type.typeof(tfObj));
	if (Std.string(untyped __typeof__(tfObj))== 'object'){
		DejaGnu.pass("The type of tfObj is 'object'");
	} else {
	    DejaGnu.fail("The type of tfObj is not 'object'");
	}

#if flash9
	if (Std.is(tfObj, TextFormat) ){
		DejaGnu.pass("'tfObj' is an instance of TextFormat");
	} else {
	    DejaGnu.fail("'tfObj' is not an instance of TextFormat");
	}	
	
#else

//Si
//Check "is" a instance
//	DejaGnu.note(""+untyped __instanceof__(tfObj, TextFormat));
	if (untyped __instanceof__(tfObj,TextFormat) ){
		DejaGnu.pass("'tfObj' is an instance of TextFormat");
	} else {
	    DejaGnu.fail("'tfObj' is not an instance of TextFormat");
	}	
#end

#if (!flash9)

	if (untyped TextFormat.prototype.hasOwnProperty("display")) {
		DejaGnu.pass("TextFormat.prototype.'display' property exists");
	} else {
		DejaGnu.fail("TextFormat.prototype.'display' property does not exist");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("bullet")) {
		DejaGnu.pass("TextFormat.prototype.'bullet' property exists");
	} else {
		DejaGnu.fail("TextFormat.prototype.'bullet' property does not exist");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("tabStops")) {
	DejaGnu.pass("TextFormat.prototype.'tabStops' property exists");
	} else {
		DejaGnu.fail("TextFormat.prototype.'tabStops' property does not exist");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("blockIndent")) {
		DejaGnu.pass("TextFormat.prototype.'blockIndent' property exists");
	} else {
		DejaGnu.fail("TextFormat.prototype.'blockIndent' property does not exist");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("leading")) {
	DejaGnu.pass("TextFormat.prototype.'leading' property exists");
	} else {
		DejaGnu.fail("TextFormat.prototype.'leading' property does not exist");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("indent")) {
		DejaGnu.pass("TextFormat.prototype.'indent' property exists");
	} else {
		DejaGnu.fail("TextFormat.prototype.'indent' property does not exist");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("rightMargin")) {
		DejaGnu.pass("TextFormat.prototype.'rightMargin' property exists");
	} else {
		DejaGnu.fail("TextFormat.prototype.'rightMargin' property does not exist");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("leftMargin")) {
		DejaGnu.pass("TextFormat.prototype.'leftMargin' property exists");
	} else {
		DejaGnu.fail("TextFormat.prototype.'leftMargin' property does not exist");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("align")) {
		DejaGnu.pass("TextFormat.prototype.'align' property exists");
	} else {
		DejaGnu.fail("TextFormat.prototype.'align' property does not exist");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("underline")) {
		DejaGnu.pass("TextFormat.prototype.'underline' property exists");
	} else {
		DejaGnu.fail("TextFormat.prototype.'underline' property does not exist");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("italic")) {
		DejaGnu.pass("TextFormat.prototype.'italic' property exists");
	} else {
		DejaGnu.fail("TextFormat.prototype.'italic' property does not exist");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("bold")) {
		DejaGnu.pass("TextFormat.prototype.'bold' property exists");
	} else {
		DejaGnu.fail("TextFormat.prototype.'bold' property does not exist");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("target")) {
		DejaGnu.pass("TextFormat.prototype.'target' property exists");
	} else {
		DejaGnu.fail("TextFormat.prototype.'target' property does not exist");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("url")) {
		DejaGnu.pass("TextFormat.prototype.'url' property exists");
	} else {
		DejaGnu.fail("TextFormat.prototype.'url' property does not exist");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("color")) {
		DejaGnu.pass("TextFormat.prototype.'color' property exists");
	} else {
		DejaGnu.fail("TextFormat.prototype.'color' property does not exist");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("size")) {
		DejaGnu.pass("TextFormat.prototype.'size' property exists");
	} else {
		DejaGnu.fail("TextFormat.prototype.'size' property does not exist");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("font")) {
		DejaGnu.pass("TextFormat.prototype.'font' property exists");
	} else {
		DejaGnu.fail("TextFormat.prototype.'font' property does not exist");
	}

	if (untyped !TextFormat.prototype.hasOwnProperty("getTextExtent")) {
		DejaGnu.pass("TextFormat.prototype.'getTextExtent' property does not exist");
	} else {
		DejaGnu.fail("TextFormat.prototype.'getTextExtent' property exists! WRONG!");
	}

	if (untyped tfObj.hasOwnProperty("getTextExtent")) {
		DejaGnu.pass("tfObj.'getTextExtent' property exists");
	} else {
		DejaGnu.fail("tfObj.'getTextExtent' property does not exist");
	}
#else
#end

//More checks.
// When you construct a TextFormat w/out args all members
// are of the 'null' type. In general, uninitialized members
// are all of the 'null' type.

#if flash9
#else
	if (Std.string(untyped __typeof__(tfObj.display)) == 'string') {
		DejaGnu.xpass("Good, tfObj.display is a  'null'.");
	} else {
	    DejaGnu.xfail("Wrong, tfObj.display is not 'null'.");
	}
//	DejaGnu.note("tfObj.display:"+Std.string(untyped tfObj.display ));

	if (Std.string(untyped tfObj.display) == 'block') {
		DejaGnu.xpass("Wrong, tfObj.display equlas to block.");
	} else {
	    DejaGnu.xfail("tfObj.display does not equal to 'block'.");
	}

	if (Std.string(untyped __typeof__(tfObj.bullet)) == 'null') {
		DejaGnu.pass("Good, tfObj.bullet is a  'null'.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.bullet is not 'null'.");
	}

	if (Std.string(untyped __typeof__(tfObj.tabStops)) == 'null') {
		DejaGnu.xpass("Good, tfObj.tabStops is a  'null'.");
	} else {
	    DejaGnu.xfail("Wrong, tfObj.tabStops is not 'null'.");
	}

	if (Std.string(untyped __typeof__(tfObj.blockIndent)) == 'null') {
		DejaGnu.pass("Good, tfObj.blockIndent is a  'null'.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.blockIndent is not 'null'.");
	}

	if (Std.string(untyped __typeof__(tfObj.leading)) == 'null') {
		DejaGnu.pass("Good, tfObj.leading is a  'null'.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.leading is not 'null'.");
	}

	if (Std.string(untyped __typeof__(tfObj.indent)) == 'null') {
		DejaGnu.pass("Good, tfObj.indent is a  'null'.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.indent is not 'null'.");
	}

	if (Std.string(untyped __typeof__(tfObj.rightMargin)) == 'null') {
		DejaGnu.pass("Good, tfObj.rightMargin is a  'null'.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.rightMargin is not 'null'.");
	}

	if (Std.string(untyped __typeof__(tfObj.leftMargin)) == 'null') {
		DejaGnu.pass("Good, tfObj.leftMargin is a  'null'.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.leftMargin is not 'null'.");
	}

	if (Std.string(untyped __typeof__(tfObj.align)) == 'null') {
		DejaGnu.pass("Good, tfObj.align is a  'null'.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.align is not 'null'.");
	}

	if (Std.string(untyped __typeof__(tfObj.underline)) == 'null') {
		DejaGnu.pass("Good, tfObj.underline is a  'null'.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.underline is not 'null'.");
	}

	if (Std.string(untyped __typeof__(tfObj.italic)) == 'null') {
		DejaGnu.pass("Good, tfObj.italic is a  'null'.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.italic is not 'null'.");
	}

	if (Std.string(untyped __typeof__(tfObj.bold)) == 'null') {
		DejaGnu.pass("Good, tfObj.bold is a  'null'.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.bold is not 'null'.");
	}

	if (Std.string(untyped __typeof__(tfObj.target)) == 'null') {
		DejaGnu.pass("Good, tfObj.target is a  'null'.");
	} else {
	    DejaGnu.xfail("Wrong, tfObj.target is not 'null'.");
	}

	if (Std.string(untyped __typeof__(tfObj.url)) == 'null') {
		DejaGnu.pass("Good, tfObj.url is a  'null'.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.url is not 'null'.");
	}

 	if (Std.string(untyped __typeof__(tfObj.color)) == 'null') {
		DejaGnu.pass("Good, tfObj.color is a  'null'.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.color is not 'null'.");
	}

	if (Std.string(untyped __typeof__(tfObj.size)) == 'null') {
		DejaGnu.pass("Good, tfObj.size is a  'null'.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.size is not 'null'.");
	}

	if (Std.string(untyped __typeof__(tfObj.font)) == 'null') {
		DejaGnu.pass("Good, tfObj.font is a  'null'.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.font is not 'null'.");
	}

	if (Std.string(untyped __typeof__(tfObj.getTextExtent))== 'function'){
		DejaGnu.pass("The type of tfObj.getTextExtent is 'function'");
	} else {
	    DejaGnu.fail("The type of tfObj.getTextExtent is not 'functino'");
	} 
#end

#if flash9
	//var tfObj:TextFormat = new TextFormat("fname", 2, 30, true, false, true,"http","tgt","cEnter",untyped "23",untyped "32", 12, 4);

	//var tfObj:TextFormat = new TextFormat();
	var tfObj:TextFormat = untyped __new__(TextFormat, ["fname", 2, 30, true, false, true,"http","tgt","center",23,32, 12, 4]);
	//var tfObj:TextFormat = 	Reflect.callMethod(TextFormat, Reflect.field(TextFormat,"new"), ["fname", 2, 30, true, false, true,"http","tgt","center",23,32, 12,4]);
#else
	var tfObj:TextFormat = new TextFormat("fname", 2, 30, true, false, true,"http","tgt","cEnter",untyped "23",untyped "32", 12, 4);

//      This is the right way.
//	var tfObj:TextFormat = untyped __new__(TextFormat, ["fname", 2, 30, true, false, true,"http","tgt","center",23,32, 12, 4]);
//	There must be a problme here.
//	var tfObj:TextFormat = 	Reflect.callMethod(TextFormat, Reflect.field(TextFormat,"new"), ["fname", 2, 30, true, false, true,"http","tgt","cEnter",23,32, 12,4]);
#end

#if !flash9
	if (Std.string(untyped __typeof__(tfObj.display)) == 'string') {
		DejaGnu.xpass("Good, tfObj.display is a  'null'.");
	} else {
	    DejaGnu.xfail("Wrong, tfObj.display is not 'null'.");
	}

	if (Std.string(untyped tfObj.display) == 'block') {
		DejaGnu.xpass("Wrong, tfObj.display equlas to block.");
	} else {
	    DejaGnu.xfail("tfObj.display does not equal to 'block'.");
	}
	
	if (Std.string(untyped __typeof__(tfObj.tabStops)) == 'null') {
		DejaGnu.xpass("Good, tfObj.tabStops is a  'null'.");
	} else {
	    DejaGnu.xfail("Wrong, tfObj.tabStops is not 'null'.");
	}

	if (untyped tfObj.leading == 4) {
		DejaGnu.pass("Good, tfObj.leading equals to 4.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.leading does not equal to 4." + untyped tfObj.leading);
	}

	if (untyped tfObj.indent == 12) {
		DejaGnu.pass("Good, tfObj.indent equals to 12.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.indent does not equal to 12.");
	}

	if (Std.string(untyped tfObj.rightMargin) == '32') {
		DejaGnu.pass("Good, tfObj.rightMargin equals to 32.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.rightMargin does not equal to 32.");
	}

	if (untyped tfObj.leftMargin == 23) {
		DejaGnu.pass("Good, tfObj.leftMargin equals to 23.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.leftMargin does not equal to 23.");
	}

	if (untyped tfObj.font == "fname") {
		DejaGnu.pass("Good, tfObj.font equals to 'fname'.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.font does not equal to 'fname'.");
	}	

	if (tfObj.underline == true) {
		DejaGnu.pass("Good, tfObj.underline equals to true.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.underline does not equal to true.");
	}	

	if (tfObj.italic == false) {
		DejaGnu.pass("Good, tfObj.italic equals to false.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.italic does not equal to false.");
	}

	if (tfObj.bold == true) {
		DejaGnu.pass("Good, tfObj.bold equals to true.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.bold does not equal to true.");
	}	

	if (tfObj.color == 30) {
		DejaGnu.pass("Good, tfObj.color equals to 30.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.color does not equal to 30.");
	}

	if (tfObj.size == 2) {
		DejaGnu.pass("Good, tfObj.size equals to 2.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.size does not equal to 2.");
	}
	

	if (Std.string(untyped __typeof__(tfObj.rightMargin) )== 'number'){
		DejaGnu.pass("Good, tfObj.rightMargin is a  'number'.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.rightMargin is not a 'number'.");
	}

	if (Std.string(untyped __typeof__(tfObj.leftMargin) )== 'number'){
		DejaGnu.pass("Good, tfObj.leftMargin is a  'number'.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.leftMargin is not a 'number'.");
	}

	if (Std.string(untyped __typeof__(tfObj.italic) )== 'boolean'){
		DejaGnu.pass("Good, tfObj.leftMargin is a  'boolean'.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.leftMargin is not a 'boolean'.");
	}

	if (Type.typeof(untyped tfObj.italic)==ValueType.TBool) {
	//if (Std.string(untyped __typeof__(tfObj.italic))== 'boolean') {
		DejaGnu.pass("Good, tfObj.italic is a  'boolean'.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.italic is not 'boolean'.");
	}	

//#if (flash6 || flash7 || flash8)
	if (tfObj.align == "center") {
		DejaGnu.pass("Good, tfObj.align equals to 'center'.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.align does not equal to 'center'.");
	}

	if (Std.string(untyped __typeof__(tfObj.string)) == 'undefined') {
		DejaGnu.pass("Good, tfObj.display is 'undefined'.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.display is not 'undefined'.");
	}

	if (Std.string(untyped __typeof__(tfObj.tabStops)) == 'undefined') {
		DejaGnu.pass("Good, tfObj.tabStops is 'undefined'.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.tabStops is not 'undefined'.");
	}

	if (tfObj.target == "tgt") {
		DejaGnu.pass("Wrong, tfObj.target equals to 'tgt'.");
	} else {
	    DejaGnu.fail("Good, tfObj.target does not equal to 'tgt'.");
	}

	if (tfObj.url == "http") {
		DejaGnu.pass("Wrong, tfObj.url equals to 'http'.");
	} else {
	    DejaGnu.fail("Good, tfObj.url does not equal to 'http'.");
	}

	if (Std.string(untyped __typeof__(tfObj.bullet)) == 'null') {
		DejaGnu.pass("Good, tfObj.bullet is a  'null'.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.bullet is not 'null'.");
	}

	if (Std.string(untyped __typeof__(tfObj.blockIndent)) == 'null') {
		DejaGnu.pass("Good, tfObj.blockIndent is a  'null'.");
	} else {
	    DejaGnu.fail("Wrong, tfObj.blockIndent is not 'null'.");
	}

#end


        DejaGnu.done();
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

