// InteractiveObject_as.hx:  ActionScript 3 "InteractiveObject" class, for Gnash.
//
// Generated on: 20090601 by "bnaugle". Remove this
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
import flash.accessibility.AccessibilityImplementation;
import flash.display.InteractiveObject;
import flash.display.MovieClip;
import flash.display.SimpleButton;
import flash.ui.ContextMenu;
#end
import flash.Lib;
import Type;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class InteractiveObject_as {
    static function main() {
#if flash9
        var x1:InteractiveObject = new SimpleButton();

        // Make sure we actually get a valid class        
        if (Std.is(x1, InteractiveObject)) {
            DejaGnu.pass("InteractiveObject class exists");
        } else {
            DejaGnu.fail("InteractiveObject lass doesn't exist");
        }
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
//FIXME: This property, and the AccessibilityImplementation class, only exist in haXe
	var a1:AccessibilityImplementation = new AccessibilityImplementation();
	x1.accessibilityImplementation = a1;
	if (Std.is(x1.accessibilityImplementation, AccessibilityImplementation)) {
	    DejaGnu.pass("InteractiveObject::accessibilityImplementation property exists");
	} else {
	    DejaGnu.fail("InteractiveObject::accessibilityImplementation property doesn't exist");
	}
	var c1:ContextMenu = new ContextMenu();
	x1.contextMenu = c1;
 	if (Std.is(x1.contextMenu, ContextMenu)) {
 	    DejaGnu.pass("InteractiveObject::contextMenu property exists");
 	} else {
 	    DejaGnu.fail("InteractiveObject::contextMenu property doesn't exist");
 	}
	if (Type.typeof(x1.doubleClickEnabled) == ValueType.TBool) {
	    DejaGnu.pass("InteractiveObject::doubleClickEnabled property exists");
	} else {
	    DejaGnu.fail("InteractiveObject::doubleClickEnabled property doesn't exist");
	}
 	if (Std.is(x1.focusRect, Dynamic)) {
 	    DejaGnu.pass("InteractiveObject::focusRect property exists");
 	} else {
 	    DejaGnu.fail("InteractiveObject::focusRect property doesn't exist");
 	}
	if (Type.typeof(x1.mouseEnabled) == ValueType.TBool) {
	    DejaGnu.pass("InteractiveObject::mouseEnabled property exists");
	} else {
	    DejaGnu.fail("InteractiveObject::mouseEnabled property doesn't exist");
	}
	if (Type.typeof(x1.tabEnabled) == ValueType.TBool) {
	    DejaGnu.pass("InteractiveObject::tabEnabled property exists");
	} else {
	    DejaGnu.fail("InteractiveObject::tabEnabled property doesn't exist");
	}
	if (Type.typeof(x1.tabIndex) == ValueType.TInt) {
	    DejaGnu.pass("InteractiveObject::tabIndex property exists");
	} else {
	    DejaGnu.fail("InteractiveObject::tabIndex property doesn't exist");
	}

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
#else
	DejaGnu.note("This class (InteractiveObject) is only available in flash9");
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

