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

#include "amfutf8.h"

namespace amf 
{

# if __WORDSIZE == 64
typedef long amfnum_t;
#define AMFNUM_F "%ld"
#else
typedef long long amfnum_t;
#define AMFNUM_F "%lld"
#endif
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
// found is 0×00. If it is anything other than 0×00 (zero), your
// system should consider the AMF file/stream to be
// 'cmalformed'd. This can happen in the IDE if AMF calls are put
// on the stack but never executed and the user exits the movie from the
// IDE; the two top bytes will be random and the number of headers will
// be unreliable.

// The second byte of the AMF file/stream is appears to be 0×00 if the
// client is the Flash Player and 0×01 if the client is the FlashCom
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
    typedef struct {
        astype_e       type;
        short          length;
        std::string     name;
        const unsigned char   *data;
    } amf_element_t;

    AMF();
    AMF(int size);
    ~AMF();
    size_t size() { return _total_size; };

    // encode an element
    void *encodeElement(astype_e type, const void *in, int nbytes);
    // encode a string
    void *encodeString(char *str)  {
        return encodeElement (STRING, str, strlen(str));
    };
    void *encodeString(std::string &str) {
        return encodeElement (STRING, static_cast<const void *>(str.c_str()), str.size());
    };
    // encode a 64 bit number
    void *encodeNumber(amfnum_t num)  {
        return encodeElement (NUMBER, &num, AMF_NUMBER_SIZE);
    };

    // encode a variable. These are a name, followed by a string or number
    void *encodeVariable(const char *name);
    void *encodeVariable(amf_element_t &el);
    void *encodeVariable(const char *name, bool flag);
    void *encodeVariable(const char *name, amfnum_t num);
    void *encodeVariable(std::string &name, std::string &val);
    void *encodeVariable(const char *name, const char *val);

    void *encodeRTMPHeader(int amf_index, amf_headersize_e head_size, int total_size,
                           content_types_e type, amfsource_e routing);
//    amfhead_t *encodeHeader(amfutf8_t *name, bool required, int nbytes, void *data);
//     amfbody_t *encodeBody(amfutf8_t *target, amfutf8_t *response, int nbytes, void *data);
//    amfpacket_t *encodePacket(std::vector<amfhead_t *> messages);

    char *readElement(void *in);
    
    astype_e extractElementHeader(void *in);
    int extractElementLength(void *in);
    char *extractString(const char *in);
    amfnum_t *extractNumber(const char *in);
    amf_element_t *extractObject(const char *in);

    
    unsigned char *extractVariable(amf_element_t *el, unsigned char *in);
    
    bool parseAMF(char *in);
    static int headerSize(char header);
    int packetReadAMF(int bytes);

    int parseHeader(unsigned char *in);
    int parseBody();
    int parseBody(unsigned char *in, int bytes);
    
    int getHeaderSize()         { return _header_size; }; 
    int getTotalSize()          { return _total_size; }; 
    int getPacketSize()         { return _packet_size; };
    int getMysteryWord()        { return _mystery_word; };
    amfsource_e getRouting()    { return _src_dest; };
    int getAMFIndex()           { return _amf_index; };

    content_types_e getType()   { return _type; };
    
    unsigned char *addPacketData(unsigned char *data, int bytes);
    std::map<std::string, amf_element_t *> *getElements() { return &_elements; };
    unsigned char *appendPtr(unsigned char *data, unsigned char *var, int bytes) {
      memcpy(data, var, bytes);
      return data += bytes;
    }
        
 private:
    content_types_e     _type;
    std::map<std::string, amf_element_t *> _elements;
    int                 _amf_index;
    int                 _header_size;
    int                 _total_size;
    int                 _packet_size;
    unsigned char       *_amf_data;
    unsigned char       *_seekptr;
    int                 _mystery_word;
    amfsource_e         _src_dest;
};

 
} // end of amf namespace

// end of _AMF_H_
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
