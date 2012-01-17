// 
//   Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012
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

#ifndef GNASH_LIBNET_RTMP_H
#define GNASH_LIBNET_RTMP_H

#include <deque>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <vector>
#include <time.h>

#include "amf.h"
#include "element.h"
#include "network.h"
#include "buffer.h"
#include "rtmp_msg.h"
#include "cque.h"
#include "dsodefs.h"
#include "utility.h"

namespace gnash
{

/// \page RTMP RTMP Protocol
/// \section rtmp_handshake RTMP Handshake
///     The headers and data for the initial RTMP handshake differ from
///     what is used by RTMP during normal message processing. The layout
///     is this:
/// <pre>
///     [version (1 byte)]
///     [1st timestamp (4 bytes)]
///     [2nd timestamp (4 bytes)]
///     [1528 bytes of random data]
/// </pre>
///
///     The handshake process is client sends a handhsake request to the
///     server. This is 1537 bytes (version + handshake).
///     The server then responds with a 3073 byte packet, which is the
///     version byte plus two 1536 byte packets, the second one being the
///     the same random data as we originally sent, but with the header
///     changed.
///
/// \section rtmp_packet RTMP Packet
///     An RTMP packet has a variablew length header field, followed by data
///     encoded in AMF format. The header can be one of 1, 4, 8, or 12 bytes.
///     


/// \var RTMP_HANDSHAKE_VERSION_SIZE
///     The RTMP version field of the handshake is 1 byte large
const int  RTMP_HANDSHAKE_VERSION_SIZE = 1;
/// \var RTMP_VERSION
///     The RTMP version number for now is always a 3.
const boost::uint8_t RTMP_VERSION = 0x3;
/// \var
///    This is the total size of an RTMP packet, not including the
///    version field.
const int  RTMP_HANDSHAKE_SIZE	= 1536;
/// \var
///    This is the size of the random data section in the RTMP handshake
const int  RTMP_RANDOM_SIZE	= 1528;
/// \var
///    The RTMP handshake header if always 2 32bit words. (8 bytes)
const int  RTMP_HANDSHAKE_HEADER_SIZE = 8;

/// \var
///    This is the maximum number of channels supported in a single
///    RTMP connection.
const int  MAX_AMF_INDEXES	= 64;

/// \par RTMP Header
///
/// \var 
///     This is a mask to get to the upper 2 bits of the header
const int  RTMP_HEADSIZE_MASK	= 0xc0;
/// \var 
///     This is a mask to get to the lower 6 bits of the header
const char RTMP_INDEX_MASK	= 0x3f;
/// \var 
///     All video data packets are 128 bytes
const int  RTMP_VIDEO_PACKET_SIZE = 128;
/// \var 
///     All audio data packets are 64 bytes
const int  RTMP_AUDIO_PACKET_SIZE = 64;
/// \var 
///     While the RTMP header can be one of 4 sizes, this is the
///     maximum size used
const int  RTMP_MAX_HEADER_SIZE = 12;
/// \var 
///     All System Ping messages are 6 bytes
const int  PING_MSG_SIZE	= 6;
/// \var 
///     This is a reserved channel for system messages
const int  RTMP_SYSTEM_CHANNEL = 2;

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

class DSOEXPORT RTMP : public Network
{
public:
    typedef std::map<const char*, cygnal::Element> AMFProperties;
    typedef std::deque<CQue *> queues_t;
    typedef enum {
	ENCODE_AMF0=0x0,
	ENCODE_AMF3=0x3
    } encoding_types_e;
    typedef enum {
	RAW     = 0x0001,
	ADPCM   = 0x0002,
	MP3     = 0x0004,
	INTEL   = 0x0005,
	CELT    = 0x0008,		// unique to Gnash
	NELLY8  = 0x0020,
	NELLY   = 0x0040,
	G711A   = 0x0080,
	G711U   = 0x0100,
	NELLY16 = 0x0200,
	AAC     = 0x0400,
	SPEEX   = 0x0800,
	DEFAULT_AUDIO_SET = 0x0267,
	ALLAUDIO = 0x0fff
    } audiocodecs_e;
    typedef enum {
	UNUSED   = 0x0001,
	JPEG     = 0x0002,
	SORENSON = 0x4,
	ADOBE    = 0x0008,
	VP6      = 0x0010,
	VP6ALPHA = 0x0020,
	SCREEN2  = 0x0040,
	H264     = 0x0080,
	DEFAULT_VIDEO_SET = 0x007c,
	ALLVIDEO = 0x00ff
    } videocodecs_e;
    typedef enum {
	SEEK = 0x1,
	AMF0 = 0x0,
	AMF3 = 0x3
    } videofunction_e;
    // The second byte of the AMF file/stream is appears to be 0x00 if the
    // client is the Flash Player and 0x01 if the client is the FlashCom
    // server.
    typedef enum {
        NONE         = 0x0,
        CHUNK_SIZE   = 0x1,
        ABORT        = 0x2,
        BYTES_READ   = 0x3,
        USER         = 0x4,
        WINDOW_SIZE  = 0x5,
        SET_BANDWITH = 0x6,
        ROUTE        = 0x7,
        AUDIO_DATA   = 0x8,
        VIDEO_DATA   = 0x9,
        SHARED_OBJ   = 0xa,
	AMF3_NOTIFY  = 0xf,
	AMF3_SHARED_OBJ = 0x10,
	AMF3_INVOKE  = 0x11,
        NOTIFY       = 0x12,
        INVOKE       = 0x14,
	FLV_DATA     = 0x16
    } content_types_e;
    typedef enum {
	STREAM_START  = 0x0,
	STREAM_EOF    = 0x1,
	STREAM_NODATA = 0x2,
	STREAM_BUFFER = 0x3,
	STREAM_LIVE   = 0x4,
	STREAM_PING   = 0x6,
	STREAM_PONG   = 0x7
    } user_control_e;
    typedef enum {
         CREATE_OBJ     = 0x1,	    // Client sends event
         DELETE_OBJ     = 0x2,	    // Client sends event
         REQUEST_CHANGE = 0x3,	    // Client sends event
         CHANGE         = 0x4,	    // Server sends event
         SUCCESS_CLIENT = 0x5,	    // Server sends event
         SEND_MESSAGE   = 0x6,	    // Client sends event
         STATUS         = 0x7,	    // Server sends evetn
         CLEAR          = 0x8,	    // Server sends event
         DELETE_SLOT    = 0x9,	    // Server sends event
         REQUEST_DELETE_SLOT = 0xa, // Client sends event
         SUCCESS_SERVER = 0xb	    // Server sends event
     } sharedobj_types_e;
    typedef enum {
	PING_CLEAR  = 0x0,	// clear the stream
	PING_PLAY   = 0x1,	// clear the playing buffer
	PING_TIME   = 0x3,	// Buffer time in milliseconds
	PING_RESET  = 0x4,	// Reset stream
	PING_CLIENT = 0x6,	// Ping the client from the server
	PONG_CLIENT = 0x7	// pong reply from client to server
    } rtmp_ping_e;
    typedef enum {
	STREAM_PLAY,		// play the existing stream
	STREAM_PAUSE,		// pause the existing stream
	STREAM_PUBLISH,		// publish the existing stream
	STREAM_STOP,		// stop the existing stream
	STREAM_SEEK		// seek in the existing stream
    } rtmp_op_e;
    typedef struct {
	rtmp_ping_e type;	// the type of the ping message
	boost::uint16_t target; // all Ping message data fields
	boost::uint16_t param1; // are 2 bytes long
	boost::uint16_t param2;
	boost::uint16_t param3;
    } rtmp_ping_t;
    typedef struct {
	user_control_e type;
	boost::uint32_t param1;
	boost::uint32_t param2;	// only used by 
    } user_event_t;
    typedef enum {
        RTMP_STATE_HANDSHAKE_SEND,
        RTMP_STATE_HANDSHAKE_RECV,
        RTMP_STATE_HANDSHAKE_ACK,
        RTMP_STATE_CONNECT,
        RTMP_STATE_NETCONNECT,
        RTMP_STATE_NETSTREAM,
        RTMP_STATE_HEADER,
        RTMP_STATE_DONE
    } rtmp_state_t;
//     typedef struct {
// 	rtmp_status_e status;
// 	std::string   method;
// 	double        streamid;
// 	std::vector<boost::shared_ptr<cygnal::Element> > objs;
//     } rtmp_msg_t;
    typedef enum {
        RTMP_ERR_UNDEF,
        RTMP_ERR_NOTFOUND,
        RTMP_ERR_PERM,
        RTMP_ERR_DISKFULL,
        RTMP_ERR_ILLEGAL,
        RTMP_ERR_UNKNOWNID,
        RTMP_ERR_EXISTS,
        RTMP_ERR_NOSUCHUSER,
        RTMP_ERR_TIMEOUT,
        RTMP_ERR_NORESPONSE
    } rtmp_error_t;

// Each header consists of the following:
// a single byte that is the index of the RTMP channel,
// then two bits that's a flag to note the size of the header,
// which can be 1, 4, 8, or 12 bytes long.
    
// More info at http://wiki.gnashdev.org/RTMP
    typedef struct {
	int             channel;
	int             head_size;
	int             bodysize;
	RTMPMsg::rtmp_source_e   src_dest;
	content_types_e type;
    } rtmp_head_t;
    typedef struct {
	boost::uint32_t uptime;
	boost::uint8_t version[4];
    } rtmp_handshake_head_t;
    typedef enum {
        HEADER_12 = 0x0,
        HEADER_8  = 0x40,
        HEADER_4  = 0x80,
        HEADER_1  = 0xc0
    } rtmp_headersize_e;    
    
// Each body consists of the following:
//
// * UTF String - Target
// * UTF String - Response
// * Long - Body length in bytes
// * Variable - Actual data (including a type code)
//     typedef struct {
//         amf::amfutf8_t target;
//         amf::amfutf8_t response;
// 	boost::uint32_t length;
//         void *data;
//     } rtmp_body_t;
    
    RTMP();
    virtual ~RTMP();

    // Decode
    boost::shared_ptr<rtmp_head_t> decodeHeader(boost::uint8_t *header);
    boost::shared_ptr<rtmp_head_t> decodeHeader(cygnal::Buffer &data);
    
    boost::shared_ptr<cygnal::Buffer> encodeHeader(int amf_index,
					rtmp_headersize_e head_size,
					size_t total_size, content_types_e type,
					RTMPMsg::rtmp_source_e routing);
    boost::shared_ptr<cygnal::Buffer> encodeHeader(int amf_index,
						rtmp_headersize_e head_size);
    
    void addProperty(cygnal::Element &el);
    void addProperty(char *name, cygnal::Element &el);
    void addProperty(std::string &name, cygnal::Element &el);
    cygnal::Element &getProperty(const std::string &name);
//     void setHandler(Handler *hand) { _handler = hand; };
    int headerSize(boost::uint8_t header);

    rtmp_head_t *getHeader()    { return &_header; };
    int getHeaderSize()         { return _header.head_size; }; 
    int getTotalSize()          { return _header.bodysize; }; 
    RTMPMsg::rtmp_source_e getRouting()  { return _header.src_dest; };
    int getChannel()            { return _header.channel; };
    int getPacketSize()         { return _packet_size; };
    int getMysteryWord()        { return _mystery_word; };

    // Decode an RTMP message
    boost::shared_ptr<RTMPMsg> decodeMsgBody(boost::uint8_t *data, size_t size);
    boost::shared_ptr<RTMPMsg> decodeMsgBody(cygnal::Buffer &buf);
    
    virtual boost::shared_ptr<rtmp_ping_t> decodePing(boost::uint8_t *data);
    boost::shared_ptr<rtmp_ping_t> decodePing(cygnal::Buffer &buf);
    
    virtual boost::shared_ptr<user_event_t> decodeUserControl(boost::uint8_t *data);
    boost::shared_ptr<user_event_t> decodeUserControl(cygnal::Buffer &buf);
    virtual boost::shared_ptr<cygnal::Buffer> encodeUserControl(user_control_e, boost::uint32_t data);
    
    
    // These are handlers for the various types
    virtual boost::shared_ptr<cygnal::Buffer> encodeChunkSize(int size);
    virtual void decodeChunkSize();

    virtual boost::shared_ptr<cygnal::Buffer> encodeBytesRead();
    virtual void decodeBytesRead();
    virtual boost::shared_ptr<cygnal::Buffer> encodeServer();
    virtual void decodeServer();
    
    virtual boost::shared_ptr<cygnal::Buffer> encodeClient();
    virtual void decodeClient();
    
    virtual boost::shared_ptr<cygnal::Buffer> encodeAudioData();
    virtual void decodeAudioData();
    
    virtual boost::shared_ptr<cygnal::Buffer> encodeVideoData();
    virtual void decodeVideoData();
    
    virtual boost::shared_ptr<cygnal::Buffer> encodeNotify();
    virtual void decodeNotify();
    
    virtual boost::shared_ptr<cygnal::Buffer> encodeSharedObj();
    virtual void decodeSharedObj();
    
    virtual boost::shared_ptr<cygnal::Buffer> encodeInvoke();
    virtual void decodeInvoke();

    // Receive a message, which is a series of AMF elements, seperated
    // by a one byte header at regular byte intervals. (128 bytes for
    // video data by default). Each message may contain multiple packets.
    boost::shared_ptr<cygnal::Buffer> recvMsg();
    boost::shared_ptr<cygnal::Buffer> recvMsg(int fd);

    // Send a message, usually a single ActionScript object. This message
    // may be broken down into a series of packets on a regular byte
    // interval. (128 bytes for video data by default). Each message main
    // contain multiple packets.
    bool sendMsg(cygnal::Buffer &data);
    bool sendMsg(int channel, rtmp_headersize_e head_size,
	      size_t total_size, content_types_e type,
	      RTMPMsg::rtmp_source_e routing, cygnal::Buffer &data);
    bool sendMsg(int fd, int channel, rtmp_headersize_e head_size,
	      size_t total_size, content_types_e type,
	      RTMPMsg::rtmp_source_e routing, cygnal::Buffer &data);
    bool sendMsg(int channel, rtmp_headersize_e head_size,
		 size_t total_size, content_types_e type,
		 RTMPMsg::rtmp_source_e routing, boost::uint8_t *data, size_t size);
    bool sendMsg(int fd, int channel, rtmp_headersize_e head_size,
		 size_t total_size, content_types_e type,
		 RTMPMsg::rtmp_source_e routing, boost::uint8_t *data, size_t size);
    
#if 0
    // Send a Msg, and expect a response back of some kind.
    RTMPMsg *sendRecvMsg(int amf_index, rtmp_headersize_e head_size,
			      size_t total_size, content_types_e type,
			      RTMPMsg::rtmp_source_e routing, cygnal::Buffer &buf);
#endif
    // Split a large buffer into multiple smaller ones of the default chunksize
    // of 128 bytes. We read network data in big chunks because it's more efficient,
    // but RTMP uses a weird scheme of a standard header, and then every chunksize
    // bytes another 1 byte RTMP header. The header itself is not part of the byte
    // count.
    boost::shared_ptr<queues_t> split(cygnal::Buffer &buf);
    boost::shared_ptr<queues_t> split(boost::uint8_t *data, size_t size);

    CQue &operator[] (size_t x) { return _queues[x]; }

    /// \method getTime
    ///    The time on most systems these days is a 64 bit long, but swf
    ///    is old, so it only uses a 32 bit integer instead. We know casting
    ///    looses precision, but that's just the way it is in RTMP.
    boost::uint32_t getTime() {
	time_t t;
	time(&t);
	return boost::lexical_cast<boost::uint32_t>(t);
    };

    void dump();
  protected:
    AMFProperties _properties;
    cygnal::Buffer	*_handshake;
//     Handler	*_handler;
    rtmp_head_t	_header;
    int         _packet_size;
    int         _mystery_word;
    size_t	_chunksize[MAX_AMF_INDEXES];
    size_t	_lastsize[MAX_AMF_INDEXES];
    std::vector<size_t> _bodysize;
    std::vector<content_types_e> _type;
    int		_timeout;
    CQue	_queues[MAX_AMF_INDEXES];
//    queues_t    _channels;
    cygnal::Buffer	_buffer;
    rtmp_handshake_head_t _handshake_header;
};

} // end of gnash namespace
// end of _RTMP_H_
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

