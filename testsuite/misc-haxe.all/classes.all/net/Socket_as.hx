// Socket_as.hx:  ActionScript 3 "Socket" class, for Gnash.
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
import flash.net.Socket;
import flash.display.MovieClip;
import flash.utils.Endian;
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class Socket_as {
    static function main() {
#if flash9
        var x1:Socket = new Socket();

        // Make sure we actually get a valid class        
        if (Std.is(x1, Socket)) {
            DejaGnu.pass("Socket class exists");
        } else {
            DejaGnu.fail("Socket class doesn't exist");
        }
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
//FIXME: This test will only work if we have a valid Socket
//	if (Std.is(x1.bytesAvailable, Int)) {
//	    DejaGnu.pass("Socket.bytesAvailable property exists");
//	} else {
//	    DejaGnu.fail("Socket.bytesAvailable property doesn't exist");
//	}
	if (Type.typeof(x1.connected) == ValueType.TBool) {
	    DejaGnu.pass("Socket.connected property exists");
	} else {
	    DejaGnu.fail("Socket.connected property doesn't exist");
	}
	if (Std.is(x1.endian, String)) {
	    DejaGnu.pass("Socket.endian property exists");
	} else {
	    DejaGnu.fail("Socket.endian property doesn't exist");
	}
	if (Type.typeof(x1.objectEncoding) == ValueType.TInt) {
	    DejaGnu.pass("Socket.objectEncoding property exists");
	} else {
	    DejaGnu.fail("Socket.objectEncoding property doesn't exist");
	}

// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (Type.typeof(x1.close) == ValueType.TFunction) {
	    DejaGnu.pass("Socket::close() method exists");
	} else {
	    DejaGnu.fail("Socket::close() method doesn't exist");
	}
	if (Type.typeof(x1.close) == ValueType.TFunction) {
	    DejaGnu.pass("Socket::connect() method exists");
	} else {
	    DejaGnu.fail("Socket::connect() method doesn't exist");
	}
	if (Type.typeof(x1.close) == ValueType.TFunction) {
	    DejaGnu.pass("Socket::flush() method exists");
	} else {
	    DejaGnu.fail("Socket::flush() method doesn't exist");
	}
 	if (Type.typeof(x1.readBoolean) == ValueType.TFunction) {
 	    DejaGnu.pass("Socket::readBoolean() method exists");
 	} else {
 	    DejaGnu.fail("Socket::readBoolean() method doesn't exist");
 	}
 	if (Type.typeof(x1.readByte) == ValueType.TFunction) {
 	    DejaGnu.pass("Socket::readByte() method exists");
 	} else {
 	    DejaGnu.fail("Socket::readByte() method doesn't exist");
 	}
 	if (Type.typeof(x1.readBytes) == ValueType.TFunction) {
 	    DejaGnu.pass("Socket::readBytes() method exists");
 	} else {
 	    DejaGnu.fail("Socket::readBytes() method doesn't exist");
 	}
 	if (Type.typeof(x1.readDouble) == ValueType.TFunction) {
 	    DejaGnu.pass("Socket::readDouble() method exists");
 	} else {
 	    DejaGnu.fail("Socket::readDouble() method doesn't exist");
 	}
 	if (Type.typeof(x1.readFloat) == ValueType.TFunction) {
 	    DejaGnu.pass("Socket::readFloat() method exists");
 	} else {
 	    DejaGnu.fail("Socket::readFloat() method doesn't exist");
 	}
 	if (Type.typeof(x1.readInt) == ValueType.TFunction) {
 	    DejaGnu.pass("Socket::readInt() method exists");
 	} else {
	    DejaGnu.fail("Socket::readInt() method doesn't exist");
 	}
 	if (Type.typeof(x1.readMultiByte) == ValueType.TFunction) {
 	    DejaGnu.pass("Socket::readMultiByte() method exists");
 	} else {
 	    DejaGnu.fail("Socket::readMultiByte() method doesn't exist");
 	}
 	if (Type.typeof(x1.readObject) == ValueType.TFunction) {
 	    DejaGnu.pass("Socket::readObject() method exists");
 	} else {
 	    DejaGnu.fail("Socket::readObject() method doesn't exist");
 	}
 	if (Type.typeof(x1.readShort) == ValueType.TFunction) {
 	    DejaGnu.pass("Socket::readShort() method exists");
 	} else {
 	    DejaGnu.fail("Socket::readShort() method doesn't exist");
 	}
 	if (Type.typeof(x1.readUnsignedByte) == ValueType.TFunction) {
 	    DejaGnu.pass("Socket::readUnsignedByte() method exists");
 	} else {
 	    DejaGnu.fail("Socket::readUnsignedByte() method doesn't exist");
 	}
 	if (Type.typeof(x1.readUnsignedInt) == ValueType.TFunction) {
 	    DejaGnu.pass("Socket::readUnsignedInt() method exists");
 	} else {
 	    DejaGnu.fail("Socket::readUnsignedInt() method doesn't exist");
 	}
 	if (Type.typeof(x1.readUnsignedShort) == ValueType.TFunction) {
 	    DejaGnu.pass("Socket::readUnsignedShort() method exists");
 	} else {
 	    DejaGnu.fail("Socket::readUnsignedShort() method doesn't exist");
 	}
	if (Type.typeof(x1.readUTF) == ValueType.TFunction) {
	    DejaGnu.pass("Socket::readUTF() method exists");
	} else {
	    DejaGnu.fail("Socket::readUTF() method doesn't exist");
	}
	if (Type.typeof(x1.readUTFBytes) == ValueType.TFunction) {
	    DejaGnu.pass("Socket::readUTFBytes() method exists");
	} else {
	    DejaGnu.fail("Socket::readUTFBytes() method doesn't exist");
	}
	if (Type.typeof(x1.writeBoolean) == ValueType.TFunction) {
	    DejaGnu.pass("Socket::writeBoolean() method exists");
	} else {
	    DejaGnu.fail("Socket::writeBoolean() method doesn't exist");
	}
	if (Type.typeof(x1.writeByte) == ValueType.TFunction) {
	    DejaGnu.pass("Socket::writeByte() method exists");
	} else {
	    DejaGnu.fail("Socket::writeByte() method doesn't exist");
	}
	if (Type.typeof(x1.writeBytes) == ValueType.TFunction) {
	    DejaGnu.pass("Socket::writeBytes() method exists");
	} else {
	    DejaGnu.fail("Socket::writeBytes() method doesn't exist");
	}
	if (Type.typeof(x1.writeDouble) == ValueType.TFunction) {
	    DejaGnu.pass("Socket::writeDouble() method exists");
	} else {
	    DejaGnu.fail("Socket::writeDouble() method doesn't exist");
	}
	if (Type.typeof(x1.writeFloat) == ValueType.TFunction) {
	    DejaGnu.pass("Socket::writeFloat() method exists");
	} else {
	    DejaGnu.fail("Socket::writeFloat() method doesn't exist");
	}
	if (Type.typeof(x1.writeInt) == ValueType.TFunction) {
	    DejaGnu.pass("Socket::writeInt() method exists");
	} else {
	    DejaGnu.fail("Socket::writeInt() method doesn't exist");
	}
	if (Type.typeof(x1.writeMultiByte) == ValueType.TFunction) {
	    DejaGnu.pass("Socket::writeMultiByte() method exists");
	} else {
	    DejaGnu.fail("Socket::writeMultiByte() method doesn't exist");
	}
	if (Type.typeof(x1.writeObject) == ValueType.TFunction) {
	    DejaGnu.pass("Socket::writeObject() method exists");
	} else {
	    DejaGnu.fail("Socket::writeObject() method doesn't exist");
	}
	if (Type.typeof(x1.writeShort) == ValueType.TFunction) {
	    DejaGnu.pass("Socket::writeShort() method exists");
	} else {
	    DejaGnu.fail("Socket::writeShort() method doesn't exist");
	}
	if (Type.typeof(x1.writeUnsignedInt) == ValueType.TFunction) {
	    DejaGnu.pass("Socket::writeUnsignedInt() method exists");
	} else {
	    DejaGnu.fail("Socket::writeUnsignedInt() method doesn't exist");
	}
	if (Type.typeof(x1.writeUTF) == ValueType.TFunction) {
	    DejaGnu.pass("Socket::writeUTF() method exists");
	} else {
	    DejaGnu.fail("Socket::writeUTF() method doesn't exist");
	}
	if (Type.typeof(x1.writeUTFBytes) == ValueType.TFunction) {
	    DejaGnu.pass("Socket::writeUTFBytes() method exists");
	} else {
	    DejaGnu.fail("Socket::writeUTFBytes() method doesn't exist");
	}

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
#else
	DejaGnu.note("This class (Socket) is only available in flash9");
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

