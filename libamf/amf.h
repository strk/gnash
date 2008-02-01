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

#ifndef _AMF_H_
#define _AMF_H_

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <vector>
#include <string>
#include <cstring>
#include <map>

#include "element.h"
#include "amfutf8.h"
#include <boost/cstdint.hpp>

namespace amf 
{

const char AMF_NUMBER_SIZE = 0x08;

typedef enum {
    CLIENT,                     // Flash player
    SERVER                      // Flash com server
} amfsource_e;

const char AMF_VERSION = 0;
const int  AMF_HEADSIZE_MASK = 0xc0;
const char AMF_HEADER_SIZE = 0x03;
const char AMF_INDEX_MASK = 0x3f;
const int  AMF_VIDEO_PACKET_SIZE = 128;
const int  AMF_AUDIO_PACKET_SIZE = 64;
// This is the sized used when reading from the network to
// be the most efficient
const int  AMF_PACKET_SIZE = 7096;

// For terminating sequences, a byte with value 0x09 is used.
const char TERMINATOR = 0x09;

// Each packet consists of the following:
//
// The first byte of the AMF file/stream is believed to be a version
// indicator. So far the only valid value for this field that has been
// found is 0x00. If it is anything other than 0x00 (zero), your
// system should consider the AMF file/stream to be
// 'malformed'd. This can happen in the IDE if AMF calls are put
// on the stack but never executed and the user exits the movie from the
// IDE; the two top bytes will be random and the number of headers will
// be unreliable.

// The second byte of the AMF file/stream is appears to be 0x00 if the
// client is the Flash Player and 0x01 if the client is the FlashCom
// server. 

// The third and fourth bytes form an integer value that specifies the
// number of headers. 
typedef struct {
    boost::uint8_t version;
    boost::uint8_t source;
    boost::uint32_t  count;
} amfpacket_t;

typedef enum {
    onStatus,
    onResult,
    onDebugEvents
} amfresponse_e;

class AMF {
public:
    typedef enum {
        HEADER_12 = 0x0,
        HEADER_8  = 0x40,
        HEADER_4  = 0x80,
        HEADER_1  = 0xc0
    } amf_headersize_e;    
    
//     typedef enum {
//         Byte,
//         Int,
//         MediumInt,
//         Long,
//         Double,
//         UTF8,
//         LongUTF8
//     } amftype_e;
    typedef enum {
        NONE = 0x0,
        CHUNK_SIZE = 0x1,
        UNKNOWN = 0x2,
        BYTES_READ = 0x3,
        PING = 0x4,
        SERVER = 0x5,
        CLIENT = 0x6,
        UNKNOWN2 = 0x7,
        AUDIO_DATA = 0x8,
        VIDEO_DATA = 0x9,
        UNKNOWN3 = 0xa,
        NOTIFY = 0x12,
        SHARED_OBJ = 0x13,
        INVOKE = 0x14
    } content_types_e;
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

    AMF();
    AMF(int size);
    ~AMF();
//    size_t size() { return _total_size; };

    //
    // Methods for encoding data into big endian formatted raw AMF data.
    //
    
    /// Encode a string object
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::uint8_t *encodeElement(const char *str);

    /// Encode a Boolean object
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::uint8_t *encodeBoolean(bool flag);

    /// Encode an "Undefined" object
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::uint8_t *encodeUndefined();

    /// Encode a "NULL" object
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::uint8_t *encodeNull();

    /// Encode a "Unsupported" object
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static  boost::uint8_t *encodeUnsupported();

    /// Encode an XML object
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::uint8_t *encodeXMLObject(boost::uint8_t *data, int size);

    /// Encode a Typed Object
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::uint8_t *encodeTypedObject(boost::uint8_t *data, int size);

    /// Encode a Reference to an object
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::uint8_t *encodeReference(boost::uint8_t *data, int size);

    /// Encode a Movie Clip
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::uint8_t *encodeMovieClip(boost::uint8_t *data, int size);

    /// Encode an ECMA Array
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::uint8_t *encodeECMAArray(boost::uint8_t *data, int size);

    /// Encode a long string
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::uint8_t *encodeLongString(boost::uint8_t *data, int size);

    /// Encode a Record Set
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::uint8_t *encodeRecordSet(boost::uint8_t *data, int size);

    /// Encode a Date
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::uint8_t *encodeDate(boost::uint8_t *data);

    /// Encode a Strict Array
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::uint8_t *encodeStrictArray(boost::uint8_t *data, int size);
    
    /// Encode an object
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::uint8_t *encodeObject(const boost::uint8_t *data, int size);

    /// Encode a 64 bit number
    ///
    /// @return a binary AMF packet in big endian format (header,data)
    ///
    static boost::uint8_t *encodeNumber(double num);
    
    /// Encode a element. 
    ///
    /// @return a binary AMF packet in big endian format (header,data)

    /// @return a newly allocated byte array.
    /// to be deleted by caller using delete [] operator, or NULL
    ///
    static boost::uint8_t *encodeElement(amf::Element *el);

    /// Encode an array of elements. 
    ///
    /// @return a binary AMF packet in big endian format (header,data)

    /// @return a newly allocated byte array.
    /// to be deleted by caller using delete [] operator, or NULL
    ///
    static std::vector<boost::uint8_t> *encodeElement(std::vector<amf::Element *> &els);

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
    boost::uint8_t *encodeVariable(amf::Element *el, size_t& outsize);

#if 0
    /// Encode an element
    //
    /// @param type
    ///		Type of element
    ///
    /// @param in
    ///		Input stream
    ///
    /// @param nbytes
    ///		Lenght of data packet (not including header).
    ///
    /// @return an amf packet (header,data)
    ///
    static boost::uint8_t* encodeElement(Element::astype_e type, const void *in, int nbytes);

    /// @return a newly allocated byte array,
    /// to be deleted by caller using delete [] operator, or NULL
    ///
    static boost::uint8_t* encodeVariable(const char *name);

    /// Encode a boolean variable. This is a name followed by a boolean value.
    //
    /// @return a newly allocated byte array,
    /// to be deleted by caller using delete [] operator, or NULL
    ///
    static boost::uint8_t* encodeVariable(const char *name, bool flag);

    /// Encode a variable. 
    //
    /// @return a newly allocated byte array,
    /// to be deleted by caller using delete [] operator, or NULL
    ///
    static boost::uint8_t* encodeVariable(const char *name, double num);

    /// Encode a variable. 
    //
    /// @return a newly allocated byte array,
    /// to be deleted by caller using delete [] operator, or NULL
    ///
    static boost::uint8_t* encodeVariable(std::string &name, std::string &val);

    /// Encode a variable. 
    //
    /// @return a newly allocated byte array,
    /// to be deleted by caller using delete [] operator, or NULL
    ///
    static boost::uint8_t* encodeVariable(const char *name, const char *val);

    void *encodeRTMPHeader(int amf_index, amf_headersize_e head_size, int total_size,
                           content_types_e type, amfsource_e routing);
//    amfhead_t *encodeHeader(amfutf8_t *name, bool required, int nbytes, void *data);
//     amfbody_t *encodeBody(amfutf8_t *target, amfutf8_t *response, int nbytes, void *data);
//    amfpacket_t *encodePacket(std::vector<amfhead_t *> messages);

//    std::vector<amf_element_t> *readElements(boost::uint8_t *in);
    
    /// Extract the string from a string-type AMF packet
    //
    /// Return a newly allocated char[],
    /// or NULL if the given AMF packet is not a string type
    ///
    /// Caller is responsible for deletion using delete [] operator.
    ///
#endif
    
    //
    // Methods for extracting data from big endian formatted raw AMF data.
    //
    amf::Element::astype_e extractElementHeader(boost::uint8_t *in)
	{ return *(reinterpret_cast<amf::Element::astype_e *>(in)); };
    
    boost::uint8_t *extractElement(amf::Element *el, boost::uint8_t *in);
    boost::uint8_t *extractVariable(amf::Element *el, boost::uint8_t *in);
    
#if 0
    // FIXME: these should return an Element, and then use to_*() to convert.
    char *extractString(const boost::uint8_t* in);

    double extractNumber(const boost::uint8_t *in);
    Element &extractObject(const boost::uint8_t *in);
    

    // FIXME: these are all for RTMP, and should be moved
    bool parseAMF(boost::uint8_t *in);
    static int headerSize(int8_t header);
    int packetReadAMF(int bytes);

    int parseHeader(boost::uint8_t *in);
    int parseBody();
    int getHeaderSize()         { return _header_size; }; 
    int getTotalSize()          { return _total_size; }; 
    int getPacketSize()         { return _packet_size; };
    int getMysteryWord()        { return _mystery_word; };
    amfsource_e getRouting()    { return _src_dest; };
    int getAMFIndex()           { return _amf_index; };
    boost::uint8_t *addPacketData(boost::uint8_t *data, int bytes);
#endif    

    content_types_e getType()   { return _type; };
//    std::map<std::string, Element> *getElements() { return &_elements; };
    boost::uint8_t *appendPtr(boost::uint8_t *data, boost::uint8_t *var,
			      int bytes) {
	memcpy(data, var, bytes);
	return data += bytes;
    }
        
 private:
#if 0
    Element::astype_e extractElementHeader(void *in);
    int extractElementLength(void *in);
//    std::map<std::string, Element &> _elements;
    int                 _amf_index;
    int                 _header_size;
    int                 _total_size;
    int                 _packet_size;
    boost::uint8_t      *_amf_data;
    boost::uint8_t      *_seekptr;
    amfsource_e         _src_dest;
#endif
    content_types_e     _type;
    int                 _mystery_word;
};
 
void *swapBytes(void *word, int size);


} // end of amf namespace

// end of _AMF_H_
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
