// rtmpdump.cpp:  RTMP file downloader utility
// 
//   Copyright (C) 2008, 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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

#include "RTMP.h"
#include <string>
#include "log.h"
#include "arg_parser.h"
#include "SimpleBuffer.h"
#include "AMF.h"
#include "GnashAlgorithm.h"
#include "GnashSleep.h"
#include "URL.h"

#include <cstdint>
#include <iomanip>
#include <map>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <iostream>

using namespace gnash;

/// The play command is initiated by NetStream.play. This must specify a
/// play path, and can add three further optional arguments. These are:
///     1. start: start offset, or -1 for a live stream, or -2 for either
///        a live stream if found, or a recorded stream if not.
///     2. length: how long to play, or -1 for all of a live stream, or 0 for
///                a single frame.
///     3. reset: (object) no documentation.
/// This class should not care about these! NetStream / NetConnection should
/// encode all the play arguments and send them. The play packet should be
/// on the video channel (what about audio?) and be an "invoke" packet.
//
/// ActionScript can send invoke packets directly using NetConnection call.
//
/// TODO:   This class needs a function to be called at heartbeat rate so that
///         the data can be processed.
/// TODO:   Work out which messages should be handled internally and which
///         should be made available to the core (maybe all).
///             1. Core events are those that should be available to AS. They
///                include: onStatus.
///             2. Internal events are not important to AS. We don't know
///                if they ever get through. They certainly aren't documented
///                and they may expect a certain response. They include:
///                _onbwdone, _onbwcheck, _result. The _result function is
///                almost certainly forwarded as onResult.
///             3. The client should send createStream. This is builtin to
///                to actionscript.
// function NetStream(connection) {
//  
//     function OnCreate(nStream) {
//          this.nStream = nStream;
//     }
//  
//     // Some kind of type thing associating NetStream and NetConnection.
//     ASnative(2101, 200)(this, connection);
//  
//     var _local2 = OnCreate.prototype;
//  
//     /// The server should send onResult with stream ID.
//     //
//     /// What does the function do? Stores the stream ID somewhere so it
//     /// can 
//     _local2.onResult = function (streamId) {
//         ASnative(2101, 201)(this.nStream, streamId);
//     };
//  
//      /// onStatus messages are forwarded to the NetStream object.
//     _local2.onStatus = function (info) {
//         this.nStream.onStatus(info);
//     };
// 
//     /// Send invoke packet with createStream. The callback is the OnCreate
//     /// object.
//     connection.call("createStream", new OnCreate(this));
// }

namespace {
    void usage(std::ostream& o);
}

class FakeNC
{
public:

    FakeNC()
        :
        _callCount(0),
        _seek(0),
        _len(-1),
        _stream(0)
    {}

    size_t callNumber() {
        return _callCount++;
    }

    void queueCall(size_t n, const std::string& call) {
        _calls.emplace(n, call);
    }

    std::string getCall(size_t n) {
        std::map<size_t, std::string>::iterator i = _calls.find(n);
        if (i == _calls.end()) return "";

        std::string s = i->second;
        _calls.erase(i);
        return s;
    }

    void setPlayPath(const std::string& p) {
        _playpath = p;
    }

    const std::string& playpath() const {
        return _playpath;
    }

    void setSeekTime(double secs) {
        _seek = secs;
    }

    double seekTime() const {
        return _seek;
    }

    void setLength(double len) {
        _len = len;
    }

    double length() const {
        return _len;
    }

    void setStreamID(int s) {
        _stream = s;
    }

    int streamID() const {
        return _stream;
    }

private:
    size_t _callCount;
    std::map<size_t, std::string> _calls;

    std::string _playpath;

    double _seek, _len;

    int _stream;
};

void
writeFLVHeader(std::ostream& o)
{
    char flvHeader[] = {
        'F',  'L',  'V',  0x01,
        0x05,
        0x00, 0x00, 0x00, 0x09,
        0x00, 0x00, 0x00, 0x00	// first prevTagSize=0
    };

    o.write(flvHeader, arraySize(flvHeader));
}


bool handleInvoke(rtmp::RTMP& r, FakeNC& nc, const std::uint8_t* payload,
        const std::uint8_t* end);

/// These functions create an RTMP call buffer and send it. They mimic
/// NetConnection.call() methods and replies to server calls.
//
/// If a call is initiated by us, we send our own call number.
/// If we are replying to a server call, we send the server's call number back.
void
sendConnectPacket(rtmp::RTMP& r, FakeNC& nc, const std::string& app,
        const std::string& ver, const std::string& swfurl,
        const std::string& tcurl, const std::string& pageurl)
{
    log_debug("Sending connect packet.");
    
    log_debug("app      : %s", app);
    log_debug("flashVer : %s", ver);
    log_debug("tcURL    : %s", tcurl);
    log_debug("swfURL   : %s", swfurl);
    log_debug("pageURL  : %s", pageurl);

    SimpleBuffer buf;

    amf::write(buf, "connect");
    const size_t cn = nc.callNumber();
    
    /// Call number?
    amf::write(buf, static_cast<double>(cn));

    buf.appendByte(amf::OBJECT_AMF0);
    if (!app.empty()) amf::writeProperty(buf, "app", app);
    if (!ver.empty()) amf::writeProperty(buf, "flashVer", ver);
    if (!swfurl.empty()) amf::writeProperty(buf, "swfUrl", swfurl);
    if (!tcurl.empty()) amf::writeProperty(buf, "tcUrl", tcurl);
    amf::writeProperty(buf, "fpad", false);
    amf::writeProperty(buf, "capabilities", 15.0);
    amf::writeProperty(buf, "audioCodecs", 3191.0);
    amf::writeProperty(buf, "videoCodecs", 252.0);
    amf::writeProperty(buf, "videoFunction", 1.0);
    if (!pageurl.empty()) amf::writeProperty(buf, "pageUrl", pageurl);
    buf.appendByte(0);
    buf.appendByte(0);
    buf.appendByte(amf::OBJECT_END_AMF0);

    nc.queueCall(cn, "connect");
    r.call(buf);

}

void
sendCheckBW(rtmp::RTMP& r, FakeNC& nc)
{
    SimpleBuffer buf;
    
    const size_t cn = nc.callNumber();

    amf::write(buf, "_checkbw");
    amf::write(buf, static_cast<double>(cn));
    buf.appendByte(amf::NULL_AMF0);

    nc.queueCall(cn, "_checkbw");
    r.call(buf);
}

void
replyBWCheck(rtmp::RTMP& r, FakeNC& /*nc*/, double txn)
{
    // Infofield1?
    SimpleBuffer buf;
    amf::write(buf, "_result");
    amf::write(buf, txn);
    buf.appendByte(amf::NULL_AMF0);
    amf::write(buf, 0.0);
    
    r.call(buf);

}

void
sendPausePacket(rtmp::RTMP& r, FakeNC& nc, bool flag, double time)
{
    const int streamid = nc.streamID();

    SimpleBuffer buf;

    amf::write(buf, "pause");

    // What is this? The play stream? Call number?
    amf::write(buf, 0.0);
    buf.appendByte(amf::NULL_AMF0);

    log_debug( "Pause: flag=%s, time=%d", flag, time);
    amf::write(buf, flag);

    // "this.time", i.e. NetStream.time.
    amf::write(buf, time * 1000.0);

    r.play(buf, streamid);
}

// Which channel to send on? Always video?
//ASnative(2101, 202)(this, "play", null, name, start * 1000, len * 1000, reset);
// This call is not queued (it's a play call, and doesn't have a callback).
void
sendPlayPacket(rtmp::RTMP& r, FakeNC& nc)
{

    const int streamid = nc.streamID();
    const double seektime = nc.seekTime() * 1000.0;
    const double length = nc.length() * 1000.0;

    log_debug("Sending play packet. Stream id: %s, playpath %s", streamid,
            nc.playpath());

    SimpleBuffer buf;

    amf::write(buf, "play");

    // What is this? The play stream? Call number?
    amf::write(buf, 0.0);
    buf.appendByte(amf::NULL_AMF0);

    log_debug( "seekTime=%.2f, dLength=%d, sending play: %s",
        seektime, length, nc.playpath());
    amf::write(buf, nc.playpath());

    // Optional parameters start and len.
    //
    // start: -2, -1, 0, positive number
    //  -2: looks for a live stream, then a recorded stream, if not found
    //  any open a live stream
    //  -1: plays a live stream
    // >=0: plays a recorded streams from 'start' milliseconds
    amf::write(buf, seektime);

    // len: -1, 0, positive number
    //  -1: plays live or recorded stream to the end (default)
    //   0: plays a frame 'start' ms away from the beginning
    //  >0: plays a live or recoded stream for 'len' milliseconds
    //enc += EncodeNumber(enc, -1.0); // len
    amf::write(buf, length);
    
    r.play(buf, streamid);
}

void
sendCreateStream(rtmp::RTMP& r, FakeNC& nc)
{
    const size_t cn = nc.callNumber();

    SimpleBuffer buf;
    amf::write(buf, "createStream");
    amf::write(buf, static_cast<double>(cn));
    buf.appendByte(amf::NULL_AMF0);
    nc.queueCall(cn, "createStream");
    r.call(buf);
}
void
sendDeleteStream(rtmp::RTMP& r, FakeNC& nc, double id)
{
    const size_t cn = nc.callNumber();

    SimpleBuffer buf;
    amf::write(buf, "deleteStream");

    // Call number?
    amf::write(buf, static_cast<double>(cn));
    buf.appendByte(amf::NULL_AMF0);
    amf::write(buf, id);
    nc.queueCall(cn, "deleteStream");
    r.call(buf);
}

void
sendFCSubscribe(rtmp::RTMP& r, FakeNC& nc, const std::string& subscribepath)
{
    const size_t cn = nc.callNumber();

    SimpleBuffer buf;
    amf::write(buf, "FCSubscribe");

    // What is this?
    amf::write(buf, static_cast<double>(cn));
    buf.appendByte(amf::NULL_AMF0);
    amf::write(buf, subscribepath);

    nc.queueCall(cn, "FCSubscribe");
    r.call(buf);
}

/// Some URLs to try are:
//
/// -u rtmp://tagesschau.fcod.llnwd.net:1935/a3705/d1
/// with -p 2010/0216/TV-20100216-0911-2401.hi or
///      -p 2010/0216/TV-20100216-0911-2401.lo
//
/// -u rtmp://ndr.fc.llnwd.net:1935/ndr/_definst_
/// with -p ndr_fs_nds_hi_flv *and* -s -1 (live stream)
int
main(int argc, char** argv)
{
   const Arg_parser::Option opts[] =
        {
        { 'h', "help",          Arg_parser::no  },
        { 'v', "verbose",       Arg_parser::no  },
        { 'u', "url",           Arg_parser::yes  },
        { 'p', "playpath",      Arg_parser::yes  },
        { 's', "seek",          Arg_parser::yes  },
        { 'l', "length",        Arg_parser::yes  },
        { 'o', "outfile",       Arg_parser::yes  }
        };

    Arg_parser parser(argc, argv, opts);

    if (!parser.error().empty())  {
        std::cout << parser.error() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    gnash::LogFile& l = gnash::LogFile::getDefaultInstance();

    std::string url;
    std::string playpath;
    std::string tc;
    std::string swf;
    std::string page;
    std::string outf;

    double seek = 0, len = -1;

    for (int i = 0; i < parser.arguments(); ++i) {
        const int code = parser.code(i);
        try {
            switch (code) {
              case 'h':
                  usage(std::cout);
                  exit(EXIT_SUCCESS);
              case 'u':
                  url = parser.argument(i);
                  break;
              case 'p':
                  playpath = parser.argument(i);
                  break;
              case 't':
                  tc = parser.argument(i);
                  break;
              case 's':
                  seek = parser.argument<double>(i);
                  break;
              case 'l':
                  len = parser.argument<double>(i);
                  break;
              case 'o':
                  outf = parser.argument(i);
                  break;
              case 'v':
                  l.setVerbosity();
                  break;
            }
        }
        catch (Arg_parser::ArgParserException &e) {
            std::cerr << _("Error parsing command line: ") << e.what() << "\n";
            std::exit(EXIT_FAILURE);
        }
    }

    if (url.empty() || playpath.empty()) {
        std::cerr << "You must specify URL and playpath\n";
        std::exit(EXIT_FAILURE);
    }

    if (outf.empty()) {
        std::cerr << "No output file specified. Will connect anyway\n";
    }

    std::ofstream flv;

    if (!outf.empty()) {
        flv.open(outf.c_str());
        if (flv) writeFLVHeader(flv);
    }

    URL playurl(url);
    if (tc.empty()) tc = playurl.str();

    const std::string app = playurl.path().substr(1);

    std::string ver = "LNX 10,0,22,87";

    gnash::rtmp::RTMP r;
    FakeNC nc;
    nc.setPlayPath(playpath);
    nc.setLength(len);
    nc.setSeekTime(seek);

    log_debug("Initial connection");

    if (!r.connect(url)) {
        log_error("Initial connection failed!");
        std::exit(EXIT_FAILURE);
    }

    do {
        r.update();
        gnashSleep(1000);
    } while (!r.connected());
    
    if (r.error()) {
        log_error("Connection attempt failed");
        std::exit(EXIT_FAILURE);
    }

    /// 1. connect.
    sendConnectPacket(r, nc, app, ver, swf, tc, page);
 
    // Some servers are fine if we send _onbwcheck here, others aren't.
    // Either way it's a SWF implementation detail, not an automatic
    // send.
    //sendCheckBW(r, nc);   
    
    // Note that rtmpdump sends the "ServerBW" control ping when the connect
    // call returns.

    log_debug("Connect packet sent.");

    while (1) {
        r.update();
        if (r.error()) {
            gnash::log_error("Connection error");
            break;
        }

        /// Retrieve messages.
        std::shared_ptr<SimpleBuffer> b = r.getMessage();
        while (b.get()) {
            handleInvoke(r, nc, b->data() + rtmp::RTMPHeader::headerSize,
                    b->data() + b->size());
            b = r.getMessage();
        }

        /// Retrieve video packets.
        std::shared_ptr<SimpleBuffer> f = r.getFLVFrame();
        while (f.get()) {
            if (flv) {
                const char* start = reinterpret_cast<const char*>(
                        f->data() + rtmp::RTMPHeader::headerSize);
                flv.write(start, f->size() - rtmp::RTMPHeader::headerSize);
            }
            f = r.getMessage();
        }
        gnashSleep(1000);

    }

}

bool
handleInvoke(rtmp::RTMP& r, FakeNC& nc, const std::uint8_t* payload,
        const std::uint8_t* end)
{
    assert(payload != end);

    // make sure it is a string method name we start with
    if (payload[0] != 0x02) {
        log_error( "%s, Sanity failed. no string method in invoke packet",
                __FUNCTION__);
        return false;
    }

    ++payload;
    std::string method = amf::readString(payload, end);

    log_debug("Invoke: read method string %s", method);
    if (*payload != amf::NUMBER_AMF0) return false;
    ++payload;


    log_debug( "%s, server invoking <%s>", __FUNCTION__, method);

    bool ret = false;

    /// _result means it's the answer to a remote method call initiated
    /// by us.
    if (method == "_result") {
        
        const double txn = amf::readNumber(payload, end);
        std::string calledMethod = nc.getCall(txn);

        log_debug("Received result for method call %s (%s)",
                calledMethod, boost::io::group(std::setprecision(15), txn));

        if (calledMethod == "connect")
        {
            // Do here.
            sendCreateStream(r, nc);
        }

        else if (calledMethod == "createStream") {
            
            log_debug("createStream invoked");
            if (*payload != amf::NULL_AMF0) return false;
            ++payload;
            
            log_debug("AMF buffer for createStream: %s\n",
                    hexify(payload, end - payload, false));

            if (*payload != amf::NUMBER_AMF0) return false;
            ++payload;
            double sid = amf::readNumber(payload, end);

            log_debug("Stream ID: %s", sid);
            nc.setStreamID(sid);

            /// Issue NetStream.play command.
            sendPlayPacket(r, nc);

            /// Allows quick downloading.
            r.setBufferTime(3600000, nc.streamID());
        }

        else if (calledMethod == "play") {
            log_debug("Play called");
        }
        return ret;
    }

    /// These are remote function calls initiated by the server .

    const double txn = amf::readNumber(payload, end);
    log_debug("Received server call %s %s",
            boost::io::group(std::setprecision(15), txn),
            txn ? "" : "(no reply expected)");

    /// This must return a value. It can be anything.
    if (method == "onBWCheck") {
        if (txn) replyBWCheck(r, nc, txn);
        else {
            log_error("Expected call number for onBWCheck");
        }
    }
    
    /// If the server sends this, we reply (the call should contain a
    /// callback object!).
    if (method == "_onbwcheck") {
        if (txn) replyBWCheck(r, nc, txn);
        else {
            log_error("Server called _onbwcheck without a callback");
        }
        return ret;
    }

    // This should be called by the server when the bandwidth test is finished.
    //
    // It contains information, but we don't have to do anything.
    if (method == "onBWDone") {
        // This is a SWF implementation detail, not required by the protocol.
        return ret;
    }

    if (method == "_onbwdone") {

        if (*payload != amf::NULL_AMF0) return false;
        ++payload;

        log_debug("AMF buffer for _onbwdone: %s\n",
                hexify(payload, end - payload, false));

        double latency = amf::readNumber(payload, end);
        double bandwidth = amf::readNumber(payload, end);
        log_debug("Latency: %s, bandwidth %s", latency, bandwidth);
        return ret;
    }

    /// Don't know when it sends this.
    if (method == "onFCSubscribe") {
        return ret;
    }

    /// Or this.
    if (method == "onFCUnsubscribe") {
        r.close();
        ret = true;
        return ret;
    }

    if (method ==  "_error") {
        log_error( "rtmp server sent error");
        std::exit(EXIT_FAILURE);
    }
    
    if (method == "close") {
        log_error( "rtmp server requested close");
        r.close();
        return ret;
    }
    
    if (method == "onStatus") {
        if (*payload != amf::NULL_AMF0) return false;
        ++payload;
#if 1
        log_debug("AMF buffer for onstatus: %s",
                hexify(payload, end - payload, true));
#endif
        if (*payload != amf::OBJECT_AMF0) {
            log_debug("not an object");
            return false;
        }
        ++payload;
        if (payload == end) return false;

        std::string code;
        std::string level;
        try {

            // Hack.
            while (payload < end && *payload != amf::OBJECT_END_AMF0) {

                const std::string& n = amf::readString(payload, end);
                if (n.empty()) continue;

                //log_debug("read string %s", n);
                if (payload == end) break;

                // There's no guarantee that all members are strings, but
                // it's usually enough for this.
                if (*payload != amf::STRING_AMF0) {
                    break;
                }

                ++payload;
                if (payload == end) break;

                const std::string& v = amf::readString(payload, end);
                if (payload == end) break;
                if (n == "code") code = v;
                if (n == "level") level = v;
            }
        }
        catch (const amf::AMFException& e) {
            throw;
            return false;
        }

        if (code.empty() || level.empty()) return false;

        log_debug("onStatus: %s, %s", code, level);
        if (code == "NetStream.Failed"
                || code == "NetStream.Play.Failed"
                || code == "NetStream.Play.StreamNotFound"
                || code == "NetConnection.Connect.InvalidApp")
        {
            r.close();
            log_error( "Closing connection: %s", code);
            std::exit(EXIT_SUCCESS);
        }

        if (code == "NetStream.Play.Start") {
            log_debug("Netstream.Play.Start called");
            return ret;
        }

        // Return 1 if this is a Play.Complete or Play.Stop
        if (code == "NetStream.Play.Complete" ||
                code == "NetStream.Play.Stop") {
            r.close();
            std::exit(EXIT_SUCCESS);
        }
    }
    return ret;
}

namespace {

void
usage(std::ostream& o)
{
    o << "usage: rtmpdump -u <app> -p playpath [ -o outfile ]\n";
    o << "\n";
    o << "\t-h          Show this help and exit\n";
    o << "\t-u <url>    The full url of the rtmp application\n";
    o << "\t-p <path>   The play path of the stream\n";
    o << "\t-s <sec>    Start at the given seek offset\n";
    o << "\t-l <sec>    Retrieve only the specified length in seconds\n";
    o << "\t-o <file>   Output file for video\n";
    o << "\t-v          Verbose output (more 'v's for more verbosity)\n";
    o << "\n";
}



}

