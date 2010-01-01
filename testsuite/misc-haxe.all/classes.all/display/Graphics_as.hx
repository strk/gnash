// Graphics_as.hx:  ActionScript 3 "Graphics" class, for Gnash.
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
import flash.display.Graphics;
import flash.display.MovieClip;
import flash.display.Shape;
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class Graphics_as {
    static function main() {
#if flash9
        var s1:Shape = new Shape();
        var x1:Graphics = s1.graphics; //Graphics property of DisplayObject is Graphics object

        // Make sure we actually get a valid class        
        if (Std.is(x1, Graphics)) {
            DejaGnu.pass("Graphics class exists");
        } else {
            DejaGnu.fail("Graphics lass doesn't exist");
        }

// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.

	if (Type.typeof(x1.beginBitmapFill) == ValueType.TFunction) {
		DejaGnu.pass("Graphics::beginBitmapFill method exists");
	} else {
		DejaGnu.fail("Graphics::beginBitmapFill method doesn't exist");
	}
	if (Type.typeof(x1.beginFill) == ValueType.TFunction) {
		DejaGnu.pass("Graphics::beginFill method exists");
	} else {
		DejaGnu.fail("Graphics::beginFill method doesn't exist");
	}
	if (Type.typeof(x1.beginGradientFill) == ValueType.TFunction) {
		DejaGnu.pass("Graphics::beginGradientFill method exists");
	} else {
		DejaGnu.fail("Graphics::beginGradientFill method doesn't exist");
	}
	if (Type.typeof(x1.clear) == ValueType.TFunction) {
		DejaGnu.pass("Graphics::clear method exists");
	} else {
		DejaGnu.fail("Graphics::clear method doesn't exist");
	}
	if (Type.typeof(x1.curveTo) == ValueType.TFunction) {
		DejaGnu.pass("Graphics::curveTo method exists");
	} else {
		DejaGnu.fail("Graphics::curveTo method doesn't exist");
	}
	if (Type.typeof(x1.drawCircle) == ValueType.TFunction) {
		DejaGnu.pass("Graphics::drawCircle method exists");
	} else {
		DejaGnu.fail("Graphics::drawCircle method doesn't exist");
	}
	if (Type.typeof(x1.drawEllipse) == ValueType.TFunction) {
		DejaGnu.pass("Graphics::drawEllipse method exists");
	} else {
		DejaGnu.fail("Graphics::drawEllipse method doesn't exist");
	}
	if (Type.typeof(x1.drawRect) == ValueType.TFunction) {
		DejaGnu.pass("Graphics::drawRect method exists");
	} else {
		DejaGnu.fail("Graphics::drawRect method doesn't exist");
	}
	if (Type.typeof(x1.drawRoundRect) == ValueType.TFunction) {
		DejaGnu.pass("Graphics::drawRoundRect method exists");
	} else {
		DejaGnu.fail("Graphics::drawRoundRect method doesn't exist");
	}
	if (Type.typeof(x1.drawRoundRectComplex) == ValueType.TFunction) {
		DejaGnu.pass("Graphics::drawRoundRectComplex method exists");
	} else {
		DejaGnu.fail("Graphics::drawRoundRectComplex method doesn't exist");
	}
	if (Type.typeof(x1.endFill) == ValueType.TFunction) {
		DejaGnu.pass("Graphics::endFill method exists");
	} else {
		DejaGnu.fail("Graphics::endFill method doesn't exist");
	}
	if (Type.typeof(x1.lineGradientStyle) == ValueType.TFunction) {
		DejaGnu.pass("Graphics::lineGradientStyle method exists");
	} else {
		DejaGnu.fail("Graphics::lineGradientStyle method doesn't exist");
	}
	if (Type.typeof(x1.lineStyle) == ValueType.TFunction) {
		DejaGnu.pass("Graphics::lineStyle method exists");
	} else {
		DejaGnu.fail("Graphics::lineStyle method doesn't exist");
	}
	if (Type.typeof(x1.lineTo) == ValueType.TFunction) {
		DejaGnu.pass("Graphics::lineTo method exists");
	} else {
		DejaGnu.fail("Graphics::lineTo method doesn't exist");
	}
	if (Type.typeof(x1.moveTo) == ValueType.TFunction) {
		DejaGnu.pass("Graphics::moveTo method exists");
	} else {
		DejaGnu.fail("Graphics::moveTo method doesn't exist");
	}

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
#else
	DejaGnu.note("This class (Graphics) only exists in flash9");
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

