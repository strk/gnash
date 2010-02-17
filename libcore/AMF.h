// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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


#ifndef GNASH_AMF_H
#define GNASH_AMF_H

#include <map>
#include <string>
#include <vector>

#include "as_value.h"
#include "dsodefs.h"

namespace gnash {
    class as_object;
    class SimpleBuffer;
    class Global_as;
}

namespace gnash {

/// Simple functions and classes for handling AMF.
//
/// AMF is a simple serialization format for ActionScript objects and values,
/// allowing them to be stored and transmitted. These classes convert between
/// AMF buffers and the objects they contain.
namespace AMF {

enum Type {
    NOTYPE            = -1,
    NUMBER_AMF0       = 0x00,
    BOOLEAN_AMF0      = 0x01,
    STRING_AMF0       = 0x02,
    OBJECT_AMF0       = 0x03,
    MOVIECLIP_AMF0    = 0x04,
    NULL_AMF0         = 0x05,
    UNDEFINED_AMF0    = 0x06,
    REFERENCE_AMF0    = 0x07,
    ECMA_ARRAY_AMF0   = 0x08,
    OBJECT_END_AMF0   = 0x09,
    STRICT_ARRAY_AMF0 = 0x0a,
    DATE_AMF0         = 0x0b,
    LONG_STRING_AMF0  = 0x0c,
    UNSUPPORTED_AMF0  = 0x0d,
    RECORD_SET_AMF0   = 0x0e,
    XML_OBJECT_AMF0   = 0x0f,
    TYPED_OBJECT_AMF0 = 0x10,
};

/// A class to compose AMF buffers.
//
/// A single AMF::Writer class can take successive values and encode them
/// in a single buffer. The class takes care of object references.
//
/// This class merely encodes basic types such as strings, numbers, and
/// ActionScript Objects. It does not handle as_values. However, it
/// is designed for use with as_value::writeAMF0(), which uses an
/// instance of this class to serialize itself.
class Writer
{
public:

    typedef std::map<as_object*, size_t> OffsetTable;

    Writer(SimpleBuffer& buf, bool strictArray = false)
        :
        _buf(buf),
        _strictArray(strictArray)
    {}

    /// Write any simple Object type: not DisplayObjects.
    //
    /// Handles functions, dates, XML, and arrays. The object must not be null.
    bool writeObject(as_object* obj);

    /// Write a string.
    //
    /// Handles long and short strings.
    bool writeString(const std::string& str);

    /// Write a null value.
    bool writeNull();

    /// Write an undefined value.
    bool writeUndefined();

    /// Write a double.
    bool writeNumber(double d);

    /// Write a boolean.
    bool writeBoolean(bool b);

    /// Encode the name of an object's property.
    //
    /// You should encode the value of the property immediately afterwards.
    bool writePropertyName(const std::string& name);

    /// Write custom data for special cases.
    void writeData(const boost::uint8_t* data, size_t length);

private:

    OffsetTable _offsets;
    SimpleBuffer& _buf;
    bool _strictArray;

};

/// Deserialize an AMF buffer to as_values.
//
/// This class relies on the public interface of as_value because we don't
/// necessarily know in advance what basic type will be read from the
/// buffer.
//
/// Note that callers may change the current buffer position. They must
/// check that the read position is not past the end when a Reader object
/// is called. This is very important!
//
/// For reading of basic types, there is no need to use VM resources. Object
/// types required the construction of objects, which in turn needs a
/// reference to a Global_as. For this reason, object reading functions
/// are member functions, and the Reader requires a Global_as& reference
/// in case it encounters object data.
class Reader
{
public:
    Reader(const boost::uint8_t*& pos, const boost::uint8_t* end, Global_as& gl)
        :
        _pos(pos),
        _end(end),
        _global(gl)
    {}

    /// Create a type from current position in the AMF buffer.
    //
    /// @param val      An as_value to be created from the AMF data.
    /// @param type     The type of the data to read.
    /// @return         false if this read failed for any reason.
    ///                 The constructed as_value is then invalid. True if
    ///                 the read succeeded and the as_value is valid.
    bool operator()(as_value& val, Type t = NOTYPE);

private:

    /// Read an XML type.
    as_value readXML();

    /// Read a Date object type.
    as_value readDate();
    
    /// Read a simple object type.
    as_value readObject();
    
    /// Read an object reference type.
    as_value readReference();
    
    /// Read an array object type.
    as_value readArray();
    
    /// Read a strict array object type.
    as_value readStrictArray();

    /// Object references.
    std::vector<as_object*> _objectRefs;

    /// The current position in the buffer.
    const boost::uint8_t*& _pos;

    /// The end of the buffer.
    const boost::uint8_t* const _end;

    /// For creating objects if necessary.
    Global_as& _global;

};

/// Swap bytes in raw data.
//
///	This only swaps bytes if the host byte order is little endian.
///
/// @param word The address of the data to byte swap.
///
/// @param size The number of bytes in the data.
///
/// @return A pointer to the raw data.
///
DSOEXPORT void* swapBytes(void *word, size_t size);


} // namespace amf
} // namespace gnash

#endif
