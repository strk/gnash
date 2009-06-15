// TextSnapshot_as.hx:  ActionScript 3 "TextSnapshot" class, for Gnash.
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

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class TextSnapshot_as {
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
            DejaGnu.pass("TextSnapshot class exists");
        } else {
            DejaGnu.fail("TextSnapshot class doesn't exist");
        }
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
#if flash9
	if (Type.typeof(x1.charCount) == ValueType.TInt) {
	    DejaGnu.pass("TextSnapshot.charCount property exists");
	} else {
	    DejaGnu.fail("TextSnapshot.charCount property doesn't exist");
	}
#end

// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
#if !flash9
	if (Type.typeof(x1.getCount) == ValueType.TFunction) {
	    DejaGnu.pass("TextSnapshot::getCount() method exists");
	} else {
	    DejaGnu.fail("TextSnapshot::getCount() method doesn't exist");
	}
#else
	if (Type.typeof(x1.getTextRunInfo) == ValueType.TFunction) {
	    DejaGnu.pass("TextSnapshot::getTextRunInfo() method exists");
	} else {
	    DejaGnu.fail("TextSnapshot::getTextRunInfo() method doesn't exist");
	}
#end
	if (Type.typeof(x1.findText) == ValueType.TFunction) {
	    DejaGnu.pass("TextSnapshot::findText() method exists");
	} else {
	    DejaGnu.fail("TextSnapshot::findText() method doesn't exist");
	}
	if (Type.typeof(x1.getSelected) == ValueType.TFunction) {
	    DejaGnu.pass("TextSnapshot::getSelected() method exists");
	} else {
	    DejaGnu.fail("TextSnapshot::getSelected() method doesn't exist");
	}
	if (Type.typeof(x1.getSelectedText) == ValueType.TFunction) {
	    DejaGnu.pass("TextSnapshot::getSelectedText() method exists");
	} else {
	    DejaGnu.fail("TextSnapshot::getSelectedText() method doesn't exist");
	}
	if (Type.typeof(x1.getText) == ValueType.TFunction) {
	    DejaGnu.pass("TextSnapshot::getText() method exists");
	} else {
	    DejaGnu.fail("TextSnapshot::getText() method doesn't exist");
	}
	if (Type.typeof(x1.hitTestTextNearPos) == ValueType.TFunction) {
	    DejaGnu.pass("TextSnapshot::hitTestTextNearPos() method exists");
	} else {
	    DejaGnu.fail("TextSnapshot::hitTestTextNearPos() method doesn't exist");
	}
	if (Type.typeof(x1.setSelectColor) == ValueType.TFunction) {
	    DejaGnu.pass("TextSnapshot::setSelectColor() method exists");
	} else {
	    DejaGnu.fail("TextSnapshot::setSelectColor() method doesn't exist");
	}
	if (Type.typeof(x1.setSelected) == ValueType.TFunction) {
	    DejaGnu.pass("TextSnapshot::setSelected() method exists");
	} else {
	    DejaGnu.fail("TextSnapshot::setSelected() method doesn't exist");
	}

        // Call this after finishing all tests. It prints out the totals.
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

