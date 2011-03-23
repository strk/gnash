//
//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

#ifndef GNASH_RTMP_H
#define GNASH_RTMP_H

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <deque>
#include <map>

#include "SimpleBuffer.h"
#include "Socket.h"
#include "dsodefs.h"

#define RTMP_DEFAULT_CHUNKSIZE	128

// Forward declarations.
namespace gnash {
    namespace rtmp {
        class HandShaker;
    }
    class URL;
}

namespace gnash {
namespace rtmp {

/// Known control / ping codes
//
/// See http://jira.red5.org/confluence/display/docs/Ping (may not exist).
//
/// 0x00: Clear the stream. No third and fourth parameters. The second
///       parameter could be 0. After the connection is established, a
///       Ping 0,0 will be sent from server to client. The message will
///       also be sent to client on the start of Play and in response of
///       a Seek or Pause/Resume request. This Ping tells client to
///       re-calibrate the clock with the timestamp of the next packet
///       server sends.
/// 0x01: Tell the stream to clear the playing buffer.
/// 0x02: Stream dry (not sure what this means!)
/// 0x03: Buffer time of the client. The third parameter is the buffer
///       time in milliseconds.
/// 0x04: Reset a stream. Used together with type 0 in the case of VOD.
///       Often sent before type 0.
/// 0x06: Ping the client from server. The second parameter is the current
///       time.
/// 0x07: Pong reply from client. The second parameter is the time the
///       server sent with his ping request.
/// 0x1a: SWFVerification request
/// 0x1b: SWFVerification response
/// 0x1f: Not sure, maybe buffer empty.
/// 0x20: Buffer ready.
enum ControlType
{
    CONTROL_CLEAR_STREAM = 0x00,
    CONTROL_CLEAR_BUFFER = 0x01,
    CONTROL_STREAM_DRY = 0x02,
    CONTROL_BUFFER_TIME = 0x03,
    CONTROL_RESET_STREAM = 0x04,
    CONTROL_PING = 0x06,
    CONTROL_PONG = 0x07,
    CONTROL_REQUEST_VERIFY = 0x1a,
    CONTROL_RESPOND_VERIFY = 0x1b,
    CONTROL_BUFFER_EMPTY = 0x1f,
    CONTROL_BUFFER_READY = 0x20
};

/// The known channels.
//
/// CHANNEL_CONTROL1 is for internal controls:
///     sendCtrl
///     send server BW
///     send bytes received.
//
/// These contain no AMF data.
//
/// CHANNEL_CONTROL2 is for ActionScript controls
///     _checkbw: AS
///     _result:
///     connect: Maybe from ASNative(2100, 0) (connect)
///     createStream: AS
///     deleteStream: Maybe ASNative(2100, 1) (close)
///     FCSubscribe: Don't know.
//
/// These all contain AMF data.
enum Channels
{
    CHANNEL_CONTROL1 = 0x02,
    CHANNEL_CONTROL2 = 0x03,
    CHANNEL_VIDEO = 0x08
};

/// The known packet types.
enum PacketType
{
    PACKET_TYPE_NONE = 0x00,
    PACKET_TYPE_CHUNK_SIZE = 0x01,
    PACKET_TYPE_BYTES_READ = 0x03,
    PACKET_TYPE_CONTROL = 0x04,
    PACKET_TYPE_SERVERBW = 0x05,
    PACKET_TYPE_CLIENTBW = 0x06,
    PACKET_TYPE_AUDIO = 0x08,
    PACKET_TYPE_VIDEO = 0x09,
    PACKET_TYPE_FLEX_STREAM_SEND = 0x0f,
    PACKET_TYPE_FLEX_SHARED_OBJECT = 0x10,
    PACKET_TYPE_FLEX_MESSAGE = 0x11,
    PACKET_TYPE_METADATA = 0x12,
    PACKET_TYPE_SHARED_OBJECT = 0x13,
    PACKET_TYPE_INVOKE = 0x14,
    PACKET_TYPE_FLV = 0x16
};

/// The PacketSize specifies the number of fields contained in the header.
//
/// 1. A large packet has all header fields.
///     We would expect the first packet on any channel to be large.
/// 2. A medium packet has the same m_nInfoField2 and packet type.
/// 3. A small packet has the same data size.
/// 4. A minimal packet has all fields the same.
//
/// The minimal and small data packets can be used to send a payload in more
/// than one packet. If the data received is smaller than the specified data
/// size, the packet remains in the channel with its payload. Data from each
/// new packet is added to this stored payload until all the advertised data
/// is read.
//
/// These names are taken from rtmpdump.
enum PacketSize {
    RTMP_PACKET_SIZE_LARGE = 0,
    RTMP_PACKET_SIZE_MEDIUM = 1,
    RTMP_PACKET_SIZE_SMALL = 2,
    RTMP_PACKET_SIZE_MINIMUM = 3
};

/// The RTMPHeader contains all the fields for the packet header.
struct RTMPHeader
{
    /// The maximum header size of an RTMP packet.
    static const size_t headerSize = 18;

    RTMPHeader()
        :
        headerType(RTMP_PACKET_SIZE_LARGE),
        packetType(PACKET_TYPE_NONE),
        _timestamp(0),
        _streamID(0),
        channel(0),
        dataSize(0)
    {}

    PacketSize headerType;
    PacketType packetType;

    /// The timestamp.
    //
    /// This is encoded either as in the 3-byte relative timestamp field or the
    /// 4 byte extended (absolute) timestamp field.
    boost::uint32_t _timestamp;

    /// This seems to be used for NetStream.play.
    boost::uint32_t _streamID;

    size_t channel;

    // The size of the data.
    size_t dataSize;

};

/// An RTMPPacket class contains a full description of an RTMP packet.
//
/// This comprises:
///     header information
///     an AMF payload.
//
/// An RTMPPacket may be copied without a large penalty. This is to allow
/// storage in the RTMP client's channels.
struct RTMPPacket
{
    /// Construct a packet with an optional reserved memory allocation.
    //
    /// @param reserve      The amount of space in bytes to reserve for the
    ///                     message body. This can save reallocations when
    ///                     appending AMF data. Space for the header is
    ///                     always reserved and is not affected by this
    ///                     parameter.
    explicit RTMPPacket(size_t reserve = 0);
    
    /// Copy constructor.
    //
    /// Creates an identical RTMPPacket with shared ownership of the
    /// buffer.
    RTMPPacket(const RTMPPacket& other);

    ~RTMPPacket() {}

    RTMPHeader header;

    /// A buffer with enough storage to write the entire message.
    //
    /// This always includes at least the header. Storage for the message
    /// payload is added as necessary.
    boost::shared_ptr<SimpleBuffer> buffer;

    size_t bytesRead;
};


/// Check whether an RTMPPacket has a payload.
//
/// Only stored packets may not have a payload. A packet without a payload
/// has already been processed and is only used for its header information.
inline bool
hasPayload(const RTMPPacket& p)
{
    return (p.buffer.get());
}

/// Clear the message body and the bytes read of an RTMPPacket.
//
/// This is to be used to free used information from packets in a channel.
/// The header information must be preserved for future packets, but the
/// payload is no longer needed once read.
inline void
clearPayload(RTMPPacket& p)
{
    p.buffer.reset();
    p.bytesRead = 0;
}

/// The current size of the space allocated for the message payload.
//
/// For messages we create, this matches the exact size of the payload. For
/// messages we read, this is the expected size of the data.
inline size_t
payloadSize(const RTMPPacket& p)
{
    assert(hasPayload(p));
    const SimpleBuffer& buf = *p.buffer;
    assert(buf.size() >= RTMPHeader::headerSize);
    return buf.size() - RTMPHeader::headerSize;
}

/// Access the payload data section of the buffer.
inline boost::uint8_t*
payloadData(RTMPPacket& p)
{
    assert(hasPayload(p));
    SimpleBuffer& buf = *p.buffer;
    return buf.data() + RTMPHeader::headerSize;
}

/// Access the payload data section of the buffer.
inline const boost::uint8_t*
payloadData(const RTMPPacket& p)
{
    assert(hasPayload(p));
    const SimpleBuffer& buf = *p.buffer;
    return buf.data() + RTMPHeader::headerSize;
}

/// Get the end of the allocated payload data section of the buffer.
//
/// Note that this is only valid for packets we create, and for packets
/// we have fully read. Stored packets that have not yet received all data
/// have allocated space that has not yet been written.
inline const boost::uint8_t*
payloadEnd(const RTMPPacket& p)
{
    assert(hasPayload(p));
    SimpleBuffer& buf = *p.buffer;
    return buf.data() + buf.size();
}

/// Check if a packet is ready for processing.
//
/// A packet is ready for processing if its payload size matches the data
/// size in its header. It may take several successive packets to form a 
/// complete packet. 
inline bool
isReady(const RTMPPacket& p) {
    return p.bytesRead == p.header.dataSize;
}


/// This class is for handling the RTMP protocol.
//
/// Only the RTMP protocol itself is handled in this class. An RTMP connection
/// is valid only when connected() is true.
//
/// An RTMP object may be closed and reconnected. As soon as connect() returns
/// true, callers are responsible for calling close().
//
/// RTMP has a set of channels for incoming and outgoing packets. Packets 
/// are stored here for two reasons:
/// 1. The payload size exceeds the chunk size, so a single payload requires
///    several complete packets. A packet is not 'ready' unless it has a
///    complete payload, or is the packet that completes the payload of
///    previous packets.
/// 2. Subsequent packets sent on the same channel can be compressed if they
///    have the same header information. The stored packet header is used for
///    comparison. For this case, the payload is no longer necessary.
//
/// A different case applies to incomplete packets. The payload of a single
/// packet (whether the packet is 'ready' or not) is the smaller of (a) the
/// advertised data size and (b) the chunk size. Until this much data has
/// been read, the packet is incomplete.  Whereas Gnash always
/// expects a complete header to be available or none at all, the payload
/// can be read over several calls to update().
struct DSOEXPORT RTMP
{

    /// Construct a non-connected RTMP handler.
    RTMP();

    ~RTMP();

    /// Initiate a network connection.
    //
    /// Note that this only creates the TCP connection and carries out the
    /// handshake. An active data connection needs an AMF connect request,
    /// which is not part of the RTMP protocol.
    //
    /// @return     true if the connection attempt starts, otherwise false.
    ///             A return of false means that the RTMP object is in a 
    ///             closed state and can be reconnected.
    bool connect(const URL& url);

    /// This is used for sending call requests from the core.
    //
    /// These are sent as invoke packets on CHANNEL_CONTROL2. The AMF data
    /// should contain:
    /// 1. method name,
    /// 2. callback number,
    /// 3. null,
    /// 4. arg0..argn
    void call(const SimpleBuffer& amf);

    /// This is used for sending NetStream requests.
    //
    /// These include play and pause. They are sent as invoke packets on the
    /// video channel. 
    //
    /// @param id       The stream ID to control. This is encoded in the header,
    ///                 not the AMF payload.
    void play(const SimpleBuffer& amf, int id);

    /// Instruct server to buffer this much data.
    //
    /// @param time     time in milliseconds.
    /// @param streamID the ID of the stream to set buffer time on.
    void setBufferTime(size_t time, int streamID);

    /// Whether we have a basic connection to a server.
    //
    /// This only means that the handshake is complete and that AMF requests
    /// can be sent to the server. It does not mean that was can send or
    /// receive media streams.
    //
    /// You should ensure that connected() is true before attempting to send
    /// or receive data.
    bool connected() const {
        return _connected;
    }

    /// Whether the RTMP connection is in error condition.
    //
    /// This is a fatal error.
    bool error() const {
        return _error;
    }

    /// This function handles reading incoming data and filling data queues.
    //
    /// You should call this function regularly once the initial connection
    /// has been initiated.
    //
    /// Its tasks involve:
    /// 1. completing the handshake
    /// 2. checking for socket errors
    /// 3. reading incoming data
    /// 4. filling data queues.
    //
    /// None of those things should concern you. Just call the function
    /// regularly and use connected(), error(), and check the message
    /// queues.
    void update();

    /// Close the connection.
    //
    /// A new connection may now be opened.
    void close();

    /// Get an AMF message received from the server.
    //
    /// TODO: this returns the whole RTMP message, which is ugly. And it
    /// only returns one at time, and can return a null pointer. We need
    /// a better way to retrieve the messages.
    boost::shared_ptr<SimpleBuffer> getMessage() {
        if (_messageQueue.empty()) return boost::shared_ptr<SimpleBuffer>();
        boost::shared_ptr<SimpleBuffer> b = _messageQueue.front();
        _messageQueue.pop_front();
        return b;
    }
    
    /// Get an FLV packet received from the server
    //
    /// TODO: this returns the whole RTMP message, which is ugly. And it
    /// only returns one at time, and can return a null pointer. We need
    /// a better way to retrieve the frames.
    boost::shared_ptr<SimpleBuffer> getFLVFrame() {
        if (_flvQueue.empty()) return boost::shared_ptr<SimpleBuffer>();
        boost::shared_ptr<SimpleBuffer> b = _flvQueue.front();
        _flvQueue.pop_front();
        return b;
    }

    /// Handle an RTMPPacket.
    void handlePacket(const RTMPPacket& packet);
    
    /// Read from the socket.
    int readSocket(boost::uint8_t* dst, int num);

    /// Send an RTMPPacket on the connection.
    bool sendPacket(RTMPPacket& packet);

    /// Store the server bandwidth
    //
    /// Not sure why we need this.
    void setServerBandwidth(boost::uint32_t bw) {
        _serverBandwidth = bw;
    }

    /// Get the stored server bandwidth.
    boost::uint32_t serverBandwidth() const {
        return _serverBandwidth;
    }

    /// Store our bandwidth
    void setBandwidth(boost::uint32_t bw) {
        _bandwidth = bw;
    }

    /// Get our bandwidth.
    boost::uint32_t bandwidth() const {
        return _bandwidth;
    }

    int _inChunkSize;
    int m_mediaChannel;
    boost::uint8_t m_nClientBW2;
    size_t _bytesIn;
    size_t _bytesInSent;

private:

    enum ChannelType {
        CHANNELS_IN,
        CHANNELS_OUT
    };
    
    /// Read an RTMP packet from the connection.
    bool readPacketHeader(RTMPPacket& packet);

    bool readPacketPayload(RTMPPacket& packet);

    /// Check whether a packet exists on a channel.
    bool hasPacket(ChannelType t, size_t channel) const;

    /// Retrieve a stored packet.
    //
    /// If no packet exists on the channel, a new one will be created and
    /// returned. Use hasPacket() to check whether a previous packet exists.
    RTMPPacket& getPacket(ChannelType t, size_t channel);

    /// Store a packet in a channel.
    //
    /// A copy of the packet is stored and returned. Any data is shared
    /// between the copies until explicitly reset.
    RTMPPacket& storePacket(ChannelType t, size_t channel, const RTMPPacket& p);

    /// A set of channels. An RTMP handler has two sets.
    //
    /// Packets are stored on these channels. As soon as a packet has been
    /// processed, its payload is removed. The header remains in memory to
    /// allow compression of later packets.
    //
    /// RTMPPackets must be stored with an absolute timestamp.
    typedef std::map<size_t, RTMPPacket> ChannelSet;
    
    Socket _socket;

    /// A set of channels for receiving packets.
    ChannelSet _inChannels;

    /// A set of channels for sending packets.
    ChannelSet _outChannels;
    
    std::deque<boost::shared_ptr<SimpleBuffer> > _messageQueue;
    std::deque<boost::shared_ptr<SimpleBuffer> > _flvQueue;

    /// Stored server bandwidth (reported by server).
    boost::uint32_t _serverBandwidth;

    /// Stored client bandwidth (ours), reported by server.
    boost::uint32_t _bandwidth;

    /// Chunk size for sending.
    size_t _outChunkSize;

    boost::scoped_ptr<HandShaker> _handShaker;

    bool _connected;

    bool _error;

    /// If a packet could not be read in one go, it is stored here.
    //
    /// This is not the same as a non-ready packet. It applies only to packets
    /// waiting for payload data.
    boost::scoped_ptr<RTMPPacket> _incompletePacket;

};

/// Send a bandwidth ping to the server.
DSOEXPORT bool sendServerBW(RTMP& r);

/// Send a control packet
bool sendCtrl(RTMP& r, ControlType, unsigned int nObject, unsigned int nTime);

/// Logging assistance for PacketType.
std::ostream& operator<<(std::ostream& o, PacketType p);

/// Logging assistance for ControlType.
std::ostream& operator<<(std::ostream& o, ControlType t);

} // namespace rtmp

} // namespace gnash
#endif
