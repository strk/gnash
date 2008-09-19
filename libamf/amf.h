// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

// This file is for the low level support for encoding and decoding AMF objects.
// As this class has no data associated with it, all the methods are static as
// they are for convenience only.
// All the encoding methods return a Buffer class, which is simply an array on
// of unsigned bytes, and a byte count.
// The only extraction classes parse either a raw AMF object or the larger
// "variable"

#ifndef _AMF_H_
#define _AMF_H_

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <vector>
#include <string>
#include <cstring>
#include <map>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>

#include "network.h"
#include "element.h"
#include "dsodefs.h"

namespace amf 
{

// forward declaration
class Buffer;

// All numbers in AMF format are 8 byte doubles.
const size_t AMF0_NUMBER_SIZE = 0x08;

// The header of an AMF object is a type field (1 byte), followed by a
// length field. (short)
const gnash::Network::byte_t AMF_HEADER_SIZE = 3;

// A variable is a little different. It always assumes the the first field is
// a string that's the variable name, then the type byte like a regular AMF
// object and length is used for the data. So a variable object header is
// then only 5 bytes instead of the 6 that one assumes would be used.
const gnash::Network::byte_t AMF_VAR_HEADER_SIZE = 5;

// Use a zero version till now till we know what this should be.
const gnash::Network::byte_t AMF_VERSION = 0;

// For terminating sequences, a byte with value 0x09 is used.
const gnash::Network::byte_t TERMINATOR = 0x09;

// As if there is a parsing error, we'll often see the symptom of the length
// for the following value is bogus. Although the length field is a short, it
// seems silly to assume we'll ever see a string 65,000 characters long. Still,
// it makes sense to make this an adjustable thing.
const int SANE_STR_SIZE = 1024;

// An AMF object is the binary representation of an ActionScript object. AMF
// is used to send objects, wheather to a SharedObject .sol file, a memory based
// LocalConnection segment, or over an RTMP connection for streaming.
class DSOEXPORT AMF {
public:
    typedef enum {
        CONNECT = 0x01,
        DISCONNECT = 0x02,
        SET_ATTRIBUTE = 0x03,
        UPDATE_DATA = 0x04,
        UPDATE_ATTRIBUTE = 0x05,
        SEND_MESSAGE = 0x06,
        STATUS = 0x07,
        CLEAR_DATA = 0x08,
        DELETE_DATA = 0x09,
        DELETE_ATTRIBYTE = 0x0a,
        INITIAL_DATA = 0x0b
    } shared_obj_types_e;
    typedef enum {
	FILETYPE_ERROR = -1,
	FILETYPE_NONE = 0,
	FILETYPE_HTML,
	FILETYPE_SWF,
	FILETYPE_VIDEO,
	FILETYPE_AUDIO,
	FILETYPE_MP3,
	FILETYPE_FCS,
	FILETYPE_OSCP
    } filetype_e;
    AMF();
    AMF(size_t size);
    ~AMF();

    //
    // Methods for encoding data into big endian formatted raw AMF data.
    // Note that while we could have had a single overloaded encode method,
    // this is more explicit, which when it comes to manipulating binary
    // protocols make the code much more readable.
    
    /// Encode a string object
    ///

    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::shared_ptr<Buffer> encodeString(const std::string &str);
    static boost::shared_ptr<Buffer> encodeString(gnash::Network::byte_t *data, size_t size);

    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::shared_ptr<Buffer> encodeNullString();

    /// Encode a Boolean object
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::shared_ptr<Buffer> encodeBoolean(bool flag);

    /// Encode an "Undefined" object
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::shared_ptr<Buffer> encodeUndefined();

    /// Encode a "NULL" object
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::shared_ptr<Buffer> encodeNull();

    /// Encode a "Unsupported" object
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static  boost::shared_ptr<Buffer> encodeUnsupported();

    /// Encode an XML object
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::shared_ptr<Buffer> encodeXMLObject(gnash::Network::byte_t *data, size_t size);

    /// Encode a Typed Object
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::shared_ptr<Buffer> encodeTypedObject(gnash::Network::byte_t *data, size_t size);

    /// Encode a Reference to an object
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::shared_ptr<Buffer> encodeReference(gnash::Network::byte_t *data, size_t size);

    /// Encode a Movie Clip
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::shared_ptr<Buffer> encodeMovieClip(gnash::Network::byte_t *data, size_t size);

    /// Encode an ECMA Array
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::shared_ptr<Buffer> encodeECMAArray(gnash::Network::byte_t *data, size_t size);

    /// Encode a long string
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::shared_ptr<Buffer> encodeLongString(gnash::Network::byte_t *data, size_t size);

    /// Encode a Record Set
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::shared_ptr<Buffer> encodeRecordSet(gnash::Network::byte_t *data, size_t size);

    /// Encode a Date
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::shared_ptr<Buffer> encodeDate(gnash::Network::byte_t *data);

    /// Encode a Strict Array
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::shared_ptr<Buffer> encodeStrictArray(gnash::Network::byte_t *data, size_t size);
    
    /// Encode an object
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::shared_ptr<Buffer> encodeObject(Element *el);

    /// Encode the end of an object
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::shared_ptr<Buffer> encodeObjectEnd();

    /// Encode a 64 bit number
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::shared_ptr<Buffer> encodeNumber(double num);

    /// Encode a element. 
    ///
    /// @return a binary AMF packet in big endian format (header,data)

    /// @return a newly allocated byte array.
    /// to be deleted by caller using delete [] operator, or NULL
    ///
    static boost::shared_ptr<Buffer> encodeElement(amf::Element *el);

    /// Encode a variable. 
    //
    /// @param el The element to encode, ownership retained by caller
    ///
    /// @param size Output parameter: size of the encoded byte array.
    /// 
    /// @return a binary AMF packet in big endian format (header,data)
    ///         in form of a newly allocated byte array.
    ///         to be deleted by caller using delete [] operator, or NULL
    ///
    boost::shared_ptr<Buffer> encodeProperty(amf::Element *el);
    static boost::shared_ptr<Buffer> encodeVariableHeader(const std::string &name);
    
    //
    // Methods for extracting data from big endian formatted raw AMF data.
    //
    
    // Extract the object type from the first byte of the header.
    static amf::Element::amf0_type_e extractElementHeader(gnash::Network::byte_t *in)
                         { return *(reinterpret_cast<amf::Element::amf0_type_e *>(in)); };

    // Unlike when we are encoding, for extracting objects we need
    // to keep track where we are in the memory buffer so these can't
    // be static.
    
    /// Extract an AMF object. These have no name like the variables do.
    //
    /// @param in
    ///    Pointer to start parsing from
    //
    /// @param tooFar
    ///    A pointer to one-byte-past the last valid memory
    ///    address within the buffer.
    ///
    /// May throw a ParserException 
    ///
    amf::Element *extractAMF(gnash::Network::byte_t *in, gnash::Network::byte_t* tooFar);

    /// Extract an AMF object. These have no name like the variables do.
    amf::Element *extractAMF(boost::shared_ptr<Buffer>buf);
    
    /// \brief
    /// Extract an AMF "variable", which is a standard AMF object preceeded by
    /// just a length and a name field.
    ///
    /// @param in
    ///    Pointer to start parsing property from
    //
    /// @param tooFar
    ///    A pointer to one-byte-past the last valid memory
    ///    address within the buffer.
    ///
    /// May throw a ParserException 
    ///
    amf::Element *extractProperty(gnash::Network::byte_t *in, gnash::Network::byte_t* tooFar);

    /// \brief
    /// Extract an AMF "variable", which is a standard AMF object preceeded by
    /// just a length and a name field.
    amf::Element *extractProperty(boost::shared_ptr<Buffer> buf);

    size_t totalsize() { return _totalsize; }
    
private:
    size_t _totalsize;
};
 
DSOEXPORT void *swapBytes(void *word, size_t size);


} // end of amf namespace

// end of _AMF_H_
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
