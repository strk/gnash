// SharedObject_as.hx:  ActionScript 3 "SharedObject" class, for Gnash.
//
// Generated on: 20090528 by "bnaugle". Remove this
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
import flash.net.SharedObject;
import flash.display.MovieClip;
#else
import flash.MovieClip;
import flash.SharedObject;
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class SharedObject_as {
    static function main() {
#if flash9
        var x1:SharedObject = SharedObject.getLocal("sharedobjecttest");
#else
		var x1:SharedObject = SharedObject.getLocal("sharedobjecttest"); 
#end         
        if (Std.is(x1, SharedObject)) {
            DejaGnu.pass("SharedObject class exists");
        } else {
            DejaGnu.fail("SharedObject class doesn't exist");
        }
        DejaGnu.note("SharedObject type is "+Type.typeof(x1));
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
#if flash9
 	if (Std.is(x1.client, Dynamic)) {
 	    DejaGnu.pass("SharedObject.client property exists");
 	} else {
 	    DejaGnu.fail("SharedObject.client property doesn't exist");
 	}
//FIXME:  Field fps cannot be accessed for reading
// 	if (Type.typeof(x1.fps) == ValueType.TFloat) {
// 	    DejaGnu.pass("SharedObject.fps property exists");
// 	} else {
// 	    DejaGnu.fail("SharedObject.fps property doesn't exist");
// 	}
	if (Type.typeof(x1.objectEncoding) == ValueType.TInt) {
	    DejaGnu.pass("SharedObject.objectEncoding property exists");
	} else {
	    DejaGnu.fail("SharedObject.objectEncoding property doesn't exist");
	}
	if (Type.typeof(x1.size) == ValueType.TInt) {
	    DejaGnu.pass("SharedObject.size property exists");
	} else {
	    DejaGnu.fail("SharedObject.size property doesn't exist");
	}
	if (Type.typeof(SharedObject.defaultObjectEncoding) == ValueType.TInt) {
	    DejaGnu.pass("SharedObject.defaultObjectEncoding property exists");
	} else {
	    DejaGnu.fail("SharedObject.defaultObjectEncoding property doesn't exist");
	}
#end
 	if (Std.is(x1.data, Dynamic)) {
 	    DejaGnu.pass("SharedObject.data property exists");
 	} else {
 	    DejaGnu.fail("SharedObject.data property doesn't exist");
 	}
// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (Type.typeof(x1.clear) == ValueType.TFunction) {
	    DejaGnu.pass("SharedObject::clear() method exists");
	} else {
	    DejaGnu.fail("SharedObject::clear() method doesn't exist");
	}
	if (Type.typeof(x1.close) == ValueType.TFunction) {
	    DejaGnu.pass("SharedObject::close() method exists");
	} else {
	    DejaGnu.fail("SharedObject::close() method doesn't exist");
	}
	if (Type.typeof(x1.connect) == ValueType.TFunction) {
	    DejaGnu.pass("SharedObject::connect() method exists");
	} else {
	    DejaGnu.fail("SharedObject::connect() method doesn't exist");
	}
	if (Type.typeof(x1.flush) == ValueType.TFunction) {
	    DejaGnu.pass("SharedObject::flush() method exists");
	} else {
	    DejaGnu.fail("SharedObject::flush() method doesn't exist");
	}
	if (Type.typeof(x1.send) == ValueType.TFunction) {
	    DejaGnu.pass("SharedObject::send() method exists");
	} else {
	    DejaGnu.fail("SharedObject::send() method doesn't exist");
	}
#if flash9
	if (Type.typeof(x1.setDirty) == ValueType.TFunction) {
	    DejaGnu.pass("SharedObject::setDirty() method exists");
	} else {
	    DejaGnu.fail("SharedObject::setDirty() method doesn't exist");
	}
	if (Type.typeof(x1.setProperty) == ValueType.TFunction) {
	    DejaGnu.pass("SharedObject::setProperty() method exists");
	} else {
	    DejaGnu.fail("SharedObject::setProperty() method doesn't exist");
	}
#end
 	if (Type.typeof(SharedObject.getLocal) == ValueType.TFunction) {
 	    DejaGnu.pass("SharedObject::getLocal() method exists");
 	} else {
 	    DejaGnu.fail("SharedObject::getLocal() method doesn't exist");
 	}
 	if (Type.typeof(SharedObject.getRemote) == ValueType.TFunction) {
 	    DejaGnu.pass("SharedObject::getRemote() method exists");
 	} else {
 	    DejaGnu.fail("SharedObject::getRemote() method doesn't exist");
 	}
    // Call this after finishing all tests. It prints out the totals.
    DejaGnu.done();
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

