// MorphShape_as.hx:  ActionScript 3 "MorphShape" class, for Gnash.
//
// Generated on: 20090603 by "bnaugle". Remove this
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
import flash.display.MorphShape;
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class MorphShape_as {
    static function main() {
#if flash9
		DejaGnu.note("This class has no methods or properties,");
		DejaGnu.note("and is only created when you create a shape tween");
		DejaGnu.note("in the Adobe/Flash authoring tool");

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
#else
	DejaGnu.note("This class (MorphShape) is only available in flash9");
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

