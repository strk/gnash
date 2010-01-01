// TextFormat_as.hx:  ActionScript 3 "TextFormat" class, for Gnash.
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
import flash.text.TextFormat;
import flash.text.TextFormatAlign;
import flash.text.TextFormatDisplay;
#else
import flash.TextFormat;
#end
import flash.Lib;
import Type;
import Std;
import haxe.PosInfos;

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

	/*FIX ME: test fails
	if (Std.is(x1.align, String)) {
	    DejaGnu.pass("TextFormat.align property exists");
	} else {
	    DejaGnu.fail("TextFormat.align property doesn't exist" + x1.align);
	}*/

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
	
	/*FIXME: This only exists in haXe, not in the Adobe specs
	if (Std.is(x1.display, TextFormatDisplay)) {
	    DejaGnu.pass("TextFormat.display property exists");
	} else {
	    DejaGnu.fail("TextFormat.display property doesn't exist");
	}*/
	
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
	x1.align = "left";
	
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
	if (Std.string(untyped __typeof__(x1.leading  ) )== 'number'){
	    DejaGnu.pass("TextFormat.leading property exists");
	} else {
	    DejaGnu.fail("TextFormat.leading property should be float, returns type "+Type.typeof(x1.leading));
	}
	//FIX ME: gnash uses incorrect data type Int
	//FIXED 
	// SI
	if (Std.string(untyped __typeof__(x1.leftMargin  ) )== 'number'){
	    DejaGnu.pass("TextFormat.leftMargin property exists");
	} else {
	    DejaGnu.fail("TextFormat.leftMargin property should be float, returns type "+Type.typeof(x1.leftMargin));
	}
	//FIX ME: gnash uses incorrect data type Int
	//FIXED 
	// SI
	if (Std.string(untyped __typeof__(x1.rightMargin  ) )== 'number'){	
	    DejaGnu.pass("TextFormat.rightMargin property exists");
	} else {
	    DejaGnu.fail("TextFormat.rightMargin property should be float, returns type "+Type.typeof(x1.rightMargin));
	}
	//FIX ME: gnash uses incorrect data type Int
	//FIXED 
	// SI
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
	if (Std.string(untyped __typeof__(x1.letterSpacing  ) )== 'number'){
		    DejaGnu.pass("TextFormat.letterSpacing property exists");
	} else {
	    DejaGnu.fail("TextFormat.letterSpacing property should be float, returns type "+Type.typeof(x1.letterSpacing));
	}

	if (Std.is(x1.display, String)) {
	    DejaGnu.pass("TextFormat.display property exists");
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
	if (Std.is(x1.tabStops, Dynamic)) {
	    DejaGnu.pass("TextFormat.tabStops property exists");
	} else {
	    DejaGnu.xfail("TextFormat.tabStops property doesn't exist");
	}

	if (Std.is(x1.target, String)) {
	    DejaGnu.pass("TextFormat.target property exists");
	} else {
	    DejaGnu.xfail("TextFormat.target property doesn't exist");
		
	}
	
	if (Std.is(x1.url, String)) {
	    DejaGnu.pass("TextFormat.url property exists");
	} else {
	    DejaGnu.xfail("TextFormat.url property doesn't exist");
	}
// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.

/*FIX ME: test fails in flashplayer
#if (flash6 || flash7)
	if (Type.typeof(x1.getTextExtent) == ValueType.TFunction) {
	    DejaGnu.pass("TextFormat.getTextExtent method exists");
	} else {
	    DejaGnu.fail("TextFormat.getTextExtent method doesn't exist");
	}
#end
*/

//Si
//The followings are ming tests:

//      Not implemeted in the haXe here.
#if flash9
#else
        untyped Object.prototype.hasOwnProperty = ASnative(101, 5);
#end

#if flash9
	if (Std.string(untyped __typeof__(TextFormat) )== 'object') {
		DejaGnu.pass("typeof(TextFormat) == 'object'");
	} else {
	    DejaGnu.fail("typeof(TextFormat) != 'object'");
	}
#else
	if (Std.string(untyped __typeof__(TextFormat) )== 'function') {
		DejaGnu.pass("typeof(TextFormat) == 'function'");
	} else {
	    DejaGnu.fail("typeof(TextFormat) != 'function'");
	}
#end

	if (Std.string(untyped __typeof__(TextFormat.prototype))== 'object') {
		DejaGnu.pass("typeof(TextFormat.prototype) == 'object'");
	} else {
	    DejaGnu.fail("typeof(TextFormat.prototype) != 'object'");
	}
	
//Si
//build an instance

	var tfObj:TextFormat = new TextFormat();

	if (Std.string(untyped __typeof__(tfObj))== 'object') {
		DejaGnu.pass("typeof(tfObj) == 'object'");
	} else {
	    DejaGnu.fail("typeof(tfObj) != 'object'");
	}

#if flash9
	if (Std.is(tfObj, TextFormat) ) {
		DejaGnu.pass("tfObj instanceOf TextFormat");
	} else {
	    DejaGnu.fail("tfObj !instanceOf TextFormat");
	}	
	
#else

//Si
//Check "is" a instance
	if (untyped __instanceof__(tfObj,TextFormat) ) {
		DejaGnu.pass("tfObj instanceOf TextFormat");
	} else {
	    DejaGnu.fail("tfObj !instanceOf TextFormat");
	}
#end

#if (!flash9)

	if (untyped TextFormat.prototype.hasOwnProperty("display")) {
		DejaGnu.pass("TextFormat.prototype.hasOwnProperty('display')");
	} else {
		DejaGnu.fail("!TextFormat.prototype.hasOwnProperty('display')");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("bullet")) {
		DejaGnu.pass("TextFormat.prototype.hasOwnProperty('bullet')");
	} else {
		DejaGnu.fail("!TextFormat.prototype.hasOwnProperty('bullet')");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("tabStops")) {
		DejaGnu.pass("TextFormat.prototype.hasOwnProperty('tabStops')");
	} else {
		DejaGnu.fail("!TextFormat.prototype.hasOwnProperty('tabStops')");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("blockIndent")) {
		DejaGnu.pass("TextFormat.prototype.hasOwnProperty('blockIndent')");
	} else {
		DejaGnu.fail("!TextFormat.prototype.hasOwnProperty('blockIndent')");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("leading")) {
		DejaGnu.pass("TextFormat.prototype.hasOwnProperty('leading')");
	} else {
		DejaGnu.fail("!TextFormat.prototype.hasOwnProperty('leading')");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("indent")) {
		DejaGnu.pass("TextFormat.prototype.hasOwnProperty('indent')");
	} else {
		DejaGnu.fail("!TextFormat.prototype.hasOwnProperty('indent')");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("rightMargin")) {
		DejaGnu.pass("TextFormat.prototype.hasOwnProperty('rightMargin')");
	} else {
		DejaGnu.fail("!TextFormat.prototype.hasOwnProperty('rightMargin')");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("leftMargin")) {
		DejaGnu.pass("TextFormat.prototype.hasOwnProperty('leftMargin')");
	} else {
		DejaGnu.fail("!TextFormat.prototype.hasOwnProperty('leftMargin')");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("align")) {
		DejaGnu.pass("TextFormat.prototype.hasOwnProperty('align')");
	} else {
		DejaGnu.fail("!TextFormat.prototype.hasOwnProperty('align')");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("underline")) {
		DejaGnu.pass("TextFormat.prototype.hasOwnProperty('underline')");
	} else {
		DejaGnu.fail("!TextFormat.prototype.hasOwnProperty('underline')");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("italic")) {
		DejaGnu.pass("TextFormat.prototype.hasOwnProperty('italic')");
	} else {
		DejaGnu.fail("!TextFormat.prototype.hasOwnProperty('italic')");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("bold")) {
		DejaGnu.pass("TextFormat.prototype.hasOwnProperty('bold')");
	} else {
		DejaGnu.fail("!TextFormat.prototype.hasOwnProperty('bold')");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("target")) {
		DejaGnu.pass("TextFormat.prototype.hasOwnProperty('target')");
	} else {
		DejaGnu.fail("!TextFormat.prototype.hasOwnProperty('target')");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("url")) {
		DejaGnu.pass("TextFormat.prototype.hasOwnProperty('url')");
	} else {
		DejaGnu.fail("!TextFormat.prototype.hasOwnProperty('url')");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("color")) {
		DejaGnu.pass("TextFormat.prototype.hasOwnProperty('color')");
	} else {
		DejaGnu.fail("!TextFormat.prototype.hasOwnProperty('color')");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("size")) {
		DejaGnu.pass("TextFormat.prototype.hasOwnProperty('size')");
	} else {
		DejaGnu.fail("!TextFormat.prototype.hasOwnProperty('size')");
	}

	if (untyped TextFormat.prototype.hasOwnProperty("font")) {
		DejaGnu.pass("TextFormat.prototype.hasOwnProperty('font')");
	} else {
		DejaGnu.fail("!TextFormat.prototype.hasOwnProperty('font')");
	}

	if (untyped !TextFormat.prototype.hasOwnProperty("getTextExtent")) {
		DejaGnu.pass("TextFormat.prototype.hasOwnProperty('getTextExtent')");
	} else {
		DejaGnu.fail("!TextFormat.prototype.hasOwnProperty('getTextExtent')");
	}

	#if (flash6 || flash7)
	if (untyped tfObj.hasOwnProperty("getTextExtent")) {
		DejaGnu.pass("tfObj.hasOwnProperty('getTextExtent')");
	} else {
		DejaGnu.fail("tfObj.hasOwnProperty('getTextExtent')");
	}
	#end
#else
#end

//More checks.
// When you construct a TextFormat w/out args all members
// are of the 'null' type. In general, uninitialized members
// are all of the 'null' type.

#if flash9
#else
	if (Std.string(untyped __typeof__(tfObj.display)) == 'string') {
		DejaGnu.pass("typeof(tfObj.display) == 'string'");
	} else {
		DejaGnu.xfail("typeof(tfObj.display) != 'string'");
	}

	// If there is no display defined, then display is simply null --
	// this test is the same as above in essence, until display is 
	// changed
	untyped tfObj.display = "block";
	if (Std.string(untyped tfObj.display) == 'block') {
		DejaGnu.pass("typeof(tfObj.display) == 'block'");
	} else {
		DejaGnu.fail("typeof(tfObj.display) != 'block'");
	}

	if (Std.string(untyped __typeof__(tfObj.bullet)) == 'null') {
		DejaGnu.pass("typeof(tfObj.bullet) == 'null'");
	} else {
		DejaGnu.fail("typeof(tfObj.bullet) != 'null'");
	}

	if (Std.string(untyped __typeof__(tfObj.tabStops)) == 'null') {
		DejaGnu.pass("typeof(tfObj.tabStops) == 'null'");
	} else {
		DejaGnu.fail("typeof(tfObj.tabStops) != 'null'");
	}

	if (Std.string(untyped __typeof__(tfObj.blockIndent)) == 'null') {
		DejaGnu.pass("typeof(tfObj.blockIndent) == 'null'");
	} else {
		DejaGnu.fail("typeof(tfObj.blockIndent) != 'null'");
	}

	if (Std.string(untyped __typeof__(tfObj.leading)) == 'null') {
		DejaGnu.pass("typeof(tfObj.leading) == 'null'");
	} else {
		DejaGnu.fail("typeof(tfObj.leading) != 'null'");
	}

	if (Std.string(untyped __typeof__(tfObj.indent)) == 'null') {
		DejaGnu.pass("typeof(tfObj.indent) == 'null'");
	} else {
		DejaGnu.fail("typeof(tfObj.indent) != 'null'");
	}

	if (Std.string(untyped __typeof__(tfObj.rightMargin)) == 'null') {
		DejaGnu.pass("typeof(tfObj.rightMargin) == 'null'");
	} else {
		DejaGnu.fail("typeof(tfObj.rightMargin) != 'null'");
	}

	if (Std.string(untyped __typeof__(tfObj.leftMargin)) == 'null') {
		DejaGnu.pass("typeof(tfObj.leftMargin) == 'null'");
	} else {
		DejaGnu.fail("typeof(tfObj.leftMargin) != 'null'");
	}

	if (Std.string(untyped __typeof__(tfObj.align)) == 'null') {
		DejaGnu.pass("typeof(tfObj.align) == 'null'");
	} else {
		DejaGnu.fail("typeof(tfObj.align) != 'null'");
	}

	if (Std.string(untyped __typeof__(tfObj.underline)) == 'null') {
		DejaGnu.pass("typeof(tfObj.underline) == 'null'");
	} else {
		DejaGnu.fail("typeof(tfObj.underline) != 'null'");
	}

	if (Std.string(untyped __typeof__(tfObj.italic)) == 'null') {
		DejaGnu.pass("typeof(tfObj.italic) == 'null'");
	} else {
		DejaGnu.fail("typeof(tfObj.italic) != 'null'");
	}

	if (Std.string(untyped __typeof__(tfObj.bold)) == 'null') {
		DejaGnu.pass("typeof(tfObj.bold) == 'null'");
	} else {
	    DejaGnu.fail("typeof(tfObj.bold) != 'null'");
	}

	if (Std.string(untyped __typeof__(tfObj.target)) == 'null') {
		DejaGnu.pass("typeof(tfObj.target) == 'null'");
	} else {
		DejaGnu.fail("typeof(tfObj.target) != 'null'");
	}

	if (Std.string(untyped __typeof__(tfObj.url)) == 'null') {
		DejaGnu.pass("typeof(tfObj.url) == 'null'");
	} else {
	    DejaGnu.fail("typeof(tfObj.url) != 'null'");
	}

 	if (Std.string(untyped __typeof__(tfObj.color)) == 'null') {
		DejaGnu.pass("typeof(tfObj.color) == 'null'");
	} else {
	    DejaGnu.fail("typeof(tfObj.color) != 'null'");
	}

	if (Std.string(untyped __typeof__(tfObj.size)) == 'null') {
		DejaGnu.pass("typeof(tfObj.size) == 'null'");
	} else {
	    DejaGnu.fail("typeof(tfObj.size) != 'null'");
	}

	if (Std.string(untyped __typeof__(tfObj.font)) == 'null') {
		DejaGnu.pass("typeof(tfObj.font) == 'null'");
	} else {
	    DejaGnu.fail("typeof(tfObj.font) != 'null'");
	}

	/*FIX ME: text fails in flashplayer
	#if (flash6 || flash7)
	if (Std.string(untyped __typeof__(tfObj.getTextExtent))== 'function') {
		DejaGnu.pass("typeof(tfObj.getTextExtent) == 'function'");
	} else {
	    DejaGnu.fail("typeof(tfObj.getTextExtent) != 'function'");
	} 
	#end*/
#end

#if flash9
	var tfObj:TextFormat = untyped __new__(TextFormat, ["fname", 2, 30, true, false, true,"http","tgt","center",23,32, 12, 4]);
#else
	var tfObj:TextFormat = new TextFormat("fname", 2, 30, true, false, true,"http","tgt","cEnter",untyped "23",untyped "32", 12, 4);
#end

#if !flash9
	if (Std.string(untyped __typeof__(tfObj.display)) == 'string') {
		DejaGnu.pass("typeof(tfObj.display) == 'string'");
	} else {
	    DejaGnu.xfail("typeof(tfObj.display) != 'string'");
	}

	untyped tfObj.display = "block";
	if (Std.string(untyped tfObj.display) == 'block') {
		DejaGnu.pass("typeof(tfObj.display) == 'block'");
	} else {
	    DejaGnu.fail("typeof(tfObj.display) != 'block'");
	}
	
	if (Std.string(untyped __typeof__(tfObj.tabStops)) == 'null') {
		DejaGnu.pass("typeof(tfObj.tabStops) == 'null'");
	} else {
	    DejaGnu.fail("typeof(tfObj.tabStops) != 'null'");
	}

	if (untyped tfObj.leading == 4) {
		DejaGnu.pass("tfObj.leading == 4");
	} else {
	    DejaGnu.fail("tfObj.leading != 4");
	}

	if (untyped tfObj.indent == 12) {
		DejaGnu.pass("tfObj.indent == 12");
	} else {
	    DejaGnu.fail("tfObj.indent != 12");
	}

	if (Std.string(untyped tfObj.rightMargin) == '32') {
		DejaGnu.pass("tfObj.rightMargin == 32");
	} else {
	    DejaGnu.fail("tfObj.rightMargin != 32");
	}

	if (untyped tfObj.leftMargin == 23) {
		DejaGnu.pass("tfObj.leftMargin == 23");
	} else {
	    DejaGnu.fail("tfObj.leftMargin != 23");
	}

	if (untyped tfObj.font == "fname") {
		DejaGnu.pass("tfObj.font == 'fname'");
	} else {
	    DejaGnu.fail("tfObj.font != 'fname'");
	}	

	if (tfObj.underline == true) {
		DejaGnu.pass("tfObj.underline == true");
	} else {
	    DejaGnu.fail("tfObj.underline != true");
	}	

	if (tfObj.italic == false) {
		DejaGnu.pass("tfObj.italic == false");
	} else {
	    DejaGnu.fail("tfObj.italic != false");
	}

	if (tfObj.bold == true) {
		DejaGnu.pass("tfObj.bold == true");
	} else {
	    DejaGnu.fail("tfObj.bold != true");
	}	

	if (tfObj.color == 30) {
		DejaGnu.pass("tfObj.color == 30");
	} else {
	    DejaGnu.fail("tfObj.color != 30");
	}

	if (tfObj.size == 2) {
		DejaGnu.pass("tfObj.size == 2");
	} else {
	    DejaGnu.fail("tfObj.size != 2");
	}
	

	if (Std.string(untyped __typeof__(tfObj.rightMargin) )== 'number'){
		DejaGnu.pass("typeof(tfObj.rightMargin) == 'number'");
	} else {
	    DejaGnu.fail("typeof(tfObj.rightMargin) != 'number'");
	}

	if (Std.string(untyped __typeof__(tfObj.leftMargin) )== 'number'){
		DejaGnu.pass("typeof(tfObj.leftMargin) == 'number'");
	} else {
	    DejaGnu.fail("typeof(tfObj.leftMargin) != 'number'");
	}

	if (Std.string(untyped __typeof__(tfObj.italic) )== 'boolean'){
		DejaGnu.pass("typeof(tfObj.italic) == 'boolean'");
	} else {
	    DejaGnu.fail("typeof(tfObj.italic) != 'boolean'");
	}

	if (tfObj.align == "center") {
		DejaGnu.pass("tfObj.align == 'center'");
	} else {
	    DejaGnu.fail("tfObj.align != 'center'");
	}

	if (Std.string(untyped __typeof__(tfObj.string)) == 'undefined') {
		DejaGnu.pass("typeof(tfObj.display) == 'undefined'");
	} else {
	    DejaGnu.fail("typeof(tfObj.display) != 'undefined'");
	}

	if (Std.string(untyped __typeof__(tfObj.tabStops)) == 'null') {
		DejaGnu.pass("typeof(tfObj.tabStops) == 'null'");
	} else {
	    DejaGnu.fail("typeof(tfObj.tabStops) != 'null'");
	}

	if (tfObj.target == "tgt") {
		DejaGnu.pass("tfObj.target == 'tgt'");
	} else {
	    DejaGnu.fail("tfObj.target != 'tgt'");
	}

	if (tfObj.url == "http") {
		DejaGnu.pass("tfObj.url == 'http'");
	} else {
	    DejaGnu.fail("tfObj.url != 'http'");
	}

	if (Std.string(untyped __typeof__(tfObj.bullet)) == 'null') {
		DejaGnu.pass("typeof(tfObj.bullet) == 'null'");
	} else {
	    DejaGnu.fail("typeof(tfObj.bullet) != 'null'");
	}

	if (Std.string(untyped __typeof__(tfObj.blockIndent)) == 'null') {
		DejaGnu.pass("typeof(tfObj.blockIndent) == 'null'");
	} else {
	    DejaGnu.fail("typeof(tfObj.blockIndent) != 'null'");
	}

#end
        DejaGnu.done();
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

