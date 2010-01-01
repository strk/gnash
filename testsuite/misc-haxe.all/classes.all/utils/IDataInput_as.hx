// IDataInput_as.hx:  ActionScript 3 "IDataInput" class, for Gnash.
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
import flash.utils.IDataInput;
import flash.display.MovieClip;
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class IDataInput_as {
    static function main() {

#if flash9
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
DejaGnu.note("This class is an interface");
DejaGnu.note("Implementors: ByteArray, FileStream, Socket, URLStream");
//	if (Std.is(IDataInput.bytesAvailable, Float)) {
//	    DejaGnu.pass("IDataInput.bytesAvailable property exists");
//	} else {
//	    DejaGnu.fail("IDataInput.bytesAvailable property doesn't exist");
//	}
//	if (Std.is(IDataInput.endian, String)) {
//	    DejaGnu.pass("IDataInput.endian property exists");
//	} else {
//	    DejaGnu.fail("IDataInput.endian property doesn't exist");
//	}
//	if (Std.is(IDataInput.objectEncoding, Float)) {
//	    DejaGnu.pass("IDataInput.objectEncoding property exists");
//	} else {
//	    DejaGnu.fail("IDataInput.objectEncoding property doesn't exist");
//	}

// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
//	if (Type.typeof(IDataInput.readBoolean) == ValueType.TFunction) {
//	    DejaGnu.pass("IDataInput::readBoolean() method exists");
//	} else {
//	    DejaGnu.fail("IDataInput::readBoolean() method doesn't exist");
//	}
//	if (Type.typeof(IDataInput.readByte) == ValueType.TFunction) {
//	    DejaGnu.pass("IDataInput::readByte() method exists");
//	} else {
//	    DejaGnu.fail("IDataInput::readByte() method doesn't exist");
//	}
//	if (Type.typeof(IDataInput.readBytes) == ValueType.TFunction) {
//	    DejaGnu.pass("IDataInput::readBytes() method exists");
//	} else {
//	    DejaGnu.fail("IDataInput::readBytes() method doesn't exist");
//	}
//	if (Type.typeof(IDataInput.readDouble) == ValueType.TFunction) {
//	    DejaGnu.pass("IDataInput::readDouble() method exists");
//	} else {
//	    DejaGnu.fail("IDataInput::readDouble() method doesn't exist");
//	}
//	if (Type.typeof(IDataInput.readFloat) == ValueType.TFunction) {
//	    DejaGnu.pass("IDataInput::readFloat() method exists");
//	} else {
//	    DejaGnu.fail("IDataInput::readFloat() method doesn't exist");
//	}
//	if (Type.typeof(IDataInput.readInt) == ValueType.TFunction) {
//	    DejaGnu.pass("IDataInput::readInt() method exists");
//	} else {
//	    DejaGnu.fail("IDataInput::readInt() method doesn't exist");
//	}
//	if (Type.typeof(IDataInput.readMultiByte) == ValueType.TFunction) {
//	    DejaGnu.pass("IDataInput::readMultiByte() method exists");
//	} else {
//	    DejaGnu.fail("IDataInput::readMultiByte() method doesn't exist");
//	}
//	if (Type.typeof(IDataInput.readObject) == ValueType.TFunction) {
//	    DejaGnu.pass("IDataInput::readObject() method exists");
//	} else {
//	    DejaGnu.fail("IDataInput::readObject() method doesn't exist");
//	}
//	if (Type.typeof(IDataInput.readShort) == ValueType.TFunction) {
//	    DejaGnu.pass("IDataInput::readShort() method exists");
//	} else {
//	    DejaGnu.fail("IDataInput::readShort() method doesn't exist");
//	}
//	if (Type.typeof(IDataInput.readUnsignedByte) == ValueType.TFunction) {
//	    DejaGnu.pass("IDataInput::readUnsignedByte() method exists");
//	} else {
//	    DejaGnu.fail("IDataInput::readUnsignedByte() method doesn't exist");
//	}
//	if (Type.typeof(IDataInput.readUnsignedInt) == ValueType.TFunction) {
//	    DejaGnu.pass("IDataInput::readUnsignedInt() method exists");
//	} else {
//	    DejaGnu.fail("IDataInput::readUnsignedInt() method doesn't exist");
//	}
//	if (Type.typeof(IDataInput.readUnsignedShort) == ValueType.TFunction) {
//	    DejaGnu.pass("IDataInput::readUnsignedShort() method exists");
//	} else {
//	    DejaGnu.fail("IDataInput::readUnsignedShort() method doesn't exist");
//	}
//	if (Type.typeof(IDataInput.readUTF) == ValueType.TFunction) {
//	    DejaGnu.pass("IDataInput::readUTF() method exists");
//	} else {
//	    DejaGnu.fail("IDataInput::readUTF() method doesn't exist");
//	}
//	if (Type.typeof(IDataInput.readUTFBytes) == ValueType.TFunction) {
//	    DejaGnu.pass("IDataInput::readUTFBytes() method exists");
//	} else {
//	    DejaGnu.fail("IDataInput::readUTFBytes() method doesn't exist");
//	}
        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
#else
	DejaGnu.note("This class (IDataInput) is only available in flash9");
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

