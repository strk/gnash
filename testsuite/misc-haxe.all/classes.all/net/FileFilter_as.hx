// FileFilter_as.hx:  ActionScript 3 "FileFilter" class, for Gnash.
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
import flash.net.FileFilter;
import flash.display.MovieClip;
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class FileFilter_as {
    static function main() {
#if flash9
        var x1:FileFilter = new FileFilter("description", "extension", "macType");

        // Make sure we actually get a valid class        
        if (Std.is(x1, FileFilter)) {
            DejaGnu.pass("FileFilter class exists");
        } else {
            DejaGnu.fail("FileFilter class doesn't exist");
        }
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (Std.is(x1.description, String)) {
	    DejaGnu.pass("FileFilter.description property exists");
	} else {
	    DejaGnu.fail("FileFilter.description property doesn't exist");
	}
	if (Std.is(x1.extension, String)) {
	    DejaGnu.pass("FileFilter.extension property exists");
	} else {
	    DejaGnu.fail("FileFilter.extension property doesn't exist");
	}
	if (Std.is(x1.macType, String)) {
	    DejaGnu.pass("FileFilter.macType property exists");
	} else {
	    DejaGnu.fail("FileFilter.macType property doesn't exist");
	}

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
#else
	DejaGnu.note("This class (FileFilter) is only available in flash9");
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

