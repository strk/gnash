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
        var x1:TextFormat = new TextFormat("font","size","color","bold","italic","underline","url","target","left","leftMargin","rightMargin","indent","leading");
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
	if (Std.is(x1.align, String)) {
	    DejaGnu.pass("TextFormat.align property exists");
	} else {
	    DejaGnu.fail("TextFormat.align property doesn't exist");
	}
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
	    DejaGnu.xpass("TextFormat.letterSpacing property exists");
	} else {
	    DejaGnu.xfail("TextFormat.letterSpacing property doesn't exist");
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
	if (Type.typeof(x1.blockIndent) == ValueType.TFloat) {
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
	if (Type.typeof(x1.indent) == ValueType.TFloat) {
	    DejaGnu.pass("TextFormat.indent property exists");
	} else {
	    DejaGnu.fail("TextFormat.indent property should be float, returns type "+Type.typeof(x1.indent));
	}
	if (Type.typeof(x1.italic) == ValueType.TBool) {
	    DejaGnu.pass("TextFormat.italic property exists");
	} else {
	    DejaGnu.fail("TextFormat.italic property doesn't exist");
	}
	if (Type.typeof(x1.leading) == ValueType.TFloat) {
	    DejaGnu.pass("TextFormat.leading property exists");
	} else {
	    DejaGnu.fail("TextFormat.leading property should be float, returns type "+Type.typeof(x1.leading));
	}
	if (Type.typeof(x1.leftMargin) == ValueType.TFloat) {
	    DejaGnu.pass("TextFormat.leftMargin property exists");
	} else {
	    DejaGnu.fail("TextFormat.leftMargin property should be float, returns type "+Type.typeof(x1.leftMargin));
	}
	if (Type.typeof(x1.rightMargin) == ValueType.TFloat) {
	    DejaGnu.pass("TextFormat.rightMargin property exists");
	} else {
	    DejaGnu.fail("TextFormat.rightMargin property should be float, returns type "+Type.typeof(x1.rightMargin));
	}
	if (Type.typeof(x1.size) == ValueType.TFloat) {
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
	if (Type.typeof(x1.letterSpacing) == ValueType.TFloat) {
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
        DejaGnu.done();
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

