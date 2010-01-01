// TextRenderer_as.hx:  ActionScript 3 "TextRenderer" class, for Gnash.
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

#if (flash9 || flash8)
import flash.text.TextRenderer;
#end
#if flash9
import flash.text.TextDisplayMode;
import flash.text.AntiAliasType;
#end
import flash.Lib;
import Type;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class TextRenderer_as {
    static function main() {
#if (flash8 || flash9)

// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
#if flash9
	if (Std.is(TextRenderer.displayMode, String)) {
	    DejaGnu.pass("TextRenderer.displayMode property exists");
	} else {
	    DejaGnu.fail("TextRenderer.displayMode property doesn't exist");
	}
	//FIXME: This property does not exist in the Adobe specs
	if (Std.is(TextRenderer.antiAliasType, AntiAliasType)) {
	    DejaGnu.pass("TextRenderer.antiAliasType property exists");
	} else {
	    DejaGnu.xfail("TextRenderer.antiAliasType property doesn't exist");
	}
#end
	//FIXME: This property does not appear to have been implemented
	if (Type.typeof(TextRenderer.maxLevel) == ValueType.TInt) {
	    DejaGnu.pass("TextRenderer.maxLevel property exists");
	} else {
	    DejaGnu.xfail("TextRenderer.maxLevel property doesn't exist");
	}

	// Tests to see if all the methods exist. All these do is test for
	// existance of a method, and don't test the functionality at all. This
	// is primarily useful only to test completeness of the API implementation.
#if flash9
	if (Type.typeof(TextRenderer.setAdvancedAntiAliasingTable) == ValueType.TFunction) {
	    DejaGnu.pass("TextRenderer::setAdvancedAntiAliasingTable() method exists");
	} else {
	    DejaGnu.xfail("TextRenderer::setAdvancedAntiAliasingTable() method doesn't exist");
	}
#else
	if (Type.typeof(TextRenderer.setAdvancedAntialiasingTable) == ValueType.TFunction) {
	    DejaGnu.pass("TextRenderer::setAdvancedAntialiasingTable() method exists");
	} else {
	    DejaGnu.xfail("TextRenderer::setAdvancedAntialiasingTable() method doesn't exist");
	}
#end
	// Call this after finishing all tests. It prints out the totals.
	DejaGnu.done();
#else
	DejaGnu.note("This class (TextRenderer) is only available in flash8 and flash9");
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

