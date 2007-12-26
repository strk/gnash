// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
#include "config.h"
#endif

#include <vector>
#include <string>
#include <cstring>
#include <map>

#include "as_object.h"

#include "amfutf8.h"
#include <boost/cstdint.hpp>

namespace amf 
{

# if __WORDSIZE == 64
typedef unsigned long amfnum_t;
#define AMFNUM_F "%ld"
#else
typedef unsigned long long amfnum_t;
#define AMFNUM_F "%lld"
#endif
// TIDO FIXME: this will be longer then the actual amfnum_t 
//             if __WORDSIZE != 64 !!
const char AMF_NUMBER_SIZE = 0x08;

// These are the data types defined by AMF
typedef char AMF_Byte_t;
typedef short AMF_Int_t ;
typedef char * AMF_MediumInt_t;
typedef int AMF_Long_t;
typedef double AMF_Double_t;

// FIXME: These are probably bogus, and need to be a UTF-8 type.
typedef char *AMF_UTF8_t;
typedef char *AMF_LongUTF8_t;

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
// 'cmalformed'd. This can happen in the IDE if AMF calls are put
// on the stack but never executed and the user exits the movie from the
// IDE; the two top bytes will be random and the number of headers will
// be unreliable.

// The second byte of the AMF file/stream is appears to be 0x00 if the
// client is the Flash Player and 0x01 if the client is the FlashCom
// server. 

// The third and fourth bytes form an integer value that specifies the
// number of headers. 
typedef struct {
    AMF_Byte_t version;
    AMF_Byte_t source;
    AMF_Int_t  count;
} amfpacket_t;

typedef enum {
    onStatus,
    onResult,
    onDebugEvents
} amfresponse_e;

class AMF {
public:
    // The following elements are defined within AMF:
    typedef enum {
        NUMBER=0x00,
        BOOLEAN=0x01,
        STRING=0x02,
        OBJECT=0x03,
        MOVIECLIP=0x04,
        NULL_VALUE=0x05,
        UNDEFINED=0x06,
        REFERENCE=0x07,
        ECMA_ARRAY=0x08,
        OBJECT_END=0x09,
        STRICT_ARRAY=0x0a,
        DATE=0x0b,
        LONG_STRING=0x0c,
        UNSUPPORTED=0x0d,
        RECORD_SET=0x0e,
        XML_OBJECT=0x0f,
        TYPED_OBJECT=0x10
    } astype_e;
    typedef enum {
        HEADER_12 = 0x0,
        HEADER_8  = 0x40,
        HEADER_4  = 0x80,
        HEADER_1  = 0xc0
    } amf_headersize_e;    
    
    typedef enum {
        Byte,
        Int,
        MediumInt,
        Long,
        Double,
        UTF8,
        LongUTF8
    } amftype_e;
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

    struct amf_element_t {
        astype_e       type;
        boost::int16_t        length;
        std::string    name;
        boost::uint8_t        *data;

        amf_element_t()
                :
                type(NUMBER),
                length(0),
                name(),
                data(NULL)
        {}

    };

    AMF();
    AMF(int size);
    ~AMF();
    size_t size() { return _total_size; };

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
    boost::uint8_t* encodeElement(astype_e type, const void *in, int nbytes);

    /// Encode a string
    ///
    /// @return an amf packet (header,data)
    ///
    boost::uint8_t* encodeString(const char *str)  {
        return encodeElement (STRING, str, strlen(str));
    };

    /// Encode a 64 bit number
    ///
    /// @return an amf packet (header,data)
    ///
    boost::uint8_t* encodeNumber(amfnum_t num)  {
        return encodeElement (NUMBER, &num, AMF_NUMBER_SIZE);
    };

    /// Encode a variable. These are a name, followed by a string or number

    ///
    /// @ return an element all filled in correctly for passing to other
    /// methods.
    amf_element_t *createElement(amf_element_t *el, astype_e type,
				 const std::string &name, boost::uint8_t *data, int nbytes);
    amf_element_t *createElement(amf_element_t *el, const std::string &name,
				 amfnum_t data);
    amf_element_t *createElement(amf_element_t *el, const char *name,
				 double data);
    amf_element_t *createElement(amf_element_t *el, const std::string &name,
				 double data);
    amf_element_t *createElement(amf_element_t *el, const char *name,
				 amfnum_t data);
    amf_element_t *createElement(amf_element_t *el, const std::string &name,
				 std::string &data);
    amf_element_t *createElement(amf_element_t *el, const char *name,
				 const char *data);
    amf_element_t *createElement(amf_element_t *el, const std::string &name,
				 bool data);
    amf_element_t *createElement(amf_element_t *el, const char *name,
				 bool data);
    amf_element_t *createElement(amf_element_t *el, const std::string &name,
				  boost::intrusive_ptr<gnash::as_object> &data);
    amf_element_t *createElement(amf_element_t *el, const char *name,
				 boost::intrusive_ptr<gnash::as_object> &data);
//     amf_element_t *createElement(amf_element_t *el, const std::string &name,
// 				 const gnash::as_value &data);
//     amf_element_t *createElement(amf_element_t *el, const char *name,
// 				 const gnash::as_value &data);
    //
    /// @return a newly allocated byte array,
    /// to be deleted by caller using delete [] operator, or NULL
    ///
    boost::uint8_t* encodeVariable(const char *name);

    /// Encode a variable. 
    //
    /// @return a newly allocated byte array,
    /// to be deleted by caller using delete [] operator, or NULL
    ///
    boost::uint8_t* encodeVariable(amf_element_t &el);
    boost::uint8_t* encodeVariable(amf_element_t *el);

    /// Encode a boolean variable. This is a name followed by a boolean value.
    //
    /// @return a newly allocated byte array,
    /// to be deleted by caller using delete [] operator, or NULL
    ///
    boost::uint8_t* encodeVariable(const char *name, bool flag);

    /// Encode a variable. 
    //
    /// @return a newly allocated byte array,
    /// to be deleted by caller using delete [] operator, or NULL
    ///
    boost::uint8_t* encodeVariable(const char *name, amfnum_t num);

    /// Encode a variable. 
    //
    /// @return a newly allocated byte array,
    /// to be deleted by caller using delete [] operator, or NULL
    ///
    boost::uint8_t* encodeVariable(std::string &name, std::string &val);

    /// Encode a variable. 
    //
    /// @return a newly allocated byte array,
    /// to be deleted by caller using delete [] operator, or NULL
    ///
    boost::uint8_t* encodeVariable(const char *name, const char *val);

    void *encodeRTMPHeader(int amf_index, amf_headersize_e head_size, int total_size,
                           content_types_e type, amfsource_e routing);
//    amfhead_t *encodeHeader(amfutf8_t *name, bool required, int nbytes, void *data);
//     amfbody_t *encodeBody(amfutf8_t *target, amfutf8_t *response, int nbytes, void *data);
//    amfpacket_t *encodePacket(std::vector<amfhead_t *> messages);

    boost::uint8_t *readElement(void *in);
    
    /// Extract the string from a string-type AMF packet
    //
    /// Return a newly allocated char[],
    /// or NULL if the given AMF packet is not a string type
    ///
    /// Caller is responsible for deletion using delete [] operator.
    ///
    boost::int8_t *extractString(const boost::uint8_t* in);

    amfnum_t *extractNumber(const boost::uint8_t *in);
    amf_element_t *extractObject(const boost::uint8_t *in);
    
    unsigned char *extractVariable(amf_element_t *el, boost::uint8_t *in);
    
    bool parseAMF(boost::uint8_t *in);
    static int headerSize(int8_t header);
    int packetReadAMF(int bytes);

    int parseHeader(boost::uint8_t *in);
    int parseBody();
    int parseBody(boost::uint8_t *in, int bytes);
    
    int getHeaderSize()         { return _header_size; }; 
    int getTotalSize()          { return _total_size; }; 
    int getPacketSize()         { return _packet_size; };
    int getMysteryWord()        { return _mystery_word; };
    amfsource_e getRouting()    { return _src_dest; };
    int getAMFIndex()           { return _amf_index; };

    content_types_e getType()   { return _type; };
    
    boost::uint8_t *addPacketData(boost::uint8_t *data, int bytes);
    std::map<std::string, amf_element_t *> *getElements() { return &_elements; };
    boost::uint8_t *appendPtr(boost::uint8_t *data, boost::uint8_t *var, int bytes) {
      memcpy(data, var, bytes);
      return data += bytes;
    }
        
 private:
    astype_e extractElementHeader(void *in);
    int extractElementLength(void *in);
    content_types_e     _type;
    std::map<std::string, amf_element_t *> _elements;
    int                 _amf_index;
    int                 _header_size;
    int                 _total_size;
    int                 _packet_size;
    boost::uint8_t             *_amf_data;
    boost::uint8_t             *_seekptr;
    int                 _mystery_word;
    amfsource_e         _src_dest;
};

 
void *swapBytes(void *word, int size);


} // end of amf namespace

// end of _AMF_H_
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
