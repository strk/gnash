// FileReference_as.hx:  ActionScript 3 "FileReference" class, for Gnash.
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
import flash.net.FileReference;
import flash.display.MovieClip;
#elseif flash8
import flash.net.FileReference;
import flash.MovieClip;
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class FileReference_as {
    static function main() {
#if (flash9 || flash8)
        var x1:FileReference = new FileReference();

        // Make sure we actually get a valid class        
        if (Std.is(x1, FileReference)) {
            DejaGnu.pass("FileReference class exists");
        } else {
            DejaGnu.xfail("FileReference class doesn't exist");
        }
//FIXME: Must browse to populate FileReference fields, and even then the fields are not correct.
		DejaGnu.note("This test is incomplete. The fields of FileReference have not been correctly populated");
// Tests to see if all the properties exist. All these do is test for
// existence of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
#if flash8
	if (Std.is(x1.postData, String)) {
	    DejaGnu.pass("FileReference.postData property exists");
	} else {
	    DejaGnu.xfail("FileReference.postData property doesn't exist");
	}
#end
	if (Std.is(x1.creator, String)) {
	    DejaGnu.pass("FileReference.creator property exists");
	} else {
	    DejaGnu.xfail("FileReference.creator property doesn't exist");
	}
	if (Std.is(x1.name, String)) {
	    DejaGnu.pass("FileReference.name property exists");
	} else {
	    DejaGnu.xfail("FileReference.name property doesn't exist");
	}
	if (Type.typeof(x1.size) == ValueType.TFloat) {
	    DejaGnu.pass("FileReference.size property exists");
	} else {
	    DejaGnu.xfail("FileReference.size property doesn't exist");
	}
	if (Std.is(x1.type, String)) {
	    DejaGnu.pass("FileReference.type property exists");
	} else {
	    DejaGnu.xfail("FileReference.type property doesn't exist");
	}
	if (Std.is(x1.creationDate, Date)) {
 	    DejaGnu.pass("FileReference.creationDate property exists");
 	} else {
 	    DejaGnu.xfail("FileReference.creationDate property doesn't exist");
 	}
 	if (Std.is(x1.modificationDate, Date)) {
 	    DejaGnu.pass("FileReference.modificationDate property exists");
 	} else {
 	    DejaGnu.xfail("FileReference.modificationDate property doesn't exist");
 	}
//FIXME: The .extension property is only available in AIR
// 	if (Std.is(x1.extension, String)) {
// 	    DejaGnu.pass("FileReference.extension property exists");
// 	} else {
// 	    DejaGnu.fail("FileReference.extension property doesn't exist");
// 	}

// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
 	if (Type.typeof(x1.browse) == ValueType.TFunction) {
 	    DejaGnu.pass("FileReference::browse() method exists");
 	} else {
 	    DejaGnu.xfail("FileReference::browse() method doesn't exist");
 	}
	if (Type.typeof(x1.cancel) == ValueType.TFunction) {
	    DejaGnu.pass("FileReference::cancel() method exists");
	} else {
	    DejaGnu.xfail("FileReference::cancel() method doesn't exist");
	}
	if (Type.typeof(x1.download) == ValueType.TFunction) {
	    DejaGnu.pass("FileReference::download() method exists");
	} else {
	    DejaGnu.xfail("FileReference::download() method doesn't exist");
	}
	if (Type.typeof(x1.upload) == ValueType.TFunction) {
	    DejaGnu.pass("FileReference::upload() method exists");
	} else {
	    DejaGnu.xfail("FileReference::upload() method doesn't exist");
	}
// This method is AIR only
// 	if (x1.uploadUnencoded == null) {
// 	    DejaGnu.pass("FileReference::uploadUnencoded() method exists");
// 	} else {
// 	    DejaGnu.fail("FileReference::uploadUnencoded() method doesn't exist");
// 	}

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
#else
	DejaGnu.note("This class (FileReference) is only available in flash8 and flash9");
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

