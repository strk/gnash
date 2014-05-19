// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#include <string>
#include <cstring>
#include <boost/cstdint.hpp>

#include "element.h"
#include "dsodefs.h"

/// Action Message Format specific classes of libamf.
namespace cygnal
{

// forward declaration
class Buffer;

/// All numbers in AMF format are 8 byte doubles.
const size_t AMF0_NUMBER_SIZE = 0x08;

/// \brief The header size in bytes of an common AMF object.
///	The size of an AMF header is a type field (1 byte), followed by a
///	length field. (short)
const boost::uint8_t AMF_HEADER_SIZE = 3;

/// \brief  The header size of a property.
///	A property is a little different. It always assumes the the
///	first field is a string that's the property name, then the
///	type byte like a regular AMF object and length is used for the
///	data. So a property object header is then only 5 bytes instead
///	of the 6 that one assumes would be used.
const boost::uint8_t AMF_PROP_HEADER_SIZE = 5;

/// AMF version 0 is supported by default
const boost::uint8_t AMF_VERSION = 0;

/// For terminating sequences, a byte with value 0x09 is used.
const boost::uint8_t TERMINATOR = 0x09;

/// \brief The maximum size for a string.
/// As if there is a parsing error, we'll often see the symptom of the length
/// for the following value is bogus. Although the length field is a short, it
/// seems silly to assume we'll ever see a string 65,000 characters long. Still,
/// it makes sense to make this an adjustable thing.
const boost::uint16_t SANE_STR_SIZE = 65535;

/// Binary representation of an ActionScript object.
//
/// AMF is used to send objects, whether to a SharedObject .sol file,
/// a memory based LocalConnection segment, or over an RTMP connection
/// for streaming.
///
class DSOEXPORT AMF {
public:

    ///	Types of SharedObjects that can be serialized or deserialized.
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

    ///	Type of file being streamed.
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

    /// Create a new AMF object.
    //
    ///	As most of the methods in the AMF class a static, this
    ///	is primarily only used when encoding complex objects
    ///	where the byte count is accumulated.
    ///
    AMF();

    /// Delete the allocated AMF object
    ~AMF();

    /// @name Encoding methods
    ///
    ///		Methods for encoding data into big endian formatted
    ///		raw AMF data. Note that while we could have had a
    ///		single overloaded encode method, this is more
    ///		explicit, which when it comes to manipulating binary
    ///		protocols make the code much more readable.
    ///
    /// @{

    /// Encode a string object to its serialized representation.
    // 
    /// @param str a string value
    ///
    /// @return a binary AMF packet in big endian format
    ///
    static std::shared_ptr<Buffer> encodeString(const std::string &str);
    
    /// Encode an array of ASCII bytes to its serialized representation.
    // 
    /// @param data The data to serialize into big endian format
    /// 
    /// @param size The size of the data in bytes
    ///
    /// @return a binary AMF packet in big endian format
    ///
    static std::shared_ptr<Buffer> encodeString(boost::uint8_t *data,
						  size_t size);

    /// Encode a String object to its serialized representation.
    //
    ///	A NULL String is a string with no associated data.
    ///
    /// @return a binary AMF packet in big endian format
    ///
    static std::shared_ptr<Buffer> encodeNullString();

    /// Encode a Boolean object to its serialized representation.
    //
    /// @param flag The boolean value to serialize.
    ///
    /// @return a binary AMF packet in big endian format
    ///
    static std::shared_ptr<Buffer> encodeBoolean(bool flag);

    /// Encode an "Undefined" object to its serialized representation.
    //
    /// @return a binary AMF packet in big endian format
    ///
    static std::shared_ptr<Buffer> encodeUndefined();

    /// Encode a NULL object to its serialized representation.
    //
    ///	A NULL object is often used as a placeholder in RTMP.
    ///
    /// @return a binary AMF packet in big endian format
    ///
    static std::shared_ptr<Buffer> encodeNull();

    /// Encode a "Unsupported" object to its serialized representation.
    //
    /// @return a binary AMF packet in big endian format
    ///
    static  std::shared_ptr<Buffer> encodeUnsupported();

    /// Encode an XML object to its serialized representation.
    //
    /// @param data A pointer to the raw bytes that becomes the XML data.
    /// 
    /// @param nbytes The number of bytes to serialize.
    ///
    /// @return a binary AMF packet in big endian format
    ///
    static std::shared_ptr<Buffer> encodeXMLObject(const boost::uint8_t *data,
						     size_t nbytes);

    /// Encode a Typed Object to its serialized representation.
    //
    /// @param data A pointer to the raw bytes that becomes the data.
    /// 
    /// @param size The number of bytes to serialize.
    ///
    /// @return a binary AMF packet in big endian format
    ///
    static std::shared_ptr<Buffer> encodeTypedObject(const cygnal::Element &data);

    /// Encode a Reference to an object to its serialized representation.
    //
    /// @param data A pointer to the raw bytes that becomes the data.
    /// 
    /// @param size The number of bytes to serialize.
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static std::shared_ptr<Buffer> encodeReference(boost::uint16_t index);

    /// Encode a Movie Clip (swf data) to its serialized representation.
    //
    /// @param data A pointer to the raw bytes that becomes the data.
    /// 
    /// @param size The number of bytes to serialize.
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static std::shared_ptr<Buffer> encodeMovieClip(const boost::uint8_t *data,
						     size_t size);

    /// Encode an ECMA Array to its serialized representation.
    //
    ///	An ECMA Array, also called a Mixed Array, contains any
    ///	AMF data type as an item in the array.
    ///
    /// @param data A pointer to the raw bytes that becomes the data.
    /// 
    /// @param size The number of bytes to serialize.
    ///
    /// @return a binary AMF packet in big endian format
    ///
    static std::shared_ptr<Buffer> encodeECMAArray(const cygnal::Element &data);

    /// Encode a Long String to its serialized representation.
    //
    /// @param data A pointer to the raw bytes that becomes the data.
    /// 
    /// @param size The number of bytes to serialize.
    ///
    /// @return a binary AMF packet in big endian format
    ///
    static std::shared_ptr<Buffer> encodeLongString(const boost::uint8_t *data,
						      size_t size);

    /// Encode a Record Set to its serialized representation.
    //
    /// @param data A pointer to the raw bytes that becomes the data.
    /// 
    /// @param size The number of bytes to serialize.
    ///
    /// @return a binary AMF packet in big endian format
    ///
    static std::shared_ptr<Buffer> encodeRecordSet(const boost::uint8_t *data,
						     size_t size);

    /// Encode a Date to its serialized representation.
    //
    /// @param data A pointer to the raw bytes that becomes the data.
    /// 
    /// @return a binary AMF packet in big endian format
    ///
    static std::shared_ptr<Buffer> encodeDate(const boost::uint8_t *data);

    /// Encode a Strict Array to its serialized representation.
    //
    ///	A Strict Array is one where all the items are the same
    ///	data type, commonly either a number or a string.
    ///
    /// @param data A pointer to the raw bytes that becomes the data.
    /// 
    /// @param size The number of bytes to serialize.
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static std::shared_ptr<Buffer> encodeStrictArray(const cygnal::Element &data);
    
    /// Encode an object to its serialized representation.
    //
    /// @param el A smart pointer to an Element class.
    /// 
    /// @return a binary AMF packet in big endian format
    ///
    static std::shared_ptr<Buffer> encodeObject(const cygnal::Element &data);

    /// Encode the end of an object to its serialized representation.
    //
    /// @return a binary AMF packet in big endian format
    ///
    static std::shared_ptr<Buffer> encodeObjectEnd();

    /// Encode a 64 bit number to its serialized representation.
    //
    /// @param num A double value to serialize.
    ///
    /// @return a binary AMF packet in big endian format
    ///
    static std::shared_ptr<Buffer> encodeNumber(double num);

    /// Encode an Element to its serialized representation.
    //
    /// @param el A smart pointer to the Element to encode.
    ///
    /// @return a binary AMF packet in big endian format
    ///
    static std::shared_ptr<Buffer> encodeElement(std::shared_ptr<cygnal::Element> el);

    /// Encode an Element to its serialized representation.
    //
    /// @param el the Element to encode.
    ///
    /// @return a binary AMF packet in big endian format
    ///
    static std::shared_ptr<Buffer> encodeElement(const cygnal::Element& el);

    /// Encode a variable to its serialized representation.
    //
    /// @param el A smart pointer to the Element to encode.
    ///
    /// @return a binary AMF packet in big endian format
    ///
    std::shared_ptr<Buffer> encodeProperty(std::shared_ptr<cygnal::Element> el);

    /// @} end of encoding methods 

    /// @name Decoding methods
    ///
    ///		Methods for extracting data from big endian formatted raw AMF data.
    ///
    /// @{

    /// Extract the AMF0 object type from the header.
    //
    /// @param in The raw data to extract values from.
    ///
    /// @return The data type from the header
    ///
    static Element::amf0_type_e extractElementHeader(boost::uint8_t *in)
                         { return *(reinterpret_cast<Element::amf0_type_e *>(in)); };

    /// Extract an AMF object from an array of raw bytes.
    //
    ///	An AMF object is one of the support data types.
    ///
    /// @param in A real pointer to the raw data to start parsing from.
    ///
    /// @param tooFar A pointer to one-byte-past the last valid memory
    ///		address within the buffer.
    ///
    /// @return A smart ptr to an Element.
    ///
    /// @remarks May throw a ParserException
    ///
    std::shared_ptr<cygnal::Element> extractAMF(boost::uint8_t *in, boost::uint8_t* tooFar);

    /// Extract an AMF object from an array of raw bytes.
    //
    /// @param buf A smart pointer to a Buffer to parse the data from.
    ///
    /// @return A smart ptr to an Element.
    ///
    /// @remarks May throw a ParserException
    ///
    std::shared_ptr<cygnal::Element> extractAMF(std::shared_ptr<Buffer> buf);
    
    /// Extract a Property.
    //
    ///		A Property is a standard AMF object preceeded by a
    ///		length and an ASCII name field. These are only used
    ///		with higher level ActionScript objects.
    ///
    /// @param in A real pointer to the raw data to start parsing from.
    ///
    /// @param tooFar A pointer to one-byte-past the last valid memory
    ///		address within the buffer.
    ///
    /// @return A smart ptr to an Element.
    ///
    /// @remarks May throw a ParserException
    ///
    std::shared_ptr<cygnal::Element> extractProperty(boost::uint8_t *in, boost::uint8_t* tooFar);

    /// Extract a Property.
    //
    ///		A Property is a standard AMF object preceeded by a
    ///		length and an ASCII name field. These are only used
    ///		with higher level ActionScript objects.
    ///
    /// @param buf A smart pointer to an Buffer to parse the data from.
    ///
    /// @return A smart ptr to an Element.
    ///
    /// @remarks May throw a ParserException
    ///
    std::shared_ptr<cygnal::Element> extractProperty(std::shared_ptr<Buffer> buf);

    /// @} end of decoding methods 

    /// Get the total number of allocated bytes used when serializing.
    //
    /// @return The total allocated bytes.
    ///
    size_t totalsize() { return _totalsize; }
    
private:

    /// The total number of bytes in serialized ActionScript object.
    size_t _totalsize;

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
DSOEXPORT void *swapBytes(void *word, size_t size);


} // end of amf namespace

// end of _AMF_H_
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
