// TextLineMetrics_as.hx:  ActionScript 3 "TextLineMetrics" class, for Gnash.
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
import flash.text.TextLineMetrics;
import flash.text.TextField;
import flash.display.MovieClip;
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class TextLineMetrics_as {
    static function main() {
#if flash9
		var t1:TextField = new TextField();
        var x1:TextLineMetrics = t1.getLineMetrics(0);

        // Make sure we actually get a valid class        
        if (Std.is(x1, TextLineMetrics)) {
            DejaGnu.pass("TextLineMetrics class exists");
        } else {
            DejaGnu.fail("TextLineMetrics class doesn't exist");
        }
		
		// Tests to see if all the properties exist. All these do is test for
		// existance of a property, and don't test the functionality at all. This
		// is primarily useful only to test completeness of the API implementation.
		if (Type.typeof(x1.ascent) == ValueType.TFloat) {
			DejaGnu.pass("TextLineMetrics.ascent property exists");
		} else {
			DejaGnu.fail("TextLineMetrics.ascent property doesn't exist");
		}
		if (Type.typeof(x1.descent) == ValueType.TFloat) {
			DejaGnu.pass("TextLineMetrics.descent property exists");
		} else {
			DejaGnu.fail("TextLineMetrics.descent property doesn't exist");
		}
		if (Type.typeof(x1.height) == ValueType.TFloat) {
			DejaGnu.pass("TextLineMetrics.height property exists");
		} else {
			DejaGnu.fail("TextLineMetrics.height property doesn't exist");
		}
		if (Type.typeof(x1.leading) == ValueType.TInt) {
			DejaGnu.pass("TextLineMetrics.leading property exists");
		} else {
			DejaGnu.fail("TextLineMetrics.leading property doesn't exist");
		}
		if (Type.typeof(x1.width) == ValueType.TInt) {
			DejaGnu.pass("TextLineMetrics.width property exists");
		} else {
			DejaGnu.fail("TextLineMetrics.width property doesn't exist");
		}
		if (Type.typeof(x1.x) == ValueType.TInt) {
			DejaGnu.pass("TextLineMetrics.x property exists");
		} else {
			DejaGnu.fail("TextLineMetrics.x property doesn't exist");
		}

		// Call this after finishing all tests. It prints out the totals.
		DejaGnu.done();
#else
	DejaGnu.note("This class (TextLineMetrics) is only available in flash9");
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

