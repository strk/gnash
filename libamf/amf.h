// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//

#ifndef _AMF_H_
#define _AMF_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <vector>
#include <string>
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
const char AMF_HEADSIZE_MASK = 0xc0;
const char AMF_INDEX_MASK = 0x03;
const int  AMF_VIDEO_PACKET_SIZE = 128;
const int  AMF_AUDIO_PACKET_SIZE = 64;

// For terminating sequences, a byte with value 0x09 is used.
const char TERMINATOR = 0x09;

// Each header consists of the following:
//
// * UTF string (including length bytes) - name
// * Boolean - specifies if understanding the header is `required'
// * Long - Length in bytes of header
// * Variable - Actual data (including a type code)
typedef struct {
    amfutf8_t name;
    AMF_Byte_t required;
    AMF_Long_t length;
    void *data;
} amfhead_t;

// Each body consists of the following:
//
// * UTF String - Target
// * UTF String - Response
// * Long - Body length in bytes
// * Variable - Actual data (including a type code)
typedef struct {
    amfutf8_t target;
    amfutf8_t response;
    AMF_Long_t length;
    void *data;
} amfbody_t;

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
    // The folowing elements are defined within AMF:
    typedef enum {
        Number=0x00,
        Boolean=0x01,
        String=0x02,
        Object=0x03,
        MovieClip=0x04,
        Null=0x05,
        Undefined=0x06,
        Reference=0x07,
        ECMAArray=0x08,
        ObjectEnd=0x09,
        StrictArray=0x0a,
        Date=0x0b,
        LongString=0x0c,
        Unsupported=0x0d,
        Recordset=0x0e,
        XMLObject=0x0f,
        TypedObject=0x10
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
//    UNKNOWN = 0x2,
        BYTES_READ = 0x3,
        PING = 0x4,
        SERVER = 0x5,
        CLIENT = 0x6,
//    UNKNOWN2 = 0x7,
        AUDIO_DATA = 0x8,
        VIDEO_DATA = 0x9,
//    UNKNOWN3 = 0xa,
        NOTIFY = 0x12,
        SHARED_OBJ = 0x13,
        INVOKE = 0x14
    } content_types_e;
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
    // Swap the bytes for Little Endian machines
    void *swapBytes(void *word, int size);
    
    void *encodeElement(astype_e type, void *in, int nbytes);
    amfhead_t *encodeHeader(amfutf8_t *name, bool required, int nbytes, void *data);
    amfbody_t *encodeBody(amfutf8_t *target, amfutf8_t *response, int nbytes, void *data);
    amfpacket_t *encodePacket(std::vector<amfhead_t *> messages);
    char *readElement(void *in);
    bool readObject(void *in);
    astype_e extractElementHeader(void *in);
    int extractElementLength(void *in);
    std::string extractString(const char *in); // FIXME: 
    int extractNumber(const char *in); // FIXME: 

    unsigned char *extractVariables(amf_element_t &el, unsigned char *in);
    
    bool parseAMF(char *in);
    static int headerSize(char header);
    int packetReadAMF(int bytes);

    int parseHeader(unsigned char *in);
    int parseBody();
    int parseBody(unsigned char *in, int bytes);
    
    int getHeaderSize() {  return _header_size; }; 
    int getTotalSize() {  return _total_size; }; 
    unsigned char *addPacketData(unsigned char *data, int bytes);
    //    std::map<amf_element_t *, std::vector<unsigned char *> > *getElements() { return &_elements; };
 private:
    content_types_e     _type;
//    std::map<std::string, amf_element_t &> _elements;
    int                 _amf_index;
    int                 _header_size;
    int                 _total_size;
    int                 _packet_size;
    unsigned char       *_amf_data;
    unsigned char       *_seekptr;
    int                 _mystery_word;
    int                 _src_dest;
};

 
} // end of amf namespace

// end of _AMF_H_
#endif
